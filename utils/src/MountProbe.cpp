#include <MountProbe.hpp>

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <vector>

#if defined(__linux__)
# include <sys/syscall.h>

namespace
{
	// Probe children abandoned in D state on timeout; reaped opportunistically
	// so they do not pile up as zombies. Never waitpid(-1) here: it would steal
	// other subsystems' children.
	std::mutex g_abandoned_mutex;
	std::vector<pid_t> g_abandoned_pids;

	void ReapAbandonedProbes()
	{
		std::lock_guard<std::mutex> lock(g_abandoned_mutex);
		for (size_t i = g_abandoned_pids.size(); i-- > 0;) {
			int status = 0;
			if (waitpid(g_abandoned_pids[i], &status, WNOHANG) == g_abandoned_pids[i])
				g_abandoned_pids.erase(g_abandoned_pids.begin() + i);
		}
	}

	void RecordAbandonedProbe(pid_t pid)
	{
		std::lock_guard<std::mutex> lock(g_abandoned_mutex);
		// uncapped: an entry lives until its child is reaped; dropping one
		// leaks that child as a zombie when it eventually exits
		g_abandoned_pids.push_back(pid);
	}

	// Memoized verdicts per (path, timeout), both outcomes plus probe errno; a
	// stalled target refuses instantly on retries, a slow server re-probes on expiry.
	struct ProbeCacheEntry {
		std::string path;
		unsigned timeout_ms;
		std::chrono::steady_clock::time_point tested_at;
		bool responsive;
		int err;
	};
	std::mutex g_probe_cache_mutex;
	std::vector<ProbeCacheEntry> g_probe_cache;
	constexpr size_t PROBE_CACHE_MAX = 32;
	constexpr unsigned PROBE_CACHE_TTL_MS = 10000;

	bool ProbeCacheLookup(const char *path, unsigned timeout_ms,
			const std::chrono::steady_clock::time_point &now, bool &responsive, int &err)
	{
		std::lock_guard<std::mutex> lock(g_probe_cache_mutex);
		for (const auto &e : g_probe_cache) {
			if (e.timeout_ms == timeout_ms && e.path == path
					&& now - e.tested_at < std::chrono::milliseconds(PROBE_CACHE_TTL_MS)) {
				responsive = e.responsive;
				err = e.err;
				return true;
			}
		}
		return false;
	}

	void ProbeCacheStore(const char *path, unsigned timeout_ms,
			const std::chrono::steady_clock::time_point &now, bool responsive, int err)
	{
		std::lock_guard<std::mutex> lock(g_probe_cache_mutex);
		for (auto &e : g_probe_cache) {
			if (e.timeout_ms == timeout_ms && e.path == path) {
				e.tested_at = now;
				e.responsive = responsive;
				e.err = err;
				return;
			}
		}
		if (g_probe_cache.size() >= PROBE_CACHE_MAX)
			g_probe_cache.erase(g_probe_cache.begin());
		g_probe_cache.push_back(ProbeCacheEntry{path, timeout_ms, now, responsive, err});
	}
}
#endif

namespace
{
#if defined(__linux__)
	static std::string UnescapeMountInfo(const std::string &str)
	{
		std::string out;
		out.reserve(str.size());
		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '\\' && i + 3 < str.size()
					&& str[i + 1] >= '0' && str[i + 1] <= '3'
					&& str[i + 2] >= '0' && str[i + 2] <= '7'
					&& str[i + 3] >= '0' && str[i + 3] <= '7') {
				out.push_back((char)((str[i + 1] - '0') * 64 + (str[i + 2] - '0') * 8 + (str[i + 3] - '0')));
				i+= 3;
			} else {
				out.push_back(str[i]);
			}
		}
		return out;
	}
#endif

#if defined(__linux__)
	static long long MonoMillis()
	{
		struct timespec ts;
		syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts);
		return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	}
#endif

	static bool HasFstypePrefix(const std::string &fstype, const char *const *prefixes, size_t count)
	{
		for (size_t i = 0; i < count; ++i) {
			if (strncmp(fstype.c_str(), prefixes[i], strlen(prefixes[i])) == 0)
				return true;
		}
		return false;
	}
}

std::string MountFsType(const std::string &path)
{
#if defined(__linux__)
	std::ifstream is("/proc/self/mountinfo");
	std::string line, best_fs;
	size_t best_len = 0;
	while (std::getline(is, line)) {
		const size_t sep = line.find(" - ");
		if (sep == std::string::npos)
			continue;

		std::vector<std::string> fields;
		size_t pos = 0;
		while (pos < sep) {
			size_t next = line.find(' ', pos);
			if (next == std::string::npos || next > sep)
				next = sep;
			fields.emplace_back(line.substr(pos, next - pos));
			pos = next + 1;
		}
		if (fields.size() < 5)
			continue;

		const std::string mnt = UnescapeMountInfo(fields[4]);
		const bool is_prefix = mnt == "/" || mnt == path
			|| (path.size() > mnt.size()
				&& path.compare(0, mnt.size(), mnt) == 0
				&& path[mnt.size()] == '/');
		if (!is_prefix || mnt.size() < best_len)
			continue;

		const size_t fs_begin = sep + 3;
		const size_t fs_end = line.find(' ', fs_begin);
		best_fs = line.substr(fs_begin,
			fs_end == std::string::npos ? std::string::npos : fs_end - fs_begin);
		best_len = mnt.size();
	}
	return best_fs;
#else
	(void)path;
	return "";
#endif
}

bool IsSharedFolderFs(const std::string &fstype)
{
	if (fstype.empty())
		return false;
	static const char *const fstypes[] = { "vboxsf", "prl_fs", "vmhgfs", "virtiofs" };
	return HasFstypePrefix(fstype, fstypes, sizeof(fstypes) / sizeof(fstypes[0]));
}

bool IsNetworkFs(const std::string &fstype)
{
	if (fstype.empty())
		return false;
	// prefixes: nfs covers nfs4, smb covers smb2/smb3
	static const char *const fstypes[] = {
		"cifs", "smb", "nfs", "9p", "sshfs", "fuse.sshfs", "curlftpfs", "davfs", "afs"
	};
	return HasFstypePrefix(fstype, fstypes, sizeof(fstypes) / sizeof(fstypes[0]));
}

bool NeedsMountProbe(const std::string &fstype)
{
	return IsSharedFolderFs(fstype) || IsNetworkFs(fstype);
}

bool MountResponsive(const char *path, unsigned timeout_ms)
{
#if defined(__linux__)
	ReapAbandonedProbes();

	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	bool responsive = true;
	int probe_errno = 0;
	if (ProbeCacheLookup(path, timeout_ms, now, responsive, probe_errno)) {
		if (!responsive)
			errno = probe_errno;
		return responsive;
	}
	const unsigned requested_timeout_ms = timeout_ms;	// the loop burns the local copy
	bool timed_out = false;

	// Raw clone, not fork(): bypasses pthread_atfork machinery in this
	// multithreaded process; the child runs raw syscalls only and exits via _exit().
	const pid_t pid = (pid_t)syscall(SYS_clone, SIGCHLD, 0, 0, 0, 0);
	if (pid < 0) {
		// fail open, but make the disabled mitigation observable once
		static std::atomic<bool> s_clone_failure_warned{false};
		if (!s_clone_failure_warned.exchange(true))
			fprintf(stderr, "MountProbe: SYS_clone failed (errno %u) - stall protection disabled\n",
				(unsigned)errno);
		return true;	// cannot probe; never refuse an operation because of that
	}

	if (pid == 0) {
		// Exercise what a folder change does: open, readdir, stat entries - a
		// partially stalled mount answers some lookups but blocks others. The
		// scan self-limits to half the timeout; a stalled mount never exits.
		char dents[1 << 16];
		const unsigned scan_budget_ms = requested_timeout_ms / 2;
		const long long started_ms = MonoMillis();
		const long fd = syscall(SYS_openat, AT_FDCWD, path,
			O_RDONLY | O_DIRECTORY | O_NONBLOCK | O_CLOEXEC, 0);
		if (fd >= 0) {
			for (;;) {
				const long n = syscall(SYS_getdents64, fd, dents, sizeof(dents));
				if (n <= 0)
					break;
				struct stat st;
				char *p = dents, *end = dents + n;
				while (p + 18 < end) {
					const unsigned short reclen = *(const unsigned short *)(p + 16);
					if (reclen <= 0 || p + reclen > end)
						break;
					const char *name = p + 19;
					if (name[0] != '.' || (name[1] != '\0'
						&& (name[1] != '.' || name[2] != '\0'))) {
						(void)syscall(SYS_newfstatat, fd, name, &st, 0);
						if (MonoMillis() - started_ms >= (long long)scan_budget_ms)
							_exit(0);	// answered all asked so far
					}
					p += reclen;
				}
			}
			(void)syscall(SYS_close, fd);
		}
		_exit(0);	// any completion means the filesystem answered
	}

	for (;;) {
		int status = 0;
		const pid_t r = waitpid(pid, &status, WNOHANG);
		if (r == pid) {
			// fail closed on a signal-killed child: a crashing filesystem kills
			// its probes; reading that as "responsive" would re-run the crash inline
			responsive = WIFEXITED(status) && WEXITSTATUS(status) == 0;
			break;
		}
		if (r < 0 && errno != EINTR) {
			responsive = true;	// ECHILD etc.: nothing left to wait for
			break;
		}
		if (timeout_ms <= 10) {
			// never waitpid() a D-state child (would block with it); record for reaping
			RecordAbandonedProbe(pid);
			responsive = false;
			timed_out = true;
			break;
		}
		usleep(10000);
		timeout_ms-= 10;
	}

	if (!responsive)
		probe_errno = timed_out ? ETIMEDOUT : EIO;	// name the stall, not a stale errno
	ProbeCacheStore(path, requested_timeout_ms, now, responsive, probe_errno);
	if (!responsive)
		errno = probe_errno;
	return responsive;
#else
	(void)path;
	(void)timeout_ms;
	return true;
#endif
}
