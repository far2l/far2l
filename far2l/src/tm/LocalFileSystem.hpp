#pragma once

#include "IFileSystem.hpp"
#include <string>
#include <vector>

class LocalFileSystem : public IFileSystem {
public:
    LocalFileSystem() = default;
    ~LocalFileSystem() override = default;

    bool exists(const std::wstring& path) override;
    bool isDirectory(const std::wstring& path) override;
    bool isSymlink(const std::wstring& path) override;
    bool isReadOnly(const std::wstring& path) override;
    FileStat statFile(const std::wstring& path, bool followSymlinks = false) override;

    int openFile(const std::wstring& path, int flags, mode_t mode = 0644) override;
    void closeFile(int fd) override;
    ssize_t readFile(int fd, void* buf, size_t count, off_t offset) override;
    ssize_t writeFile(int fd, const void* buf, size_t count, off_t offset) override;
    bool truncateFile(int fd, off_t length) override;
    bool syncFile(int fd) override;

    bool tryCloneOrCoW(int srcFd, int dstFd, off_t size) override;
    ssize_t copyFileRange(int srcFd, off_t* srcOff, int dstFd, off_t* dstOff, size_t len) override;

    off_t seekData(int fd, off_t offset) override;
    off_t seekHole(int fd, off_t offset) override;
    bool punchHole(int fd, off_t offset, off_t length) override;

    void* mapFile(int fd, size_t length, off_t offset, bool readOnly) override;
    void unmapFile(void* addr, size_t length) override;

    bool createDirectory(const std::wstring& path, mode_t mode = 0755) override;
    bool createDirectories(const std::wstring& path) override;
    std::vector<DirEntry> listDirectory(const std::wstring& path) override;
    bool removeFile(const std::wstring& path) override;
    bool removeDirectory(const std::wstring& path) override;
    bool renameFile(const std::wstring& oldPath, const std::wstring& newPath) override;

    std::wstring readSymlink(const std::wstring& path) override;
    bool createSymlink(const std::wstring& target, const std::wstring& linkPath) override;

    bool copyMetadata(const std::wstring& src, const std::wstring& dst, bool copyOwnership = false) override;
    bool copyXattrs(const std::wstring& src, const std::wstring& dst) override;
    bool setPermissions(const std::wstring& path, mode_t mode) override;
    bool setOwnership(const std::wstring& path, uid_t uid, gid_t gid) override;
    bool setTimestamps(const std::wstring& path, int64_t atime_sec, long atime_nsec,
                       int64_t mtime_sec, long mtime_nsec) override;
};
