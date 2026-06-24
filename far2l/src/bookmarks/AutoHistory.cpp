/*
AutoHistory.cpp

File-backed plugin panel navigation history.
Separate from bookmarks.ini — writes to auto-history.ini in the
same settings/ directory. Never touches a user-visible slot.
*/

#include "headers.hpp"

#include "AutoHistory.hpp"
#include "BookmarksLog.hpp"
#include <cerrno>
#include <cstdio>
#include <unistd.h>

static std::string DefaultAutoHistoryIniPath()
{
	return InMyConfig("settings/auto-history.ini");
}

// =============================================================================
// Lifetime — encapsulated in a Meyers singleton so the mutable state is not
// a module-level global (avoids test pollution and teardown hazards).
// =============================================================================

namespace {
struct HistoryState
{
	std::unique_ptr<AutoHistory> instance;
	std::mutex mutex;
};

HistoryState& HistoryCache()
{
	static HistoryState state;
	return state;
}
} // namespace

AutoHistory& GetAutoHistory()
{
	HistoryState& state = HistoryCache();
	std::scoped_lock lock(state.mutex);
	if (!state.instance) {
		state.instance = std::make_unique<AutoHistory>();
	}
	return *state.instance;
}

void ShutdownAutoHistory()
{
	HistoryState& state = HistoryCache();
	std::scoped_lock lock(state.mutex);
	state.instance.reset(); // destructor calls Save()
}
// =============================================================================
// Lifecycle
// =============================================================================

AutoHistory::AutoHistory()
	: _ini_path(DefaultAutoHistoryIniPath())
{
	Load();
}

AutoHistory::~AutoHistory()
{
	// Save() → WriteAll() allocates and KeyFileHelper::Save can throw
	// std::runtime_error on filesystem errors. A throwing destructor
	// during normal exit would std::terminate — swallow and log.
	try {
		Save();
	} catch (const std::exception& ex) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"AutoHistory::~AutoHistory: save failed: %s", ex.what());
	}
}


// =============================================================================
// Load / Save
// =============================================================================

void AutoHistory::Load()
{
	std::scoped_lock lock(_mtx);

	KeyFileHelper kfh(_ini_path, true);
	if (!kfh.IsLoaded()) {
		BookmarksLog::Log(BookmarksLog::Level::Info,
			"AutoHistory::Load: %s not loadable, starting empty", _ini_path.c_str());
		return;
	}

	const std::string sec = "History";
	_entries.clear();
	for (int m = 0; ; ++m) {
		const std::string prefix = "Entry" + std::to_string(m) + "/";
		const std::string folder_key = prefix + "Folder";
		if (!kfh.HasKey(sec, folder_key)) break;

		BookmarkEntry e;
		e.Folder = kfh.GetString(sec, folder_key, L"");
		e.Name = kfh.GetString(sec, prefix + "Name", L"");
		e.Plugin = kfh.GetString(sec, prefix + "Plugin", L"");
		e.PluginFile = kfh.GetString(sec, prefix + "PluginFile", L"");
		e.PluginData = kfh.GetString(sec, prefix + "PluginData", L"");
		if (!e.IsValid()) continue;
		_entries.push_back(std::move(e));
	}

	BookmarksLog::Log(BookmarksLog::Level::Debug,
		"AutoHistory::Load: loaded %zu entries from %s", _entries.size(), _ini_path.c_str());
}

void AutoHistory::WriteAll()
{
	// Must be called with _mtx held.
	// Wrap the body: KeyFileHelper::Save can throw std::runtime_error on
	// filesystem errors (EROFS, ENOSPC) and the string/key operations
	// allocate. Swallow so Save() never propagates to the destructor.
	try {
		const std::string sec = "History";

		const std::string tmp_path = _ini_path + ".tmp";
		{
			KeyFileHelper tmp_kfh(tmp_path, false);
			tmp_kfh.RemoveSection(sec);

			for (size_t i = 0; i < _entries.size(); ++i) {
				const std::string prefix = "Entry" + std::to_string(i) + "/";
				const BookmarkEntry& e = _entries[i];

				tmp_kfh.SetString(sec, prefix + "Folder", e.Folder.CPtr());
				if (!e.Name.IsEmpty())
					tmp_kfh.SetString(sec, prefix + "Name", e.Name.CPtr());
				if (!e.Plugin.IsEmpty())
					tmp_kfh.SetString(sec, prefix + "Plugin", e.Plugin.CPtr());
				if (!e.PluginFile.IsEmpty())
					tmp_kfh.SetString(sec, prefix + "PluginFile", e.PluginFile.CPtr());
				if (!e.PluginData.IsEmpty())
					tmp_kfh.SetString(sec, prefix + "PluginData", e.PluginData.CPtr());
			}

			if (!tmp_kfh.Save(false)) {
				BookmarksLog::Log(BookmarksLog::Level::Warning,
					"AutoHistory::WriteAll: tmp save failed (errno=%d) for %s",
					errno, _ini_path.c_str());
				::unlink(tmp_path.c_str());
				return;
			}
		}

		if (::rename(tmp_path.c_str(), _ini_path.c_str()) != 0) {
			const int err = errno;
			BookmarksLog::Log(BookmarksLog::Level::Error,
				"AutoHistory::WriteAll: rename failed (errno=%d)", err);
			::unlink(tmp_path.c_str());
			return;
		}

		bChanged = false;
	} catch (const std::exception& ex) {
		BookmarksLog::Log(BookmarksLog::Level::Error,
			"AutoHistory::WriteAll: exception '%s' for %s", ex.what(), _ini_path.c_str());
	}
}

void AutoHistory::Save()
{
	std::scoped_lock lock(_mtx);
	if (!bChanged) return;
	WriteAll();
}

// =============================================================================
// Public API
// =============================================================================

void AutoHistory::Add(const BookmarkEntry& entry)
{
	std::scoped_lock lock(_mtx);

	if (!entry.IsValid()) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"AutoHistory::Add: entry invalid");
		return;
	}

	// Deduplicate: remove existing entry with same Plugin+PluginFile+Folder.
	for (size_t i = 0; i < _entries.size(); ++i) {
		if (_entries[i].Plugin == entry.Plugin
			&& _entries[i].PluginFile == entry.PluginFile
			&& _entries[i].Folder == entry.Folder) {
			_entries.erase(_entries.begin() + i);
			break;
		}
	}

	// Insert at front (most-recently-visited first).
	_entries.insert(_entries.begin(), entry);

	// Trim to kMaxEntries.
	while (_entries.size() > kMaxEntries) {
		_entries.pop_back();
	}

	bChanged = true;
}
