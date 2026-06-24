/*
test_location_bookmarks.cpp

Tests the bookmark row format used by the Location dialog
(panel.cpp:AddBookmarkItems). The bug that motivated these tests was a
leaked '&' from a user-typed folder/name appearing in the visible row
because HiText (interf.cpp:653-720) processes only the FIRST '&X' marker
per input.

We mirror the inline helpers StripAmpersands / LocationBookmarkRow from
Bookmarks.hpp because Bookmarks.hpp transitively pulls in lang.hpp which
has pre-existing -Werror incompatibilities in the test build (see
test_bookmarks.cpp:12-17 for the same constraint). The local FARString
mirror is the same one test_displayname.cpp / test_isvalid.cpp use, with
a small Format addition so we can pin the row layout exactly.

DRIFT HAZARD: changes to Bookmarks.hpp:StripAmpersands or
Bookmarks::LocationBookmarkRow MUST be mirrored here. Tests are not a
substitute for runtime checking of the real Location dialog rendering.
*/

#include <gtest/gtest.h>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>

// ============================================================================
// Local FARString mirror — matches the contract of FARString used by the
// production helper, *not* the production header (lang.hpp -> locale.hpp
// -> WINPORT breakage, see test_bookmarks.cpp:12-17). Same shape as
// test_displayname.cpp's mirror, plus a swprintf-driven Format().
// ============================================================================
struct FARString {
	std::wstring data;

	FARString() = default;
	FARString(const wchar_t* s) : data(s ? s : L"") {}

	FARString& operator=(const wchar_t* s) { data = s ? s : L""; return *this; }
	FARString& operator+=(const wchar_t* s)   { if (s) data += s; return *this; }
	FARString& operator+=(wchar_t c)          { data.push_back(c); return *this; }

	void Clear() { data.clear(); }
	bool IsEmpty() const { return data.empty(); }
	size_t GetLength() const { return data.size(); }
	const wchar_t* CPtr() const { return data.c_str(); }
	void Append(wchar_t c) { data.push_back(c); }

	void AppendFormat(const wchar_t* fmt, size_t arg) {
		// Mimics FARString::AppendFormat(L" (%zu)", arg).
		std::wstring tail = L" (";
		tail += std::to_wstring(arg);
		tail += L")";
		data += tail;
	}

	// Mimics FARString::Format(format, args...). Uses a 1024-character
	// wchar_t buffer (the production FARString does the same with FARString
	// stack-bound formatting). Good enough for our short row test cases.
	void Format(const wchar_t* fmt, ...) {
		wchar_t buf[1024];
		va_list ap;
		va_start(ap, fmt);
		int n = std::vswprintf(buf, sizeof(buf) / sizeof(buf[0]), fmt, ap);
		va_end(ap);
		if (n < 0) { data.clear(); return; }
		data.assign(buf, static_cast<size_t>(n));
	}
};

namespace {

// ---- Mirror of Bookmarks.hpp:StripAmpersands -----------------------------
inline FARString StripAmpersands(const FARString& s)
{
	FARString out;
	const size_t len = s.GetLength();
	const wchar_t* p = s.CPtr();
	for (size_t i = 0; i < len; ++i) {
		if (p[i] != L'&') out.Append(p[i]);
	}
	return out;
}

// ---- Mirror of Bookmarks.hpp:LocationBookmarkRow -------------------------
inline FARString LocationBookmarkRow(int Pos, const FARString& display_name, size_t count)
{
	FARString visible = StripAmpersands(display_name);
	if (count > 1) {
		visible.AppendFormat(L" (%zu)", count);
	}
	FARString out;
	out.Format(L"&%d  %ls", Pos, visible.CPtr());
	return out;
}

std::wstring W(const FARString& s)
{
	return std::wstring(s.CPtr(), s.GetLength());
}

} // namespace

// ============================================================================
// StripAmpersands
// ============================================================================

TEST(StripAmpersands, EmptyStringReturnsEmpty)
{
	EXPECT_EQ(W(StripAmpersands(FARString(L""))), L"");
}

TEST(StripAmpersands, NoAmpersandsIsIdentity)
{
	EXPECT_EQ(W(StripAmpersands(FARString(L"/home/user/work/dsrc-obu"))),
		L"/home/user/work/dsrc-obu");
}

TEST(StripAmpersands, AllAmpersandsBecomesEmpty)
{
	EXPECT_EQ(W(StripAmpersands(FARString(L"&&&&"))), L"");
}

TEST(StripAmpersands, RemovesEachIndividualAmpersand)
{
	EXPECT_EQ(W(StripAmpersands(FARString(L"&foo&bar"))), L"foobar");
}

// ============================================================================
// LocationBookmarkRow
// ============================================================================

TEST(LocationBookmarkRow, SlotDigitIsFirstMarker)
{
	FARString row = LocationBookmarkRow(0, FARString(L"/usr"), 1);
	EXPECT_EQ(W(row), L"&0  /usr");
}

TEST(LocationBookmarkRow, UserTypedAmpersandIsStripped)
{
	// Reported bug: a bookmark whose display name carries "far2l-maste&r"
	// used to leak the literal '&r' into the Location dialog row because
	// HiText processed only the slot-digit marker '&9' first.
	FARString row = LocationBookmarkRow(9, FARString(L"/h (/home/user/build/far2l-maste&r)"), 1);
	EXPECT_EQ(W(row), L"&9  /h (/home/user/build/far2l-master)");
}

TEST(LocationBookmarkRow, MultiEntryAppendsCountSuffix)
{
	FARString row = LocationBookmarkRow(3, FARString(L"/home/user/build/far2l-other/install"), 7);
	EXPECT_EQ(W(row), L"&3  /home/user/build/far2l-other/install (7)");
}

TEST(LocationBookmarkRow, SingleEntryHasNoCountSuffix)
{
	FARString row = LocationBookmarkRow(0, FARString(L"/usr"), 1);
	EXPECT_EQ(W(row).find(L'('), std::wstring::npos);
}

TEST(LocationBookmarkRow, EmptyDisplayNameRendersAsBareSlot)
{
	FARString row = LocationBookmarkRow(2, FARString(L""), 1);
	EXPECT_EQ(W(row), L"&2  ");
}

TEST(LocationBookmarkRow, EverySlotDigitRenders)
{
	// Every slot 0..9 must surface its own '&%d' accelerator so RightCtrl
	// + digit still jumps there.
	for (int slot = 0; slot < 10; ++slot) {
		FARString row = LocationBookmarkRow(slot, FARString(L"/path"), 1);
		wchar_t expected[12];
		std::swprintf(expected, 12, L"&%d  /path", slot);
		EXPECT_EQ(W(row), std::wstring(expected)) << "slot " << slot << " drift";
	}
}

TEST(LocationBookmarkRow, VisibleCarriesAtMostOneAmpersand)
{
	// Belt-and-braces: even after multi-entry suffixing, no stray '&'
	// ever survives into the visible portion. The first and last '&' must
	// both be the slot-digit '&%d'.
	const FARString name(L"a&b c&d e&f");
	for (int slot = 0; slot < 10; ++slot) {
		FARString row = LocationBookmarkRow(slot, name, 12);
		const auto first = W(row).find(L'&');
		const auto last  = W(row).rfind(L'&');
		ASSERT_NE(first, std::wstring::npos) << "slot " << slot;
		EXPECT_EQ(first, last) << "slot " << slot << " row='" << W(row) << "'";
		EXPECT_EQ(first, 0u) << "slot " << slot << " row='" << W(row) << "'";
	}
}

TEST(LocationBookmarkRow, ReportedUserCaseRendersClean)
{
	// Pin the exact visible row from the user's report so any future
	// regression of the &-strip will fail loudly. Slot 9 with five
	// entries, display name carrying a user-typed '&'.
	FARString row = LocationBookmarkRow(
		9,
		FARString(L"/h (/home/user/build/far2l-maste&r)"),
		5);
	EXPECT_EQ(W(row), L"&9  /h (/home/user/build/far2l-master) (5)");
}
TEST(LocationBookmarkRow, CountSuffixIsNotDuplicated)
{
	// Regression: the caller used to append `(N)` to `display_name`
	// before calling the helper, which then appended its own `(N)` —
	// producing "... (5) (5)". The helper now owns the suffix; the
	// caller passes the raw display name.
	FARString row = LocationBookmarkRow(2, FARString(L"/home/user/projects/far2l/far2l-dev"), 5);
	EXPECT_EQ(W(row), L"&2  /home/user/projects/far2l/far2l-dev (5)");
	// And exactly one "(N)" segment, not two.
	const std::wstring s = W(row);
	const auto first = s.find(L" (5)");
	ASSERT_NE(first, std::wstring::npos);
	EXPECT_EQ(s.find(L" (5)", first + 1), std::wstring::npos)
		<< "row='" << s << "' carries more than one (N) suffix";
}

TEST(LocationBookmarkRow, CountSuffixSingleEntryNeverAppears)
{
	FARString row = LocationBookmarkRow(4, FARString(L"/usr/local"), 1);
	EXPECT_EQ(W(row), L"&4  /usr/local");
	EXPECT_EQ(W(row).find(L'('), std::wstring::npos);
}

TEST(LocationBookmarkRow, LargeCountIsNotMisformatted)
{
	// count of 9997 must still render as one "(9997)" suffix, not
	// "(9997) (9997)" and not an exponential "1e4" shenanigan.
	FARString row = LocationBookmarkRow(7, FARString(L"/srv/data"), 9997);
	EXPECT_EQ(W(row), L"&7  /srv/data (9997)");
	const std::wstring s = W(row);
	EXPECT_EQ(std::count(s.begin(), s.end(), L'('), 1u);
}
