#include "LocalFileSystem.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <system_error>

#if defined(__linux__)
    #include <sys/ioctl.h>
    #include <sys/xattr.h>
    #include <linux/fs.h>
    #ifndef FICLONE
        #define FICLONE _IOW(0x94, 9, int)
    #endif
#elif defined(__APPLE__)
    #include <sys/xattr.h>
    #include <sys/clonefile.h>
    #include <copyfile.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #include <sys/extattr.h>
#endif

bool LocalFileSystem::exists(const std::string& path) {
    struct stat st;
    return (lstat(path.c_str(), &st) == 0);
}

bool LocalFileSystem::isDirectory(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

bool LocalFileSystem::isSymlink(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) return false;
    return S_ISLNK(st.st_mode);
}

bool LocalFileSystem::isReadOnly(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) return false;
    // Check if write bit is missing for owner
    return !(st.st_mode & S_IWUSR);
}

FileStat LocalFileSystem::statFile(const std::string& path, bool followSymlinks) {
    FileStat info;
    struct stat st;
    int res = followSymlinks ? stat(path.c_str(), &st) : lstat(path.c_str(), &st);
    if (res != 0) {
        info.exists = false;
        return info;
    }

    info.exists = true;
    info.size = st.st_size;
    info.mode = st.st_mode;
    info.uid = st.st_uid;
    info.gid = st.st_gid;
    info.is_directory = S_ISDIR(st.st_mode);
    info.is_symlink = S_ISLNK(st.st_mode);
    info.is_regular = S_ISREG(st.st_mode);
    info.is_read_only = !(st.st_mode & S_IWUSR);

#if defined(__APPLE__)
    info.atime_sec = st.st_atimespec.tv_sec;
    info.atime_nsec = st.st_atimespec.tv_nsec;
    info.mtime_sec = st.st_mtimespec.tv_sec;
    info.mtime_nsec = st.st_mtimespec.tv_nsec;
#else
    info.atime_sec = st.st_atim.tv_sec;
    info.atime_nsec = st.st_atim.tv_nsec;
    info.mtime_sec = st.st_mtim.tv_sec;
    info.mtime_nsec = st.st_mtim.tv_nsec;
#endif

    // Detect if file is sparse: allocated 512-byte blocks < st_size / 512
    if (info.is_regular && st.st_size > 4096) {
        blkcnt_t expectedBlocks = (st.st_size + 511) / 512;
        if (st.st_blocks + 8 < expectedBlocks) {
            info.is_sparse = true;
        }
    }

    return info;
}

int LocalFileSystem::openFile(const std::string& path, int flags, mode_t mode) {
    return open(path.c_str(), flags, mode);
}

void LocalFileSystem::closeFile(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

ssize_t LocalFileSystem::readFile(int fd, void* buf, size_t count, off_t offset) {
    return pread(fd, buf, count, offset);
}

ssize_t LocalFileSystem::writeFile(int fd, const void* buf, size_t count, off_t offset) {
    return pwrite(fd, buf, count, offset);
}

bool LocalFileSystem::truncateFile(int fd, off_t length) {
    return (ftruncate(fd, length) == 0);
}

bool LocalFileSystem::syncFile(int fd) {
#if defined(__linux__)
    return (fdatasync(fd) == 0);
#else
    return (fsync(fd) == 0);
#endif
}

bool LocalFileSystem::tryCloneOrCoW(int srcFd, int dstFd, off_t size) {
    (void)size;
#if defined(__linux__) && defined(FICLONE)
    // Try Btrfs / XFS reflink copy via ioctl(FICLONE)
    if (ioctl(dstFd, FICLONE, srcFd) == 0) {
        return true;
    }
#elif defined(__APPLE__)
    // On macOS, fclonefile can clone directly between open descriptors if supported
    #if defined(CLONE_NOFOLLOW)
    if (fclonefile(srcFd, dstFd, 0) == 0) {
        return true;
    }
    #endif
#endif
    return false;
}

ssize_t LocalFileSystem::copyFileRange(int srcFd, off_t* srcOff, int dstFd, off_t* dstOff, size_t len) {
#if (defined(__linux__) || defined(__FreeBSD__))
    // copy_file_range allows server-side zero-copy data transfer inside kernel
    ssize_t ret = ::copy_file_range(srcFd, srcOff, dstFd, dstOff, len, 0);
    return ret;
#else
    (void)srcFd; (void)srcOff; (void)dstFd; (void)dstOff; (void)len;
    errno = ENOSYS;
    return -1;
#endif
}

off_t LocalFileSystem::seekData(int fd, off_t offset) {
#if defined(SEEK_DATA)
    off_t res = lseek(fd, offset, SEEK_DATA);
    return res;
#else
    (void)fd; (void)offset;
    return offset;
#endif
}

off_t LocalFileSystem::seekHole(int fd, off_t offset) {
#if defined(SEEK_HOLE)
    off_t res = lseek(fd, offset, SEEK_HOLE);
    return res;
#else
    (void)fd; (void)offset;
    return -1;
#endif
}

bool LocalFileSystem::punchHole(int fd, off_t offset, off_t length) {
#if defined(__linux__) && defined(FALLOC_FL_PUNCH_HOLE) && defined(FALLOC_FL_KEEP_SIZE)
    return (fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset, length) == 0);
#else
    (void)fd; (void)offset; (void)length;
    return false;
#endif
}

void* LocalFileSystem::mapFile(int fd, size_t length, off_t offset, bool readOnly) {
    int prot = readOnly ? PROT_READ : (PROT_READ | PROT_WRITE);
    int flags = MAP_SHARED;
    void* addr = mmap(nullptr, length, prot, flags, fd, offset);
    if (addr == MAP_FAILED) {
        return nullptr;
    }
    // Advise kernel that sequential access is anticipated
    posix_madvise(addr, length, POSIX_MADV_WILLNEED | POSIX_MADV_SEQUENTIAL);
    return addr;
}

void LocalFileSystem::unmapFile(void* addr, size_t length) {
    if (addr && addr != MAP_FAILED) {
        munmap(addr, length);
    }
}

bool LocalFileSystem::createDirectory(const std::string& path, mode_t mode) {
    if (mkdir(path.c_str(), mode) == 0) return true;
    return (errno == EEXIST);
}

bool LocalFileSystem::createDirectories(const std::string& path) {
    if (path.empty()) return false;
    std::string current;
    size_t start = 0;
    if (path[0] == '/') {
        current = "/";
        start = 1;
    }
    for (size_t i = start; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '/') {
            current = path.substr(0, i);
            if (!current.empty() && current != "/") {
                if (mkdir(current.c_str(), 0755) != 0 && errno != EEXIST) {
                    return false;
                }
            }
        }
    }
    return true;
}

std::vector<DirEntry> LocalFileSystem::listDirectory(const std::string& path) {
    std::vector<DirEntry> entries;
    DIR* dir = opendir(path.c_str());
    if (!dir) return entries;

    struct dirent* entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        DirEntry de;
        de.name = name;
        de.full_path = (path.back() == '/') ? (path + name) : (path + "/" + name);

        struct stat st;
        if (lstat(de.full_path.c_str(), &st) == 0) {
            de.is_directory = S_ISDIR(st.st_mode);
            de.is_symlink = S_ISLNK(st.st_mode);
            de.size = st.st_size;
        }
        entries.push_back(std::move(de));
    }
    closedir(dir);
    return entries;
}

bool LocalFileSystem::removeFile(const std::string& path) {
    return (unlink(path.c_str()) == 0);
}

bool LocalFileSystem::removeDirectory(const std::string& path) {
    return (rmdir(path.c_str()) == 0);
}

std::string LocalFileSystem::readSymlink(const std::string& path) {
    std::vector<char> buffer(1024, 0);
    while (true) {
        ssize_t len = readlink(path.c_str(), buffer.data(), buffer.size() - 1);
        if (len < 0) {
            return "";
        }
        if (static_cast<size_t>(len) < buffer.size() - 1) {
            buffer[len] = '\0';
            return std::string(buffer.data());
        }
        buffer.resize(buffer.size() * 2);
    }
}

bool LocalFileSystem::createSymlink(const std::string& target, const std::string& linkPath) {
    // If destination already exists, unlink it first
    unlink(linkPath.c_str());
    return (symlink(target.c_str(), linkPath.c_str()) == 0);
}

bool LocalFileSystem::copyMetadata(const std::string& src, const std::string& dst, bool copyOwnership) {
    FileStat st = statFile(src, false);
    if (!st.exists) return false;

    // Permissions (unless symlink)
    if (!st.is_symlink) {
        setPermissions(dst, st.mode & 07777);
    }

    // Timestamps
    setTimestamps(dst, st.atime_sec, st.atime_nsec, st.mtime_sec, st.mtime_nsec);

    // Ownership (sudo / root escalated)
    if (copyOwnership && (geteuid() == 0)) {
        setOwnership(dst, st.uid, st.gid);
    }

    return true;
}

bool LocalFileSystem::copyXattrs(const std::string& src, const std::string& dst) {
#if defined(__linux__)
    ssize_t listLen = listxattr(src.c_str(), nullptr, 0);
    if (listLen <= 0) return true;

    std::vector<char> listBuf(listLen);
    listLen = listxattr(src.c_str(), listBuf.data(), listLen);
    if (listLen <= 0) return true;

    const char* key = listBuf.data();
    while (key < listBuf.data() + listLen) {
        ssize_t valLen = getxattr(src.c_str(), key, nullptr, 0);
        if (valLen >= 0) {
            std::vector<char> valBuf(valLen > 0 ? valLen : 1);
            valLen = getxattr(src.c_str(), key, valBuf.data(), valLen);
            if (valLen >= 0) {
                setxattr(dst.c_str(), key, valBuf.data(), valLen, 0);
            }
        }
        key += strlen(key) + 1;
    }
    return true;
#elif defined(__APPLE__)
    ssize_t listLen = listxattr(src.c_str(), nullptr, 0, XATTR_NOFOLLOW);
    if (listLen <= 0) return true;

    std::vector<char> listBuf(listLen);
    listLen = listxattr(src.c_str(), listBuf.data(), listLen, XATTR_NOFOLLOW);
    if (listLen <= 0) return true;

    const char* key = listBuf.data();
    while (key < listBuf.data() + listLen) {
        ssize_t valLen = getxattr(src.c_str(), key, nullptr, 0, 0, XATTR_NOFOLLOW);
        if (valLen >= 0) {
            std::vector<char> valBuf(valLen > 0 ? valLen : 1);
            valLen = getxattr(src.c_str(), key, valBuf.data(), valLen, 0, XATTR_NOFOLLOW);
            if (valLen >= 0) {
                setxattr(dst.c_str(), key, valBuf.data(), valLen, 0, XATTR_NOFOLLOW);
            }
        }
        key += strlen(key) + 1;
    }
    return true;
#else
    (void)src; (void)dst;
    return true;
#endif
}

bool LocalFileSystem::setPermissions(const std::string& path, mode_t mode) {
    return (chmod(path.c_str(), mode) == 0);
}

bool LocalFileSystem::setOwnership(const std::string& path, uid_t uid, gid_t gid) {
    return (lchown(path.c_str(), uid, gid) == 0);
}

bool LocalFileSystem::setTimestamps(const std::string& path, int64_t atime_sec, long atime_nsec,
                                    int64_t mtime_sec, long mtime_nsec) {
    struct timespec times[2];
    times[0].tv_sec = atime_sec;
    times[0].tv_nsec = atime_nsec;
    times[1].tv_sec = mtime_sec;
    times[1].tv_nsec = mtime_nsec;

    // Use utimensat with AT_SYMLINK_NOFOLLOW so symlink timestamps can be updated
    return (utimensat(AT_FDCWD, path.c_str(), times, AT_SYMLINK_NOFOLLOW) == 0);
}
