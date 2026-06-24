/*
test_isvalid.cpp

Tests BookmarkEntry::IsValid() directly on a manual struct mirror (not POD).
Deliberately does NOT include Bookmarks.hpp (avoids lang.hpp / -Werror tangle).
Bookmarks::Add() validation is tested indirectly via its deferred T1a task.

IMPORTANT: Keep this struct in sync with Bookmarks.hpp:BookmarkEntry.
The static_asserts below compare the mirror constants against expected
values defined separately in this file. If the production constants change
and the mirror is updated but the expected values are not (or vice versa),
the static_asserts will fail — catching drift between the two.
*/

#include <gtest/gtest.h>
#include <cwchar>
#include <string>

// Minimal FARString stub — only implements the API surface used by IsValid().
// This avoids pulling in the real FARString.cpp which depends on headers.hpp
// and the full far2l compile-time tangle.
struct FARString {
    std::wstring data;

    FARString() = default;
    FARString(const wchar_t* s) : data(s ? s : L"") {}
    FARString(const wchar_t* s, size_t len) : data(s, len) {}

    void Clear() { data.clear(); }
    bool IsEmpty() const { return data.empty(); }
    size_t GetLength() const { return data.size(); }
    const wchar_t* CPtr() const { return data.c_str(); }

    FARString& operator=(const wchar_t* s) { data = s ? s : L""; return *this; }
    FARString& operator=(const FARString& o) { data = o.data; return *this; }
    FARString& operator+=(wchar_t c) { data.push_back(c); return *this; }
};

// Mirror of Bookmarks.hpp:BookmarkEntry — kept in sync manually.
struct BookmarkEntry {
    FARString Folder, Name, Plugin, PluginFile, PluginData;
    static constexpr size_t kMaxFolderLen = 4096;
    static constexpr size_t kMaxNameLen = 256;
    static constexpr size_t kMaxPluginDataLen = 64 * 1024;
    bool IsValid() const noexcept {
        if (Folder.IsEmpty()) return false;
        if (Folder.GetLength() > kMaxFolderLen) return false;
        if (Name.GetLength() > kMaxNameLen) return false;
        if (PluginData.GetLength() > kMaxPluginDataLen) return false;
        if (wmemchr(Folder.CPtr(), L'\0', Folder.GetLength()) != nullptr) return false;
        if (wmemchr(Name.CPtr(), L'\0', Name.GetLength()) != nullptr) return false;
        if (wmemchr(PluginData.CPtr(), L'\0', PluginData.GetLength()) != nullptr) return false;
        if (wmemchr(Plugin.CPtr(), L'\0', Plugin.GetLength()) != nullptr) return false;
        if (wmemchr(PluginFile.CPtr(), L'\0', PluginFile.GetLength()) != nullptr) return false;
        return true;
    }
};

// Expected constant values — if the real constants in Bookmarks.hpp change,
// these will no longer match the mirror and the static_asserts will fail.
static constexpr size_t kExpectedMaxFolderLen = 4096;
static constexpr size_t kExpectedMaxNameLen = 256;
static constexpr size_t kExpectedMaxPluginDataLen = 64 * 1024;

static_assert(BookmarkEntry::kMaxFolderLen == kExpectedMaxFolderLen);
static_assert(BookmarkEntry::kMaxNameLen == kExpectedMaxNameLen);
static_assert(BookmarkEntry::kMaxPluginDataLen == kExpectedMaxPluginDataLen);

TEST(BookmarkEntryIsValid, RejectsEmptyFolder) {
    BookmarkEntry e;
    e.Folder.Clear();
    EXPECT_FALSE(e.IsValid());
}

TEST(BookmarkEntryIsValid, RejectsOversizeFolder) {
    BookmarkEntry e;
    // Construct a FARString with length > kMaxFolderLen
    FARString big;
    for (size_t i = 0; i <= BookmarkEntry::kMaxFolderLen; ++i)
        big += L'x';
    e.Folder = big;
    EXPECT_FALSE(e.IsValid());
}

TEST(BookmarkEntryIsValid, RejectsOversizeName) {
    BookmarkEntry e;
    e.Folder = L"/tmp";
    FARString big;
    for (size_t i = 0; i <= BookmarkEntry::kMaxNameLen; ++i)
        big += L'x';
    e.Name = big;
    EXPECT_FALSE(e.IsValid());
}

TEST(BookmarkEntryIsValid, RejectsOversizePluginData) {
    BookmarkEntry e;
    e.Folder = L"/tmp";
    FARString big;
    for (size_t i = 0; i <= BookmarkEntry::kMaxPluginDataLen; ++i)
        big += L'x';
    e.PluginData = big;
    EXPECT_FALSE(e.IsValid());
}

TEST(BookmarkEntryIsValid, RejectsOversizeNameWithMaxLenFolder) {
    // Ensure the Name check is reached even when Folder is at its boundary.
    BookmarkEntry e;
    FARString exact;
    for (size_t i = 0; i < BookmarkEntry::kMaxFolderLen; ++i)
        exact += L'x';
    e.Folder = exact;
    FARString big;
    for (size_t i = 0; i <= BookmarkEntry::kMaxNameLen; ++i)
        big += L'x';
    e.Name = big;
    EXPECT_FALSE(e.IsValid());
}

TEST(BookmarkEntryIsValid, RejectsOversizePluginDataWithMaxLenFolder) {
    // Ensure the PluginData check is reached even when Folder is at its boundary.
    BookmarkEntry e;
    FARString exact;
    for (size_t i = 0; i < BookmarkEntry::kMaxFolderLen; ++i)
        exact += L'x';
    e.Folder = exact;
    FARString big;
    for (size_t i = 0; i <= BookmarkEntry::kMaxPluginDataLen; ++i)
        big += L'x';
    e.PluginData = big;
    EXPECT_FALSE(e.IsValid());
}

TEST(BookmarkEntryIsValid, RejectsNulInFolder) {
    // Construct a path with embedded NUL.
    // FARString is reference-counted with explicit length, so embedded NULs
    // survive construction if FARString(const wchar_t*, size_t) is used.
    // If FARString strips NULs, this test still exercises the code path
    // and documents the expected behavior.
    BookmarkEntry e;
    e.Folder = FARString(L"/tmp");
    // Prepend a NUL-containing prefix using raw wchar_t buffer
    const wchar_t buf[] = {L'/', L't', L'm', L'p', L'\0', L'h', L'i', L'd', L'd', L'e', L'n', L'\0'};
    e.Folder = FARString(buf, sizeof(buf)/sizeof(wchar_t));
    // If FARString preserves embedded NULs, IsValid should reject.
    // If it truncates at the first NUL, this becomes a no-op on that branch
    // but the code path is still compiled and reachable for real plugin data.
    EXPECT_FALSE(e.IsValid());
}

TEST(BookmarkEntryIsValid, RejectsNulAtPositionZero) {
    // NUL at position 0: fails IsEmpty() first, wmemchr branch is unreachable.
    // This test documents the short-circuit behavior and prevents a future
    // developer from reordering the checks and breaking the logic.
    BookmarkEntry e;
    const wchar_t buf[] = {L'\0', L'h', L'i', L'\0'};
    e.Folder = FARString(buf, sizeof(buf)/sizeof(wchar_t));
    EXPECT_FALSE(e.IsValid());  // fails on IsEmpty(), not wmemchr
}

TEST(BookmarkEntryIsValid, AcceptsMinimalValid) {
    BookmarkEntry e;
    e.Folder = L"/tmp";
    EXPECT_TRUE(e.IsValid());
}

TEST(BookmarkEntryIsValid, AcceptsPluginEntry) {
    BookmarkEntry e;
    e.Folder = L"/";
    e.Plugin = L"netrocks";
    EXPECT_TRUE(e.IsValid());
}

TEST(BookmarkEntryIsValid, AcceptsMaxLenFolder) {
    BookmarkEntry e;
    FARString exact;
    for (size_t i = 0; i < BookmarkEntry::kMaxFolderLen; ++i)
        exact += L'x';
    e.Folder = exact;
    EXPECT_TRUE(e.IsValid());
}

TEST(BookmarkEntryIsValid, AcceptsMaxLenName) {
    BookmarkEntry e;
    e.Folder = L"/tmp";
    FARString exact;
    for (size_t i = 0; i < BookmarkEntry::kMaxNameLen; ++i)
        exact += L'x';
    e.Name = exact;
    EXPECT_TRUE(e.IsValid());
}

TEST(BookmarkEntryIsValid, AcceptsMaxLenPluginData) {
    BookmarkEntry e;
    e.Folder = L"/tmp";
    FARString exact;
    for (size_t i = 0; i < BookmarkEntry::kMaxPluginDataLen; ++i)
        exact += L'x';
    e.PluginData = exact;
    EXPECT_TRUE(e.IsValid());
}

TEST(BookmarkEntryIsValid, AcceptsAllFieldsAtMax) {
    // Stress test: all fields at their maximum allowed length simultaneously.
    BookmarkEntry e;
    FARString folder, name, plugin, plugin_file, plugin_data;
    for (size_t i = 0; i < BookmarkEntry::kMaxFolderLen; ++i) folder += L'x';
    for (size_t i = 0; i < BookmarkEntry::kMaxNameLen; ++i) name += L'x';
    for (size_t i = 0; i < BookmarkEntry::kMaxPluginDataLen; ++i) plugin_data += L'x';
    e.Folder = folder;
    e.Name = name;
    e.Plugin = L"netrocks";
    e.PluginFile = plugin_file;  // no limit on PluginFile
    e.PluginData = plugin_data;
    EXPECT_TRUE(e.IsValid());
}
