#include "TransferManager.hpp"
#include "LocalFileSystem.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cerrno>

namespace fs = std::filesystem;

TransferManager::TransferManager(std::shared_ptr<IFileSystem> fs_impl, TransferOptions options)
    : fs_(fs_impl ? std::move(fs_impl) : std::make_shared<LocalFileSystem>()),
      options_(options) {
    if (options_.buffer_ring_size_mb >= 16 && options_.buffer_ring_size_mb <= 1024) {
        options_.buffer_ring_capacity = (options_.buffer_ring_size_mb * 1024 * 1024) / options_.chunk_size;
    }
    buffer_ring_ = std::make_unique<BufferRing>(options_.buffer_ring_capacity, options_.chunk_size);
}

TransferManager::~TransferManager() {
    cancel();
    wait();
}

void TransferManager::setBufferRingSizeMB(size_t sizeMB, size_t chunkSize) {
    if (running_.load(std::memory_order_relaxed)) {
        return;
    }
    sizeMB = std::clamp<size_t>(sizeMB, 16, 1024);
    options_.buffer_ring_size_mb = sizeMB;
    options_.chunk_size = (chunkSize > 0) ? chunkSize : (1024 * 1024);
    options_.buffer_ring_capacity = (sizeMB * 1024 * 1024) / options_.chunk_size;
    buffer_ring_ = std::make_unique<BufferRing>(options_.buffer_ring_capacity, options_.chunk_size);
}

size_t TransferManager::getBufferRingSizeMB() const {
    return buffer_ring_ ? buffer_ring_->getTotalSizeMB() : options_.buffer_ring_size_mb;
}

size_t TransferManager::getBufferRingCapacityBytes() const {
    return buffer_ring_ ? buffer_ring_->getTotalSizeBytes() : (options_.buffer_ring_size_mb * 1024 * 1024);
}

bool TransferManager::startService() {
    options_.service_mode = true;
    return start({}, "");
}

void TransferManager::stopService() {
    options_.service_mode = false;
    cancel();
    wait();
}

bool TransferManager::start(const std::vector<std::string>& sources, const std::string& targetDir) {
    if (running_.load(std::memory_order_relaxed)) {
        return false;
    }

    target_dir_ = targetDir;
    if (!target_dir_.empty()) {
        if (!fs_->exists(target_dir_)) {
            if (!fs_->createDirectories(target_dir_)) {
                return false;
            }
        } else if (!fs_->isDirectory(target_dir_)) {
            return false;
        }
    }

    running_.store(true, std::memory_order_release);
    cancelled_.store(false, std::memory_order_release);
    paused_.store(false, std::memory_order_release);
    io_error_paused_.store(false, std::memory_order_release);
    discovery_finished_.store(false, std::memory_order_release);
    start_time_ = std::chrono::steady_clock::now();

    // Push initial sources / jobs
    if (!sources.empty() && !targetDir.empty()) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        for (const auto& src : sources) {
            TransferJobRequest req;
            req.job_id = next_job_id_++;
            req.source_path = src;
            req.destination_dir = targetDir;
            req.mode = options_.mode;
            pending_job_requests_.push_back(std::move(req));
        }
    }

    // Spawn scanner/discovery thread
    discovery_thread_ = std::thread(&TransferManager::discoveryWorker, this);

    // Spawn Reader thread pool
    size_t readers = std::max<size_t>(1, options_.num_readers);
    for (size_t i = 0; i < readers; ++i) {
        reader_threads_.emplace_back(&TransferManager::readerWorker, this, i);
    }

    // Spawn Writer thread pool
    size_t writers = std::max<size_t>(1, options_.num_writers);
    for (size_t i = 0; i < writers; ++i) {
        writer_threads_.emplace_back(&TransferManager::writerWorker, this, i);
    }

    // Spawn throttled Progress thread (~10 Hz)
    progress_thread_ = std::thread(&TransferManager::progressWorker, this);

    return true;
}

uint64_t TransferManager::addTransferJob(const std::string& sourcePath, const std::string& destinationDir, TransferMode mode) {
    if (!running_.load(std::memory_order_acquire)) {
        startService();
    }
    if (!fs_->exists(destinationDir)) {
        fs_->createDirectories(destinationDir);
    }

    uint64_t jId = next_job_id_++;
    TransferJobRequest req;
    req.job_id = jId;
    req.source_path = sourcePath;
    req.destination_dir = destinationDir;
    req.mode = mode;

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        pending_job_requests_.push_back(std::move(req));
    }
    cv_sources_.notify_one();
    return jId;
}

std::vector<uint64_t> TransferManager::addTransferJobs(const std::vector<std::string>& sources, const std::string& destinationDir, TransferMode mode) {
    std::vector<uint64_t> ids;
    ids.reserve(sources.size());
    for (const auto& src : sources) {
        ids.push_back(addTransferJob(src, destinationDir, mode));
    }
    return ids;
}

void TransferManager::addSource(const std::string& sourcePath) {
    addTransferJob(sourcePath, target_dir_.empty() ? "." : target_dir_, options_.mode);
}

void TransferManager::addSource(const std::string& sourcePath, const std::string& destinationDir) {
    addTransferJob(sourcePath, destinationDir, options_.mode);
}

void TransferManager::wait() {
    // Wait for discovery to finish scanning all items
    if (discovery_thread_.joinable()) {
        discovery_thread_.join();
    }

    // Wait until all pending items are processed
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_all_done_.wait(lock, [this]() {
            return (pending_items_.empty() && active_transfers_.load() == 0) ||
                   cancelled_.load();
        });
    }

    // Stop and unblock reader/writer workers
    running_.store(false, std::memory_order_release);
    if (buffer_ring_) {
        buffer_ring_->shutdown();
    }
    cv_items_.notify_all();

    for (auto& t : reader_threads_) {
        if (t.joinable()) t.join();
    }
    reader_threads_.clear();

    for (auto& t : writer_threads_) {
        if (t.joinable()) t.join();
    }
    writer_threads_.clear();

    if (progress_thread_.joinable()) {
        progress_thread_.join();
    }
}

void TransferManager::cancel() {
    cancelled_.store(true, std::memory_order_release);
    running_.store(false, std::memory_order_release);
    if (buffer_ring_) {
        buffer_ring_->shutdown();
    }
    cv_items_.notify_all();
    cv_sources_.notify_all();
    cv_all_done_.notify_all();
    cv_questions_.notify_all();
}

void TransferManager::pause() {
    paused_.store(true, std::memory_order_release);
}

void TransferManager::resume() {
    paused_.store(false, std::memory_order_release);
    cv_items_.notify_all();
}

void TransferManager::discoveryWorker() {
    while (running_.load(std::memory_order_acquire) && !cancelled_.load(std::memory_order_acquire)) {
        TransferJobRequest currentJob;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (pending_job_requests_.empty()) {
                discovery_finished_.store(true, std::memory_order_release);
                if (options_.service_mode) {
                    cv_sources_.wait(lock, [this]() {
                        return !pending_job_requests_.empty() || cancelled_.load() || !running_.load();
                    });
                    if (cancelled_.load() || !running_.load()) {
                        break;
                    }
                } else {
                    cv_sources_.wait_for(lock, std::chrono::milliseconds(50), [this]() {
                        return !pending_job_requests_.empty() || cancelled_.load() || !running_.load();
                    });
                    if (pending_job_requests_.empty()) {
                        break;
                    }
                }
                discovery_finished_.store(false, std::memory_order_release);
            }

            if (pending_job_requests_.empty()) continue;

            currentJob = std::move(pending_job_requests_.front());
            pending_job_requests_.pop_front();
            total_jobs_processed_++;
        }

        if (!currentJob.source_path.empty()) {
            scanSource(currentJob.source_path, currentJob.destination_dir);
        }
    }
    discovery_finished_.store(true, std::memory_order_release);
}

void TransferManager::scanSource(const std::string& srcPath, const std::string& targetBase) {
    FileStat st = fs_->statFile(srcPath, false);
    if (!st.exists) {
        error_count_++;
        return;
    }

    std::string baseName;
    try {
        fs::path p(srcPath);
        baseName = p.filename().string();
        if (baseName.empty() || baseName == ".") {
            baseName = p.parent_path().filename().string();
        }
    } catch (...) {
        baseName = "copied_item";
    }

    std::string destRootItem = (targetBase.back() == '/') ? (targetBase + baseName) : (targetBase + "/" + baseName);

    if (st.is_directory && !st.is_symlink) {
        // Recursive folder scan
        total_dirs_++;
        TransferItem dirItem;
        dirItem.id = next_item_id_++;
        dirItem.src_path = srcPath;
        dirItem.dst_path = destRootItem;
        dirItem.root_src = srcPath;
        dirItem.root_dst = destRootItem;
        dirItem.is_directory = true;
        dirItem.is_symlink = false;
        dirItem.size = 0;

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            pending_items_.push_back(std::move(dirItem));
        }
        cv_items_.notify_one();

        // Recursively list contents
        std::deque<std::pair<std::string, std::string>> dirQueue;
        dirQueue.push_back({srcPath, destRootItem});

        while (!dirQueue.empty() && !cancelled_.load(std::memory_order_relaxed)) {
            auto [currentSrcDir, currentDstDir] = dirQueue.front();
            dirQueue.pop_front();

            std::vector<DirEntry> entries = fs_->listDirectory(currentSrcDir);
            for (const auto& entry : entries) {
                std::string childDst = currentDstDir + "/" + entry.name;
                TransferItem childItem;
                childItem.id = next_item_id_++;
                childItem.src_path = entry.full_path;
                childItem.dst_path = childDst;
                childItem.root_src = srcPath;
                childItem.root_dst = destRootItem;
                childItem.is_directory = entry.is_directory && !entry.is_symlink;
                childItem.is_symlink = entry.is_symlink;
                childItem.size = entry.size;

                if (childItem.is_directory) {
                    total_dirs_++;
                    dirQueue.push_back({entry.full_path, childDst});
                } else {
                    total_files_++;
                    total_bytes_.fetch_add(entry.size, std::memory_order_relaxed);
                }

                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    pending_items_.push_back(std::move(childItem));
                }
                cv_items_.notify_one();
            }
        }
    } else {
        // Single file or symlink
        total_files_++;
        total_bytes_.fetch_add(st.size, std::memory_order_relaxed);

        TransferItem fileItem;
        fileItem.id = next_item_id_++;
        fileItem.src_path = srcPath;
        fileItem.dst_path = destRootItem;
        fileItem.root_src = srcPath;
        fileItem.root_dst = destRootItem;
        fileItem.is_directory = false;
        fileItem.is_symlink = st.is_symlink;
        fileItem.size = st.size;

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            pending_items_.push_back(std::move(fileItem));
        }
        cv_items_.notify_one();
    }
}

void TransferManager::readerWorker(size_t /*threadIndex*/) {
    while (running_.load(std::memory_order_acquire) && !cancelled_.load(std::memory_order_acquire)) {
        TransferItem item;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_items_.wait(lock, [this]() {
                return !pending_items_.empty() || !running_.load() || cancelled_.load();
            });

            if (cancelled_.load() || (!running_.load() && pending_items_.empty())) {
                break;
            }

            if (paused_.load(std::memory_order_relaxed) || io_error_paused_.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            if (pending_items_.empty()) continue;

            item = std::move(pending_items_.front());
            pending_items_.pop_front();
            active_transfers_++;
        }

        {
            std::lock_guard<std::mutex> lock(display_mutex_);
            current_source_display_ = item.src_path;
            current_target_display_ = item.dst_path;
        }

        // Process item (file, folder, symlink)
        bool success = processSingleItem(item);
        if (!success) {
            error_count_++;
        }

        active_transfers_--;
        cv_all_done_.notify_all();
    }
}

void TransferManager::writerWorker(size_t /*threadIndex*/) {
    while (running_.load(std::memory_order_acquire) && !cancelled_.load(std::memory_order_acquire)) {
        if (paused_.load(std::memory_order_relaxed) || io_error_paused_.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }

        // Writers grab chunks prepared by reader pipelines
        BufferChunk* chunk = buffer_ring_->acquireReadyChunk();
        if (!chunk) {
            // BufferRing shutdown
            break;
        }

        if (chunk->bytes_valid > 0 && chunk->dst_fd >= 0) {
            ssize_t written = fs_->writeFile(chunk->dst_fd, chunk->buffer, chunk->bytes_valid, chunk->dst_offset);
            if (written < 0 || static_cast<size_t>(written) != chunk->bytes_valid) {
                int err = errno ? errno : EIO;
                if (isCriticalIoError(err)) {
                    QuestionAnswer act = handleCriticalIo("chunk write offset " + std::to_string(chunk->dst_offset), err, "write");
                    if (act == QuestionAnswer::RETRY) {
                        // User recovered disk space or device; retry writing
                        written = fs_->writeFile(chunk->dst_fd, chunk->buffer, chunk->bytes_valid, chunk->dst_offset);
                        if (written > 0 && static_cast<size_t>(written) == chunk->bytes_valid) {
                            bytes_transferred_.fetch_add(written, std::memory_order_relaxed);
                            buffer_ring_->releaseCommittedChunk(chunk);
                            continue;
                        }
                    } else if (act == QuestionAnswer::CANCEL) {
                        buffer_ring_->markChunkFailed(chunk, err);
                        cancel();
                        break;
                    }
                }
                buffer_ring_->markChunkFailed(chunk, err);
                error_count_++;
                continue;
            }
            bytes_transferred_.fetch_add(written, std::memory_order_relaxed);
        }

        buffer_ring_->releaseCommittedChunk(chunk);
    }
}

bool TransferManager::isCriticalIoError(int err) {
    return (err == ENOSPC || err == EDQUOT || err == EROFS || err == EIO || err == EBUSY || err == ENODEV);
}

std::string TransferManager::getIoErrorReason() const {
    std::lock_guard<std::mutex> lock(io_error_mutex_);
    return io_error_reason_;
}

QuestionAnswer TransferManager::handleCriticalIo(const std::string& path, int errCode, const std::string& opName) {
    had_critical_io_error_.store(true, std::memory_order_relaxed);
    io_error_paused_.store(true, std::memory_order_release);

    std::string errStr = std::strerror(errCode);
    std::string reason = (errCode == ENOSPC) ? "No space left on device (ENOSPC)" :
                         (errCode == EDQUOT) ? "Disk quota exceeded (EDQUOT)" :
                         (errCode == EROFS)  ? "Read-only file system (EROFS)" :
                         ("Critical I/O error: " + errStr + " (errno " + std::to_string(errCode) + ")");
    {
        std::lock_guard<std::mutex> lock(io_error_mutex_);
        io_error_reason_ = reason + " during " + opName + " on: " + path;
    }

    TransferQuestion q;
    q.id = next_question_id_++;
    q.type = (errCode == ENOSPC) ? QuestionType::NO_SPACE_LEFT : QuestionType::CRITICAL_IO_ERROR;
    q.source_path = path;
    q.target_path = path;
    q.error_code = errCode;
    q.message = reason + ". All transfers paused to preserve data integrity. Free storage space or fix device, then select [Retry] or [Cancel].";

    active_critical_question_id_ = q.id;

    // Add to question pool
    {
        std::lock_guard<std::mutex> lock(question_mutex_);
        active_questions_.push_back(q);
    }
    cv_questions_.notify_all();

    // Check callback
    bool rememberDummy = false;
    QuestionAnswer answer = QuestionAnswer::PROMPT;
    if (question_cb_) {
        answer = question_cb_(q, rememberDummy);
    } else {
        answer = onQuestion(q, rememberDummy);
    }

    if (answer == QuestionAnswer::PROMPT) {
        std::unique_lock<std::mutex> lock(question_mutex_);
        cv_questions_.wait(lock, [this, qId = q.id, &answer]() {
            if (cancelled_.load(std::memory_order_relaxed)) {
                answer = QuestionAnswer::CANCEL;
                return true;
            }
            if (!io_error_paused_.load(std::memory_order_relaxed)) {
                answer = QuestionAnswer::RETRY;
                return true;
            }
            for (const auto& activeQ : active_questions_) {
                if (activeQ.id == qId) return false;
            }
            return true;
        });
    }

    if (answer == QuestionAnswer::RETRY) {
        io_error_paused_.store(false, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lock(io_error_mutex_);
            io_error_reason_.clear();
        }
        cv_items_.notify_all();
    } else if (answer == QuestionAnswer::CANCEL) {
        cancel();
    }

    return answer;
}

bool TransferManager::recoverCriticalIo(bool retry) {
    if (!io_error_paused_.load(std::memory_order_relaxed)) {
        return false;
    }

    if (retry) {
        io_error_paused_.store(false, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lock(io_error_mutex_);
            io_error_reason_.clear();
        }
        {
            std::lock_guard<std::mutex> lock(question_mutex_);
            for (auto it = active_questions_.begin(); it != active_questions_.end(); ++it) {
                if (it->type == QuestionType::CRITICAL_IO_ERROR || it->type == QuestionType::NO_SPACE_LEFT) {
                    active_questions_.erase(it);
                    break;
                }
            }
        }
        cv_questions_.notify_all();
        cv_items_.notify_all();
        return true;
    } else {
        cancel();
        return true;
    }
}

std::string TransferManager::generateUniqueRenamedPath(IFileSystem* fs, const std::string& originalPath) {
    try {
        fs::path p(originalPath);
        fs::path parent = p.parent_path();
        std::string stem = p.stem().string();
        std::string ext = p.extension().string();

        int counter = 1;
        while (counter < 100000) {
            std::string candidateName = stem + " (" + std::to_string(counter) + ")" + ext;
            fs::path candidate = parent.empty() ? fs::path(candidateName) : (parent / candidateName);
            if (!fs->exists(candidate.string())) {
                return candidate.string();
            }
            counter++;
        }
    } catch (...) {}
    return originalPath;
}

bool TransferManager::processSingleItem(TransferItem& item) {
    if (cancelled_.load(std::memory_order_relaxed)) return false;

    // Check if item belongs to a skipped folder
    {
        std::lock_guard<std::mutex> lock(skipped_dirs_mutex_);
        for (const auto& prefix : skipped_dir_prefixes_) {
            if (item.src_path == prefix || item.src_path.rfind(prefix + "/", 0) == 0) {
                // Item is inside a skipped folder tree
                return true;
            }
        }
    }

    if (item.is_directory) {
        return transferDirectory(item);
    } else if (item.is_symlink) {
        return transferSymlink(item);
    } else {
        return transferFile(item);
    }
}

bool TransferManager::transferDirectory(TransferItem& item) {
    bool dstExists = fs_->exists(item.dst_path);
    if (dstExists) {
        if (fs_->isDirectory(item.dst_path)) {
            // Destination is already a folder: simply omit mkdir() without errors
        } else {
            // Destination is a file/link: ask user for overwrite, skip, or rename
            QuestionAnswer answer = resolveQuestion(
                item, QuestionType::FOLDER_DESTINATION_IS_FILE,
                "Destination exists and is a file, but source is a folder: " + item.dst_path);
            if (answer == QuestionAnswer::CANCEL) {
                cancel();
                return false;
            }
            if (answer == QuestionAnswer::SKIP) {
                // Skip source folder with all its files and subfolders
                {
                    std::lock_guard<std::mutex> lock(skipped_dirs_mutex_);
                    skipped_dir_prefixes_.push_back(item.src_path);
                }
                return true;
            }
            if (answer == QuestionAnswer::RENAME) {
                // Rename destination file preserving its extension, then make directory
                std::string renamedDst = generateUniqueRenamedPath(fs_.get(), item.dst_path);
                bool okRename = fs_->renameFile(item.dst_path, renamedDst);
                if (!okRename) {
                    error_count_++;
                    return false;
                }
                bool created = fs_->createDirectories(item.dst_path);
                if (!created) return false;
            } else {
                // OVERWRITE: delete file, create folder and go forward
                fs_->removeFile(item.dst_path);
                bool created = fs_->createDirectories(item.dst_path);
                if (!created) return false;
            }
        }
    } else {
        bool created = fs_->createDirectories(item.dst_path);
        if (!created) return false;
    }

    if (options_.preserve_permissions || options_.preserve_timestamps) {
        fs_->copyMetadata(item.src_path, item.dst_path, options_.preserve_ownership);
    }
    if (options_.preserve_xattrs) {
        fs_->copyXattrs(item.src_path, item.dst_path);
    }

    if (options_.mode == TransferMode::MOVE) {
        // Move directory: delete source dir if empty
        fs_->removeDirectory(item.src_path);
    }

    dirs_transferred_++;
    return true;
}

std::string TransferManager::recomputeRelativeSymlink(const std::string& rawTarget,
                                                      const std::string& symlinkSrcPath,
                                                      const std::string& symlinkDstPath,
                                                      const std::string& rootSrcTree) {
    if (rawTarget.empty() || rawTarget[0] == '/') {
        // Absolute symlink: copied as is
        return rawTarget;
    }

    try {
        fs::path srcDir = fs::path(symlinkSrcPath).parent_path();
        fs::path canonicalTarget = fs::weakly_canonical(srcDir / rawTarget);
        fs::path canonicalRootSrc = fs::weakly_canonical(rootSrcTree);

        // Check if the target is inside the source subtree
        auto rootStr = canonicalRootSrc.string();
        auto targetStr = canonicalTarget.string();

        bool insideSourceTree = false;
        if (targetStr.rfind(rootStr, 0) == 0) {
            // Target begins with source tree path
            insideSourceTree = true;
        }

        if (insideSourceTree) {
            // Belongs to source sub-folders -> copy it as is!
            return rawTarget;
        } else {
            // Points to something outside the source tree:
            // Recompute relative path so destination points to the exact same external target!
            fs::path dstDir = fs::path(symlinkDstPath).parent_path();
            fs::path recomputed = fs::relative(canonicalTarget, dstDir);
            return recomputed.string();
        }
    } catch (...) {
        return rawTarget;
    }
}

bool TransferManager::transferSymlink(TransferItem& item) {
    std::string rawTarget = fs_->readSymlink(item.src_path);
    if (rawTarget.empty()) {
        return false;
    }

    std::string finalTarget = recomputeRelativeSymlink(rawTarget, item.src_path, item.dst_path, item.root_src);

    // Collision check for symlink destination
    if (fs_->exists(item.dst_path)) {
        // Link case: ask user for overwrite or skip.
        QuestionAnswer answer = resolveQuestion(
            item, QuestionType::LINK_DESTINATION_EXISTS,
            "Target destination already exists for symlink: " + item.dst_path);
        if (answer == QuestionAnswer::CANCEL) {
            cancel();
            return false;
        }
        if (answer == QuestionAnswer::SKIP) {
            // Skip means do nothing and move to the next source
            return true;
        }
        // Overwrite means delete old file when possible and make symlink
        if (fs_->isDirectory(item.dst_path)) {
            fs_->removeDirectory(item.dst_path);
        } else {
            fs_->removeFile(item.dst_path);
        }
    }

    bool ok = fs_->createSymlink(finalTarget, item.dst_path);
    if (ok) {
        if (options_.preserve_timestamps) {
            FileStat st = fs_->statFile(item.src_path, false);
            fs_->setTimestamps(item.dst_path, st.atime_sec, st.atime_nsec, st.mtime_sec, st.mtime_nsec);
        }
        if (options_.mode == TransferMode::MOVE) {
            fs_->removeFile(item.src_path);
        }
        files_transferred_++;
    }
    return ok;
}

QuestionAnswer TransferManager::resolveQuestion(TransferItem& item, QuestionType type, const std::string& msg, int err) {
    // Check if we already have a remembered answer for this QuestionType
    {
        std::lock_guard<std::mutex> lock(question_mutex_);
        auto it = remembered_answers_.find(type);
        if (it != remembered_answers_.end()) {
            questions_resolved_++;
            return it->second;
        }
    }

    TransferQuestion q;
    q.id = next_question_id_++;
    q.type = type;
    q.source_path = item.src_path;
    q.target_path = item.dst_path;
    q.message = msg;
    q.error_code = err;

    FileStat srcSt = fs_->statFile(item.src_path, false);
    FileStat dstSt = fs_->statFile(item.dst_path, false);
    q.source_size = srcSt.size;
    q.target_size = dstSt.size;
    q.source_mtime = srcSt.mtime_sec;
    q.target_mtime = dstSt.mtime_sec;

    // Check callback or virtual method first
    bool rememberForType = false;
    QuestionAnswer answer = QuestionAnswer::PROMPT;

    if (question_cb_) {
        answer = question_cb_(q, rememberForType);
    } else {
        answer = onQuestion(q, rememberForType);
    }

    if (answer != QuestionAnswer::PROMPT) {
        if (rememberForType) {
            std::lock_guard<std::mutex> lock(question_mutex_);
            remembered_answers_[type] = answer;
        }
        questions_resolved_++;
        return answer;
    }

    // Otherwise, post to active question pool and wait for interactive resolution
    {
        std::lock_guard<std::mutex> lock(question_mutex_);
        active_questions_.push_back(q);
    }
    cv_questions_.notify_all();

    // Block this item until answered or cancelled
    std::unique_lock<std::mutex> lock(question_mutex_);
    cv_questions_.wait(lock, [this, qId = q.id, type, &answer, &rememberForType]() {
        if (cancelled_.load(std::memory_order_relaxed)) {
            answer = QuestionAnswer::CANCEL;
            return true;
        }
        // Check if remembered answers now covers this
        auto it = remembered_answers_.find(type);
        if (it != remembered_answers_.end()) {
            answer = it->second;
            return true;
        }
        // Check if question was removed from active_questions_ with a decision
        for (const auto& activeQ : active_questions_) {
            if (activeQ.id == qId) {
                return false; // Still pending
            }
        }
        return true;
    });

    questions_resolved_++;
    return answer;
}

bool TransferManager::answerQuestion(uint64_t questionId, QuestionAnswer answer, bool rememberForType) {
    std::lock_guard<std::mutex> lock(question_mutex_);
    for (auto it = active_questions_.begin(); it != active_questions_.end(); ++it) {
        if (it->id == questionId) {
            QuestionType type = it->type;
            if (rememberForType) {
                remembered_answers_[type] = answer;
            }
            active_questions_.erase(it);
            cv_questions_.notify_all();
            return true;
        }
    }
    return false;
}

bool TransferManager::transferFile(TransferItem& item) {
    FileStat srcSt = fs_->statFile(item.src_path, false);
    if (!srcSt.exists) return false;

    // Check move with read-only source requirement
    if (options_.mode == TransferMode::MOVE && srcSt.is_read_only) {
        QuestionAnswer answer = resolveQuestion(item, QuestionType::MOVE_READ_ONLY_SOURCE,
                                                "Source file is read-only. Proceed with move: " + item.src_path);
        if (answer == QuestionAnswer::CANCEL) {
            cancel();
            return false;
        }
        if (answer == QuestionAnswer::SKIP) {
            return true;
        }
    }

    // Check collision if destination exists
    int dstFlags = O_WRONLY | O_CREAT;
    off_t startOffset = 0;
    off_t srcOffset = 0;

    bool dstExists = fs_->exists(item.dst_path);
    if (dstExists) {
        FileStat dstLStat = fs_->statFile(item.dst_path, false);
        bool isRealDir = (!dstLStat.is_symlink && dstLStat.is_directory);

        // Case: source/file is a file and destination/file is a folder
        if (isRealDir) {
            QuestionAnswer answer = resolveQuestion(
                item, QuestionType::FILE_DESTINATION_IS_FOLDER,
                "Destination exists and is a folder, but source is a file: " + item.dst_path);
            if (answer == QuestionAnswer::CANCEL) {
                cancel();
                return false;
            }
            if (answer == QuestionAnswer::SKIP) {
                return true;
            }
            // PLACE_INTO: place as destination/file/file
            fs::path srcP(item.src_path);
            std::string fname = srcP.filename().string();
            item.dst_path = (item.dst_path.back() == '/') ? (item.dst_path + fname) : (item.dst_path + "/" + fname);

            // Re-evaluate if destination/file/file already exists
            dstExists = fs_->exists(item.dst_path);
            if (dstExists) {
                dstLStat = fs_->statFile(item.dst_path, false);
                isRealDir = (!dstLStat.is_symlink && dstLStat.is_directory);
            }
        }

        // Case: destination/file exists and is a regular file or link
        if (dstExists && !isRealDir) {
            FileStat dstStatFollow = fs_->statFile(item.dst_path, true);
            QuestionAnswer answer = resolveQuestion(
                item, QuestionType::OVERWRITE_EXISTING,
                "Destination file already exists (" + std::string(dstLStat.is_symlink ? "symlink" : "file") + "): " + item.dst_path);

            if (answer == QuestionAnswer::CANCEL) {
                cancel();
                return false;
            }
            if (answer == QuestionAnswer::SKIP) {
                // simply move to the next item
                return true;
            }
            if (answer == QuestionAnswer::RENAME) {
                // Rename source copy destination, keeping original destination file intact
                item.dst_path = generateUniqueRenamedPath(fs_.get(), item.dst_path);
                dstFlags = O_WRONLY | O_CREAT | O_TRUNC;
                startOffset = 0;
                srcOffset = 0;
            } else if (answer == QuestionAnswer::APPEND) {
                // Open destination file for writing, set pointer to end of file
                dstFlags = O_WRONLY | O_CREAT;
                startOffset = dstStatFollow.exists ? dstStatFollow.size : 0;
                srcOffset = 0;
                item.is_append = true;
            } else if (answer == QuestionAnswer::RESUME) {
                // Open destination at end, open source at destination size
                off_t resumePos = dstStatFollow.exists ? dstStatFollow.size : 0;
                if (resumePos >= srcSt.size) {
                    files_transferred_++;
                    return true;
                }
                dstFlags = O_WRONLY | O_CREAT;
                startOffset = resumePos;
                srcOffset = resumePos;
                bytes_transferred_.fetch_add(resumePos, std::memory_order_relaxed);
            } else {
                // OVERWRITE: do not delete destination/file, open and truncate prior to writes
                // If destination is a link, this updates the referred file without breaking the link
                dstFlags = O_WRONLY | O_CREAT | O_TRUNC;
                startOffset = 0;
                srcOffset = 0;
            }
        }
    }

    item.src_start_offset = srcOffset;
    item.dst_start_offset = startOffset;

    int srcFd = fs_->openFile(item.src_path, O_RDONLY);
    if (srcFd < 0) {
        QuestionAnswer ans = resolveQuestion(item, QuestionType::PERMISSION_DENIED,
                                             "Cannot open source file: " + item.src_path, errno);
        if (ans == QuestionAnswer::CANCEL) cancel();
        return false;
    }

    int dstFd = fs_->openFile(item.dst_path, dstFlags, srcSt.mode ? (srcSt.mode & 0777) : 0644);
    if (dstFd < 0) {
        int err = errno ? errno : EIO;
        if (isCriticalIoError(err)) {
            QuestionAnswer act = handleCriticalIo(item.dst_path, err, "open destination");
            if (act == QuestionAnswer::RETRY) {
                dstFd = fs_->openFile(item.dst_path, dstFlags, srcSt.mode ? (srcSt.mode & 0777) : 0644);
            }
        }
    }
    if (dstFd < 0) {
        fs_->closeFile(srcFd);
        QuestionAnswer ans = resolveQuestion(item, QuestionType::PERMISSION_DENIED,
                                             "Cannot open destination file: " + item.dst_path, errno);
        if (ans == QuestionAnswer::CANCEL) cancel();
        return false;
    }

    bool ok = false;

    // Fast Path 1: Copy-on-Write (FICLONE on Linux, clonefile on macOS)
    // CoW can only be used if we are starting from offset 0 with full overwrite
    if (options_.enable_cow && startOffset == 0 && dstFlags & O_TRUNC) {
        if (tryCoWTransfer(item, srcFd, dstFd)) {
            ok = true;
            cow_bytes_.fetch_add(srcSt.size, std::memory_order_relaxed);
            bytes_transferred_.fetch_add(srcSt.size, std::memory_order_relaxed);
        }
    }

    // Fast Path 2: Sparse File transfer (SEEK_DATA / SEEK_HOLE)
    if (!ok && options_.enable_sparse && srcSt.is_sparse && startOffset == 0) {
        ok = transferSparseFile(item, srcFd, dstFd);
    }

    // Fast Path 3: Memory-mapped I/O for small files (<= small_file_mmap_threshold)
    if (!ok && static_cast<size_t>(srcSt.size) <= options_.small_file_mmap_threshold && startOffset == 0) {
        ok = transferSmallFileMmap(item, srcFd, dstFd);
        if (ok) {
            mmap_bytes_.fetch_add(srcSt.size, std::memory_order_relaxed);
            bytes_transferred_.fetch_add(srcSt.size, std::memory_order_relaxed);
        }
    }

    // Fast Path 4: Multithreaded chunked read/write ring or copy_file_range
    if (!ok) {
        ok = transferChunked(item, srcFd, dstFd);
    }

    fs_->syncFile(dstFd);
    fs_->closeFile(srcFd);
    fs_->closeFile(dstFd);

    if (ok) {
        if (options_.preserve_permissions || options_.preserve_timestamps) {
            fs_->copyMetadata(item.src_path, item.dst_path, options_.preserve_ownership);
        }
        if (options_.preserve_xattrs) {
            fs_->copyXattrs(item.src_path, item.dst_path);
        }
        if (options_.mode == TransferMode::MOVE) {
            fs_->removeFile(item.src_path);
        }
        files_transferred_++;
    }

    return ok;
}

bool TransferManager::tryCoWTransfer(TransferItem& item, int srcFd, int dstFd) {
    return fs_->tryCloneOrCoW(srcFd, dstFd, item.size);
}

bool TransferManager::transferSparseFile(TransferItem& item, int srcFd, int dstFd) {
    off_t offset = 0;
    off_t fileSize = item.size;
    fs_->truncateFile(dstFd, fileSize);

    std::vector<uint8_t> buffer(64 * 1024);

    while (offset < fileSize && !cancelled_.load(std::memory_order_relaxed)) {
        off_t dataStart = fs_->seekData(srcFd, offset);
        if (dataStart < 0) {
            // No more data blocks
            break;
        }

        off_t holeStart = fs_->seekHole(srcFd, dataStart);
        if (holeStart < 0) {
            holeStart = fileSize;
        }

        if (dataStart > offset) {
            sparse_bytes_skipped_.fetch_add(dataStart - offset, std::memory_order_relaxed);
        }

        // Copy extent [dataStart, holeStart)
        off_t extentOffset = dataStart;
        while (extentOffset < holeStart && !cancelled_.load(std::memory_order_relaxed)) {
            size_t toRead = std::min<size_t>(buffer.size(), holeStart - extentOffset);
            ssize_t bytesRead = fs_->readFile(srcFd, buffer.data(), toRead, extentOffset);
            if (bytesRead <= 0) break;

            ssize_t written = fs_->writeFile(dstFd, buffer.data(), bytesRead, extentOffset);
            if (written != bytesRead) {
                return false;
            }
            bytes_transferred_.fetch_add(written, std::memory_order_relaxed);
            extentOffset += written;
        }

        offset = holeStart;
    }

    return true;
}

bool TransferManager::transferSmallFileMmap(TransferItem& item, int srcFd, int dstFd) {
    if (item.size == 0) {
        fs_->truncateFile(dstFd, 0);
        return true;
    }

    void* mapped = fs_->mapFile(srcFd, item.size, 0, true);
    if (!mapped) {
        return false;
    }

    ssize_t written = fs_->writeFile(dstFd, mapped, item.size, 0);
    fs_->unmapFile(mapped, item.size);

    return (written == item.size);
}

bool TransferManager::transferChunked(TransferItem& item, int srcFd, int dstFd) {
    off_t srcStart = item.src_start_offset;
    off_t dstStart = item.dst_start_offset;
    off_t fileSize = item.size;

    if (srcStart >= fileSize) {
        return true;
    }

    off_t bytesToTransfer = fileSize - srcStart;
    off_t bytesCopied = 0;

    // First attempt kernel zero-copy copy_file_range if supported
    while (bytesCopied < bytesToTransfer && !cancelled_.load(std::memory_order_relaxed)) {
        off_t srcOff = srcStart + bytesCopied;
        off_t dstOff = dstStart + bytesCopied;
        size_t chunkLen = std::min<size_t>(16 * 1024 * 1024, bytesToTransfer - bytesCopied);

        ssize_t ret = fs_->copyFileRange(srcFd, &srcOff, dstFd, &dstOff, chunkLen);
        if (ret > 0) {
            bytes_transferred_.fetch_add(ret, std::memory_order_relaxed);
            bytesCopied += ret;
            continue;
        }
        // If copy_file_range returns EXDEV or ENOSYS, fall through to BufferRing
        break;
    }

    if (bytesCopied >= bytesToTransfer) {
        return true;
    }

    // Multithreaded BufferRing pipeline
    std::atomic<size_t> inFlight{0};
    std::condition_variable cvFileDone;
    std::mutex fileMutex;

    while (bytesCopied < bytesToTransfer && !cancelled_.load(std::memory_order_relaxed)) {
        BufferChunk* chunk = buffer_ring_->acquireFreeChunk();
        if (!chunk) {
            // Ring shutdown
            return false;
        }

        chunk->src_fd = srcFd;
        chunk->dst_fd = dstFd;
        chunk->src_offset = srcStart + bytesCopied;
        chunk->dst_offset = dstStart + bytesCopied;
        chunk->task_id = item.id;
        chunk->in_flight_counter = &inFlight;
        chunk->cv_file_done = &cvFileDone;
        chunk->file_done_mutex = &fileMutex;

        size_t toRead = std::min<size_t>(chunk->capacity, bytesToTransfer - bytesCopied);
        ssize_t bytesRead = fs_->readFile(srcFd, chunk->buffer, toRead, chunk->src_offset);

        if (bytesRead <= 0) {
            int err = (bytesRead < 0) ? errno : 0;
            buffer_ring_->publishReadyChunk(chunk, 0, true, err);
            if (bytesRead < 0) return false;
            break;
        }

        if (options_.verify_integrity) {
            chunk->crc32 = BufferRing::computeCRC32(chunk->buffer, bytesRead);
        }

        bytesCopied += bytesRead;
        bool isEof = (bytesCopied >= bytesToTransfer);
        inFlight.fetch_add(1, std::memory_order_release);
        buffer_ring_->publishReadyChunk(chunk, bytesRead, isEof, 0);
    }

    // Wait until all in-flight chunks for this file are written by writer workers
    if (inFlight.load(std::memory_order_acquire) > 0) {
        std::unique_lock<std::mutex> lock(fileMutex);
        cvFileDone.wait(lock, [&]() {
            return inFlight.load(std::memory_order_acquire) == 0 || cancelled_.load(std::memory_order_relaxed);
        });
    }

    return true;
}

void TransferManager::progressWorker() {
    auto lastUpdate = std::chrono::steady_clock::now();
    uint64_t lastBytes = 0;

    while (running_.load(std::memory_order_acquire) && !cancelled_.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Throttled to ~10 Hz

        auto now = std::chrono::steady_clock::now();
        double elapsedDelta = std::chrono::duration<double>(now - lastUpdate).count();
        uint64_t currentBytes = bytes_transferred_.load(std::memory_order_relaxed);
        uint64_t bytesDelta = (currentBytes >= lastBytes) ? (currentBytes - lastBytes) : 0;

        double speedMBs = (elapsedDelta > 0.001) ? (static_cast<double>(bytesDelta) / (1024.0 * 1024.0 * elapsedDelta)) : 0.0;

        lastUpdate = now;
        lastBytes = currentBytes;

        TransferProgress prog = getProgress();
        prog.current_speed_mb_s = speedMBs;

        if (progress_cb_) {
            progress_cb_(prog);
        } else {
            onProgress(prog);
        }
    }
}

TransferMetrics TransferManager::getMetrics() const {
    TransferMetrics m;
    m.transferred_bytes = bytes_transferred_.load(std::memory_order_relaxed);
    m.total_bytes = total_bytes_.load(std::memory_order_relaxed);
    m.transferred_files = files_transferred_.load(std::memory_order_relaxed);
    m.total_files = total_files_.load(std::memory_order_relaxed);
    m.transferred_dirs = dirs_transferred_.load(std::memory_order_relaxed);
    m.total_dirs = total_dirs_.load(std::memory_order_relaxed);
    m.error_count = error_count_.load(std::memory_order_relaxed);
    m.questions_resolved_count = questions_resolved_.load(std::memory_order_relaxed);
    m.cow_cloned_bytes = cow_bytes_.load(std::memory_order_relaxed);
    m.sparse_bytes_skipped = sparse_bytes_skipped_.load(std::memory_order_relaxed);
    m.mmap_transferred_bytes = mmap_bytes_.load(std::memory_order_relaxed);
    m.had_critical_io_error = had_critical_io_error_.load(std::memory_order_relaxed);
    m.buffer_ring_size_mb = getBufferRingSizeMB();
    m.total_jobs_processed = total_jobs_processed_.load(std::memory_order_relaxed);

    std::lock_guard<std::mutex> lock(question_mutex_);
    m.active_questions_count = static_cast<uint32_t>(active_questions_.size());

    auto now = std::chrono::steady_clock::now();
    m.elapsed_seconds = std::chrono::duration<double>(now - start_time_).count();
    if (m.elapsed_seconds > 0.001) {
        m.average_speed_mb_s = (static_cast<double>(m.transferred_bytes) / (1024.0 * 1024.0 * m.elapsed_seconds));
    }
    return m;
}

TransferProgress TransferManager::getProgress() const {
    TransferProgress p;
    p.bytes_transferred = bytes_transferred_.load(std::memory_order_relaxed);
    p.total_bytes = total_bytes_.load(std::memory_order_relaxed);
    p.files_transferred = files_transferred_.load(std::memory_order_relaxed);
    p.total_files = total_files_.load(std::memory_order_relaxed);
    p.dirs_transferred = dirs_transferred_.load(std::memory_order_relaxed);
    p.total_dirs = total_dirs_.load(std::memory_order_relaxed);
    p.errors_count = error_count_.load(std::memory_order_relaxed);
    p.questions_resolved_count = questions_resolved_.load(std::memory_order_relaxed);
    p.is_paused_for_io_error = io_error_paused_.load(std::memory_order_relaxed);
    {
        std::lock_guard<std::mutex> lock(io_error_mutex_);
        p.io_error_reason = io_error_reason_;
    }
    p.buffer_ring_size_mb = getBufferRingSizeMB();

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        p.active_jobs_count = static_cast<uint32_t>(pending_job_requests_.size());
    }

    {
        std::lock_guard<std::mutex> lock(question_mutex_);
        p.active_questions_count = static_cast<uint32_t>(active_questions_.size());
    }

    {
        std::lock_guard<std::mutex> lock(display_mutex_);
        p.current_source_item = current_source_display_;
        p.current_target_item = current_target_display_;
    }

    if (p.total_bytes > 0) {
        p.progress_percent = std::min<double>(100.0, (static_cast<double>(p.bytes_transferred) / p.total_bytes) * 100.0);
    } else if (p.total_files > 0) {
        p.progress_percent = std::min<double>(100.0, (static_cast<double>(p.files_transferred) / p.total_files) * 100.0);
    } else {
        p.progress_percent = 0.0;
    }

    auto now = std::chrono::steady_clock::now();
    p.elapsed_seconds = std::chrono::duration<double>(now - start_time_).count();

    return p;
}

std::vector<TransferQuestion> TransferManager::getActiveQuestions() const {
    std::lock_guard<std::mutex> lock(question_mutex_);
    return active_questions_;
}

// Default virtual overrides
void TransferManager::onProgress(const TransferProgress& /*progress*/) {
    // Default no-op, overridden by subclasses or custom progress callback
}

QuestionAnswer TransferManager::onQuestion(const TransferQuestion& /*question*/, bool& /*rememberForType*/) {
    // Default returns PROMPT to allow active question pool collection
    return QuestionAnswer::PROMPT;
}
