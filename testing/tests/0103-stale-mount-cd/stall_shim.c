// stall_shim.c — LD_PRELOAD repro tool for far2l stale-folder debugging (dev-shell only, never shipped).
// Makes every file lookup under STALL_SHIM_DIR hang like a stalled vboxsf mount:
//  - libc stat family, open/openat, chdir: sleep-forever loop for paths under the magic dir
//  - raw syscall(SYS_openat, ...): same, so MountResponsive()'s raw-syscall probe child stalls too
//  - /proc/self/mountinfo: served from STALL_SHIM_MOUNTINFO (fake template) so MountFsType()
//    detects the magic dir as fstype "vboxsf" and the production gate engages
// Everything else passes through untouched. Intercepts are logged when STALL_SHIM_LOG is set.
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define FD_TRACK_MAX 64
static struct {
	int fd;
	char path[PATH_MAX];
} g_fdtrack[FD_TRACK_MAX];

static char g_dir[PATH_MAX];	// magic stalled dir; empty = shim inert
static char g_mntinf[PATH_MAX];	// fake mountinfo template; empty = no fake
static char g_log[PATH_MAX];
static int g_logfd = -1;

typedef long (*syscall_fn)(long, ...);
typedef int (*open2_fn)(const char *, int, ...);
typedef int (*openat_fn)(int, const char *, int, ...);
typedef int (*chdir_fn)(const char *);

static syscall_fn real_syscall;
static open2_fn real_open, real_open64;
static openat_fn real_openat, real_openat64;
static chdir_fn real_chdir;
typedef FILE *(*fopen_fn)(const char *, const char *);
static fopen_fn real_fopen, real_fopen64;
static ssize_t (*real_write)(int, const void *, size_t);
static int (*real_stat)(const char *, struct stat *);
static int (*real_stat64)(const char *, struct stat64 *);
static int (*real_lstat)(const char *, struct stat *);
static int (*real_lstat64)(const char *, struct stat64 *);
static int (*real_xstat)(int, const char *, struct stat *);
static int (*real_xstat64)(int, const char *, struct stat64 *);
static int (*real_lxstat)(int, const char *, struct stat *);
static int (*real_lxstat64)(int, const char *, struct stat64 *);
static int (*real_statx)(int, const char *, int, unsigned int, struct statx *);

static void resolve_one(void **slot, const char *name)
{
	if (*slot == NULL)
		*slot = dlsym(RTLD_NEXT, name);
}

static void resolve_all(void)
{
	resolve_one((void **)&real_syscall, "syscall");
	resolve_one((void **)&real_open, "open");
	resolve_one((void **)&real_open64, "open64");
	resolve_one((void **)&real_openat, "openat");
	resolve_one((void **)&real_openat64, "openat64");
	resolve_one((void **)&real_chdir, "chdir");
	resolve_one((void **)&real_fopen, "fopen");
	resolve_one((void **)&real_fopen64, "fopen64");
	resolve_one((void **)&real_write, "write");
	resolve_one((void **)&real_stat, "stat");
	resolve_one((void **)&real_stat64, "stat64");
	resolve_one((void **)&real_lstat, "lstat");
	resolve_one((void **)&real_lstat64, "lstat64");
	resolve_one((void **)&real_xstat, "__xstat");
	resolve_one((void **)&real_xstat64, "__xstat64");
	resolve_one((void **)&real_lxstat, "__lxstat");
	resolve_one((void **)&real_lxstat64, "__lxstat64");
	resolve_one((void **)&real_statx, "statx");
}

static void shim_init(void) __attribute__((constructor));
static void shim_init(void)
{
	for (int i = 0; i < FD_TRACK_MAX; i++)
		g_fdtrack[i].fd = -1;
	const char *e = getenv("STALL_SHIM_DIR");
	snprintf(g_dir, sizeof(g_dir), "%s", (e && *e) ? e : "/tmp/far2l-stale/deep");
	e = getenv("STALL_SHIM_MOUNTINFO");
	if (e && *e)
		snprintf(g_mntinf, sizeof(g_mntinf), "%s", e);
	e = getenv("STALL_SHIM_LOG");
	if (e && *e) {
		snprintf(g_log, sizeof(g_log), "%s", e);
		g_logfd = open(g_log, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
	}
}

static void shim_log(const char *what, const char *path)
{
	if (g_logfd == -1)
		return;
	char buf[PATH_MAX + 128];
	int n = snprintf(buf, sizeof(buf), "[stall-shim] %s pid=%d path=%s\n", what, (int)getpid(), path ? path : "(null)");
	if (n > 0)
		real_write(g_logfd, buf, (size_t)n);
}

// Exact dir or anything below it. Parents of the magic dir stay responsive
// (panel parking in /tmp/far2l-stale must keep working; only 'deep' stalls).
static int stall_match(const char *path)
{
	if (!path || !g_dir[0])
		return 0;
	const size_t len = strlen(g_dir);
	if (strncmp(path, g_dir, len) != 0)
		return 0;
	return path[len] == '\0' || path[len] == '/';
}

static int mountinfo_match(const char *path)
{
	return g_mntinf[0] && path && strcmp(path, "/proc/self/mountinfo") == 0;
}

static void do_stall(const char *hook, const char *path)
{
	char msg[PATH_MAX + 64];
	int n = snprintf(msg, sizeof(msg), "STALL hook=%s path=%s\n", hook, path ? path : "(null)");
	if (g_logfd != -1 && n > 0)
		real_write(g_logfd, msg, (size_t)n);
	for (;;)
		sleep(3600);	// like D state: never returns; still killable and GDB-visible
}

static int maybe_stall_path(const char *hook, const char *path)
{
	if (stall_match(path)) {
		shim_log(hook, path);
		do_stall(hook, path);
		return 1;
	}
	return 0;
}

static int open_common(const char *hook, const char *path, int flags, mode_t mode, open2_fn realfn)
{
	resolve_all();
	if (mountinfo_match(path)) {
		shim_log("MOUNTINFO-FAKE", path);
		return realfn(g_mntinf, flags & ~(O_CREAT | O_EXCL | O_TRUNC), mode);
	}
	maybe_stall_path(hook, path);
	return realfn(path, flags, mode);
}

int open(const char *path, int flags, ...)
{
	mode_t mode = 0;
	if (flags & (O_CREAT | O_TMPFILE)) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
	return open_common("open", path, flags, mode, real_open ? real_open : open64);
}

int open64(const char *path, int flags, ...)
{
	mode_t mode = 0;
	if (flags & (O_CREAT | O_TMPFILE)) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
	return open_common("open64", path, flags, mode, real_open64 ? real_open64 : real_open);
}

static int openat_common(const char *hook, int dirfd, const char *path, int flags, mode_t mode, openat_fn realfn)
{
	resolve_all();
	if (mountinfo_match(path)) {
		shim_log("MOUNTINFO-FAKE", path);
		return realfn(dirfd, g_mntinf, flags & ~(O_CREAT | O_EXCL | O_TRUNC), mode);
	}
	maybe_stall_path(hook, path);
	return realfn(dirfd, path, flags, mode);
}

int openat(int dirfd, const char *path, int flags, ...)
{
	mode_t mode = 0;
	if (flags & (O_CREAT | O_TMPFILE)) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
	return openat_common("openat", dirfd, path, flags, mode, real_openat ? real_openat : real_openat64);
}

int openat64(int dirfd, const char *path, int flags, ...)
{
	mode_t mode = 0;
	if (flags & (O_CREAT | O_TMPFILE)) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
	return openat_common("openat64", dirfd, path, flags, mode, real_openat64 ? real_openat64 : real_openat);
}

int chdir(const char *path)
{
	resolve_all();
	maybe_stall_path("chdir", path);
	return real_chdir(path);
}

static FILE *fopen_common(const char *hook, const char *path, const char *mode, fopen_fn realfn)
{
	resolve_all();
	if (mountinfo_match(path)) {
		shim_log("MOUNTINFO-FAKE", path);
		return realfn(g_mntinf, mode);
	}
	maybe_stall_path(hook, path);
	return realfn(path, mode);
}

FILE *fopen(const char *path, const char *mode)
{
	return fopen_common("fopen", path, mode, real_fopen ? real_fopen : (real_fopen64 ? real_fopen64 : fopen64));
}

FILE *fopen64(const char *path, const char *mode)
{
	return fopen_common("fopen64", path, mode, real_fopen64 ? real_fopen64 : (real_fopen ? real_fopen : fopen64));
}

#define STAT_WRAPPER(name, sig_args, sig_decl, call_args, realfn)                                     \
	int name sig_args sig_decl                                                                    \
	{                                                                                             \
		resolve_all();                                                                        \
		maybe_stall_path(#name, path);                                                        \
		if (!realfn) {                                                                        \
			errno = ENOSYS;                                                               \
			return -1;                                                                    \
		}                                                                                     \
		return realfn call_args;                                                              \
	}

STAT_WRAPPER(stat, (const char *path, struct stat *buf), , (path, buf), real_stat)
STAT_WRAPPER(stat64, (const char *path, struct stat64 *buf), , (path, buf), real_stat64)
STAT_WRAPPER(lstat, (const char *path, struct stat *buf), , (path, buf), real_lstat)
STAT_WRAPPER(lstat64, (const char *path, struct stat64 *buf), , (path, buf), real_lstat64)
STAT_WRAPPER(__xstat, (int ver, const char *path, struct stat *buf), , (ver, path, buf), real_xstat)
STAT_WRAPPER(__xstat64, (int ver, const char *path, struct stat64 *buf), , (ver, path, buf), real_xstat64)
STAT_WRAPPER(__lxstat, (int ver, const char *path, struct stat *buf), , (ver, path, buf), real_lxstat)
STAT_WRAPPER(__lxstat64, (int ver, const char *path, struct stat64 *buf), , (ver, path, buf), real_lxstat64)
STAT_WRAPPER(statx, (int dirfd, const char *path, int flags, unsigned int mask, struct statx *sbuf), ,
	(dirfd, path, flags, mask, sbuf), real_statx)

// Raw-syscall layer: MountResponsive()'s probe child calls syscall(SYS_openat, ...) directly,
// bypassing libc openat — interpose syscall() so the probe stalls like the target really would.
// The probe also enumerates the opened dir and newfstatat()s each entry dirfd-relative, so fds
// opened on ancestors of the magic dir are tracked and per-entry newfstatat stalls when the
// resolved entry is the magic dir (a partially stalled mount answers readdir but blocks stats).
static int fdtrack_put(int fd, const char *path)
{
	for (int i = 0; i < FD_TRACK_MAX; i++) {
		if (g_fdtrack[i].fd == -1) {
			g_fdtrack[i].fd = fd;
			snprintf(g_fdtrack[i].path, sizeof(g_fdtrack[i].path), "%s", path);
			return 1;
		}
	}
	return 0;
}

static const char *fdtrack_get(int fd)
{
	for (int i = 0; i < FD_TRACK_MAX; i++)
		if (g_fdtrack[i].fd == fd)
			return g_fdtrack[i].path;
	return NULL;
}

static void fdtrack_del(int fd)
{
	for (int i = 0; i < FD_TRACK_MAX; i++)
		if (g_fdtrack[i].fd == fd)
			g_fdtrack[i].fd = -1;
}

// path is an ancestor (proper prefix) of the magic dir
static int ancestor_of_magic(const char *path)
{
	if (!path || !g_dir[0])
		return 0;
	const size_t len = strlen(path);
	if (len == 0 || strlen(g_dir) <= len)
		return 0;
	if (strncmp(g_dir, path, len) != 0 || g_dir[len] != '/')
		return 0;
	return 1;
}

static void syscall_stall(const char *hook, int fd, const char *name)
{
	char joined[PATH_MAX];
	const char *victim = NULL;
	if (name && name[0] == '/') {
		victim = name;
	} else if (name) {
		const char *base = fdtrack_get(fd);
		if (base) {
			snprintf(joined, sizeof(joined), "%s%s%s", base, base[strlen(base) - 1] == '/' ? "" : "/", name);
			victim = joined;
		}
	}
	if (victim && stall_match(victim)) {
		shim_log(hook, victim);
		do_stall(hook, victim);
	}
}

long syscall(long number, ...)
{
	resolve_all();
	unsigned long a[6] = {0, 0, 0, 0, 0, 0};
	if (number == SYS_openat) {
		va_list ap;
		va_start(ap, number);
		a[0] = va_arg(ap, unsigned long);
		a[1] = va_arg(ap, unsigned long);
		a[2] = va_arg(ap, unsigned long);
		a[3] = va_arg(ap, unsigned long);
		va_end(ap);
		const char *path = (const char *)a[1];
		int flags = (int)(a[2] & 0xffffffffu);
		if (mountinfo_match(path)) {
			shim_log("MOUNTINFO-FAKE-syscall", path);
			return real_syscall(SYS_openat, a[0], g_mntinf, flags & ~(O_CREAT | O_EXCL | O_TRUNC), a[3]);
		}
		if (stall_match(path)) {
			shim_log("syscall-openat", path);
			do_stall("syscall-openat", path);
		}
		const long fd = real_syscall(number, a[0], a[1], a[2], a[3]);
		if (fd >= 0 && ancestor_of_magic(path))
			fdtrack_put((int)fd, path);
		return fd;
	}
	if (number == SYS_newfstatat || number == SYS_getdents64 || number == SYS_close) {
		va_list ap;
		va_start(ap, number);
		for (int i = 0; i < 6; i++)
			a[i] = va_arg(ap, unsigned long);
		va_end(ap);
		const int fd = (int)a[0];
		if (number == SYS_newfstatat)
			syscall_stall("syscall-newfstatat", fd, (const char *)a[1]);
		if (number == SYS_close)
			fdtrack_del(fd);
		return real_syscall(number, a[0], a[1], a[2], a[3], a[4], a[5]);
	}
	{
		va_list ap;
		va_start(ap, number);
		for (int i = 0; i < 6; i++)
			a[i] = va_arg(ap, unsigned long);
		va_end(ap);
	}
	return real_syscall(number, a[0], a[1], a[2], a[3], a[4], a[5]);
}
