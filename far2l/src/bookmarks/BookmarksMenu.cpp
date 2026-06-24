/*
BookmarksMenu.cpp

Folder shortcuts menu - supports nested multi-entry slots with submenus.
*/

#include "headers.hpp"

#include "Bookmarks.hpp"
#include "BookmarksLog.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "KeyFileHelper.h"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "dialog.hpp"
#include "DialogBuilder.hpp"
#include <farplug-wide.h>
#include "plugins.hpp"
#include <cassert>

static const wchar_t HelpBookmarks[] = L"Bookmarks";

// Sentinel return from ShowSubMenu: submenu closed because the slot
// contents were mutated (Ins/Del/F4/Shift+Up/Shift+Down) and the main
// menu must redraw. Distinct from -1 which means the user explicitly
// dismissed the submenu.
static constexpr int kSlotMutated = -2;
static constexpr int kContinue = -3;
// Pick a bookmark from the current panel's location (plugin or filesystem).
// Returns false if the source is empty / unusable (E20).
static bool CaptureCurrentBookmark(BookmarkEntry& out)
{
	Panel *ActivePanel = CtrlObject->Cp()->ActivePanel;
	if (!ActivePanel) return false;

	out.Plugin.Clear();
	out.PluginFile.Clear();
	out.PluginData.Clear();
	out.Folder.Clear();
	out.Name.Clear();

	if (ActivePanel->GetMode() == PLUGIN_PANEL) {
		HANDLE hPlugin = ActivePanel->GetPluginHandle();
		if (!hPlugin) {
			BookmarksLog::Log(BookmarksLog::Level::Info,
				"CaptureCurrentBookmark: null plugin handle, skipping");
			return false;
		}
		OpenPluginInfo Info{};
		Info.StructSize = sizeof(OpenPluginInfo);
		CtrlObject->Plugins.GetOpenPluginInfo(hPlugin, &Info);
		if (auto *ph = static_cast<PluginHandle *>(hPlugin); ph && ph->pPlugin) {
			out.Plugin = ph->pPlugin->GetModuleName();
		}
		out.PluginFile = Info.HostFile;
		out.Folder = Info.CurDir;
		out.PluginData = Info.ShortcutData;
	} else {
		ActivePanel->GetCurDir(out.Folder);
	}

	if (out.Folder.IsEmpty()) {
		BookmarksLog::Log(BookmarksLog::Level::Info,
			"BookmarksMenu: empty CurDir, skipping");
		return false;
	}
	return true;
}

// Build a single menu item for a slot.
// count > 0: one entry, count == 0: empty slot, count > 1: multi-entry (with submenu).
//
// Main-menu row formatting adds its OWN `&%d` slot-digit accelerator
// (line 93 below). HiText (interf.cpp:653-720) only processes the FIRST
// `&X` marker in the row, then renders everything after it verbatim.
// If `(Name)` contains a user-typed `&X` (e.g. `&v`), that marker would
// fall through HiText and leak as a stray `&v` into the visible row.
// Strip `&` from the visible portion here ONLY — the submenu
static MenuItemEx BuildSlotItem(int Pos, const FARString& display_name, size_t count)
{
	// Slot-digit accelerator exists only for the historical 0..9 set.
	// If this ever changes, revisit the strip logic below.
	assert(Pos < 10 && "BuildSlotItem supports slot digits only for Pos<10");

	MenuItemEx item;
	item.Clear();

	// Strip every '&' so only the slot-digit '&%d' reaches HiText.
	// The submenu path (ShowSubMenu / Repopulate lambda) bypasses
	// BuildSlotItem and preserves its own '&X' accelerator.
	FARString visible = StripAmpersands(display_name);

	if (count > 1) {
		item.Flags |= MIF_SUBMENU;
		visible.AppendFormat(L" (%zu)", count);
	}
	item.strName.Format(L"[%ls | Ctrl+Alt] + &%d   %ls",
		Msg::RightCtrl.CPtr(), Pos, visible.CPtr());
	return item;
}

// =============================================================================
// Helper functions for ShowSubMenu
// =============================================================================

// Build and show the submenu contents for a given slot.
static void RepopulateSubMenu(VMenu& SubMenu, int Pos, std::vector<BookmarkEntry>& entries, int sel)
{
	SubMenu.Hide();
	entries.clear();
	BookmarksCache::Enumerator(Pos, entries);
	SubMenu.DeleteItems();
	for (size_t i = 0; i < entries.size(); ++i) {
		MenuItemEx item;
		item.Clear();
		FARString name = BookmarkEntry::DisplayNameFor(entries[i]);
		// Display 1-based labels; SelPos and new_pos are 0-based.
		item.strName.Format(L"%zu.  %ls", i + 1, name.CPtr());
		item.SetSelect((int)i == (sel >= 0 ? sel : (int)entries.size() - 1));
		SubMenu.AddItem(&item);
	}
	SubMenu.SetMaxHeight(std::min((int)entries.size(), ScrY / 2));
	SubMenu.SetPosition(-1, -1, 0, 0);
	SubMenu.Show();
}

// Handle KEY_SHIFTUP/KEY_SHIFTDOWN in ShowSubMenu.
// Returns kContinue to keep the loop running.
static int SubMenuHandleShift(int Pos, int SelPos, std::vector<BookmarkEntry>& entries, FarKey Key, VMenu& SubMenu)
{
	const int new_pos = (Key == KEY_SHIFTUP) ? SelPos - 1 : SelPos + 1;
	if (new_pos < 0 || new_pos >= (int)entries.size()) return kContinue;
	std::swap(entries[SelPos], entries[new_pos]);
	BookmarksCache::SetAll(Pos, entries);
	if (!BookmarksCache::Save()) {
		SubMenu.SetBottomTitle(L"Failed to save bookmarks");
	}
	RepopulateSubMenu(SubMenu, Pos, entries, new_pos);
	return kContinue;
}

// Handle KEY_DEL in ShowSubMenu: remove the selected entry.
// Returns kSlotMutated if the slot becomes empty, kContinue otherwise.
static int SubMenuHandleDelete(int Pos, int SelPos, std::vector<BookmarkEntry>& entries, VMenu& SubMenu)
{
	if (SelPos < 0 || SelPos >= (int)entries.size()) return kContinue;
	(void)BookmarksCache::RemoveAt(Pos, (size_t)SelPos);
	if (!BookmarksCache::Save()) {
		SubMenu.SetBottomTitle(L"Failed to save bookmarks");
	}
	entries.clear();
	BookmarksCache::Enumerator(Pos, entries);
	if (entries.empty()) {
		return kSlotMutated;
	}
	RepopulateSubMenu(SubMenu, Pos, entries, (SelPos < (int)entries.size()) ? SelPos : 0);
	return kContinue;
}

// Handle KEY_INS in ShowSubMenu: capture + dialog + add new entry.
// Always returns kContinue.
static int SubMenuHandleInsert(int Pos, std::vector<BookmarkEntry>& entries, VMenu& SubMenu)
{
	// Capture first, edit, then commit — mirrors the F4 path.
	// Pre-fix code Add()ed + Save()d before the dialog, so a
	// cancelled dialog left an unedited entry persisted on disk.
	BookmarkEntry new_entry;
	if (!CaptureCurrentBookmark(new_entry)) {
		return kContinue;
	}
	BookmarkEntry edit_entry = new_entry;
	FARString strNewDir = edit_entry.Folder;
	FARString strNewName = edit_entry.Name;
	if (strNewName.IsEmpty()) {
		strNewName = edit_entry.Folder;
	}
	DialogBuilder Builder(Msg::BookmarksTitle, HelpBookmarks);
	Builder.AddText(Msg::FSShortcutName);
	Builder.AddEditField(&strNewName, 50, L"FS_Name", DIF_EDITPATH);
	Builder.AddText(Msg::FSShortcut);
	Builder.AddEditField(&strNewDir, 50, L"FS_Path", DIF_EDITPATH);
	Builder.AddOKCancel();
	if (!Builder.ShowDialog()) {
		// Dialog cancelled — do NOT persist the captured entry.
		return kContinue;
	}
	Unquote(strNewDir);
	if (!IsLocalRootPath(strNewDir))
		DeleteEndSlash(strNewDir);
	edit_entry.Name = strNewName;
	edit_entry.Folder = strNewDir;
	if (edit_entry.Name == edit_entry.Folder) {
		edit_entry.Name.Clear();
	}
	BookmarksLog::Log(BookmarksLog::Level::Debug,
		"ShowSubMenu Ins: slot=%d name='%ls' folder='%ls'",
		Pos, strNewName.CPtr(), strNewDir.CPtr());
	if (!BookmarksCache::Add(Pos, edit_entry)) {
		SubMenu.SetBottomTitle(L"Invalid bookmark entry");
		return kContinue;
	}
	if (!BookmarksCache::Save()) {
		SubMenu.SetBottomTitle(L"Failed to save bookmarks");
	}
	RepopulateSubMenu(SubMenu, Pos, entries, 0);
	return kContinue;
}

// Handle KEY_F4 in ShowSubMenu: edit the selected entry via dialog.
// Always returns kContinue.
static int SubMenuHandleEdit(int Pos, int SelPos, std::vector<BookmarkEntry>& entries, VMenu& SubMenu)
{
	if (SelPos < 0 || SelPos >= (int)entries.size()) return kContinue;
	BookmarkEntry edit_entry = entries[SelPos];
	FARString strNewDir = edit_entry.Folder;
	FARString strNewName = edit_entry.Name;
	if (strNewName.IsEmpty()) {
		strNewName = edit_entry.Folder;
	}
	DialogBuilder Builder(Msg::BookmarksTitle, HelpBookmarks);
	Builder.AddText(Msg::FSShortcutName);
	Builder.AddEditField(&strNewName, 50, L"FS_Name", DIF_EDITPATH);
	Builder.AddText(Msg::FSShortcut);
	Builder.AddEditField(&strNewDir, 50, L"FS_Path", DIF_EDITPATH);
	Builder.AddOKCancel();
	if (Builder.ShowDialog()) {
		Unquote(strNewDir);
		if (!IsLocalRootPath(strNewDir))
			DeleteEndSlash(strNewDir);
		edit_entry.Name = strNewName;
		edit_entry.Folder = strNewDir;
		if (edit_entry.Name == edit_entry.Folder) {
			edit_entry.Name.Clear();
		}
		BookmarksLog::Log(BookmarksLog::Level::Debug,
			"ShowSubMenu F4: slot=%d entry=%d name='%ls' folder='%ls'",
			Pos, SelPos, strNewName.CPtr(), strNewDir.CPtr());
		if (!BookmarksCache::ReplaceAt(Pos, (size_t)SelPos, edit_entry)) {
			SubMenu.SetBottomTitle(L"Invalid bookmark entry");
			return kContinue;
		}
		if (!BookmarksCache::Save()) {
			SubMenu.SetBottomTitle(L"Failed to save bookmarks");
		}
		RepopulateSubMenu(SubMenu, Pos, entries, SelPos);
	}
	return kContinue;
}

// Show the submenu for slot `Pos` and return the selected entry index
// (0..N-1) via `out_entry`. Returns -1 if the user cancelled.
static int ShowSubMenu(int Pos)
{
	std::vector<BookmarkEntry> entries;
	BookmarksCache::Enumerator(Pos, entries);
	if (entries.empty()) return -1;
	VMenu SubMenu(Msg::BookmarksTitle, nullptr, 0,
		std::min((int)entries.size(), ScrY / 2));
	SubMenu.SetFlags(VMENU_WRAPMODE | VMENU_AUTOHIGHLIGHT);
	SubMenu.SetHelp(HelpBookmarks);
	SubMenu.SetPosition(-1, -1, 0, 0);
	SubMenu.SetBottomTitle(Msg::BookmarkBottom);

	RepopulateSubMenu(SubMenu, Pos, entries, 0);
	int exit_code = -1;
	while (!SubMenu.Done()) {
		const FarKey Key = SubMenu.ReadInput();
		const int SelPos = SubMenu.GetSelectPos();
		switch (Key) {
			case KEY_NUMENTER:
			case KEY_ENTER: {
				exit_code = SelPos;
				SubMenu.Hide();
				SubMenu.SetExitCode(exit_code);
				break;
			}
			case KEY_ESC:
			case KEY_F10: {
				exit_code = -1;
				SubMenu.Hide();
				SubMenu.SetExitCode(-1);
				break;
			}
			case KEY_SHIFTUP:
			case KEY_SHIFTDOWN: {
				SubMenuHandleShift(Pos, SelPos, entries, Key, SubMenu);
				continue;
			}
			case KEY_NUMDEL:
			case KEY_DEL: {
				const int h = SubMenuHandleDelete(Pos, SelPos, entries, SubMenu);
				if (h == kContinue) continue;
				exit_code = h;
				SubMenu.SetExitCode(h);
				SubMenu.Hide();
				break;
			}
			case KEY_NUMPAD0:
			case KEY_INS: {
				SubMenuHandleInsert(Pos, entries, SubMenu);
				continue;
			}
			case KEY_F4: {
				SubMenuHandleEdit(Pos, SelPos, entries, SubMenu);
				continue;
			}
			default:
				SubMenu.ProcessInput();
				break;
		}
	}
	if (SubMenu.Modal::GetExitCode() < 0 && exit_code == -1) {
		return -1;
	}
	return exit_code;
}

static int ShowBookmarksMenuIteration(int Pos)
{
	(void)BookmarksCache::Instance(); // ensure loaded

	// Drain deferred notifications from MigrateOnceIfNeeded (called before
	// CtrlObject was initialized, so couldn't Message() there). This is the
	// first user-facing entry point with a valid CtrlObject.
	{
		std::vector<int> dropped = BookmarksCache::ConsumeDeferredNotifications();
		std::string backup_path = BookmarksCache::ConsumeDeferredBackupPath();
		if (!dropped.empty()) {
			Message(MSG_WARNING, 1, Msg::BookmarksDropped, Msg::Ok);
		}
		if (BookmarksCache::IsEmpty() && !backup_path.empty()) {
			// Convert narrow UTF-8 backup_path to FARString so the %ls
			// format specifiers get matching wide-string arguments. The
			// path originates from DefaultBookmarksIniPath() (UTF-8)
			// so CP_UTF8 is correct.
			const FARString backup_path_w(backup_path, CP_UTF8);
			BookmarksLog::Log(BookmarksLog::Level::Debug,
				"ShowBookmarksMenuIteration migration-notice: backup='%ls'",
				backup_path_w.CPtr());
			FARString msg;
			msg.Format(L"%ls\n\n%ls\n%ls",
				Msg::BookmarksEmpty.CPtr(),
				Msg::BookmarksDroppedBackup.CPtr(),
				backup_path_w.CPtr());
			Message(MSG_WARNING, 1, msg.CPtr(), Msg::Ok);
			return -1;
		}
	}

	// Empty-state guard (AC37): if all slots are empty, show message and return.
	if (BookmarksCache::IsEmpty()) {
		Message(MSG_WARNING, 1, Msg::BookmarksEmpty, Msg::Ok);
		return -1;
	}
	MenuItemEx ListItem;
	VMenu FolderList(Msg::BookmarksTitle, nullptr, 0, ScrY - 4);
	FolderList.SetFlags(VMENU_WRAPMODE | VMENU_AUTOHIGHLIGHT);
	FolderList.SetHelp(HelpBookmarks);
	FolderList.SetPosition(-1, -1, 0, 0);
	FolderList.SetBottomTitle(Msg::BookmarkBottom);

	for (int I = 0; I < 10; ++I) {
		ListItem.Clear();
		const size_t count = BookmarksCache::GetCount(I);
		const FARString name = BookmarksCache::GetDisplayName(I);
		ListItem = BuildSlotItem(I, name, count);
		ListItem.SetSelect(I == Pos);
		FolderList.AddItem(&ListItem);
	}

	FolderList.Show();

	int ExitCode = -1;
	// Note: VMenu::Done() returns true once SetExitCode has been called,
	// even for negative codes (ESC / F10). The loop body handles break-out.
	while (!FolderList.Done()) {
		const FarKey Key = FolderList.ReadInput();
		const int SelPos = FolderList.GetSelectPos();

		switch (Key) {
			case KEY_NUMENTER:
			case KEY_ENTER: {
				const size_t cnt = BookmarksCache::GetCount(SelPos);
				if (cnt == 0) break;
				if (cnt == 1) {
					ExitCode = SelPos;
					FolderList.Hide();
					FolderList.SetExitCode(ExitCode);
				} else {
					FolderList.Hide();
					const int sub_result = ShowSubMenu(SelPos);
					if (sub_result == -1 || sub_result == kSlotMutated) {
						// cancelled or slot mutated - redraw main menu
						return SelPos;
					}
					// user picked entry sub_result
					CtrlObject->Cp()->ActivePanel->ExecShortcutFolder(SelPos, sub_result);
					return -1;
				}
				break;
			}
			case KEY_ESC:
			case KEY_F10: {
				ExitCode = -1;
				FolderList.Hide();
				FolderList.SetExitCode(-1);
				break;
			}
			case KEY_NUMDEL:
			case KEY_DEL: {
				const size_t cnt = BookmarksCache::GetCount(SelPos);
				if (cnt == 0) break;
				if (cnt == 1) {
					(void)BookmarksCache::Clear(SelPos);
					if (!BookmarksCache::Save()) {
						FolderList.SetBottomTitle(L"Failed to save bookmarks");
					}
				} else {
					FolderList.Hide();
					ShowSubMenu(SelPos); // user can pick which entry to delete
				}
				return SelPos;
			}
			case KEY_NUMPAD0:
			case KEY_INS: {
				BookmarkEntry new_entry;
				if (!CaptureCurrentBookmark(new_entry)) break;
				const size_t cnt = BookmarksCache::GetCount(SelPos);
				if (!BookmarksCache::Add(SelPos, new_entry)) {
					FolderList.SetBottomTitle(L"Invalid bookmark entry");
					continue;
				}
				if (cnt > 0) {
					FolderList.Hide();
					ShowSubMenu(SelPos);
				}
				return SelPos;
			}
			case KEY_F4: {
				const size_t cnt = BookmarksCache::GetCount(SelPos);
				if (cnt == 0) break;
				if (cnt == 1) {
					std::vector<BookmarkEntry> e;
					BookmarksCache::Enumerator(SelPos, e);
					if (e.empty()) break;
					BookmarkEntry edit_entry = e[0];
					FARString strNewDir = edit_entry.Folder;
					FARString strNewName = edit_entry.Name;
					if (strNewName.IsEmpty()) {
						strNewName = edit_entry.Folder;
					}
					DialogBuilder Builder(Msg::BookmarksTitle, HelpBookmarks);
					Builder.AddText(Msg::FSShortcutName);
					Builder.AddEditField(&strNewName, 50, L"FS_Name", DIF_EDITPATH);
					Builder.AddText(Msg::FSShortcut);
					Builder.AddEditField(&strNewDir, 50, L"FS_Path", DIF_EDITPATH);
					Builder.AddOKCancel();
					if (Builder.ShowDialog()) {
						Unquote(strNewDir);
						if (!IsLocalRootPath(strNewDir))
							DeleteEndSlash(strNewDir);
						BOOL Saved = TRUE;
						FARString strTemp;
						apiExpandEnvironmentStrings(strNewDir, strTemp);
						if (apiGetFileAttributes(strTemp) == INVALID_FILE_ATTRIBUTES) {
							WINPORT(SetLastError)(ERROR_PATH_NOT_FOUND);
							Saved = !Message(MSG_WARNING | MSG_ERRORTYPE, 2, Msg::Error, strNewDir,
									Msg::SaveThisShortcut, Msg::Yes, Msg::No);
						}
						if (Saved) {
							edit_entry.Name = strNewName;
							edit_entry.Folder = strNewDir;
							if (edit_entry.Name == edit_entry.Folder) {
								edit_entry.Name.Clear();
							}
							BookmarksLog::Log(BookmarksLog::Level::Debug,
								"ShowBookmarksMenuIteration F4: slot=%d name='%ls' folder='%ls'",
								SelPos, strNewName.CPtr(), strNewDir.CPtr());
							if (!BookmarksCache::ReplaceAt(SelPos, 0, edit_entry)) {
								FolderList.SetBottomTitle(L"Invalid bookmark entry");
								continue;
							}
							if (!BookmarksCache::Save()) {
								FolderList.SetBottomTitle(L"Failed to save bookmarks");
							}
							return SelPos;
						}
					}
				} else {
					FolderList.Hide();
					ShowSubMenu(SelPos);
					return SelPos;
				}
				break;
			}
			case KEY_SHIFTUP:
			case KEY_SHIFTDOWN: {
				if (SelPos < 0 || SelPos >= 10) break;
				const int OtherPos = (Key == KEY_SHIFTUP) ? SelPos - 1 : SelPos + 1;
				if (OtherPos < 0 || OtherPos >= 10) break;
				std::vector<BookmarkEntry> a;
				std::vector<BookmarkEntry> b;
				BookmarksCache::Enumerator(SelPos, a);
				BookmarksCache::Enumerator(OtherPos, b);
				BookmarksCache::SetAll(SelPos, b);
				BookmarksCache::SetAll(OtherPos, a);
				if (!BookmarksCache::Save()) {
					FolderList.SetBottomTitle(L"Failed to save bookmarks");
				}
				return OtherPos;
			}
			default:
				FolderList.ProcessInput();
				break;
		}
	}

	if (FolderList.Modal::GetExitCode() < 0) {
		return -1;
	}
	ExitCode = FolderList.Modal::GetExitCode();
	if (ExitCode >= 0) {
		CtrlObject->Cp()->ActivePanel->ExecShortcutFolder(ExitCode);
	}
	return -1;
}

void ShowBookmarksMenu(int Pos)
{
	while (Pos != -1) {
		Pos = ShowBookmarksMenuIteration(Pos);
	}
}

// =============================================================================
// Submenu-aware resolution helpers (used by panel hotkeys and cmdline).
// =============================================================================

// Shared implementation: fills out_* and out_entry_pos from the chosen entry.
// Returns Empty if the slot is empty or the chosen entry has no Folder (cmdline-only).
// - require_folder: if true, also returns Empty when entry's Folder is empty
//   (cmdline semantics: a blank Folder has no useful destination).
// - out_entry_pos: index of the picked entry (0 if single-entry slot).
static BookmarksCache::GetResult ResolveImpl(int Pos, FARString& out_path,
	FARString& out_plugin, FARString& out_plugin_file, FARString& out_plugin_data,
	int& out_entry_pos, bool require_folder)
{
	out_entry_pos = 0;
	if (Pos < 0 || Pos >= 10) return BookmarksCache::GetResult::Empty;
	(void)BookmarksCache::Instance();
	const size_t cnt = BookmarksCache::GetCount(Pos);
	if (cnt == 0) return BookmarksCache::GetResult::Empty;

	if (cnt == 1) {
		std::vector<BookmarkEntry> e;
		BookmarksCache::Enumerator(Pos, e);
		if (e.empty()) return BookmarksCache::GetResult::Empty;
		if (require_folder && e[0].Folder.IsEmpty()) {
			return BookmarksCache::GetResult::Empty;
		}
		out_path = e[0].Folder;
		out_plugin = e[0].Plugin;
		out_plugin_file = e[0].PluginFile;
		out_plugin_data = e[0].PluginData;
		out_entry_pos = 0;
		return BookmarksCache::GetResult::Ok;
	}

	const int picked = ShowSubMenu(Pos);
	if (picked < 0 || picked == kSlotMutated) {
		// Cancelled or slot exchange: no nav.
		return BookmarksCache::GetResult::Cancelled;
	}
	std::vector<BookmarkEntry> e;
	BookmarksCache::Enumerator(Pos, e);
	if (picked >= (int)e.size()) return BookmarksCache::GetResult::Cancelled;
	if (require_folder && e[picked].Folder.IsEmpty()) {
		return BookmarksCache::GetResult::Empty;
	}
	out_path = e[picked].Folder;
	out_plugin = e[picked].Plugin;
	out_plugin_file = e[picked].PluginFile;
	out_plugin_data = e[picked].PluginData;
	out_entry_pos = picked;
	return BookmarksCache::GetResult::Ok;
}

BookmarksCache::GetResult BookmarksCache::ResolveForCmdline(int Pos, FARString& out_path,
	FARString& out_plugin, FARString& out_plugin_file, FARString& out_plugin_data)
{
	int discarded_entry_pos = 0;
	return ResolveImpl(Pos, out_path, out_plugin, out_plugin_file, out_plugin_data,
		discarded_entry_pos, /*require_folder=*/true);
}

BookmarksCache::GetResult BookmarksCache::ResolveForSlot(int Pos, FARString& out_path,
	FARString& out_plugin, FARString& out_plugin_file, FARString& out_plugin_data, int& out_entry_pos)
{
	return ResolveImpl(Pos, out_path, out_plugin, out_plugin_file, out_plugin_data,
		out_entry_pos, /*require_folder=*/false);
}
