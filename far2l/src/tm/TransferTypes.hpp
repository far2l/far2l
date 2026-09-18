#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <sys/types.h>

enum class TransferMode {
    COPY,
    MOVE
};

enum class QuestionType {
    OVERWRITE_EXISTING,     // Destination file already exists
    MOVE_READ_ONLY_SOURCE,  // Move requested but source is read-only
    IO_ERROR,               // Read or write I/O error occurred
    PERMISSION_DENIED       // Permission error accessing source or target
};

enum class QuestionAnswer {
    PROMPT,     // Needs user interaction
    OVERWRITE,  // Overwrite destination file entirely
    APPEND,     // Append source to destination
    RESUME,     // Resume copying from destination file size
    SKIP,       // Skip current file / item
    RETRY,      // Retry the failed operation
    CANCEL      // Abort entire transfer session
};

struct FileStat {
    off_t size = 0;
    mode_t mode = 0;
    uid_t uid = 0;
    gid_t gid = 0;
    int64_t atime_sec = 0;
    long atime_nsec = 0;
    int64_t mtime_sec = 0;
    long mtime_nsec = 0;
    bool exists = false;
    bool is_directory = false;
    bool is_symlink = false;
    bool is_regular = false;
    bool is_sparse = false;
    bool is_read_only = false;
};

struct DirEntry {
    std::string name;
    std::string full_path;
    bool is_directory = false;
    bool is_symlink = false;
    off_t size = 0;
};

struct TransferQuestion {
    uint64_t id = 0;
    QuestionType type = QuestionType::OVERWRITE_EXISTING;
    std::string source_path;
    std::string target_path;
    std::string message;
    off_t source_size = 0;
    off_t target_size = 0;
    int64_t source_mtime = 0;
    int64_t target_mtime = 0;
    int error_code = 0;
    std::string details;
};

struct TransferOptions {
    TransferMode mode = TransferMode::COPY;
    size_t num_readers = 4;
    size_t num_writers = 4;
    size_t buffer_ring_capacity = 32;       // Common buffer chunk pool count
    size_t chunk_size = 1024 * 1024;        // 1MB default adaptive chunk
    size_t small_file_mmap_threshold = 2 * 1024 * 1024; // 2MB threshold for mmap
    bool enable_cow = true;                 // Attempt copy-on-write (FICLONE / clonefile)
    bool enable_sparse = true;              // Preserve sparse file holes
    bool preserve_xattrs = true;            // Preserve extended attributes
    bool preserve_timestamps = true;        // Preserve atime/mtime with nanoseconds
    bool preserve_permissions = true;       // Preserve POSIX mode bits
    bool preserve_ownership = true;         // Preserve uid/gid (when privileged / sudo)
    bool verify_integrity = false;          // Verify CRC32 checksum of data blocks
};

struct TransferProgress {
    uint64_t bytes_transferred = 0;
    uint64_t total_bytes = 0;
    uint32_t files_transferred = 0;
    uint32_t total_files = 0;
    uint32_t dirs_transferred = 0;
    uint32_t total_dirs = 0;
    uint32_t errors_count = 0;
    uint32_t questions_resolved_count = 0;
    uint32_t active_questions_count = 0;
    double current_speed_mb_s = 0.0;
    double progress_percent = 0.0;
    std::string current_source_item;
    std::string current_target_item;
    double elapsed_seconds = 0.0;
};

struct TransferMetrics {
    uint64_t transferred_bytes = 0;
    uint64_t total_bytes = 0;
    uint32_t transferred_files = 0;
    uint32_t total_files = 0;
    uint32_t transferred_dirs = 0;
    uint32_t total_dirs = 0;
    uint32_t error_count = 0;
    uint32_t questions_resolved_count = 0;
    uint32_t active_questions_count = 0;
    double average_speed_mb_s = 0.0;
    double elapsed_seconds = 0.0;
    uint64_t cow_cloned_bytes = 0;
    uint64_t sparse_bytes_skipped = 0;
    uint64_t mmap_transferred_bytes = 0;
};
