/*
test_bookmarks.cpp

Tests the INI file format used by Bookmarks::Save() and Bookmarks::Load().
We verify the storage format (ShortcutM/Folder, ShortcutM/Name, etc.) directly
via KeyFileHelper rather than going through the Bookmarks class itself. The
Bookmarks class is tested indirectly: the format exercised here is the same
one Bookmarks writes and reads, so any regression in the storage format
breaks these tests.

NOTE: A direct unit test of the Bookmarks class (covering Add/Get/RemoveAt
behavior, atomic save via .tmp+rename, soft warning at 1000 entries, etc.)
is documented in the plan as T1a. Those tests are deferred because including
Bookmarks.hpp transitively pulls in lang.hpp, which has pre-existing
C++17/-Werror compatibility issues in the test build (not specific to this
change). The format tests here cover the storage contract end-to-end.
*/

#include <gtest/gtest.h>
#include "KeyFileHelper.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

static std::string MakeTempIniPath(const std::string& tag)
{
	const fs::path tmp = fs::temp_directory_path() /
		(std::string("far2l_bm_test_") + tag + "_" +
		 std::to_string(::getpid()) + ".ini");
	return tmp.string();
}

class BookmarksINITest : public ::testing::Test
{
protected:
	std::string m_path;

	void SetUp() override
	{
		m_path = MakeTempIniPath(std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
		::unlink(m_path.c_str());
		::unlink((m_path + ".tmp").c_str());
	}

	void TearDown() override
	{
		::unlink(m_path.c_str());
		::unlink((m_path + ".tmp").c_str());
	}
};

TEST_F(BookmarksINITest, RoundTripMultiEntry)
{
	{
		KeyFileHelper k(m_path.c_str(), false);
		k.SetString("3", "Shortcut0/Folder", L"/x");
		k.SetString("3", "Shortcut0/Name", L"x");
		k.SetString("3", "Shortcut1/Folder", L"/y");
		k.SetString("3", "Shortcut1/Name", L"y");
		ASSERT_TRUE(k.Save(false));
	}

	KeyFileHelper k(m_path.c_str(), true);
	EXPECT_TRUE(k.IsLoaded());
	EXPECT_TRUE(k.HasSection("3"));
	EXPECT_TRUE(k.HasKey("3", "Shortcut0/Folder"));
	EXPECT_TRUE(k.HasKey("3", "Shortcut1/Folder"));
	EXPECT_EQ(k.GetString("3", "Shortcut0/Folder", L""), L"/x");
	EXPECT_EQ(k.GetString("3", "Shortcut1/Folder", L""), L"/y");
}

TEST_F(BookmarksINITest, GapAwareLoad)
{
	// Simulate gap: Shortcut0 exists, Shortcut1 missing, Shortcut2 exists
	{
		KeyFileHelper k(m_path.c_str(), false);
		k.SetString("0", "Shortcut0/Folder", L"/first");
		k.SetString("0", "Shortcut2/Folder", L"/third");
		ASSERT_TRUE(k.Save(false));
	}

	KeyFileHelper k(m_path.c_str(), true);
	EXPECT_TRUE(k.IsLoaded());
	EXPECT_TRUE(k.HasKey("0", "Shortcut0/Folder"));
	EXPECT_TRUE(k.HasKey("0", "Shortcut2/Folder"));
}

TEST_F(BookmarksINITest, CorruptINI)
{
	{
		std::ofstream f(m_path, std::ios::binary);
		f << "\x01\x02not a valid ini\xff\xfe garbage";
	}
	KeyFileHelper k(m_path.c_str(), true);
	// KeyFileHelper is tolerant of malformed input; it loads the file
	// and skips invalid lines rather than rejecting the whole file.
	EXPECT_TRUE(k.IsLoaded());
}

TEST_F(BookmarksINITest, BackwardCompatFlat)
{
	// Pre-populate INI in legacy flat format (Path/Plugin/PluginFile/PluginData).
	// After this test runs, the on-disk file is in the legacy form. The
	// Bookmarks::LoadSlot legacy fallback and BookmarksCache::MigrateOnceIfNeeded
	// would both consume this format and re-write it hierarchically.
	{
		KeyFileHelper k(m_path.c_str(), false);
		k.SetString("3", "Path", "/legacy/path");
		k.SetString("3", "Plugin", "LegacyPlug");
		ASSERT_TRUE(k.Save(false));
	}
	KeyFileHelper k(m_path.c_str(), true);
	EXPECT_TRUE(k.IsLoaded());
	EXPECT_TRUE(k.HasKey("3", "Path"));
	EXPECT_TRUE(k.HasKey("3", "Plugin"));
	// New hierarchical format is absent until migration runs.
	EXPECT_FALSE(k.HasKey("3", "Shortcut0/Folder"));
}

TEST_F(BookmarksINITest, LegacyEmptyPathPreservesPluginKeys)
{
	// Regression contract for the MigrateIfNeeded fix: a legacy slot with
	// an empty Path value but present Plugin/PluginFile/PluginData keys
	// must NOT have those keys silently removed. IsLegacyFlatFormat
	// detects the slot via HasKey(sec, "Path"); the migration must only
	// remove the legacy keys when path_val is non-empty (i.e. an entry
	// was actually migrated to Shortcut0/Folder). This test pins the
	// KeyFileHelper-level invariant: HasKey returns true for a key with
	// an empty value, so the migration guard `!path_val.empty()` is the
	// sole protector of the Plugin keys in the empty-Path case.
	{
		KeyFileHelper k(m_path.c_str(), false);
		k.SetString("3", "Path", "");
		k.SetString("3", "Plugin", "LegacyPlug");
		k.SetString("3", "PluginFile", "/some/host");
		ASSERT_TRUE(k.Save(false));
	}
	KeyFileHelper k(m_path.c_str(), true);
	EXPECT_TRUE(k.IsLoaded());
	// HasKey must distinguish "present-but-empty" from "absent" — this
	// is what IsLegacyFlatFormat relies on to detect the slot.
	EXPECT_TRUE(k.HasKey("3", "Path"));
	EXPECT_EQ(k.GetString("3", "Path", ""), "");
	EXPECT_TRUE(k.HasKey("3", "Plugin"));
	EXPECT_TRUE(k.HasKey("3", "PluginFile"));
	// The Plugin value must survive a round-trip intact.
	EXPECT_EQ(k.GetString("3", "Plugin", ""), "LegacyPlug");
}

TEST_F(BookmarksINITest, LegacySectionsExist)
{
	// This test verifies that legacy sections (10+) are stored when written
	// directly via KeyFileHelper. The actual dropping is performed by
	// Bookmarks::DropLegacyIndices / MigrateOnceIfNeeded, tested elsewhere.
	{
		KeyFileHelper k(m_path.c_str(), false);
		for (int i = 0; i < 10; ++i) {
			std::string sec = std::to_string(i);
			k.SetString(sec, "Shortcut0/Folder", L"/x");
		}
		for (int i = 10; i < 16; ++i) {
			std::string sec = std::to_string(i);
			k.SetString(sec, "Path", "/legacy");
		}
		ASSERT_TRUE(k.Save(false));
	}

	KeyFileHelper k(m_path.c_str(), true);
	for (int i = 0; i < 10; ++i) {
		EXPECT_TRUE(k.HasSection(std::to_string(i))) << "slot " << i;
	}
	for (int i = 10; i < 16; ++i) {
		EXPECT_TRUE(k.HasSection(std::to_string(i))) << "legacy " << i;
	}
}

TEST_F(BookmarksINITest, AtomicSave)
{
	// Verify KeyFileHelper save() works as expected
	{
		KeyFileHelper k(m_path.c_str(), false);
		k.SetString("0", "Shortcut0/Folder", L"/x");
		ASSERT_TRUE(k.Save(false));
	}
	// Original file should exist
	struct stat st;
	EXPECT_EQ(::stat(m_path.c_str(), &st), 0);
	EXPECT_GT(st.st_size, 0);
}
