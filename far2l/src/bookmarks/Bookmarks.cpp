/*
Bookmarks.cpp

Folder shortcuts - nested multi-entry slots with hierarchical INI storage.
*/

#include "headers.hpp"

#include "Bookmarks.hpp"
#include "BookmarksLog.hpp"
#include "lang.hpp"        // Msg::ShortcutNone, Msg::BookmarksSlotLarge, Msg::BookmarksDropped
#include "message.hpp"     // Message()
#include "ctrlobj.hpp"     // CtrlObject
#include "filepanels.hpp"  // FilePanels (for CtrlObject->Cp()->ActivePanel)
#include "strmix.hpp"      // StrPrintf helpers
#include "KeyFileHelper.h"
#include <cerrno>
#include <cstdio>
#include <mutex>
#include <cassert>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <exception>
// =============================================================================
// BookmarkEntry
// =============================================================================

bool BookmarkEntry::IsValid() const noexcept
{
	if (Folder.IsEmpty()) return false;
	if (Folder.GetLength() > kMaxFolderLen) return false;
	if (Name.GetLength() > kMaxNameLen) return false;
	if (PluginData.GetLength() > kMaxPluginDataLen) return false;
	// Reject embedded NUL - KeyFileHelper line-based parser would truncate silently.
	// Note: FARString::Contains uses wcschr which always finds the buffer terminator,
	// so we must use wmemchr over the explicit length instead.
	if (wmemchr(Folder.CPtr(), L'\0', Folder.GetLength()) != nullptr) return false;
	if (wmemchr(Name.CPtr(), L'\0', Name.GetLength()) != nullptr) return false;
	if (wmemchr(PluginData.CPtr(), L'\0', PluginData.GetLength()) != nullptr) return false;
	if (wmemchr(Plugin.CPtr(), L'\0', Plugin.GetLength()) != nullptr) return false;
	if (wmemchr(PluginFile.CPtr(), L'\0', PluginFile.GetLength()) != nullptr) return false;
	return true;
}

// Middle-truncate a string to kDisplayMaxLen: keep the first kDisplayFront
// and the last kDisplayBack characters, joined by "...". The budget
// invariant kDisplayFront + 3 + kDisplayBack == kDisplayMaxLen (static_asserted
// in BookmarkEntry) guarantees the result is exactly kDisplayMaxLen when the
// input exceeds it. Returns the input unchanged if it fits.
static FARString MiddleTruncate(const FARString& s)
{
	const size_t len = s.GetLength();
	if (len <= BookmarkEntry::kDisplayMaxLen) return s;
	const wchar_t* str = s.CPtr();
	FARString truncated(str, BookmarkEntry::kDisplayFront);
	truncated += L"...";
	truncated += FARString(str + len - BookmarkEntry::kDisplayBack, BookmarkEntry::kDisplayBack);
	return truncated;
}

FARString BookmarkEntry::DisplayNameFor(const BookmarkEntry& entry)
{
	constexpr size_t kMaxLen = BookmarkEntry::kDisplayMaxLen;

	if (entry.Folder.IsEmpty()) {
		FARString r;
		r = Msg::ShortcutNone;
		return r;
	}

	// Compute the visible Folder portion (middle-truncated to MAX).
	//
	// VMenu's render path (interf.cpp:680-720) interprets a single `&`
	// followed by a letter as a literal highlight marker that hides
	// the `&` and colors the next character — that is the desired
	// behavior for Letter shortcuts the user types in (Name). But
	// Folder is a path; user-typed `&` there has NO shortcut
	// semantics, so any `&` in Folder must be stripped or it will
	// spuriously highlight the next character (e.g. `far2l-maste&r`
	// would highlight `r` and visually corrupt the path). The Name
	// portion is left intact so that user-typed `&X` becomes a
	// working menu shortcut via VMenu::AssignHighlights.
	//
	// Strip `&` BEFORE truncating so the budget arithmetic accounts
	// for the post-strip length, producing a tighter result in one
	// pass instead of the pre-fix triple-truncation.
	FARString folder_visible = entry.Folder;
	ReplaceStrings(folder_visible, L"&", L"", -1);
	folder_visible = MiddleTruncate(folder_visible);

	if (entry.Name.IsEmpty()) {
		return folder_visible;
	}

	// Build the (Name) suffix. Preserve user-typed `&X` here so that
	// the next character becomes a working menu shortcut on the
	// submenu (slot 2 entry 0 example: Name='far2l-de&v' produces a
	// visible `far2l-de(v)` and binds `v` as the accelerator; on the
	// main menu the same text is also fine because VMENU_AUTOHIGHLIGHT
	// is set on both menus per commit 5b8b0f08e).
	const size_t suffix_overhead = entry.Name.GetLength() + 4; // " (X)"
	FARString result;
	if (suffix_overhead >= kMaxLen) {
		// Name alone exceeds budget — drop the suffix and let the
		// safety-net re-truncate below cap the result.
		result = folder_visible;
	} else {
		const size_t folder_budget = kMaxLen - suffix_overhead;
		if (folder_visible.GetLength() > folder_budget) {
			FARString truncated(folder_visible.CPtr(), folder_budget);
			result = truncated;
		} else {
			result = folder_visible;
		}
		result += L" (";
		result += entry.Name;
		result += L")";
	}

	// Safety-net re-truncation: covers the Name-too-long fallback
	// branch AND any user-typed extreme where the un-truncated
	// `folder + (Name)` still overflows kDisplayMaxLen.
	result = MiddleTruncate(result);
	return result;
}

// Strip every '&' from an FARString so user-typed '&' in folder / name
// does not leak as a second HiText marker alongside the slot-digit '&%d'.
FARString StripAmpersands(const FARString& s)
{
	FARString out;
	const size_t len = s.GetLength();
	const wchar_t* p = s.CPtr();
	for (size_t i = 0; i < len; ++i) {
		if (p[i] != L'&') out.Append(p[i]);
	}
	return out;
}

// Format a bookmark row for the Location dialog (ChangeDiskMenu).
FARString LocationBookmarkRow(int Pos, const FARString& display_name, size_t count)
{
	FARString visible = StripAmpersands(display_name);
	if (count > 1) {
		visible.AppendFormat(L" (%zu)", count);
	}
	FARString out;
	out.Format(L"&%d  %ls", Pos, visible.CPtr());
	return out;
}


// =============================================================================

static std::string DefaultBookmarksIniPath()
{
	return InMyConfig("settings/bookmarks.ini");
}

std::string Bookmarks::EntryKeyPrefix(int EntryPos)
{
	return "Shortcut" + std::to_string(EntryPos) + "/";
}

Bookmarks::Bookmarks()
	: _kfh(DefaultBookmarksIniPath(), true), _ini_path(DefaultBookmarksIniPath())
{
	Load();
}

Bookmarks::Bookmarks(const std::string& ini_path)
	: _kfh(ini_path, true), _ini_path(ini_path)
{
	Load();
}

Bookmarks::~Bookmarks()
{
	// Best-effort lazy save on destruction (D1). Failures are logged, not raised.
	if (bChanged && !Save()) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"Bookmarks::~Bookmarks: save failed for %s", _ini_path.c_str());
	}
}

bool Bookmarks::IsLoaded() const noexcept
{
	return _kfh.IsLoaded();
}

bool Bookmarks::IsEmpty() const noexcept
{
	for (int Pos = 0; Pos < 10; ++Pos) {
		if (!_entries[Pos].empty()) return false;
	}
	return true;
}

void Bookmarks::Load()
{
	if (!_kfh.IsLoaded()) {
		// Treat as empty - do NOT overwrite (E15). Caller can still add entries
		// and Save() will write a fresh file at that point.
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"Bookmarks::Load: %s not loadable, treating as empty", _ini_path.c_str());
		return;
	}

	for (int Pos = 0; Pos < 10; ++Pos) {
		LoadSlot(Pos);
	}
}

void Bookmarks::LoadSlot(int Pos)
{
	if (Pos < 0 || Pos >= 10) return;
	const std::string sec = ToDec(Pos);
	if (!_kfh.HasSection(sec)) {
		_entries[Pos].clear();
		return;
	}

	// Try multi-entry layout first: ShortcutM/Folder for M=0,1,2,...
	// Stop at first missing ShortcutM/Folder (D10 gap-aware).
	_entries[Pos].clear();
	for (int m = 0; ; ++m) {
		const std::string prefix = EntryKeyPrefix(m);
		const std::string folder_key = prefix + "Folder";
		if (!_kfh.HasKey(sec, folder_key)) {
			break;
		}
		BookmarkEntry e;
		e.Folder = _kfh.GetString(sec, folder_key, L"");
		e.Name = _kfh.GetString(sec, prefix + "Name", L"");
		e.Plugin = _kfh.GetString(sec, prefix + "Plugin", L"");
		e.PluginFile = _kfh.GetString(sec, prefix + "PluginFile", L"");
		e.PluginData = _kfh.GetString(sec, prefix + "PluginData", L"");
		// Skip obviously corrupt entries (defense against malformed INI).
		if (!e.IsValid()) continue;
		_entries[Pos].push_back(std::move(e));
	}

	// Fall back to legacy flat format: Path/Plugin/PluginFile/PluginData
	// (no Shortcut0/ prefix). We migrate it on Save().
	if (_entries[Pos].empty() && _kfh.HasKey(sec, "Path")) {
		BookmarkEntry e;
		e.Folder = _kfh.GetString(sec, "Path", L"");
		e.Plugin = _kfh.GetString(sec, "Plugin", L"");
		e.PluginFile = _kfh.GetString(sec, "PluginFile", L"");
		e.PluginData = _kfh.GetString(sec, "PluginData", L"");
		// Defense against malformed INI: same invariants as Add().
		if (!e.IsValid()) {
			BookmarksLog::Log(BookmarksLog::Level::Warning,
				"Bookmarks::LoadSlot: legacy entry at [%s] failed validation; skipped",
				sec.c_str());
		} else {
			_entries[Pos].push_back(std::move(e));
		}
	}
}

void Bookmarks::WriteSlot(KeyFileHelper& out, int Pos) const
{
	if (Pos < 0 || Pos >= 10) return;
	const std::string sec = ToDec(Pos);
	out.RemoveSection(sec);
	if (_entries[Pos].empty()) return;

	for (size_t i = 0; i < _entries[Pos].size(); ++i) {
		const BookmarkEntry& e = _entries[Pos][i];
		const std::string prefix = EntryKeyPrefix((int)i);
		out.SetString(sec, prefix + "Folder", e.Folder.CPtr());
		if (!e.Name.IsEmpty())
			out.SetString(sec, prefix + "Name", e.Name.CPtr());
		if (!e.Plugin.IsEmpty())
			out.SetString(sec, prefix + "Plugin", e.Plugin.CPtr());
		if (!e.PluginFile.IsEmpty())
			out.SetString(sec, prefix + "PluginFile", e.PluginFile.CPtr());
		if (!e.PluginData.IsEmpty())
			out.SetString(sec, prefix + "PluginData", e.PluginData.CPtr());
	}
}

bool Bookmarks::Save() noexcept
{
	if (!bChanged) return true;

	// Atomic save (D17): write to .tmp, rename over original.
	// Save is noexcept but allocates (std::string, KeyFileHelper, FARString
	// conversions in WriteSlot) and KeyFileHelper::Save can throw
	// std::runtime_error on filesystem errors. Wrap the body so any
	// exception becomes a logged false return — a noexcept function
	// rethrowing would std::terminate, and Save is called from ~Bookmarks
	// and BookmarksCache::Shutdown where a crash is unacceptable.
	try {
		const std::string tmp_path = _ini_path + ".tmp";
		{
			KeyFileHelper tmp_kfh(tmp_path, false);
			for (int Pos = 0; Pos < 10; ++Pos) {
				WriteSlot(tmp_kfh, Pos);
			}
			if (!tmp_kfh.Save(false)) {
				BookmarksLog::Log(BookmarksLog::Level::Warning,
					"Bookmarks::Save: tmp save failed (errno=%d) for %s",
					errno, _ini_path.c_str());
				::unlink(tmp_path.c_str());
				return false;
			}
		}

		if (::rename(tmp_path.c_str(), _ini_path.c_str()) != 0) {
			const int err = errno;
			BookmarksLog::Log(BookmarksLog::Level::Error,
				"Bookmarks::Save: rename %s -> %s failed (errno=%d)",
				tmp_path.c_str(), _ini_path.c_str(), err);
			::unlink(tmp_path.c_str());
			return false;
		}

		bChanged = false;
		return true;
	} catch (const std::exception& ex) {
		BookmarksLog::Log(BookmarksLog::Level::Error,
			"Bookmarks::Save: exception '%s' for %s", ex.what(), _ini_path.c_str());
		return false;
	}
}

bool Bookmarks::Set(int Pos, const FARString *path,
	const FARString *plugin, const FARString *plugin_file, const FARString *plugin_data)
{
	if (Pos < 0 || Pos >= 10) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"Bookmarks::Set: out-of-range Pos=%d", Pos);
		return false;
	}
	if ( (!path || path->IsEmpty()) && (!plugin || plugin->IsEmpty())) {
		return Clear(Pos);
	}

	// D5: replace entry 0 only, preserve entries 1..N.
	BookmarkEntry e;
	e.Folder = path ? *path : FARString();
	e.Plugin = plugin ? *plugin : FARString();
	e.PluginFile = plugin_file ? *plugin_file : FARString();
	e.PluginData = plugin_data ? *plugin_data : FARString();


	if (!e.IsValid()) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"Bookmarks::Set: entry invalid (Pos=%d)", Pos);
		return false;
	}

	if (_entries[Pos].empty()) {
		_entries[Pos].push_back(e);
	} else {
		_entries[Pos][0] = e;
	}
	bChanged = true;
	return true;
}

bool Bookmarks::Get(int Pos, FARString *path,
	FARString *plugin, FARString *plugin_file, FARString *plugin_data) const
{
	if (Pos < 0 || Pos >= 10 || _entries[Pos].empty()) {
		if (path) path->Clear();
		if (plugin) plugin->Clear();
		if (plugin_file) plugin_file->Clear();
		if (plugin_data) plugin_data->Clear();
		return false;
	}

	const BookmarkEntry& e = _entries[Pos][0];

	if (path) {
		FARString strFolder = e.Folder;
		if (!strFolder.IsEmpty())
			apiExpandEnvironmentStrings(strFolder, *path);
		else
			path->Clear();
	}
	if (plugin)       *plugin       = e.Plugin;
	if (plugin_file)  *plugin_file  = e.PluginFile;
	if (plugin_data)  *plugin_data  = e.PluginData;

	return (!e.Folder.IsEmpty() || !e.Plugin.IsEmpty());
}

bool Bookmarks::Clear(int Pos) noexcept
{
	if (Pos < 0 || Pos >= 10) return false;
	_entries[Pos].clear();
	bChanged = true;
	return true;
}

static void LogInvalidEntry(const char* caller, const BookmarkEntry& entry, int Pos, size_t EntryPos = 0)
{
	if (entry.Folder.IsEmpty()) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"%s: Folder empty (Pos=%d, EntryPos=%zu)", caller, Pos, EntryPos);
	} else if (entry.Folder.GetLength() > BookmarkEntry::kMaxFolderLen) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"%s: Folder too long (len=%zu, max=%zu, Pos=%d, EntryPos=%zu)",
			caller, entry.Folder.GetLength(), BookmarkEntry::kMaxFolderLen, Pos, EntryPos);
	} else if (entry.Name.GetLength() > BookmarkEntry::kMaxNameLen) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"%s: Name too long (len=%zu, max=%zu, Pos=%d, EntryPos=%zu)",
			caller, entry.Name.GetLength(), BookmarkEntry::kMaxNameLen, Pos, EntryPos);
	} else if (entry.PluginData.GetLength() > BookmarkEntry::kMaxPluginDataLen) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"%s: PluginData too long (len=%zu, max=%zu, Pos=%d, EntryPos=%zu)",
			caller, entry.PluginData.GetLength(), BookmarkEntry::kMaxPluginDataLen, Pos, EntryPos);
	} else {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"%s: Folder contains NUL (Pos=%d, EntryPos=%zu)", caller, Pos, EntryPos);
	}
}

// =============================================================================
// List-aware API
// =============================================================================

bool Bookmarks::Add(int Pos, const BookmarkEntry& entry)
{
	if (Pos < 0 || Pos >= 10) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"Bookmarks::Add: out-of-range Pos=%d", Pos);
		return false;
	}
	if (!entry.IsValid()) {
		LogInvalidEntry("Bookmarks::Add", entry, Pos);
		return false;
	}
	_entries[Pos].insert(_entries[Pos].begin(), entry);
	bChanged = true;
	return true;
}

size_t Bookmarks::GetCount(int Pos) const noexcept
{
	if (Pos < 0 || Pos >= 10) return 0;
	return _entries[Pos].size();
}

FARString Bookmarks::GetDisplayName(int Pos) const
{
	if (Pos < 0 || Pos >= 10 || _entries[Pos].empty()) {
		return FARString(Msg::ShortcutNone);
	}
	return BookmarkEntry::DisplayNameFor(_entries[Pos][0]);
}

size_t Bookmarks::Enumerator(int Pos, std::vector<BookmarkEntry>& out) const
{
	out.clear();
	if (Pos < 0 || Pos >= 10) return 0;
	out = _entries[Pos];
	return out.size();
}

bool Bookmarks::GetEntry(int Pos, size_t EntryPos, BookmarkEntry& out) const noexcept
{
	if (Pos < 0 || Pos >= 10) return false;
	if (EntryPos >= _entries[Pos].size()) return false;
	// BookmarkEntry copy-assigns five FARStrings, each of which allocates
	// and can throw std::bad_alloc. GetEntry is noexcept and is called
	// from the panel-hotkey path — swallow allocation failure as "not
	// found" so a transient OOM does not std::terminate far2l.
	try {
		out = _entries[Pos][EntryPos];
		return true;
	} catch (const std::exception&) {
		return false;
	}
}


bool Bookmarks::ReplaceAt(int Pos, size_t EntryPos, const BookmarkEntry& entry)
{
	if (Pos < 0 || Pos >= 10) return false;
	if (EntryPos >= _entries[Pos].size()) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"Bookmarks::ReplaceAt: EntryPos=%zu out of range (size=%zu)",
			EntryPos, _entries[Pos].size());
		return false;
	}
	if (!entry.IsValid()) {
		LogInvalidEntry("Bookmarks::ReplaceAt", entry, Pos, EntryPos);
		return false;
	}
	_entries[Pos][EntryPos] = entry;
	bChanged = true;
	return true;
}
size_t Bookmarks::RemoveAt(int Pos, size_t EntryPos)
{
	if (Pos < 0 || Pos >= 10) return 0;
	if (EntryPos >= _entries[Pos].size()) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"Bookmarks::RemoveAt: EntryPos=%zu out of range (size=%zu)",
			EntryPos, _entries[Pos].size());
		return 0;
	}
	_entries[Pos].erase(_entries[Pos].begin() + EntryPos);
	bChanged = true;
	return 1;
}

void Bookmarks::SetAll(int Pos, const std::vector<BookmarkEntry>& entries)
{
	if (Pos < 0 || Pos >= 10) return;
	_entries[Pos] = entries;
	bChanged = true;
}

bool Bookmarks::DropLegacyIndices(int max_index, std::vector<int>& dropped_out)
{
	dropped_out.clear();
	if (max_index < 10) max_index = 10;
	for (int Pos = max_index; Pos < max_index + 100; ++Pos) {
		const std::string sec = ToDec(Pos);
		if (_kfh.HasSection(sec)) {
			dropped_out.push_back(Pos);
			_kfh.RemoveSection(sec);
		}
	}
	if (!dropped_out.empty()) {
		bChanged = true;
	}
	return !dropped_out.empty();
}

// =============================================================================
// All mutable session state lives inside the Bookmarks object itself;
// BookmarksCache is a stateless static facade; the ownership pointer
// is module-local (no class-level statics).
// =============================================================================
namespace {
struct CacheState
{
	std::unique_ptr<Bookmarks> instance;
	std::mutex mutex;
	bool shutdown = false;
};

CacheState& Cache()
{
	static CacheState state;
	return state;
}
} // namespace

Bookmarks& BookmarksCache::Instance()
{
	// Contract: Instance() must not be called after Shutdown().
	// The mutex protects the lifecycle (Instance/Shutdown/ResetForTest)
	// against teardown ordering races. All other BookmarksCache
	// forwarding methods and the deferred-state accessors
	// (WarnedSlots, DeferredBackupPath, DeferredDroppedSlots,
	// MigratedFlag) are UI-thread-only — far2l's console UI is
	// single-threaded, so no additional synchronization is needed
	// for the in-memory Bookmarks state they touch.
	CacheState& state = Cache();
	std::scoped_lock lock(state.mutex);
	assert(!state.shutdown && "BookmarksCache::Instance() called after Shutdown()");
	if (!state.instance) {
		state.instance.reset(new Bookmarks());
	}
	return *state.instance;
}

void BookmarksCache::Shutdown() noexcept
{
	CacheState& state = Cache();
	std::scoped_lock lock(state.mutex);
	if (state.instance) {
		if (!state.instance->Save()) {
			BookmarksLog::Log(BookmarksLog::Level::Warning,
				"BookmarksCache::Shutdown: save failed");
		}
		state.instance.reset();
	}
	state.shutdown = true;
}

#if defined(FAR2L_TESTING)
void BookmarksCache::ResetForTest(std::unique_ptr<Bookmarks> injected) noexcept
{
	CacheState& state = Cache();
	std::scoped_lock lock(state.mutex);
	state.instance = std::move(injected);
	state.shutdown = false;
}
#endif
// ---- Forwarding statics (thin wrappers, no state of their own) ----

bool BookmarksCache::Add(int Pos, const BookmarkEntry& entry)
{
	Bookmarks& b = Instance();
	if (!b.Add(Pos, entry)) {
		return false;
	}
	// D2: soft warning per slot, once per session.
	if (b.GetCount(Pos) >= Bookmarks::kSlotWarnEntries && b.WarnedSlots().insert(Pos).second) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"BookmarksCache::Add: slot %d reached %zu entries",
			Pos, Bookmarks::kSlotWarnEntries);
		if (CtrlObject && CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel) {
			Message(MSG_WARNING, 1, Msg::BookmarksSlotLarge, Msg::Ok);
		}
	}
	return true;
}

size_t BookmarksCache::GetCount(int Pos) noexcept
{
	return Instance().GetCount(Pos);
}

bool BookmarksCache::IsEmpty() noexcept
{
	return Instance().IsEmpty();
}

FARString BookmarksCache::GetDisplayName(int Pos)
{
	return Instance().GetDisplayName(Pos);
}

size_t BookmarksCache::Enumerator(int Pos, std::vector<BookmarkEntry>& out)
{
	return Instance().Enumerator(Pos, out);
}

bool BookmarksCache::GetEntry(int Pos, size_t EntryPos, BookmarkEntry& out) noexcept
{
	return Instance().GetEntry(Pos, EntryPos, out);
}

bool BookmarksCache::ReplaceAt(int Pos, size_t EntryPos, const BookmarkEntry& entry)
{
	return Instance().ReplaceAt(Pos, EntryPos, entry);
}

size_t BookmarksCache::RemoveAt(int Pos, size_t EntryPos)
{
	return Instance().RemoveAt(Pos, EntryPos);
}

bool BookmarksCache::Clear(int Pos) noexcept
{
	return Instance().Clear(Pos);
}

void BookmarksCache::SetAll(int Pos, const std::vector<BookmarkEntry>& entries)
{
	Instance().SetAll(Pos, entries);
}

bool BookmarksCache::Save() noexcept
{
	return Instance().Save();
}
// =============================================================================
// Migration: legacy flat bookmarks.ini -> new hierarchical format
// =============================================================================

static bool IsLegacyFlatFormat(const KeyFileHelper& kfh, int Pos) noexcept
{
	const std::string sec = ToDec(Pos);
	if (!kfh.HasSection(sec)) return false;
	return kfh.HasKey(sec, "Path") && !kfh.HasKey(sec, "Shortcut0/Folder");
}


static void MigrateLegacySlot(KeyFileHelper& kfh, int Pos, bool& bNeedsSave)
{
	if (!IsLegacyFlatFormat(kfh, Pos)) return;

	const std::string sec = ToDec(Pos);
	const std::string path_val = kfh.GetString(sec, "Path", "");
	if (!path_val.empty()) {
		kfh.SetString(sec, "Shortcut0/Folder", path_val.c_str());
		if (kfh.HasKey(sec, "Plugin")) {
			kfh.SetString(sec, "Shortcut0/Plugin",
				kfh.GetString(sec, "Plugin", L"").c_str());
			kfh.RemoveKey(sec, "Plugin");
		}
		if (kfh.HasKey(sec, "PluginFile")) {
			kfh.SetString(sec, "Shortcut0/PluginFile",
				kfh.GetString(sec, "PluginFile", L"").c_str());
			kfh.RemoveKey(sec, "PluginFile");
		}
		if (kfh.HasKey(sec, "PluginData")) {
			kfh.SetString(sec, "Shortcut0/PluginData",
				kfh.GetString(sec, "PluginData", L"").c_str());
			kfh.RemoveKey(sec, "PluginData");
		}
		// Only remove Path and flag save when an entry was actually
		// migrated. An empty-but-present Path (corrupted slot) leaves
		// nothing to migrate; removing the legacy keys would silently
		// destroy the slot with no Shortcut0/ replacement.
		kfh.RemoveKey(sec, "Path");
		bNeedsSave = true;
	}
}

static void CleanupLegacySlots(KeyFileHelper& kfh, std::vector<int>& dropped, bool& bNeedsSave)
{
	for (int Pos = 10; Pos < 110; ++Pos) {
		const std::string sec = ToDec(Pos);
		if (kfh.HasSection(sec)) {
			dropped.push_back(Pos);
			kfh.RemoveSection(sec);
			bNeedsSave = true;
		}
	}
	if (!dropped.empty()) {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"MigrateIfNeeded: dropped legacy slots %d..%d (new far2l supports only 0..9)",
			dropped.front(), dropped.back());
	}
}

static void BackupIniFile(const std::string& ini_path, std::string& deferred_backup_path)
{
	const std::string backup_path = ini_path + ".backup";
	int src_fd = open(ini_path.c_str(), O_RDONLY | O_CLOEXEC);
	if (src_fd != -1) {
		// Get source size for completeness verification.
		struct stat src_st;
		if (::fstat(src_fd, &src_st) != 0) {
			BookmarksLog::Log(BookmarksLog::Level::Warning,
				"BackupIniFile: fstat failed (errno=%d)", errno);
			close(src_fd);
		} else {
			int dst_fd = open(backup_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
			if (dst_fd != -1) {
				// ReadWritePiece copies only one read chunk (up to 32KB)
				// per call — loop until EOF so the full file is backed
				// up, and verify the total matches the source size to
				// detect short copies (e.g. disk full mid-backup).
				ssize_t total_copied = 0;
				for (;;) {
					ssize_t r = ReadWritePiece(src_fd, dst_fd);
					if (r < 0) { total_copied = -1; break; }
					if (r == 0) break; // EOF
					total_copied += r;
				}
				if (total_copied < 0 || (size_t)total_copied != (size_t)src_st.st_size) {
					BookmarksLog::Log(BookmarksLog::Level::Warning,
						"BackupIniFile: backup incomplete (copied=%zd, expected=%lld, errno=%d)",
						total_copied, (long long)src_st.st_size, errno);
					unlink(backup_path.c_str());
				} else {
					BookmarksLog::Log(BookmarksLog::Level::Info,
						"BackupIniFile: backed up %s (%zd bytes)",
						backup_path.c_str(), total_copied);
					deferred_backup_path = backup_path;
				}
				close(dst_fd);
			} else {
				BookmarksLog::Log(BookmarksLog::Level::Warning,
					"BackupIniFile: could not create backup file (errno=%d)", errno);
			}
			close(src_fd);
		}
	} else {
		BookmarksLog::Log(BookmarksLog::Level::Warning,
			"BackupIniFile: could not open INI for reading (errno=%d)", errno);
	}
}
void Bookmarks::MigrateIfNeeded()
{
	if (_migrated) return;
	_migrated = true;

	if (!_kfh.IsLoaded()) {
		BookmarksLog::Log(BookmarksLog::Level::Info,
			"MigrateIfNeeded: INI not loadable; skipping");
		return;
	}

	bool bNeedsSave = false;
	for (int Pos = 0; Pos < 10; ++Pos) {
		MigrateLegacySlot(_kfh, Pos, bNeedsSave);
	}

	std::vector<int> dropped;
	CleanupLegacySlots(_kfh, dropped, bNeedsSave);
	if (!dropped.empty()) {
		_deferred_dropped_slots.insert(_deferred_dropped_slots.end(),
			dropped.begin(), dropped.end());
	}

	if (bNeedsSave) {
		BackupIniFile(_ini_path, _deferred_backup_path);
	}

	if (bNeedsSave) {
		if (!_kfh.Save(false)) {
			BookmarksLog::Log(BookmarksLog::Level::Warning,
				"MigrateIfNeeded: save failed; will retry next session");
		} else {
			Load();
		}
	}
}

void BookmarksCache::MigrateOnceIfNeeded()
{
	Instance().MigrateIfNeeded();
}

std::vector<int> BookmarksCache::ConsumeDeferredNotifications()
{
	Bookmarks& b = Instance();
	std::vector<int> out;
	out.swap(b.DeferredDroppedSlots());
	return out;
}

std::string BookmarksCache::ConsumeDeferredBackupPath() noexcept
{
	Bookmarks& b = Instance();
	std::string out;
	out.swap(b.DeferredBackupPath());
	return out;
}