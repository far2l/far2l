#pragma once

#include "TransferTypes.hpp"
#include "IFileSystem.hpp"
#include "BufferRing.hpp"

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>

struct TransferItem {
    uint64_t id = 0;
    uint64_t job_id = 0;
    std::wstring src_path;
    std::wstring dst_path;
    std::wstring root_src;     // Original source tree root for relative symlink calculation
    std::wstring root_dst;     // Original destination directory
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
    bool start(const std::vector<std::wstring>& sources = {}, const std::wstring& targetDir = L"");

    // Start as a persistent global/common background transfer service
    bool startService();
    void stopService();

    // Dynamic multi-destination transfers: add files/folders with individual target destinations
    uint64_t addTransferJob(const std::wstring& sourcePath, const std::wstring& destinationDir, TransferMode mode = TransferMode::COPY);
    uint64_t addDynamicJob(const std::wstring& sourcePath, const std::wstring& destinationDir, TransferMode mode = TransferMode::COPY) {
        return addTransferJob(sourcePath, destinationDir, mode);
    }
    std::vector<uint64_t> addTransferJobs(const std::vector<std::wstring>& sources, const std::wstring& destinationDir, TransferMode mode = TransferMode::COPY);

    // Dynamic background transfer: add new sources to copy queue while transfer is in progress
    void addSource(const std::wstring& sourcePath);
    void addSource(const std::wstring& sourcePath, const std::wstring& destinationDir);

    void setAnswerForType(QuestionType type, QuestionAnswer answer) {
        std::lock_guard<std::mutex> lock(question_mutex_);
        remembered_answers_[type] = answer;
    }

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
    std::wstring getIoErrorReason() const;

    // Retry or resolve critical I/O error (e.g. after freeing disk space for ENOSPC)
    bool recoverCriticalIo(bool retry);

    // Metrics and status inspection (thread-safe)
    TransferMetrics getMetrics() const;
    TransferProgress getProgress() const;
    std::vector<TransferQuestion> getActiveQuestions() const;

    // Per-folder independent scan progress inspection
    std::vector<FolderScanProgress> getFolderScanProgresses() const;
    bool getFolderScanProgress(uint64_t jobId, FolderScanProgress& outProgress) const;

    // Per-job independent copy progress inspection (thread-safe)
    std::vector<TransferProgress> getJobProgresses() const;
    bool getJobProgress(uint64_t jobId, TransferProgress& outProgress) const;
    std::unordered_map<uint64_t, TransferProgress> getJobProgressMap() const;

    // Answer an active question from the question pool manually (e.g. from UI or interactive CLI)
    bool answerQuestion(uint64_t questionId, QuestionAnswer answer, bool rememberForType = false);

    // Set callback lambdas (alternative to inheriting and overriding virtual methods)
    void setProgressCallback(std::function<void(const TransferProgress& jobProgress, const TransferProgress& totalProgress)> cb) {
        progress_cb_ = std::move(cb);
    }
    void setProgressCallback(std::function<void(const TransferProgress& totalProgress)> cb) {
        progress_cb_ = [cb = std::move(cb)](const TransferProgress&, const TransferProgress& tot) {
            cb(tot);
        };
    }
    void setJobProgressCallback(std::function<void(const TransferProgress& jobProgress)> cb) {
        job_progress_cb_ = std::move(cb);
    }
    void setQuestionCallback(std::function<QuestionAnswer(const TransferQuestion&, bool&)> cb) {
        question_cb_ = std::move(cb);
    }
    void setScanProgressCallback(std::function<void(const FolderScanProgress&)> cb) {
        scan_progress_cb_ = std::move(cb);
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
    virtual void onProgress(const TransferProgress& jobProgress, const TransferProgress& totalProgress);
    virtual void onProgress(const TransferProgress& totalProgress);
    virtual void onJobProgress(const TransferProgress& jobProgress);
    virtual QuestionAnswer onQuestion(const TransferQuestion& question, bool& rememberForType);
    virtual void onFolderScanProgress(const FolderScanProgress& scan);

private:
    std::shared_ptr<IFileSystem> fs_;
    TransferOptions options_;

    std::wstring target_dir_;
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
    std::wstring io_error_reason_;
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
    std::wstring current_source_display_;
    std::wstring current_target_display_;
    mutable std::mutex display_mutex_;

    // Callbacks
    std::function<void(const TransferProgress& jobProgress, const TransferProgress& totalProgress)> progress_cb_;
    std::function<void(const TransferProgress& jobProgress)> job_progress_cb_;
    std::function<QuestionAnswer(const TransferQuestion&, bool&)> question_cb_;
    std::function<void(const FolderScanProgress&)> scan_progress_cb_;

    // Internal thread routines
    void discoveryWorker();
    void readerWorker(size_t threadIndex);
    void writerWorker(size_t threadIndex);
    void progressWorker();

    // Critical I/O handling
    static bool isCriticalIoError(int err);
    QuestionAnswer handleCriticalIo(const std::wstring& path, int errCode, const std::wstring& opName);

    // Transfer algorithms
    void scanSource(const std::wstring& srcPath, const std::wstring& targetBase);
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
    QuestionAnswer resolveQuestion(TransferItem& item, QuestionType type, const std::wstring& msg, int err = 0);
    static std::wstring generateUniqueRenamedPath(IFileSystem* fs, const std::wstring& originalPath);

    // Skipped directory tree tracking (when user skips a folder)
    std::vector<std::wstring> skipped_dir_prefixes_;
    mutable std::mutex skipped_dirs_mutex_;

    // Symlink path manipulation
    std::wstring recomputeRelativeSymlink(const std::wstring& rawTarget,
                                         const std::wstring& symlinkSrcPath,
                                         const std::wstring& symlinkDstPath,
                                         const std::wstring& rootSrcTree);

    // Multithreaded Folder Scanning via shared reader/worker thread pool
    struct FolderScanTask {
        uint64_t job_id = 0;
        std::wstring current_src_dir;
        std::wstring current_dst_dir;
        std::wstring root_src;
        std::wstring root_dst;
    };

    mutable std::mutex scan_mutex_;
    std::map<uint64_t, FolderScanProgress> folder_scans_;
    std::deque<FolderScanTask> pending_scan_tasks_;
    std::map<uint64_t, size_t> active_tasks_per_job_;
    std::atomic<uint32_t> total_scanned_files_{0};
    std::atomic<uint32_t> total_scanned_folders_{0};
    std::atomic<uint64_t> total_scanned_bytes_{0};
    std::atomic<bool> any_scanning_active_{false};

    void enqueueFolderScan(uint64_t jobId, const std::wstring& srcPath, const std::wstring& targetBase);
    bool processScanTask(const FolderScanTask& task);
    void updateJobScanProgress(uint64_t jobId, const std::wstring& currentItem, uint32_t newFiles, uint32_t newFolders, uint64_t newBytes);
    void completeJobScan(uint64_t jobId);

    // Per-job copy progress tracking
    mutable std::mutex job_mutex_;
    std::map<uint64_t, TransferProgress> job_progresses_;
    std::atomic<uint64_t> current_active_job_id_{0};

    void recordJobBytesTransferred(uint64_t jobId, uint64_t bytes);
    void recordJobFileCompleted(uint64_t jobId);
    void recordJobDirectoryCompleted(uint64_t jobId);
    void recordJobError(uint64_t jobId);
    void checkJobCompletion(TransferProgress& jp);
};
