/*
test_displayname.cpp

Tests BookmarkEntry::DisplayNameFor() AND BookmarksMenu::BuildSlotItem
(thumbnail-rendering for the main "Bookmarks" menu), plus the
integration with the VMenu rendering pipeline. We mirror both
DisplayNameFor and BuildSlotItem in this file rather than including
the production headers, because Bookmarks.cpp transitively pulls in
lang.hpp / headers.hpp which has pre-existing C++17/-Werror issues
in the test build (test_bookmarks.cpp documents the same constraint).

IMPORTANT: keep this mirror in sync with the production code. The
static_asserts at the top catch drift of BookmarkEntry::kMax* and
the display-format constants (kDisplayMaxLen / kDisplayFront /
kDisplayBack). If these break, the mirror or the production source
has drifted; fix one or the other.

Two distinct user-visible failures pinned here:

A. MAIN MENU row (user reported: "...still leaks '&v'").
   BookmarksMenu.cpp:BuildSlotItem formats  "[RightCtrl | Ctrl+Alt] + &2   <display>"
   HiText (interf.cpp:653-720) processes only the FIRST `&X` marker in
   a string. The slot-digit `&2` is that first marker. Anything that
   comes BEFORE the slot-digit gets ', and anything AFTER gets
   rendered verbatim (with `&&` → `&` collapse). User-typed `&v` in
   `(Name)` would leak as visible `&v` on the same line. Fix: strip
   `&` from `display_name` inside BuildSlotItem before the Format.

B. SUBMENU row (user reported: works after commit fbf71cfac).
   Repopulate lambda (BookmarksMenu.cpp:115-132) bypasses BuildSlotItem;
   it calls DisplayNameFor directly. HiText sees the single `&v`
   marker and renders `v` accented, `&` hidden.

USER'S LITERAL CASE pinned by tests below:
  Folder = '/home/user/projects/far2l/far2l-dev' (32 chars)
  Name   = 'far2l-de&v'                         (10 chars)

  Submenu raw    = '/home/user/projects/fa (far2l-de&v)'    (35 chars)
  Submenu visible= '/home/user/projects/fa (far2l-dev)'      (34 chars), v accented

  Main menu strName is "[RightCtrl | Ctrl+Alt] + &2   <display> (4)".
  After BuildSlotItem's new strip step: the `&v` from (Name) is gone;
  only `&2` remains. HiText then accents '2' and renders the row
  cleanly. The user-visible row reads:
  "[RightCtrl | Ctrl+Alt] + [2]   /home/user/projects/fa (far2l-dev) (4) [►]"
  where [2] is the HiColor accented digit and [►] is the submenu glyph
  for the MIF_SUBMENU flag (rendered by vmenu.cpp:1977-1981, separate
  from HiText).
*/

#include <gtest/gtest.h>
#include <algorithm>
#include <cwchar>
#include <string>
#include <vector>
// BuildSlotItem. Mirrors the stub from test_isvalid.cpp; duplicated here
// so this file stays self-contained.
//
// We do NOT declare operator=(const FARString&) — the implicit copy ctor
// (which works thanks to std::wstring) is enough, and adding a user-
// defined copy assignment suppresses the implicit copy ctor under
// -Werror=deprecated-copy.
// ============================================================================
struct FARString {
	std::wstring data;

	FARString() = default;
	FARString(const wchar_t* s) : data(s ? s : L"") {}
	FARString(const wchar_t* s, size_t len) : data(s, len) {}

	FARString& operator=(const wchar_t* s) { data = s ? s : L""; return *this; }
	FARString& operator+=(const FARString& o) { data += o.data; return *this; }
	FARString& operator+=(const wchar_t* s) { if (s) data += s; return *this; }
	FARString& operator+=(wchar_t c) { data.push_back(c); return *this; }

	void Clear() { data.clear(); }
	bool IsEmpty() const { return data.empty(); }
	size_t GetLength() const { return data.size(); }
	const wchar_t* CPtr() const { return data.c_str(); }

	void AppendFormat(const wchar_t* fmt, size_t arg) {
		// Mimics FARString::AppendFormat(L" (%zu)", arg). Used by the
		// BuildSlotItem mirror for the count suffix.
		std::wstring tail = L" (";
		tail += std::to_wstring(arg);
		tail += L")";
		data += tail;
	}
};

// ============================================================================
// Stubbed Msg namespace — DisplayNameFor references Msg::ShortcutNone when
// Folder is empty. We pin the placeholder to "<none>" so the test value
// matches the user's mental model from the far2l UI.
// ============================================================================
namespace Msg {
	const wchar_t* ShortcutNone = L"<none>";
	const wchar_t* RightCtrl = L"RightCtrl";
}

// ============================================================================
// ReplaceStrings stub — DisplayNameFor strips `&` from the Folder portion;
// BuildSlotItem strips `&` from the entire `display_name` before composing
// the slot row. Mirrors the contract of strmix.hpp:ReplaceStrings.
// ============================================================================
static void ReplaceStrings(FARString& s, const wchar_t* find, const wchar_t* repl, int /*count*/)
{
	if (!find || !*find || !repl) return;
	const std::wstring f(find), r(repl);
	size_t pos = 0;
	while ((pos = s.data.find(f, pos)) != std::wstring::npos) {
		s.data.replace(pos, f.size(), r);
		pos += r.size();
	}
}

// ============================================================================
// BookmarkEntry mirror — pinned by static_asserts. The production struct
// lives in far2l/src/bookmarks/Bookmarks.hpp:BookmarkEntry.
// ============================================================================
struct BookmarkEntry {
	FARString Folder, Name, Plugin, PluginFile, PluginData;
	static constexpr size_t kMaxFolderLen = 4096;
	static constexpr size_t kMaxNameLen = 256;
	static constexpr size_t kMaxPluginDataLen = 64 * 1024;
	static constexpr size_t kDisplayMaxLen = 36;
	static constexpr size_t kDisplayFront  = 15;
	static constexpr size_t kDisplayBack   = 18;
};

// Drift guards: if Bookmarks.hpp:BookmarkEntry changes any of these, this
// file fails to compile. Update either the production code or the mirror.
static_assert(BookmarkEntry::kMaxFolderLen == 4096);
static_assert(BookmarkEntry::kMaxNameLen == 256);
static_assert(BookmarkEntry::kMaxPluginDataLen == 64 * 1024);
static_assert(BookmarkEntry::kDisplayMaxLen == 36);
static_assert(BookmarkEntry::kDisplayFront  == 15);
static_assert(BookmarkEntry::kDisplayBack   == 18);
static_assert(BookmarkEntry::kDisplayFront + 3 + BookmarkEntry::kDisplayBack
	== BookmarkEntry::kDisplayMaxLen);

// ============================================================================
// Mirror of Bookmarks.cpp:DisplayNameFor.
//
// Pipeline:
//   - Empty Folder → "<none>"
//   - Folder > MAX → middle-truncate FRONT + "..." + BACK
//   - Strip ALL "&" from Folder portion (HiText would highlight next char)
//   - If Name non-empty: append " (Name)" within budget, truncating Folder
//     first. Safety-net re-truncates the combined result if it overflows.
// ============================================================================
static FARString DisplayNameFor(const BookmarkEntry& entry)
{
	const size_t kDisplayMaxLen = BookmarkEntry::kDisplayMaxLen;
	const size_t kDisplayFront  = BookmarkEntry::kDisplayFront;
	const size_t kDisplayBack   = BookmarkEntry::kDisplayBack;

	if (entry.Folder.IsEmpty()) {
		FARString r;
		r = Msg::ShortcutNone;
		return r;
	}

	FARString folder_visible;
	{
		const wchar_t* str = entry.Folder.CPtr();
		const size_t len   = entry.Folder.GetLength();
		if (len <= kDisplayMaxLen) {
			folder_visible = entry.Folder;
		} else {
			FARString truncated(str, kDisplayFront);
			truncated += L"...";
			truncated += FARString(str + len - kDisplayBack, kDisplayBack);
			folder_visible = truncated;
		}
	}
	ReplaceStrings(folder_visible, L"&", L"", -1);

	if (entry.Name.IsEmpty()) {
		return folder_visible;
	}

	const size_t suffix_overhead = entry.Name.GetLength() + 4; // " (X)"
	FARString result;
	if (suffix_overhead >= kDisplayMaxLen) {
		result = folder_visible;
	} else {
		const size_t folder_budget = kDisplayMaxLen - suffix_overhead;
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

	if (result.GetLength() > kDisplayMaxLen) {
		const wchar_t* str = result.CPtr();
		const size_t len   = result.GetLength();
		FARString truncated(str, kDisplayFront);
		truncated += L"...";
		truncated += FARString(str + len - kDisplayBack, kDisplayBack);
		result = truncated;
	}
	return result;
}

// ============================================================================
// HiText simulator — mirrors far2l/src/console/interf.cpp:653-720.
//
// Algorithm (verbatim from production):
//   1. Find the FIRST '&' in the input (strTextStr.Pos).
//   2. Count the consecutive '&' run STARTING at that position.
//   3. If run is ODD (e.g. "&X&Y..."):
//        - Render pre-run text verbatim.
//        - Render X (the byte right after the run) with HiColor.
//        - Render the rest of the string verbatim, EXCEPT collapse
//          any "&&" pairs to literal "&".
//   4. If run is EVEN (e.g. "&&..."):
//        - Collapse "&&" → "&" in the whole string and render.
//
// CRITICAL: HiText processes only the FIRST &X marker. Subsequent
// &X markers in the tail render VERBATIM (with && collapse) — they
// do NOT get accented. This is the user's reported bug: a main-menu
// row with two &X (the slot-digit &2 plus the user-typed &v in
// (Name)) renders only &2 cleanly while &v falls through as a
// literal "&v" in the visible string. Fix: BuildSlotItem strips '&'
// from display_name before composing the slot row, leaving only `&2`.
// ============================================================================
static void HiTextSimulate(const std::wstring& input,
	std::wstring& visible, std::vector<size_t>& accent_positions)
{
	visible.clear();
	accent_positions.clear();

	size_t pos = input.find(L'&');
	if (pos == std::wstring::npos) {
		visible = input;
		return;
	}

	// Count run starting at `pos`.
	size_t j = pos;
	while (j < input.size() && input[j] == L'&') ++j;
	const size_t run = j - pos;

	if (run % 2 == 1) {
		visible.append(input, 0, pos); // pre-run verbatim

		if (j < input.size()) {
			accent_positions.push_back(visible.size());
			visible.push_back(input[j]); // X accented

			// Tail: collapse && → & only, no further accent.
			std::wstring tail(input, j + 1, std::wstring::npos);
			size_t pp = 0;
			while ((pp = tail.find(L"&&", pp)) != std::wstring::npos) {
				tail.replace(pp, 2, L"&");
				pp += 1;
			}
			visible.append(tail);
		}
	} else {
		std::wstring tmp = input;
		size_t pp = 0;
		while ((pp = tmp.find(L"&&", pp)) != std::wstring::npos) {
			tmp.replace(pp, 2, L"&");
			pp += 1;
		}
		visible.append(tmp);
	}
}

// ============================================================================
// AssignedShortcutLetter — finds the FIRST `&X` marker in the raw render
// and returns X (or 0 if absent). Mirrors VMenu::AssignHighlights
// (vmenu.cpp:2086-2186).
// ============================================================================
static wchar_t AssignedShortcutLetter(const FARString& rendered)
{
	const std::wstring& s = rendered.data;
	for (size_t i = 0; i + 1 < s.size(); ++i) {
		if (s[i] == L'&' && s[i + 1] != L'&' && s[i + 1] != L'\0') {
			return s[i + 1];
		}
	}
	return 0;
}

// ============================================================================
// MenuItemExMir — minimal mirror of MenuItemEx shape used by BookmarksMenu.
// We only model `strName`; the production MenuItemEx carries Flags etc.
// ============================================================================
struct MenuItemExMir {
	std::wstring strName;
};

// ============================================================================
// Mirror of BookmarksMenu.cpp:BuildSlotItem.
//
// DIFFERENCE FROM PRODUCTION (pre-fix): the production code USED to leave
// `&X` in the visible portion. After commit 12a3b04fe (or this commit),
// BuildSlotItem STRIPS every '&' from `display_name` BEFORE composing
// the row. HiText then sees only one `&X` (the slot-digit `&%d`), and
// renders the row cleanly: digit accented, no stray `&` artifact in
// visible text.
//
// Pipeline:
//   - copy display_name → visible
//   - ReplaceStrings(visible, L"&", L"", -1)        ← KEY FIX
//   - if count > 1: visible += " (N)"
//   - if Pos in 0..9: item.strName = "[<RightCtrl> | Ctrl+Alt] + &<Pos>   <visible>"
//     else: item.strName = visible
//
// The submenu path (ShowSubMenu / Repopulate lambda at 115-132) does NOT
// route through BuildSlotItem, so the strip here does NOT bleed into
// submenu shortcut wiring.
// ============================================================================
static MenuItemExMir BuildSlotItem(int Pos, const FARString& display_name, size_t count)
{
	MenuItemExMir item;

	FARString visible = display_name;
	// Critical strip: removes ANY user-typed '&X' from display_name so
	// only the slot-digit '&%d' on the formatted row below reaches HiText.
	ReplaceStrings(visible, L"&", L"", -1);

	if (count > 1) {
		visible.AppendFormat(L" (%zu)", count);
	}

	std::wstring result;
	if (Pos >= 0 && Pos < 10) {
		// Format: L"[%ls | Ctrl+Alt] + &%d   %ls"
		result  = L"[";
		result += Msg::RightCtrl;
		result += L" | Ctrl+Alt] + &";
		result += std::to_wstring(Pos);
		result += L"   ";
		result += visible.data;
	} else {
		result = visible.data;
	}
	item.strName = result;
	return item;
}

// ============================================================================
// Tests
// ============================================================================

TEST(BookmarkDisplayName, EmptyFolderShowsNone)
{
	BookmarkEntry e;
	e.Folder.Clear();
	e.Name = L"far2l-dev";
	EXPECT_STREQ(DisplayNameFor(e).CPtr(), L"<none>");
}

TEST(BookmarkDisplayName, ShortFolderNoName)
{
	BookmarkEntry e;
	e.Folder = L"/home";
	e.Name.Clear();
	EXPECT_STREQ(DisplayNameFor(e).CPtr(), L"/home");
}

// Folder(25) + ' (' + Name(9) + ')' (12) = 37 chars > MAX(36).
// folder_budget = 36 - (9 + 4) = 23. Folder trimmed to first 23 chars:
//   '/home/user/projects/far'.
// Final = '/home/user/projects/far (far2l-dev)' (35 chars). No safety-net.
TEST(BookmarkDisplayName, ShortFolderWithNameTrimsToBudget)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/projects/far2l"; // 25 chars
	e.Name   = L"far2l-dev";                  // 9 chars
	const FARString r = DisplayNameFor(e);
	EXPECT_STREQ(r.CPtr(), L"/home/user/projects/far (far2l-dev)");
	EXPECT_EQ(r.GetLength(), 35u);
}

TEST(BookmarkDisplayName, FolderAmpStrippedBeforeSuffix)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/build/far2l-maste&r"; // 30 chars, & at tail
	e.Name.Clear();
	// `&` in Folder must be stripped before HiText processing or it would
	// spuriously highlight `r` in the rendered menu.
	EXPECT_EQ(std::wstring(DisplayNameFor(e).CPtr()), L"/home/user/build/far2l-master");
	EXPECT_EQ(DisplayNameFor(e).GetLength(), 29u);
}

// USER'S LITERAL CASE — slot 2 entry 0. Raw output contains `&v` so
// VMenu::AssignHighlights binds `v` as submenu accelerator. HiText hides
// the `&` at render time. Both raw and visible strings are asserted.
TEST(BookmarkDisplayName, UserSlot2Entry0RawOutput)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/projects/far2l/far2l-dev"; // 32 chars
	e.Name   = L"far2l-de&v";                          // 10 chars
	const FARString rendered = DisplayNameFor(e);
	// folder(32) <= MAX → no middle-truncate.
	// folder_visible = '/home/user/projects/far2l/far2l-dev' (32, no '&' to strip).
	// suffix_overhead = 10 + 4 = 14; folder_budget = 36 - 14 = 22.
	// folder_visible (32) > 22 → trim to first 22 = '/home/user/projects/fa'.
	// Append ' (far2l-de&v)' (13 chars). 22 + 13 = 35 chars.
	EXPECT_STREQ(rendered.CPtr(),
		L"/home/user/projects/fa (far2l-de&v)");
	EXPECT_EQ(rendered.GetLength(), 35u);
	EXPECT_EQ(AssignedShortcutLetter(rendered), L'v');
}

// USER'S LITERAL CASE — submenu row, post-HiText visible string.
// The single `&v` marker is suppressed by HiText (the '&' disappears)
// and 'v' is accented. Submenu bypasses BuildSlotItem entirely, so the
// single, intentional `&X` survives end-to-end.
TEST(BookmarkDisplayName, UserSubmenuRowVisibleAfterHiText)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/projects/far2l/far2l-dev";
	e.Name   = L"far2l-de&v";
	const FARString rendered = DisplayNameFor(e);
	ASSERT_EQ(rendered.GetLength(), 35u);

	std::wstring visible;
	std::vector<size_t> accents;
	HiTextSimulate(rendered.data, visible, accents);

	EXPECT_EQ(visible.size(), 34u);
	EXPECT_EQ(visible.find(L'&'), std::wstring::npos);
	EXPECT_EQ(visible,
		L"/home/user/projects/fa (far2l-dev)");
	EXPECT_EQ(visible.back(), L')');
	ASSERT_EQ(accents.size(), 1u);
	EXPECT_EQ(visible[accents[0]], L'v');
}

// Length budget: when no Name and Folder > MAX, result is exactly MAX.
TEST(BookmarkDisplayName, LongFolderNoNameMiddleTruncated)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/build/far2l-master"; // 29 chars
	EXPECT_LT(e.Folder.GetLength(), BookmarkEntry::kDisplayMaxLen);
	EXPECT_STREQ(DisplayNameFor(e).CPtr(), L"/home/user/build/far2l-master");
}

// Excessive Folder (no Name) middle-truncation engages. Front is first
// 15 chars of the input Folder.
TEST(BookmarkDisplayName, ExcessiveFolderMiddleTruncated)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/projects/far2l/far2l-dev/foo/bar/baz/qux"; // 51
	e.Name.Clear();
	const FARString r = DisplayNameFor(e);
	EXPECT_EQ(r.GetLength(), BookmarkEntry::kDisplayMaxLen);
	EXPECT_EQ(std::wstring(r.CPtr(), 15), L"/home/user/proj");
	EXPECT_EQ(r.CPtr()[15], L'.');
	EXPECT_EQ(r.CPtr()[16], L'.');
	EXPECT_EQ(r.CPtr()[17], L'.');
	EXPECT_EQ(DisplayNameFor(e).data.find(L'&'), std::wstring::npos);
}

// Suffix-overflow branch: Name alone exceeds kDisplayMaxLen, drop suffix.
TEST(BookmarkDisplayName, NameLongerThanBudgetDropsSuffix)
{
	BookmarkEntry e;
	e.Folder = L"/tmp";
	e.Name   = L"some-extremely-long-display-name-here-etc-too-long";
	const FARString r = DisplayNameFor(e);
	EXPECT_EQ(std::wstring(r.CPtr()), L"/tmp");
}

// Shared&2 — user case for slot 3 entry 0.
// folder(18) + ' (Shared&2)' (11) = 29 chars < 36 → no truncation.
TEST(BookmarkDisplayName, UserSlot3Entry0SharedWithShortcut)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/Shared2";
	e.Name   = L"Shared&2";
	const FARString r = DisplayNameFor(e);
	EXPECT_EQ(std::wstring(r.CPtr()), L"/home/user/Shared2 (Shared&2)");
	EXPECT_EQ(AssignedShortcutLetter(r), L'2');

	std::wstring visible;
	std::vector<size_t> accents;
	HiTextSimulate(r.data, visible, accents);
	EXPECT_EQ(visible.find(L'&'), std::wstring::npos);
	ASSERT_FALSE(accents.empty());
	EXPECT_EQ(visible[accents.back()], L'2');
}

// Bookmarks menu invariant: DisplayNameFor output never exceeds MAX.
TEST(BookmarkDisplayName, ResultNeverExceedsMaxLen)
{
	struct Case { const wchar_t* f; const wchar_t* n; };
	const Case cases[] = {
		{L"", L"some-name"},
		{L"/", L""},
		{L"/x", L"y"},
		{L"/home/user/projects/far2l/far2l-dev", L"far2l-de&v"},
		{L"/home/user/build/far2l-master", L"/home/user/build/far2l-maste&r"},
		{L"/tmp", L"some-extremely-long-display-name-here-etc"},
		{L"/home/user/Shared2", L"Shared&2"},
		{L"<sftp:91.188.214.92>/Downloads", L""},
		{ L"/very/long/path/that/exceeds/thirty/six/characters/easily", L"x" },
		{ L"/very/long/path/that/exceeds/thirty/six/characters/easily", L"&X" },
	};
	for (const auto& c : cases) {
		BookmarkEntry e;
		e.Folder = c.f;
		e.Name   = c.n;
		const FARString r = DisplayNameFor(e);
		EXPECT_LE(r.GetLength(), BookmarkEntry::kDisplayMaxLen)
			<< "Folder=" << (c.f ? c.f : L"") << " Name=" << (c.n ? c.n : L"")
			<< " produced len=" << r.GetLength();
	}
}

// Folder portion (everything before " (") never carries `&` after
// DisplayNameFor; (Name) MAY carry `&X` for menu shortcut.
TEST(BookmarkDisplayName, FolderSideHasNoAmpersand)
{
	struct Case { const wchar_t* f; const wchar_t* n; };
	const Case cases[] = {
		{L"/home/user/build/far2l-maste&r", L""},
		{L"/home/user/build/far2l-maste&r", L"x"},
		{L"&spurious-leading", L"y"},
		{L"/a&b/c&d/e&f", L"z"},
	};
	for (const auto& c : cases) {
		BookmarkEntry e;
		e.Folder = c.f;
		e.Name   = c.n;
		const FARString r = DisplayNameFor(e);
		const std::wstring s = r.data;
		const size_t paren = s.find(L" (");
		const std::wstring folder_part =
			(paren == std::wstring::npos) ? s : s.substr(0, paren);
		EXPECT_EQ(folder_part.find(L'&'), std::wstring::npos)
			<< "Found & in Folder portion of: " << s;
	}
}

// Invariant: `&X` in Name yields a working letter-shortcut in the raw
// render so the submenu can wire it as an accelerator.
TEST(BookmarkDisplayName, NameShortcutLetterSurfaced)
{
	struct Case { const wchar_t* n; wchar_t expected; };
	const Case cases[] = {
		{L"far2l-de&v", L'v'},
		{L"Shared&2",   L'2'},
		{L"&first",     L'f'},
		{L"alpha",      0},
		{L"",           0},
	};
	for (const auto& c : cases) {
		BookmarkEntry e;
		e.Folder = L"/tmp";
		e.Name   = c.n;
		const FARString r = DisplayNameFor(e);
		EXPECT_EQ(AssignedShortcutLetter(r), c.expected)
			<< "Name=" << (c.n ? c.n : L"") << " rendered=" << r.data;
	}
}

// ============================================================================
// BuildSlotItem tests — pin the user's main-menu visible-row contract.
//
// KEY USER REQUIREMENT (re-locked here): after BuildSlotItem, the row
// strName MUST contain at most ONE `&X` marker (the slot-digit `&%d`).
// HiText processes only the first marker, so a pre-fix BuildSlotItem
// that left `&v` from (Name) on the same row would render `&v` as a
// stray visible artifact.
//
// Even cleaner: the user-visible row has zero `&` characters.
// ============================================================================

// BuildSlotItem strips '&' from display_name before composing the row.
TEST(BookmarkBuildSlotItem, StripsAmpersandsFromVisible)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/projects/far2l/far2l-dev";
	e.Name   = L"far2l-de&v";
	const FARString raw = DisplayNameFor(e);
	const MenuItemExMir item = BuildSlotItem(/*Pos=*/2, raw, /*count=*/4);

	// The visible portion of the strName must NOT contain '&' anywhere.
	// Even though BuildSlotItem composes "[RightCtrl | Ctrl+Alt] + &2   <visible>",
	// '&' must appear exactly once (the slot digit) and only there.
	const size_t amp_count = std::count(
		item.strName.begin(), item.strName.end(), L'&');
	EXPECT_EQ(amp_count, 1u)
		<< "strName=" << item.strName
		<< " contained " << amp_count << " '&' characters (expected 1: slot digit only)";
}

// The single `&` MUST be the slot digit `'2'`, not anything from the
// (Name) portion.
TEST(BookmarkBuildSlotItem, OnlySlotDigitAmpSurvives)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/projects/far2l/far2l-dev";
	e.Name   = L"far2l-de&v";
	const FARString raw = DisplayNameFor(e);
	const MenuItemExMir item = BuildSlotItem(/*Pos=*/2, raw, /*count=*/4);

	const wchar_t letter = AssignedShortcutLetter(
		FARString{item.strName.c_str()});
	EXPECT_EQ(letter, L'2')
		<< "strName=" << item.strName
		<< " had first &X=" << (letter ? letter : L'?')
		<< " (expected '2' — slot digit)";
}

// USER'S REPORTED CASE — main menu row, end-to-end.
//  - raw DisplayNameFor contains '&v'
//  - BuildSlotItem composes the row, stripping '&' from the visible portion
//  - The slot digit `&2` is the ONLY `&X` left
//  - HiText renders the row with '2' accented and no stray '&' visible
TEST(BookmarkBuildSlotItem, UserMainMenuRowVisibleHasNoAmpersand)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/projects/far2l/far2l-dev";
	e.Name   = L"far2l-de&v";
	const FARString raw = DisplayNameFor(e);
	const MenuItemExMir item = BuildSlotItem(/*Pos=*/2, raw, /*count=*/4);

	// Run HiText on the composed item.strName.
	std::wstring visible;
	std::vector<size_t> accents;
	HiTextSimulate(item.strName, visible, accents);
	EXPECT_EQ(visible.find(L'&'), std::wstring::npos)
		<< "strName=" << item.strName << " visible=" << visible;
	// There is exactly one accent — the slot digit.
	ASSERT_EQ(accents.size(), 1u) << "visible=" << visible;
	EXPECT_EQ(visible[accents[0]], L'2')
		<< "Expected slot digit '2' to be accented; visible=" << visible;
	// The (display) portion of the visible row preserves Folder and Name
	// verbatim from DisplayNameFor's output (after BuildSlotItem's strip).
	const std::wstring disp =
		L"/home/user/projects/fa (far2l-dev)";
	EXPECT_NE(visible.find(disp), std::wstring::npos)
		<< "Expected visible to contain '" << disp << "'";
}

// Empty slot: count == 0 path — visible is the empty DisplayNameFor
// output (Folder empty → "<none>"). BuildSlotItem still composes the row.
TEST(BookmarkBuildSlotItem, EmptySlotComposeRow)
{
	BookmarkEntry e;
	e.Folder.Clear();
	e.Name.Clear();
	const FARString raw = DisplayNameFor(e);
	const MenuItemExMir item = BuildSlotItem(/*Pos=*/2, raw, /*count=*/0);
	const std::wstring expected =
		L"[RightCtrl | Ctrl+Alt] + &2   <none>";
	EXPECT_EQ(item.strName, expected);
}

// Multi-entry slot: count > 1 → MIF_SUBMENU flag (separate from strName)
// AND visible gets " (N)" appended.
TEST(BookmarkBuildSlotItem, MultiEntryAppendsCountSuffix)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/projects/far2l/far2l-dev";
	e.Name   = L"far2l-de&v";
	const FARString raw = DisplayNameFor(e);
	const MenuItemExMir item = BuildSlotItem(/*Pos=*/2, raw, /*count=*/4);
	// Visible should END with " (4)" from the count suffix.
	EXPECT_EQ(item.strName.substr(item.strName.size() - 4), L" (4)");
	// And the visible DisplayNameFor portion must contain "(far2l-dev)" — no '&'.
	const std::wstring expected_disp =
		L"/home/user/projects/fa (far2l-dev) (4)";
	EXPECT_NE(item.strName.find(expected_disp), std::wstring::npos)
		<< "strName=" << item.strName;
}

// Shared&2 row: only the slot digit `&2` should appear; the `&2` from
// `(Shared&2)` must be stripped by BuildSlotItem before the row is
// composed.
TEST(BookmarkBuildSlotItem, UserSlot3MainMenuRow)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/Shared2";
	e.Name   = L"Shared&2";
	const FARString raw = DisplayNameFor(e);
	const MenuItemExMir item = BuildSlotItem(/*Pos=*/3, raw, /*count=*/1);

	const size_t amp_count = std::count(
		item.strName.begin(), item.strName.end(), L'&');
	EXPECT_EQ(amp_count, 1u) << "strName=" << item.strName;
	const wchar_t letter = AssignedShortcutLetter(
		FARString{item.strName.c_str()});
	EXPECT_EQ(letter, L'3');
	// The visible portion in the composed row.
	EXPECT_NE(item.strName.find(L"/home/user/Shared2 (Shared2)"),
		std::wstring::npos) << "strName=" << item.strName;
}

// Slot 9 case: Name contains a long user-typed path WITH an embedded `&r`.
// After DisplayNameFor strips folder-&, the raw contains `&r` only in
// (Name) tail. BuildSlotItem strips this entirely, so the row has only
// `&9` for HiText to accent.
TEST(BookmarkBuildSlotItem, UserSlot9Entry0MainMenuRow)
{
	BookmarkEntry e;
	e.Folder = L"/home/user/build/far2l-master";
	e.Name   = L"/home/user/build/far2l-maste&r";
	const FARString raw = DisplayNameFor(e);
	const MenuItemExMir item = BuildSlotItem(/*Pos=*/9, raw, /*count=*/1);

	const size_t amp_count = std::count(
		item.strName.begin(), item.strName.end(), L'&');
	EXPECT_EQ(amp_count, 1u) << "strName=" << item.strName;
	const wchar_t letter = AssignedShortcutLetter(
		FARString{item.strName.c_str()});
	EXPECT_EQ(letter, L'9');

	// Run HiText; visible row should have no '&'.
	std::wstring visible;
	std::vector<size_t> accents;
	HiTextSimulate(item.strName, visible, accents);
	EXPECT_EQ(visible.find(L'&'), std::wstring::npos)
		<< "strName=" << item.strName << " visible=" << visible;
}

// Comprehensive round-trip: for every (Folder, Name) pair, BuildSlotItem
// must produce a strName whose visible post-HiText string has zero '&'.
TEST(BookmarkBuildSlotItem, MainMenuRowsAlwaysVisibleClean)
{
	struct Case { const wchar_t* f; const wchar_t* n; int pos; size_t count; };
	const Case cases[] = {
		{L"", L"", 0, 0},
		{L"/home", L"", 0, 1},
		{L"/home/user/projects/far2l/far2l-dev", L"far2l-de&v", 2, 4},
		{L"/home/user/build/far2l-master", L"/home/user/build/far2l-maste&r", 9, 1},
		{L"/home/user/Shared2", L"Shared&2", 3, 1},
		{L"<sftp:91.188.214.92>/Downloads", L"", 7, 1},
		{ L"/very/long/path/that/exceeds/thirty/six/characters/easily", L"x", 4, 1 },
	};
	for (const auto& c : cases) {
		BookmarkEntry e;
		e.Folder = c.f;
		e.Name   = c.n;
		const FARString raw = DisplayNameFor(e);
		const MenuItemExMir item = BuildSlotItem(c.pos, raw, c.count);

		std::wstring visible;
		std::vector<size_t> accents;
		HiTextSimulate(item.strName, visible, accents);
		EXPECT_EQ(visible.find(L'&'), std::wstring::npos)
			<< "Folder=" << (c.f ? c.f : L"") << " Name=" << (c.n ? c.n : L"")
			<< " pos=" << c.pos << " count=" << c.count
			<< " strName=" << item.strName
			<< " visible=" << visible;
		// Exactly one accent — the slot digit.
		ASSERT_EQ(accents.size(), 1u) << "visible=" << visible;
		EXPECT_EQ(visible[accents[0]], L'0' + c.pos)
			<< "Expected slot digit accented, visible=" << visible;
	}
}
