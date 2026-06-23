#pragma once
/*
BookmarksLog.hpp

Lightweight thread-safe file logger for the Bookmarks subsystem.
Writes to ~/.config/far2l/bookmarks.log (rotated at 1MB).
In production builds never writes to stderr.
In debug builds also writes to stderr for development convenience.
*/

#include <cstdarg>

class BookmarksLog
{
public:
	enum class Level { Debug, Info, Warning, Error };

	static void Log(Level level, const char* fmt, ...) noexcept
#if defined(__GNUC__)
		__attribute__((format(printf, 2, 3)))
#endif
		;

	// Flush and close the log file. Call at session exit.
	static void Flush();
};
