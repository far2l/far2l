#pragma once
#include <KeyFileHelper.h>
#include "FARString.hpp"
#include <vector>
#include <memory>
#include <set>
#include <cstddef>
#include <mutex>
void CheckForImportLegacyShortcuts();

struct BookmarkEntry
{
	FARString Folder;
	FARString Name;
	FARString Plugin;
	FARString PluginFile;
	FARString PluginData;

	static constexpr size_t kMaxFolderLen = 4096;
	static constexpr size_t kMaxNameLen = 256;
	static constexpr size_t kMaxPluginDataLen = 64 * 1024;

	// Display-format constants used by DisplayNameFor(). Hoisted to the
	// header so unit tests can static_assert them and catch drift.
	// FRONT + 3 ('…') + BACK == MAX guarantees the safety-net branch
	// produces a string of exactly MAX chars.
	static constexpr size_t kDisplayMaxLen = 36;
	static constexpr size_t kDisplayFront  = 15;
	static constexpr size_t kDisplayBack   = 18;
	static_assert(kDisplayFront + 3 + kDisplayBack == kDisplayMaxLen,
		"DisplayNameFor budget arithmetic depends on this invariant");

	bool IsValid() const noexcept;

	static FARString DisplayNameFor(const BookmarkEntry& entry);
};
// Strip every '&' from an FARString. Used wherever a row that already
// carries an '&X' accelerator (slot-digit) is composed, so user-typed
// '&' in folder / name does not leak as a second HiText marker.
[[nodiscard]] FARString StripAmpersands(const FARString& s);

// Format a bookmark row for the Location dialog (ChangeDiskMenu). The row
// carries its own '&%d' slot-digit accelerator; user-typed '&' in the
// display name MUST be stripped or it would be processed verbatim by
// HiText as a second marker. The count suffix " (N)" is appended when the
// slot holds multiple entries so the user can see the size at a glance.
[[nodiscard]] FARString LocationBookmarkRow(int Pos, const FARString& display_name, size_t count);

class Bookmarks
{
	KeyFileHelper _kfh;
	std::string _ini_path;
	std::vector<BookmarkEntry> _entries[10];
	bool bChanged = false;
	std::set<int> _warned_slots;
	std::string _deferred_backup_path;
	std::vector<int> _deferred_dropped_slots;
	bool _migrated = false;

public:
	// Default ctor: production use, opens the user-wide bookmarks.ini
	Bookmarks();
	// Test ctor: explicit path, no ASSERT
	explicit Bookmarks(const std::string& ini_path);

	~Bookmarks();

	// Existing single-entry API - operate on entry 0 of slot.
	// Set() preserves entries 1..N (does not wipe the whole slot).
	[[nodiscard]] bool Set(int Pos, const FARString *path, const FARString *plugin = nullptr,
		const FARString *plugin_file = nullptr, const FARString *plugin_data = nullptr);

	[[nodiscard]] bool Get(int Pos, FARString *path, FARString *plugin = nullptr,
		FARString *plugin_file = nullptr, FARString *plugin_data = nullptr) const;

	[[nodiscard]] bool Clear(int Pos) noexcept;

	// Soft warning threshold per slot, used by BookmarksCache::Add.
	// Centralised here so ADRs / plans can reference one source of truth.
	static constexpr size_t kSlotWarnEntries = 1000;

	// New list-aware API
	[[nodiscard]] bool Add(int Pos, const BookmarkEntry& entry);
	size_t GetCount(int Pos) const noexcept;
	FARString GetDisplayName(int Pos) const;
	size_t Enumerator(int Pos, std::vector<BookmarkEntry>& out) const;
	// Single-entry access by index (avoids copying the whole vector).
	// Returns false if Pos or EntryPos is out of range.
	[[nodiscard]] bool GetEntry(int Pos, size_t EntryPos, BookmarkEntry& out) const noexcept;
	[[nodiscard]] bool ReplaceAt(int Pos, size_t EntryPos, const BookmarkEntry& entry);
	[[nodiscard]] size_t RemoveAt(int Pos, size_t EntryPos);
	void SetAll(int Pos, const std::vector<BookmarkEntry>& entries);
	// Lifecycle
	[[nodiscard]] bool Save() noexcept;
	[[nodiscard]] bool IsChanged() const noexcept { return bChanged; }
	[[nodiscard]] bool IsLoaded() const noexcept;
	[[nodiscard]] bool IsEmpty() const noexcept;

	// Drop legacy "walk-past-10" sections (returns true if anything was dropped)
	[[nodiscard]] bool DropLegacyIndices(int max_index, std::vector<int>& dropped_out);

	// Session-scattered state (was static in BookmarksCache, now per-instance)
	std::set<int>& WarnedSlots() noexcept { return _warned_slots; }
	std::string& DeferredBackupPath() noexcept { return _deferred_backup_path; }
	std::vector<int>& DeferredDroppedSlots() noexcept { return _deferred_dropped_slots; }
	bool& MigratedFlag() noexcept { return _migrated; }

	// One-time legacy format migration. Called by BookmarksCache::MigrateOnceIfNeeded.
	void MigrateIfNeeded();

private:
	void Load();
	void LoadSlot(int Pos);
	void WriteSlot(KeyFileHelper& out, int Pos) const;
	static std::string EntryKeyPrefix(int EntryPos);
	// (No test-only reset hook: ResetForTest constructs a fresh Bookmarks
	// with the desired ini_path via the test ctor.)
};

class BookmarksCache
{
public:
	[[nodiscard]] static Bookmarks& Instance();
	[[nodiscard]] static bool Add(int Pos, const BookmarkEntry& entry);
	static size_t GetCount(int Pos) noexcept;
	[[nodiscard]] static FARString GetDisplayName(int Pos);
	static size_t Enumerator(int Pos, std::vector<BookmarkEntry>& out);
	[[nodiscard]] static bool ReplaceAt(int Pos, size_t EntryPos, const BookmarkEntry& entry);
	[[nodiscard]] static bool GetEntry(int Pos, size_t EntryPos, BookmarkEntry& out) noexcept;
	[[nodiscard]] static size_t RemoveAt(int Pos, size_t EntryPos);
	[[nodiscard]] static bool Clear(int Pos) noexcept;
	static void SetAll(int Pos, const std::vector<BookmarkEntry>& entries);
	[[nodiscard]] static bool Save() noexcept;
	[[nodiscard]] static bool IsEmpty() noexcept;

	enum class GetResult { Ok, Cancelled, Empty };
	static GetResult ResolveForSlot(int Pos, FARString& out_path, FARString& out_plugin,
		FARString& out_plugin_file, FARString& out_plugin_data, int& out_entry_pos);
	static GetResult ResolveForCmdline(int Pos, FARString& out_path, FARString& out_plugin,
		FARString& out_plugin_file, FARString& out_plugin_data);

	static void Shutdown() noexcept;
#if defined(FAR2L_TESTING)
	static void ResetForTest(std::unique_ptr<Bookmarks> injected = nullptr) noexcept;
#endif
	static void MigrateOnceIfNeeded();
	static std::vector<int> ConsumeDeferredNotifications();
	static std::string ConsumeDeferredBackupPath() noexcept;

	BookmarksCache() = delete;
};

void ShowBookmarksMenu(int Pos = 0);
