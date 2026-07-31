#pragma once
/*
AutoHistory.hpp

Separate storage for plugin panel navigation history.
Decoupled from user bookmarks — owns its own INI file
(auto-history.ini) so it never pollutes a user-visible slot.
Thread-safe: all public methods are internally locked.
*/

#include "Bookmarks.hpp"
#include <vector>
#include <mutex>
#include <string>
#include <cstddef>

class AutoHistory
{
	std::vector<BookmarkEntry> _entries;
	std::string _ini_path;
	mutable std::mutex _mtx;
	bool bChanged = false;

public:
	AutoHistory();
	~AutoHistory();

	// Thread-safe add: deduplicates (same Plugin+PluginFile+Folder),
	// inserts at front, trims to kMaxEntries. Saves immediately.
	void Add(const BookmarkEntry& entry);

private:
	void Load();
	void Save();
	void WriteAll();

	static constexpr size_t kMaxEntries = 50;
};

AutoHistory& GetAutoHistory();
void ShutdownAutoHistory();
