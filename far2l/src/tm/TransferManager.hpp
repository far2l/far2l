#pragma once

#include "TransferTypes.hpp"
#include "IFileSystem.hpp"
#include "BufferRing.hpp"

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>

struct TransferItem {
    uint64_t id = 0;
    std::string src_path;
    std::string dst_path;
    std::string root_src;     // Original source tree root for relative symlink calculation
    std::string root_dst;     // Original destination directory
    bool is_directory = false;
    bool is_symlink = false;
    off_t size = 0;
    bool checked = false;
    bool resolved = false;
    QuestionAnswer user_decision = QuestionAnswer::PROMPT;
    off_t resume_offset = 0;
    off_t src_start_offset = 0;
    off_t dst_start_offset = 0;
    bool is_append = false;
};

/**
 * @brief High-performance asynchronous multithreaded file transfer engine.
 *
 * Implements pipelined readers/writers, adaptive buffer ring, copy-on-write (CoW),
 * sparse file hole-skipping, zero-copy mmap for small files, symlink path recomputation,
 * and non-blocking question pool with remembered modal answers.
 */
class TransferManager {
public:
    explicit TransferManager(std::shared_ptr<IFileSystem> fs = nullptr,
                             TransferOptions options = TransferOptions());
    virtual ~TransferManager();

    // Non-copyable
    TransferManager(const TransferManager&) = delete;
    TransferManager& operator=(const TransferManager&) = delete;

    // Start transfer operation with initial source paths and target destination directory
    // Target is always a folder. Sources can be files or folders.
    bool start(const std::vector<std::string>& sources = {}, const std::string& targetDir = "");

    // Start as a persistent global/common background transfer service
    bool startService();
    void stopService();

    // Dynamic multi-destination transfers: add files/folders with individual target destinations
    uint64_t addTransferJob(const std::string& sourcePath, const std::string& destinationDir, TransferMode mode = TransferMode::COPY);
    std::vector<uint64_t> addTransferJobs(const std::vector<std::string>& sources, const std::string& destinationDir, TransferMode mode = TransferMode::COPY);

    // Dynamic background transfer: add new sources to copy queue while transfer is in progress
    void addSource(const std::string& sourcePath);
    void addSource(const std::string& sourcePath, const std::string& destinationDir);

    // Buffer ring configuration (16 MB to 1024 MB)
    void setBufferRingSizeMB(size_t sizeMB, size_t chunkSize = 1024 * 1024);
    size_t getBufferRingSizeMB() const;
    size_t getBufferRingCapacityBytes() const;

    // Wait until all queued items and file transfers are completed
    void wait();

    // Cancel all active and pending transfer operations
    void cancel();

    // Pause and resume
    void pause();
    void resume();
    bool isRunning() const { return running_.load(std::memory_order_acquire); }
    bool isPaused() const { return paused_.load(std::memory_order_acquire) || io_error_paused_.load(std::memory_order_acquire); }
    bool isCancelled() const { return cancelled_.load(std::memory_order_acquire); }
    bool isPausedForIoError() const { return io_error_paused_.load(std::memory_order_acquire); }
    std::string getIoErrorReason() const;

    // Retry or resolve critical I/O error (e.g. after freeing disk space for ENOSPC)
    bool recoverCriticalIo(bool retry);

    // Metrics and status inspection (thread-safe)
    TransferMetrics getMetrics() const;
    TransferProgress getProgress() const;
    std::vector<TransferQuestion> getActiveQuestions() const;

    // Answer an active question from the question pool manually (e.g. from UI or interactive CLI)
    bool answerQuestion(uint64_t questionId, QuestionAnswer answer, bool rememberForType = false);

    // Set callback lambdas (alternative to inheriting and overriding virtual methods)
    void setProgressCallback(std::function<void(const TransferProgress&)> cb) {
        progress_cb_ = std::move(cb);
    }
    void setQuestionCallback(std::function<QuestionAnswer(const TransferQuestion&, bool&)> cb) {
        question_cb_ = std::move(cb);
    }

    // Set custom filesystem abstraction layer (e.g. for sudo proxy escalation)
    void setFileSystem(std::shared_ptr<IFileSystem> fs) {
        fs_ = fs;
    }
    std::shared_ptr<IFileSystem> getFileSystem() const {
        return fs_;
    }

protected:
    // Virtual callbacks intended to be overridden in derived classes (e.g. far2l dialogs/TUI)
    virtual void onProgress(const TransferProgress& progress);
    virtual QuestionAnswer onQuestion(const TransferQuestion& question, bool& rememberForType);

private:
    std::shared_ptr<IFileSystem> fs_;
    TransferOptions options_;

    std::string target_dir_;
    std::unique_ptr<BufferRing> buffer_ring_;

    // Threading
    std::vector<std::thread> reader_threads_;
    std::vector<std::thread> writer_threads_;
    std::thread discovery_thread_;
    std::thread progress_thread_;

    std::atomic<bool> running_{false};
    std::atomic<bool> cancelled_{false};
    std::atomic<bool> paused_{false};
    std::atomic<bool> discovery_finished_{false};

    // Item queues
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_items_;
    std::condition_variable cv_all_done_;
    std::deque<TransferItem> pending_items_;
    std::atomic<uint64_t> next_item_id_{1};
    std::atomic<uint32_t> active_transfers_{0};

    // Dynamic sources & jobs pending scanner
    std::deque<TransferJobRequest> pending_job_requests_;
    std::condition_variable cv_sources_;
    std::atomic<uint64_t> next_job_id_{1};
    std::atomic<uint32_t> total_jobs_processed_{0};

    // Critical I/O pause & recovery state
    std::atomic<bool> io_error_paused_{false};
    std::string io_error_reason_;
    mutable std::mutex io_error_mutex_;
    std::condition_variable cv_io_recover_;
    uint64_t active_critical_question_id_{0};

    // Question Pool
    mutable std::mutex question_mutex_;
    std::condition_variable cv_questions_;
    std::vector<TransferQuestion> active_questions_;
    std::map<QuestionType, QuestionAnswer> remembered_answers_;
    std::atomic<uint64_t> next_question_id_{1};

    // Metrics (atomic counters for minimal lock contention)
    std::atomic<uint64_t> bytes_transferred_{0};
    std::atomic<uint64_t> total_bytes_{0};
    std::atomic<uint32_t> files_transferred_{0};
    std::atomic<uint32_t> total_files_{0};
    std::atomic<uint32_t> dirs_transferred_{0};
    std::atomic<uint32_t> total_dirs_{0};
    std::atomic<uint32_t> error_count_{0};
    std::atomic<uint32_t> questions_resolved_{0};
    std::atomic<uint64_t> cow_bytes_{0};
    std::atomic<uint64_t> sparse_bytes_skipped_{0};
    std::atomic<uint64_t> mmap_bytes_{0};
    std::atomic<bool> had_critical_io_error_{false};

    std::chrono::steady_clock::time_point start_time_;
    std::string current_source_display_;
    std::string current_target_display_;
    mutable std::mutex display_mutex_;

    // Callbacks
    std::function<void(const TransferProgress&)> progress_cb_;
    std::function<QuestionAnswer(const TransferQuestion&, bool&)> question_cb_;

    // Internal thread routines
    void discoveryWorker();
    void readerWorker(size_t threadIndex);
    void writerWorker(size_t threadIndex);
    void progressWorker();

    // Critical I/O handling
    static bool isCriticalIoError(int err);
    QuestionAnswer handleCriticalIo(const std::string& path, int errCode, const std::string& opName);

    // Transfer algorithms
    void scanSource(const std::string& srcPath, const std::string& targetBase);
    bool processSingleItem(TransferItem& item);
    bool transferFile(TransferItem& item);
    bool transferDirectory(TransferItem& item);
    bool transferSymlink(TransferItem& item);

    // Fast paths
    bool tryCoWTransfer(TransferItem& item, int srcFd, int dstFd);
    bool transferSparseFile(TransferItem& item, int srcFd, int dstFd);
    bool transferSmallFileMmap(TransferItem& item, int srcFd, int dstFd);
    bool transferChunked(TransferItem& item, int srcFd, int dstFd);

    // Modal Interaction
    QuestionAnswer resolveQuestion(TransferItem& item, QuestionType type, const std::string& msg, int err = 0);
    static std::string generateUniqueRenamedPath(IFileSystem* fs, const std::string& originalPath);

    // Skipped directory tree tracking (when user skips a folder)
    std::vector<std::string> skipped_dir_prefixes_;
    mutable std::mutex skipped_dirs_mutex_;

    // Symlink path manipulation
    std::string recomputeRelativeSymlink(const std::string& rawTarget,
                                         const std::string& symlinkSrcPath,
                                         const std::string& symlinkDstPath,
                                         const std::string& rootSrcTree);
};
