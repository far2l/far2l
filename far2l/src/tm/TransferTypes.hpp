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
    OVERWRITE_EXISTING,         // Destination file already exists (regular file or link)
    LINK_DESTINATION_EXISTS,    // Source is symlink, destination exists
    FOLDER_DESTINATION_IS_FILE, // Source is folder, destination exists as a file
    FILE_DESTINATION_IS_FOLDER, // Source is file, destination exists as a folder
    MOVE_READ_ONLY_SOURCE,      // Move requested but source is read-only
    IO_ERROR,                   // Non-critical read or write I/O error
    PERMISSION_DENIED,          // Permission error accessing source or target
    CRITICAL_IO_ERROR,          // Fatal/Device I/O error (ENOSPC, EDQUOT, EROFS, EIO) - engine pauses all transfers
    NO_SPACE_LEFT               // Specific out-of-disk-space error
};

enum class QuestionAnswer {
    PROMPT,     // Needs user interaction
    OVERWRITE,  // Overwrite destination file (or truncate/update symlink target in-place)
    APPEND,     // Append source to destination
    RESUME,     // Resume copying from destination file size
    SKIP,       // Skip current file / item (or skip folder tree if folder skipped)
    RENAME,     // Rename destination file (folder case) or rename source output target (file case) preserving extension
    PLACE_INTO, // Place file inside existing destination folder as destination/folder/file
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

// Dynamic transfer job request with individual destination folder
struct TransferJobRequest {
    uint64_t job_id = 0;
    std::string source_path;
    std::string destination_dir;
    TransferMode mode = TransferMode::COPY;
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
    size_t buffer_ring_size_mb = 64;        // Configurable ring buffer size: 16 MB to 1024 MB (1 GB)
    size_t chunk_size = 1024 * 1024;        // 1MB default adaptive chunk
    size_t buffer_ring_capacity = 64;       // Common buffer chunk pool count (e.g. size_mb * 1MB / chunk_size)
    size_t small_file_mmap_threshold = 2 * 1024 * 1024; // 2MB threshold for mmap
    bool enable_cow = true;                 // Attempt copy-on-write (FICLONE / clonefile)
    bool enable_sparse = true;              // Preserve sparse file holes
    bool preserve_xattrs = true;            // Preserve extended attributes
    bool preserve_timestamps = true;        // Preserve atime/mtime with nanoseconds
    bool preserve_permissions = true;       // Preserve POSIX mode bits
    bool preserve_ownership = true;         // Preserve uid/gid (when privileged / sudo)
    bool verify_integrity = false;          // Verify CRC32 checksum of data blocks
    bool service_mode = false;              // Run as persistent background transfer service
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
    uint32_t active_jobs_count = 0;
    double current_speed_mb_s = 0.0;
    double progress_percent = 0.0;
    std::string current_source_item;
    std::string current_target_item;
    double elapsed_seconds = 0.0;
    bool is_paused_for_io_error = false;
    std::string io_error_reason;
    size_t buffer_ring_size_mb = 64;
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
    uint32_t total_jobs_processed = 0;
    double average_speed_mb_s = 0.0;
    double elapsed_seconds = 0.0;
    uint64_t cow_cloned_bytes = 0;
    uint64_t sparse_bytes_skipped = 0;
    uint64_t mmap_transferred_bytes = 0;
    bool had_critical_io_error = false;
    size_t buffer_ring_size_mb = 64;
};
