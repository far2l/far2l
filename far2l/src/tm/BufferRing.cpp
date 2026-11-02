#include "BufferRing.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

BufferRing::BufferRing(size_t chunkCount, size_t chunkSize)
    : chunk_count_(chunkCount > 0 ? chunkCount : 16),
      chunk_size_(chunkSize > 0 ? chunkSize : 1024 * 1024),
      chunks_(chunk_count_) {
    
    for (size_t i = 0; i < chunk_count_; ++i) {
        chunks_[i].chunk_id = static_cast<uint32_t>(i);
        chunks_[i].capacity = chunk_size_;
        chunks_[i].bytes_valid = 0;
        chunks_[i].state.store(ChunkState::FREE, std::memory_order_relaxed);

        // Page-aligned memory buffer (4096 bytes) for zero-copy kernel transfers and Direct I/O
        void* ptr = nullptr;
        if (posix_memalign(&ptr, 4096, chunk_size_) == 0) {
            chunks_[i].buffer = static_cast<uint8_t*>(ptr);
        } else {
            chunks_[i].buffer = static_cast<uint8_t*>(std::malloc(chunk_size_));
        }
    }
}

BufferRing::BufferRing(size_t totalSizeMB)
    : BufferRing(
        std::clamp<size_t>(totalSizeMB, 16, 1024),
        1024 * 1024
      ) {}

BufferRing::~BufferRing() {
    shutdown();
    for (size_t i = 0; i < chunk_count_; ++i) {
        if (chunks_[i].buffer) {
            std::free(chunks_[i].buffer);
            chunks_[i].buffer = nullptr;
        }
    }
}

BufferChunk* BufferRing::acquireFreeChunk() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!shutdown_flag_.load(std::memory_order_acquire)) {
        for (size_t i = 0; i < chunk_count_; ++i) {
            if (chunks_[i].state.load(std::memory_order_relaxed) == ChunkState::FREE) {
                chunks_[i].state.store(ChunkState::ACQUIRED_FOR_READ, std::memory_order_release);
                chunks_[i].bytes_valid = 0;
                chunks_[i].error_code = 0;
                chunks_[i].is_eof = false;
                return &chunks_[i];
            }
        }
        cv_free_.wait(lock);
    }
    return nullptr;
}

void BufferRing::publishReadyChunk(BufferChunk* chunk, size_t bytesRead, bool isEof, int errorCode) {
    if (!chunk) return;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        chunk->bytes_valid = bytesRead;
        chunk->is_eof = isEof;
        chunk->error_code = errorCode;
        
        if (errorCode != 0) {
            chunk->state.store(ChunkState::FAILED, std::memory_order_release);
        } else {
            chunk->state.store(ChunkState::READY_FOR_WRITE, std::memory_order_release);
        }
    }
    cv_ready_.notify_one();
}

BufferChunk* BufferRing::acquireReadyChunk() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!shutdown_flag_.load(std::memory_order_acquire)) {
        for (size_t i = 0; i < chunk_count_; ++i) {
            ChunkState st = chunks_[i].state.load(std::memory_order_relaxed);
            if (st == ChunkState::READY_FOR_WRITE) {
                chunks_[i].state.store(ChunkState::WRITING, std::memory_order_release);
                return &chunks_[i];
            }
        }
        cv_ready_.wait(lock);
    }
    return nullptr;
}

void BufferRing::releaseCommittedChunk(BufferChunk* chunk) {
    if (!chunk) return;
    
    // Notify file-level completion if tracked
    if (chunk->in_flight_counter) {
        std::atomic<size_t>* counter = chunk->in_flight_counter;
        std::condition_variable* cv = chunk->cv_file_done;
        std::mutex* mtx = chunk->file_done_mutex;

        size_t remaining = counter->fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (remaining == 0 && cv && mtx) {
            std::lock_guard<std::mutex> lock(*mtx);
            cv->notify_all();
        }
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        chunk->bytes_valid = 0;
        chunk->task_id = 0;
        chunk->src_fd = -1;
        chunk->dst_fd = -1;
        chunk->src_offset = 0;
        chunk->dst_offset = 0;
        chunk->is_eof = false;
        chunk->error_code = 0;
        chunk->in_flight_counter = nullptr;
        chunk->cv_file_done = nullptr;
        chunk->file_done_mutex = nullptr;
        chunk->state.store(ChunkState::FREE, std::memory_order_release);
    }
    cv_free_.notify_one();
}

void BufferRing::markChunkFailed(BufferChunk* chunk, int errorCode) {
    if (!chunk) return;

    if (chunk->in_flight_counter) {
        std::atomic<size_t>* counter = chunk->in_flight_counter;
        std::condition_variable* cv = chunk->cv_file_done;
        std::mutex* mtx = chunk->file_done_mutex;

        size_t remaining = counter->fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (remaining == 0 && cv && mtx) {
            std::lock_guard<std::mutex> lock(*mtx);
            cv->notify_all();
        }
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        chunk->error_code = errorCode;
        chunk->in_flight_counter = nullptr;
        chunk->cv_file_done = nullptr;
        chunk->file_done_mutex = nullptr;
        chunk->state.store(ChunkState::FAILED, std::memory_order_release);
    }
    cv_free_.notify_one();
}

void BufferRing::shutdown() {
    shutdown_flag_.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cv_free_.notify_all();
        cv_ready_.notify_all();
    }
}

size_t BufferRing::getFreeChunkCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (size_t i = 0; i < chunk_count_; ++i) {
        if (chunks_[i].state.load(std::memory_order_relaxed) == ChunkState::FREE) {
            count++;
        }
    }
    return count;
}

size_t BufferRing::getReadyChunkCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (size_t i = 0; i < chunk_count_; ++i) {
        if (chunks_[i].state.load(std::memory_order_relaxed) == ChunkState::READY_FOR_WRITE) {
            count++;
        }
    }
    return count;
}

// CRC32 table implementation
static const uint32_t crc32_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

uint32_t BufferRing::computeCRC32(const uint8_t* data, size_t length) {
    uint32_t crc = ~0u;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        crc = (crc >> 4) ^ crc32_table[crc & 0x0f];
        crc = (crc >> 4) ^ crc32_table[crc & 0x0f];
    }
    return ~crc;
}
