/*
BookmarksLog.cpp

	File-backed logger. Thread-safe via internal mutex.
	NOT async-signal-safe: do not call from signal handlers (localtime_r
	is not in the async-signal-safe POSIX list).
	Writes to ~/.config/far2l/bookmarks.log; rotates when file exceeds 1MB.
*/

#include "headers.hpp"
#include "BookmarksLog.hpp"
#include <cerrno>
#include <mutex>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

#if defined(_DEBUG) || !defined(NDEBUG)
#define BOOKMARKS_LOG_TO_STDERR 1
#else
#define BOOKMARKS_LOG_TO_STDERR 0
#endif

namespace {
constexpr size_t kRotateBytes = 1024 * 1024; // 1MB
const char* kLogFileName = "bookmarks.log";
const char* kLogFileOldName = "bookmarks.log.1";

struct LogState
{
	std::mutex mutex;
	FILE* file = nullptr;
};

LogState& LoggerState()
{
	static LogState state;
	return state;
}

const char* LevelStr(BookmarksLog::Level level) noexcept
{
	switch (level) {
		case BookmarksLog::Level::Debug:   return "DEBUG";
		case BookmarksLog::Level::Info:    return "INFO";
		case BookmarksLog::Level::Warning: return "WARN";
		case BookmarksLog::Level::Error:   return "ERROR";
	}
	return "INFO";
}

std::string LogPath()
{
	return InMyConfig(kLogFileName);
}

// Rotate bookmarks.log -> bookmarks.log.1 if over 1MB, then close and
// reopen so the new file starts fresh. Must be called with the
// LoggerState() mutex held.
void MaybeRotateAndReopen(const std::string& path, LogState& state)
{
	if (!state.file) return;
	struct stat st;
	if (::stat(path.c_str(), &st) != 0) {
		std::fclose(state.file);
		state.file = nullptr;
		return;
	}
	if ((size_t)st.st_size < kRotateBytes) return;

	std::fclose(state.file);
	state.file = nullptr;

	std::string old_path = InMyConfig(kLogFileOldName);
	// rename atomically replaces an existing destination on POSIX — no
	// need to unlink first (the pre-unlink created a loss window where
	// a crash between unlink and rename left no rotated log at all).
	(void)::rename(path.c_str(), old_path.c_str());
}

// Open the log file if not already open. Must be called with the
// LoggerState() mutex held.
void EnsureOpen(const std::string& path, LogState& state)
{
	if (state.file) return;
	state.file = std::fopen(path.c_str(), "a");
}

} // namespace

void BookmarksLog::Log(Level level, const char* fmt, ...) noexcept
{
	if (!fmt) return;

	va_list ap;
	va_start(ap, fmt);

	// Logging must never throw — it is called from noexcept contexts
	// (Bookmarks::Save, BookmarksCache::Shutdown, validation-failure
	// paths in Add/ReplaceAt/RemoveAt). Swallow any exception
	// (std::bad_alloc from std::string, std::system_error from
	// lock_guard) so the caller's noexcept contract holds.
	try {
		LogState& state = LoggerState();
		std::lock_guard<std::mutex> lock(state.mutex);

#if BOOKMARKS_LOG_TO_STDERR
		{
			va_list ap2;
			va_copy(ap2, ap);
			std::fprintf(stderr, "[bookmarks] %s: ", LevelStr(level));
			std::vfprintf(stderr, fmt, ap2);
			std::fputc('\n', stderr);
			va_end(ap2);
		}
#endif

		const std::string path = LogPath();
		MaybeRotateAndReopen(path, state);
		EnsureOpen(path, state);
		if (!state.file) {
			va_end(ap);
			return;
		}

		// Simple ISO-8601-ish timestamp (best effort, no timezone).
		time_t now = ::time(nullptr);
		struct tm tm_buf {};
		::localtime_r(&now, &tm_buf);
		char ts[32];
		std::strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%S", &tm_buf);

		std::fprintf(state.file, "%s [%s] ", ts, LevelStr(level));
		std::vfprintf(state.file, fmt, ap);
		std::fputc('\n', state.file);
		std::fflush(state.file);
	} catch (...) {
		// Swallow — logging failures must not propagate.
	}
	va_end(ap);
}

void BookmarksLog::Flush()
{
	LogState& state = LoggerState();
	std::lock_guard<std::mutex> lock(state.mutex);
	if (state.file) {
		std::fflush(state.file);
		std::fclose(state.file);
		state.file = nullptr;
	}
}
