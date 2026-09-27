#pragma once

#include "TransferTypes.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstddef>
#include <cstdint>

enum class ChunkState {
    FREE,
    ACQUIRED_FOR_READ,
    READING,
    READY_FOR_WRITE,
    WRITING,
    COMMITTED,
    FAILED
};

struct BufferChunk {
    uint32_t chunk_id = 0;
    uint64_t task_id = 0;
    int src_fd = -1;
    int dst_fd = -1;
    off_t src_offset = 0;
    off_t dst_offset = 0;
    size_t capacity = 0;
    size_t bytes_valid = 0;
    uint32_t crc32 = 0;
    uint8_t* buffer = nullptr;
    std::atomic<ChunkState> state{ChunkState::FREE};
    int error_code = 0;
    bool is_eof = false;

    // Per-file in-flight chunk synchronization
    std::atomic<size_t>* in_flight_counter = nullptr;
    std::condition_variable* cv_file_done = nullptr;
    std::mutex* file_done_mutex = nullptr;
};

/**
 * @brief Common synchronized ring buffer coordinating many concurrent readers
 * and many concurrent writers.
 *
 * Implements fault-tolerant atomic state transitions, page-aligned zero-copy buffers,
 * and adaptive chunk capacity.
 */
class BufferRing {
public:
    BufferRing(size_t chunkCount, size_t chunkSize);
    explicit BufferRing(size_t totalSizeMB);
    ~BufferRing();

    // Non-copyable
    BufferRing(const BufferRing&) = delete;
    BufferRing& operator=(const BufferRing&) = delete;

    // Acquire a FREE chunk for a reader thread (blocks until available or shutdown)
    BufferChunk* acquireFreeChunk();

    // Publish a chunk once reading completes, making it READY_FOR_WRITE
    void publishReadyChunk(BufferChunk* chunk, size_t bytesRead, bool isEof, int errorCode = 0);

    // Acquire a READY_FOR_WRITE chunk for a writer thread (blocks until available or shutdown)
    BufferChunk* acquireReadyChunk();

    // Release chunk back to FREE after successful write
    void releaseCommittedChunk(BufferChunk* chunk);

    // Mark chunk as failed on I/O error
    void markChunkFailed(BufferChunk* chunk, int errorCode);

    // Shutdown signaling to unblock all waiting threads
    void shutdown();
    bool isShutdown() const { return shutdown_flag_.load(std::memory_order_acquire); }

    // Metrics & Diagnostics
    size_t getCapacity() const { return chunk_count_; }
    size_t getChunkSize() const { return chunk_size_; }
    size_t getTotalSizeBytes() const { return chunk_count_ * chunk_size_; }
    size_t getTotalSizeMB() const { return (chunk_count_ * chunk_size_) / (1024 * 1024); }
    size_t getFreeChunkCount();
    size_t getReadyChunkCount();

    // Static CRC32 calculation for fault tolerance verification
    static uint32_t computeCRC32(const uint8_t* data, size_t length);

private:
    size_t chunk_count_;
    size_t chunk_size_;
    std::vector<BufferChunk> chunks_;
    std::atomic<bool> shutdown_flag_{false};

    std::mutex mutex_;
    std::condition_variable cv_free_;
    std::condition_variable cv_ready_;
};
