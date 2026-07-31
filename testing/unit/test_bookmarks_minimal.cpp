/*
test_bookmarks_minimal.cpp

Smoke tests verifying the KeyFileHelper round-trip behavior for the
Hierarchical ShortcutM/Folder format produced by Bookmarks::WriteSlot.
Companion to test_bookmarks.cpp: this file exercises single-entry,
multi-entry, and section-removal paths.

NOTE: See test_bookmarks.cpp header for why these tests use KeyFileHelper
directly instead of the Bookmarks class API.
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

class BookmarksMinimalTest : public ::testing::Test
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

TEST_F(BookmarksMinimalTest, WriteAndReadSingleEntry)
{
	{
		KeyFileHelper k(m_path.c_str(), false);
		k.SetString("5", "Shortcut0/Folder", L"/home");
		k.SetString("5", "Shortcut0/Name", L"home");
		ASSERT_TRUE(k.Save(false));
	}
	KeyFileHelper k(m_path.c_str(), true);
	EXPECT_TRUE(k.HasKey("5", "Shortcut0/Folder"));
	EXPECT_EQ(k.GetString("5", "Shortcut0/Folder", L""), L"/home");
}

TEST_F(BookmarksMinimalTest, MultipleEntries)
{
	{
		KeyFileHelper k(m_path.c_str(), false);
		for (int i = 0; i < 5; ++i) {
			std::string key = "Shortcut" + std::to_string(i) + "/Folder";
			std::wstring val = std::wstring(L"/entry") + std::to_wstring(i);
			k.SetString(std::string("7"), key, val.c_str());
		}
		ASSERT_TRUE(k.Save(false));
	}
	KeyFileHelper k(m_path.c_str(), true);
	for (int i = 0; i < 5; ++i) {
		std::string key = "Shortcut" + std::to_string(i) + "/Folder";
		EXPECT_TRUE(k.HasKey("7", key)) << "key " << key;
	}
}

TEST_F(BookmarksMinimalTest, SectionRemoval)
{
	{
		KeyFileHelper k(m_path.c_str(), false);
		k.SetString("0", "Shortcut0/Folder", L"/x");
		k.SetString("1", "Shortcut0/Folder", L"/y");
		ASSERT_TRUE(k.Save(false));
	}
	{
		KeyFileHelper k(m_path.c_str(), true);
		k.RemoveSection("1");
		ASSERT_TRUE(k.Save(false));
	}
	KeyFileHelper k(m_path.c_str(), true);
	EXPECT_TRUE(k.HasSection("0"));
	EXPECT_FALSE(k.HasSection("1"));
}
