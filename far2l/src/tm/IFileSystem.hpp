#pragma once

#include "TransferTypes.hpp"
#include <string>
#include <vector>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * @brief IFileSystem defines the abstraction layer for all file, directory,
 * metadata, and extended attribute operations.
 *
 * In far2l, this allows swapping the direct local OS syscalls with a
 * SudoProxyFileSystem (or remote/virtual VFS) that communicates with an elevated
 * worker process over a UNIX domain socket.
 */
class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    // Path & Stat operations
    virtual bool exists(const std::string& path) = 0;
    virtual bool isDirectory(const std::string& path) = 0;
    virtual bool isSymlink(const std::string& path) = 0;
    virtual bool isReadOnly(const std::string& path) = 0;
    virtual FileStat statFile(const std::string& path, bool followSymlinks = false) = 0;

    // File I/O operations
    virtual int openFile(const std::string& path, int flags, mode_t mode = 0644) = 0;
    virtual void closeFile(int fd) = 0;
    virtual ssize_t readFile(int fd, void* buf, size_t count, off_t offset) = 0;
    virtual ssize_t writeFile(int fd, const void* buf, size_t count, off_t offset) = 0;
    virtual bool truncateFile(int fd, off_t length) = 0;
    virtual bool syncFile(int fd) = 0;

    // High-performance Zero-Copy and CoW
    virtual bool tryCloneOrCoW(int srcFd, int dstFd, off_t size) = 0;
    virtual ssize_t copyFileRange(int srcFd, off_t* srcOff, int dstFd, off_t* dstOff, size_t len) = 0;

    // Sparse File Support (SEEK_DATA / SEEK_HOLE)
    virtual off_t seekData(int fd, off_t offset) = 0;
    virtual off_t seekHole(int fd, off_t offset) = 0;
    virtual bool punchHole(int fd, off_t offset, off_t length) = 0;

    // Memory Mapping for small files
    virtual void* mapFile(int fd, size_t length, off_t offset, bool readOnly) = 0;
    virtual void unmapFile(void* addr, size_t length) = 0;

    // Directory operations
    virtual bool createDirectory(const std::string& path, mode_t mode = 0755) = 0;
    virtual bool createDirectories(const std::string& path) = 0;
    virtual std::vector<DirEntry> listDirectory(const std::string& path) = 0;
    virtual bool removeFile(const std::string& path) = 0;
    virtual bool removeDirectory(const std::string& path) = 0;
    virtual bool renameFile(const std::string& oldPath, const std::string& newPath) = 0;

    // Symbolic link operations
    virtual std::string readSymlink(const std::string& path) = 0;
    virtual bool createSymlink(const std::string& target, const std::string& linkPath) = 0;

    // Metadata, times, permissions, and extended attributes
    virtual bool copyMetadata(const std::string& src, const std::string& dst, bool copyOwnership = false) = 0;
    virtual bool copyXattrs(const std::string& src, const std::string& dst) = 0;
    virtual bool setPermissions(const std::string& path, mode_t mode) = 0;
    virtual bool setOwnership(const std::string& path, uid_t uid, gid_t gid) = 0;
    virtual bool setTimestamps(const std::string& path, int64_t atime_sec, long atime_nsec,
                               int64_t mtime_sec, long mtime_nsec) = 0;
};
