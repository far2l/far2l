/*
config.cpp

Конфигурация
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include <algorithm>
#include <langinfo.h> // for nl_langinfo
#include "config.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "filefilter.hpp"
#include "poscache.hpp"
#include "findfile.hpp"
#include "hilight.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "farcolors.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "panelmix.hpp"
#include "strmix.hpp"
#include "udlist.hpp"
#include "datetime.hpp"
#include "DialogBuilder.hpp"
#include "vtshell.h"
#include "ConfigRW.hpp"
#include "ConfigOptSaveLoad.hpp"
#include "AllXLats.hpp"
#include "xlat.hpp"

DWORD ApplyConsoleTweaks();
void ApplySudoConfiguration();
// void AddHistorySettings(DialogBuilder &Builder, FarLangMsg MTitle, int *OptEnabled, int *OptCount);
void SanitizeHistoryCounts();
void SanitizeIndentationCounts();
void DirectoryNameSettings();
void SetFolderInfoFiles();
void ApplyConfig();
void PanelSettings_HighlightMarks();

// static int HighlightMarksID = -1, ChangeSizeColumnStyleID = -1;
static int ChangeFontID = -1;
static int DateTimeDefaultID = -1, DateTimeCurrentID = -1, DateTimeFromSystemID = -1;
static int OKButtonID = -1, CancelButtonID = -1;

static DWORD supported_tweaks = -1;
static int DateFormatIndex = -1;
static FARString strDateSeparator; //Opt.strDateSeparator
static FARString strTimeSeparator; //Opt.strTimeSeparator
static FARString strDecimalSeparator; //Opt.strDecimalSeparator
static std::vector<DialogBuilderListItem> XLatItems;
static AllXlats xlats;
static int SelectedXLat = -1;
static int cmdHist_optExecCmdLine, cmdHist_optExecPanel;
static BOOL AutoUpdate, oldColorCC, TreeScanDepthEnabled;
static int cmdHist_optAssSys, cmdHist_optAssFar;

typedef struct dircfg_data_s {
	int DirNameStyle;
	int SurrIndex;
	BOOL bCentered;
	BOOL bSurr;

} dircfg_data_t;

static dircfg_data_t dircfg_data;

void AddHistorySettings2(DialogBuilder &Builder, 
	FarLangMsg MTitle1, int *OptEnabled1, int *OptCount1,
	FarLangMsg MTitle2, int *OptEnabled2, int *OptCount2,
	FarLangMsg MTitle3 = Msg::ConfigSaveHistory, int *OptEnabled3 = 0, int *OptCount3 = 0)
{
	auto EnabledCheckBox1 = Builder.AddCheckbox(MTitle1, OptEnabled1);
	auto EnabledCheckBox2 = Builder.AddCheckbox(MTitle2, OptEnabled2);
	auto EnabledCheckBox3 = OptEnabled3 ? Builder.AddCheckbox(MTitle3, OptEnabled3) : Builder.AddNone();

	Builder.ColumnBreak();

	auto CountText1 = Builder.AddText(Msg::ConfigMaxHistoryCount, false);
	auto CountEdit1 = Builder.AddIntEditField(OptCount1, 6);

	auto CountText2 = Builder.AddText(Msg::ConfigMaxHistoryCount, false);
	auto CountEdit2 = Builder.AddIntEditField(OptCount2, 6);

	auto CountText3 = OptEnabled3 ? Builder.AddText(Msg::ConfigMaxHistoryCount, false) : Builder.AddNone();
	auto CountEdit3 = OptEnabled3 ? Builder.AddIntEditField(OptCount3, 6) : Builder.AddNone();

	Builder.LinkFlags(EnabledCheckBox1, CountEdit1, DIF_DISABLE);
	Builder.LinkFlags(EnabledCheckBox1, CountText1, DIF_DISABLE);
	Builder.LinkFlags(EnabledCheckBox2, CountEdit2, DIF_DISABLE);
	Builder.LinkFlags(EnabledCheckBox2, CountText2, DIF_DISABLE);

	if (OptEnabled3) {
		Builder.LinkFlags(EnabledCheckBox3, CountEdit3, DIF_DISABLE);
		Builder.LinkFlags(EnabledCheckBox3, CountText3, DIF_DISABLE);
	}
}

static int Do_AllSystemSettings()
{
	DialogBuilder Builder(Msg::AllOptsMenuItem, L"AllSystemSettings");

    /* interface settings  */

	Builder.AddSeparator(Msg::ConfigInterfaceTitle);

	Builder.StartColumns();

	Builder.AddCheckbox(Msg::ConfigClock, &Opt.Clock);
	Builder.AddCheckbox(Msg::ConfigViewerEditorClock, &Opt.ViewerEditorClock);
	Builder.AddCheckbox(Msg::ConfigKeyBar, &Opt.ShowKeyBar);
	Builder.AddCheckbox(Msg::ConfigMenuBar, &Opt.ShowMenuBar);
	// auto SaverCheckbox = Builder.AddCheckbox(Msg::ConfigSaver, &Opt.ScreenSaver, false);
	// auto SaverEdit = Builder.AddIntEditField(&Opt.ScreenSaverTime, 2, 0, false);
	// Builder.AddText(Msg::ConfigSaverMinutes);
	// Builder.LinkFlags(SaverCheckbox, SaverEdit, DIF_DISABLE);

	Builder.ColumnBreak();

	Builder.AddCheckbox(Msg::ConfigMouse, &Opt.Mouse);
	Builder.AddCheckbox(Msg::CopyToPrimarySelection, &Opt.CopyToPrimarySelection);
	Builder.AddCheckbox(Msg::PasteFromPrimarySelection, &Opt.PasteFromPrimarySelection);

	Builder.EndColumns();

	if(Opt.Backend.UseModernLook) Builder.AddEmptyLine();
	Builder.AddText(Msg::ConfigWindowTitle, false);
	Builder.AddEditField(&Opt.strWindowTitle, 47);

	Builder.AddEmptyLine();

    /* tweak settings */

	const DWORD supported_tweaks = ApplyConsoleTweaks();
	if (supported_tweaks & TWEAK_STATUS_SUPPORT_BLINK_RATE) {
		Builder.AddIntEditField(&Opt.CursorBlinkTime, 3, 0, false);
		Builder.AddText(Msg::ConfigCursorBlinkInt);
	}

	if (supported_tweaks & TWEAK_STATUS_SUPPORT_CHANGE_FONT) {
		Builder.AddEmptyLine();
		Builder.AddButton(Msg::ConfigConsoleChangeFont, ChangeFontID, false);
		if (supported_tweaks & TWEAK_STATUS_SUPPORT_PAINT_SHARP) 
			Builder.AddCheckbox(Msg::ConfigConsolePaintSharp, &Opt.ConsolePaintSharp);
		else
			Builder.AddEmptyLine();
	}

	Builder.AddEmptyLine();
	Builder.StartColumns();

	if (supported_tweaks & TWEAK_STATUS_SUPPORT_OSC52CLIP_SET) 
		Builder.AddCheckbox(Msg::ConfigOSC52ClipSet, &Opt.OSC52ClipSet);

	if (supported_tweaks & TWEAK_STATUS_SUPPORT_TTY_PALETTE) 
		Builder.AddCheckbox(Msg::ConfigTTYPaletteOverride, &Opt.TTYPaletteOverride);

	if (supported_tweaks & TWEAK_STATUS_SUPPORT_TTY_CURSOR_SHAPE) {
		static FarLangMsg CursorShapeOptions[] = {Msg::ConfigCursorShapeBar,
				Msg::ConfigCursorShapeBlock, Msg::ConfigCursorShapeUnderline};
		Builder.AddText(Msg::ConfigTTYCursorShapeInsert, false);
		Builder.AddRadioButtonsHorz(&Opt.TTYCursorShapeInsert,
				ARRAYSIZE(CursorShapeOptions), CursorShapeOptions);
		Builder.AddText(Msg::ConfigTTYCursorShapeOvertype, false);
		Builder.AddRadioButtonsHorz(&Opt.TTYCursorShapeOvertype,
				ARRAYSIZE(CursorShapeOptions), CursorShapeOptions);
	}

	if(Opt.Backend.UseModernLook) Builder.AddEmptyLine();
	Builder.AddCheckbox(Msg::EnforceColorCorrection, &Opt.Dialogs.EnforceColorCorrection);
	Builder.AddCheckbox(Msg::EnforceThemeCorrection, &Opt.Dialogs.EnforceThemeCorrection);

	if(Opt.Backend.UseModernLook) Builder.AddEmptyLine();
	Builder.AddCheckbox(Msg::UseModernLook, &Opt.Backend.UseModernLook);
	Builder.AddCheckbox(Msg::EliminateOuterBorders, &Opt.Dialogs.EliminateOuterBorders);

    if ((supported_tweaks & TWEAK_STATUS_SUPPORT_CHANGE_FONT) && Opt.Backend.UseModernLook) {
        Builder.ColumnBreak();

    	// Builder.AddCheckbox(Msg::UseModernLookRoundedBorders, &Opt.Backend.UseRoundedBorders);
    	Builder.AddCheckbox(Msg::UseModernLookEmbossAsBold, &Opt.Backend.UseEmbossAsBold);
    	Builder.AddCheckbox(Msg::UseModernLookUseSoftenBevels, &Opt.Backend.UseSoftenBevels);
    	Builder.AddCheckbox(Msg::UseModernLookUse3D, &Opt.Backend.Use3D);
       	Builder.AddCheckbox(Msg::UseModernLookSingleBorders, &Opt.Backend.UseSingleBordersOnly);
	   	Builder.AddCheckbox(Msg::UseModernLookNoBorders, &Opt.Backend.UseNoBorders);
	   	Builder.AddCheckbox(Msg::UseModernLookUseFlyGirl, &Opt.Backend.UseFlyGirl);
    }

    Builder.EndColumns();

    /* auto complete and highlight section */

	Builder.AddSeparator(Msg::ConfigAutoCompleteTitle);
	auto ListCheck = Builder.AddCheckbox(Msg::ConfigAutoCompleteShowList, &Opt.AutoComplete.ShowList, false);
	auto ModalModeCheck = Builder.AddCheckbox(Msg::ConfigAutoCompleteModalList, &Opt.AutoComplete.ModalList, false);
	Builder.AddCheckbox(Msg::ConfigAutoCompleteAutoAppend, &Opt.AutoComplete.AppendCompletion);
	Builder.LinkFlags(ListCheck, ModalModeCheck, DIF_DISABLE);

	Builder.AddText(Msg::ConfigAutoCompleteExceptions, false);
	Builder.AddEditField(&Opt.AutoComplete.Exceptions, 27);

	Builder.AddEmptyLine();

	Builder.AddText(Msg::ConfigSaveHistoryOpt);
	Builder.Indent(8);
	Builder.StartColumns();
	Builder.AddCheckbox(Msg::ConfigSaveHistoryOptAssSys, &cmdHist_optAssSys);
	Builder.AddCheckbox(Msg::ConfigSaveHistoryOptAssFar, &cmdHist_optAssFar);
	Builder.ColumnBreak();
	Builder.AddCheckbox(Msg::ConfigSaveHistoryOptExecPanel, &cmdHist_optExecPanel);
	Builder.AddCheckbox(Msg::ConfigSaveHistoryOptExecCmdLine, &cmdHist_optExecCmdLine);
	Builder.EndColumns();
	
	Builder.AddEmptyLine();

    /* history settings */

    Builder.StartColumns();
	AddHistorySettings2(Builder, 
		Msg::ConfigSaveHistory, &Opt.SaveHistory, &Opt.HistoryCount,
		Msg::ConfigSaveFoldersHistory, &Opt.SaveFoldersHistory,	&Opt.FoldersHistoryCount,
        Msg::ConfigSaveViewHistory, &Opt.SaveViewHistory, &Opt.ViewHistoryCount);
	Builder.EndColumns();

	DialogBuilderListItem CAHistRemoveListItems[] = {
			{Msg::ConfigHistoryRemoveDupsRuleNever, HISTORY_REMOVE_DUPS_NEVER},
			{Msg::ConfigHistoryRemoveDupsRuleByName, HISTORY_REMOVE_DUPS_BY_NAME},
			{Msg::ConfigHistoryRemoveDupsRuleByNameExtra, HISTORY_REMOVE_DUPS_BY_NAME_EXTRA},
	};

	Builder.AddText(Msg::ConfigHistoryRemoveDupsRule, false);
	Builder.AddComboBox((int *)&Opt.HistoryRemoveDupsRule, 20, CAHistRemoveListItems, ARRAYSIZE(CAHistRemoveListItems),
			DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE);
	Builder.AddCheckbox(Msg::ConfigAutoHighlightHistory, &Opt.AutoHighlightHistory);

	Builder.AddEmptyLine();

	Builder.StartColumns();
	Builder.AddCheckbox(Msg::ConfigDialogsAutoComplete, &Opt.Dialogs.AutoComplete);
	Builder.AddCheckbox(Msg::ConfigCmdlineAutoComplete, &Opt.CmdLine.AutoComplete);
	Builder.ColumnBreak();
	AddHistorySettings2(Builder, 
		Msg::ConfigDialogsEditHistory, &Opt.Dialogs.EditHistory, &Opt.DialogsHistoryCount,
		Msg::ConfigSaveHistory, &Opt.SaveHistory, &Opt.HistoryCount);
	Builder.EndColumns();

    /* Dialog settings */

	Builder.AddSeparator(Msg::ConfigDlgSetsTitle);

	Builder.StartColumns();
	
	Builder.AddCheckbox(Msg::ConfigDialogsEditBlock, &Opt.Dialogs.EditBlock);
	Builder.AddCheckbox(Msg::ConfigDialogsEULBsClear, &Opt.Dialogs.EULBsClear);
	auto m1 = Builder.AddCheckbox(Msg::ConfigDialogsMouseButton, &Opt.Dialogs.MouseButton, false);

	Builder.ColumnBreak();

	Builder.AddCheckbox(Msg::ConfigDialogsDelRemovesBlocks, &Opt.Dialogs.DelRemovesBlocks);
	Builder.AddCheckbox(Msg::ConfigDialogsShowArrowsInEdit, &Opt.Dialogs.ShowArrowsInEdit);
	auto m2 = Builder.AddCheckbox(Msg::EnableAccidentalConfirmation, &Opt.Dialogs.EnableAccidentalConfirmation);
	Builder.LinkFlags(m1, m2, DIF_DISABLE);

	Builder.EndColumns();

    /* input settings */

	Builder.AddSeparator(Msg::ConfigInputTitle);

	Builder.StartColumns();

	Builder.AddText(Msg::ConfigXLats, false);
	Builder.AddComboBox(&SelectedXLat, 20, XLatItems.data(), XLatItems.size(),
			DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE);

	Builder.ColumnBreak();

	Builder.AddCheckbox(Msg::ConfigXLatFastFileFind, &Opt.XLat.EnableForFastFileFind);
	Builder.AddCheckbox(Msg::ConfigXLatDialogs, &Opt.XLat.EnableForDialogs);

	Builder.EndColumns();

	if (supported_tweaks & TWEAK_STATUS_SUPPORT_EXCLUSIVE_KEYS) {
		Builder.AddText(Msg::ConfigExclusiveKeys, false);

		Builder.StartColumns();
		Builder.AddCheckbox(Msg::ConfigExclusiveCtrlLeft, &Opt.ExclusiveCtrlLeft);
		Builder.AddCheckbox(Msg::ConfigExclusiveCtrlRight, &Opt.ExclusiveCtrlRight);
		Builder.ColumnBreak();
		Builder.AddCheckbox(Msg::ConfigExclusiveAltLeft, &Opt.ExclusiveAltLeft);
		Builder.AddCheckbox(Msg::ConfigExclusiveAltRight, &Opt.ExclusiveAltRight);
		Builder.AddCheckbox(Msg::ConfigUseRightAltAsAltGr, &Opt.UseRightAltAsAltGr);
		Builder.ColumnBreak();
		Builder.AddCheckbox(Msg::ConfigExclusiveWinLeft, &Opt.ExclusiveWinLeft);
		Builder.AddCheckbox(Msg::ConfigExclusiveWinRight, &Opt.ExclusiveWinRight);
		Builder.EndColumns();
	}

    /* system settings */

	Builder.AddSeparator(Msg::ConfigSystemTitle);

	auto DeleteToRecycleBin = Builder.AddCheckbox(Msg::ConfigRecycleBin, &Opt.DeleteToRecycleBin, false);
	auto DeleteLinks = Builder.AddCheckbox(Msg::ConfigRecycleBinLink, &Opt.DeleteToRecycleBinKillLink);
	Builder.LinkFlags(DeleteToRecycleBin, DeleteLinks, DIF_DISABLE);

	// auto InactivityExit = Builder.AddCheckbox(Msg::ConfigInactivity, &Opt.InactivityExit, false);
	// auto InactivityExitTime = Builder.AddIntEditField(&Opt.InactivityExitTime, 2, 0, false);
	// Builder.AddText(Msg::ConfigInactivityMinutes);
	// Builder.LinkFlags(InactivityExit, InactivityExitTime, DIF_DISABLE);

	// if(Opt.Backend.UseModernLook) Builder.AddEmptyLine();

	Builder.AddEmptyLine();

	// auto SudoEnabledItem = Builder.AddCheckbox(Msg::ConfigSudoEnabled, &Opt.SudoEnabled, false);
	Builder.AddText(Msg::ConfigSudoPasswordExpiration, false);
	/*auto SudoPasswordExpirationEdit = */Builder.AddIntEditField(&Opt.SudoPasswordExpiration, 4, 0, false);
	/*auto SudoConfirmModifyItem = */Builder.AddCheckbox(Msg::ConfigSudoConfirmModify, &Opt.SudoConfirmModify);
	// Builder.LinkFlags(SudoEnabledItem, SudoConfirmModifyItem, DIF_DISABLE);
	// Builder.LinkFlags(SudoEnabledItem, SudoPasswordExpirationEdit, DIF_DISABLE);

	Builder.AddEmptyLine();

	auto AutoSaveSetup = Builder.AddCheckbox(Msg::ConfigAutoSave, &Opt.AutoSaveSetup, false);
	auto AutoSavePanels = Builder.AddCheckbox(Msg::ConfigAutoSavePanels, &Opt.AutoSavePanels);
	Builder.LinkFlags(AutoSaveSetup, AutoSavePanels, DIF_DISABLE, false, false);

    /* confirmation settings */

	Builder.AddSeparator(Msg::SetConfirmTitle);

	Builder.StartColumns();

	Builder.AddCheckbox(Msg::SetConfirmCopy, &Opt.Confirm.Copy);
	Builder.AddCheckbox(Msg::SetConfirmMove, &Opt.Confirm.Move);
	Builder.AddCheckbox(Msg::SetConfirmRO, &Opt.Confirm.RO);
	Builder.AddCheckbox(Msg::SetConfirmDelete, &Opt.Confirm.Delete);
	Builder.AddCheckbox(Msg::SetConfirmDeleteFolders, &Opt.Confirm.DeleteFolder);
	Builder.AddCheckbox(Msg::SetConfirmExit, &Opt.Confirm.ExitEffective());

	Builder.ColumnBreak();

	Builder.AddCheckbox(Msg::SetConfirmClearVT, &Opt.Confirm.ClearVT);
	Builder.AddCheckbox(Msg::SetConfirmEsc, &Opt.Confirm.Esc);
	Builder.AddCheckbox(Msg::SetConfirmRemoveConnection, &Opt.Confirm.RemoveConnection);
	Builder.AddCheckbox(Msg::SetConfirmRemoveHotPlug, &Opt.Confirm.RemoveHotPlug);
	Builder.AddCheckbox(Msg::SetConfirmAllowReedit, &Opt.Confirm.AllowReedit);
	Builder.AddCheckbox(Msg::SetConfirmHistoryClear, &Opt.Confirm.HistoryClear);

	Builder.EndColumns();

    /* notification settings */

	NotificationsOptions &NotifOpt = Opt.NotifOpt;
	Builder.AddSeparator(Msg::NotifConfigTitle);

	Builder.StartColumns();
	Builder.AddCheckbox(Msg::NotifConfigOnFileOperation, &NotifOpt.OnFileOperation);
	Builder.AddCheckbox(Msg::NotifConfigOnConsole, &NotifOpt.OnConsole);
	Builder.ColumnBreak();
	Builder.AddCheckbox(Msg::NotifConfigOnlyIfBackground, &NotifOpt.OnlyIfBackground);
	Builder.EndColumns();

    /* date time settings */

	Builder.AddSeparator(Msg::ConfigDateFormat);

	FarLangMsg DateFormatOptions[] = {Msg::ConfigDateFormatMDY, Msg::ConfigDateFormatDMY, Msg::ConfigDateFormatYMD};
	Builder.AddRadioButtonsHorz(&DateFormatIndex, ARRAYSIZE(DateFormatOptions), DateFormatOptions);

	Builder.AddEmptyLine();

	Builder.StartColumns();

	auto DateSeparatorEdit = Builder.AddEditField(&strDateSeparator, 1, nullptr, 0, false);
	DateSeparatorEdit->Type = DI_FIXEDIT;
	DateSeparatorEdit->Flags |= DIF_MASKEDIT;
	DateSeparatorEdit->strMask = L"X";
	Builder.AddText(Msg::ConfigDateSeparator);

	auto TimeSeparatorEdit = Builder.AddEditField(&strTimeSeparator, 1, nullptr, 0, false);
	TimeSeparatorEdit->Type = DI_FIXEDIT;
	TimeSeparatorEdit->Flags |= DIF_MASKEDIT;
	TimeSeparatorEdit->strMask = L"X";
	Builder.AddText(Msg::ConfigTimeSeparator);

	auto DecimalSeparatorEdit = Builder.AddEditField(&strDecimalSeparator, 1, nullptr, 0, false);
	DecimalSeparatorEdit->Type = DI_FIXEDIT;
	DecimalSeparatorEdit->Flags |= DIF_MASKEDIT;
	DecimalSeparatorEdit->strMask = L"X";
	Builder.AddText(Msg::ConfigDecimalSeparator);

	Builder.ColumnBreak();

	Builder.AddButton(Msg::ConfigDateTimeDefault, DateTimeDefaultID);
	Builder.AddButton(Msg::ConfigDateTimeCurrent, DateTimeCurrentID);
	Builder.AddButton(Msg::ConfigDateTimeFromSystem, DateTimeFromSystemID);

	Builder.EndColumns();

    /* menu settings */

	Builder.AddSeparator(Msg::ConfigVMenuTitle);

	DialogBuilderListItem CAListItems2[] = {
			{Msg::ConfigVMenuClickCancel, VMENUCLICK_CANCEL},
			{Msg::ConfigVMenuClickApply,  VMENUCLICK_APPLY },
			{Msg::ConfigVMenuClickIgnore, VMENUCLICK_IGNORE},
	};
    
    Builder.StartColumns();

	Builder.AddText(Msg::ConfigVMenuLBtnClick);
	Builder.AddText(Msg::ConfigVMenuRBtnClick);
	Builder.AddText(Msg::ConfigVMenuMBtnClick);

	Builder.ColumnBreak();

	Builder.AddComboBox((int *)&Opt.VMenu.LBtnClick, 40, CAListItems2, ARRAYSIZE(CAListItems2),
			DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE);
	Builder.AddComboBox((int *)&Opt.VMenu.RBtnClick, 40, CAListItems2, ARRAYSIZE(CAListItems2),
			DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE);
	Builder.AddComboBox((int *)&Opt.VMenu.MBtnClick, 40, CAListItems2, ARRAYSIZE(CAListItems2),
			DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE);
	Builder.EndColumns();

	Builder.AddCheckbox(Msg::ConfigVMenuStopEdge, &Opt.VMenu.MenuLoopScroll);

    /* panel settings */

	Builder.AddSeparator(Msg::ConfigPanelTitle);

	Builder.AddCheckbox(Msg::ConfigHidden, &Opt.ShowHidden);
	Builder.AddCheckbox(Msg::ConfigSelectFolders, &Opt.SelectFolders);
	Builder.AddCheckbox(Msg::CtrlEnterMultipleItems, &Opt.CmdLine.CtrlEnterMultipleItems);
	Builder.AddCheckbox(Msg::ConfigClassicHotkeyLinkResolving, &Opt.ClassicHotkeyLinkResolving);
	Builder.AddCheckbox(Msg::ConfigPgUpChangeDisk, &Opt.PgUpChangeDisk);

	Builder.AddEmptyLine();
	Builder.StartColumns();

	Builder.AddCheckbox(Msg::ConfigShowScrollbar, &Opt.ShowPanelScrollbar);
	Builder.AddCheckbox(Msg::ConfigShowScreensNumber, &Opt.ShowScreensNumber);
	Builder.AddCheckbox(Msg::ConfigShowSortMode, &Opt.ShowSortMode);
	Builder.AddCheckbox(Msg::ConfigShowColumns, &Opt.ShowColumnTitles);
	Builder.AddCheckbox(Msg::ConfigShowStatus, &Opt.ShowPanelStatus);
	Builder.AddCheckbox(Msg::ConfigShowTotal, &Opt.ShowPanelTotals);
	Builder.AddCheckbox(Msg::ConfigShowFree, &Opt.ShowPanelFree);

	Builder.ColumnBreak();

	Builder.AddCheckbox(Msg::ConfigScanJunction, &Opt.ScanJunction);
	Builder.AddCheckbox(Msg::ConfigOnlyFilesSize, &Opt.OnlyFilesSize);
	Builder.AddCheckbox(Msg::ConfigCopyTotal, &Opt.CMOpt.CopyShowTotal);
	Builder.AddCheckbox(Msg::ConfigCopyTimeRule, &Opt.CMOpt.CopyTimeRule);
	Builder.AddCheckbox(Msg::ConfigDeleteTotal, &Opt.DelOpt.DelShowTotal);
	Builder.AddCheckbox(Msg::ConfigCaseSensitiveCompareSelect, &Opt.PanelCaseSensitiveCompareSelect);
	Builder.AddCheckbox(Msg::ConfigSortFolderExt, &Opt.SortFolderExt);
	Builder.AddCheckbox(Msg::ConfigReverseSort, &Opt.ReverseSort);

	Builder.EndColumns();
	Builder.AddEmptyLine();

	auto AutoUpdateEnabled = Builder.AddCheckbox(Msg::ConfigAutoUpdateLimit, &AutoUpdate, false);
	auto AutoUpdateNetwork = Builder.AddCheckbox(Msg::ConfigAutoUpdateRemoteDrive, &Opt.AutoUpdateRemoteDrive);
	Builder.AddText(Msg::ConfigAutoUpdateLimit2, false);
	auto AutoUpdateLimit = Builder.AddIntEditField((int *)&Opt.AutoUpdateLimit, 6);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateLimit, DIF_DISABLE, false);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateNetwork, DIF_DISABLE, false);

	Builder.AddEmptyLine();
	DialogBuilderListItem CAListItems[] = {
			{Msg::ConfigMakeLinkSuggestByFileDir, 0},
			{Msg::ConfigMakeLinkSuggestSymlinkAlways,  1},
	};
	Builder.AddText(Msg::ConfigMakeLinkSuggest, false);
	Builder.AddComboBox((int *)&Opt.MakeLinkSuggestSymlinkAlways, 48, CAListItems, ARRAYSIZE(CAListItems),
				DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE);

    /* tree options */

	Builder.AddSeparator(Msg::ConfigTreeOptions);

	Builder.AddCheckbox(Msg::ConfigAutoChange, &Opt.Tree.AutoChangeFolder);
	auto TreeScanDepthSwitch = Builder.AddCheckbox(Msg::ConfigDefaultTreeScanDepth, &TreeScanDepthEnabled, false);
	auto DefaultScanDepth = Builder.AddIntEditField((int *)&Opt.Tree.DefaultScanDepth, 6);
	Builder.LinkFlags(TreeScanDepthSwitch, DefaultScanDepth, DIF_DISABLE, false);
	Builder.AddText(Msg::ConfigExclSubTreeMask, false);
	Builder.AddEditField(&Opt.Tree.ExclSubTreeMask, 7);

    /* info panel settings */
    
	// Builder.AddSeparator(Msg::ConfigInfoPanelTitle);

    /* highlights marks sestion */
    
	Builder.AddSeparator(Msg::ConfigPanelHighlightMarksTitle);

	auto CbHighlight = Builder.AddCheckbox(Msg::ConfigHighlight, &Opt.Highlight, 0, false);

	auto CbShowFilenameMarks = Builder.AddCheckbox(Msg::ConfigFilenameMarks, &Opt.ShowFilenameMarks);
	Builder.Indent(4);
	auto CbFilenameMarksAlign = Builder.AddCheckbox(Msg::ConfigFilenameMarksAlign, &Opt.FilenameMarksAlign);
	Builder.LinkFlags(CbShowFilenameMarks, CbFilenameMarksAlign, DIF_DISABLE);
	Builder.Indent(4);
	auto CbShowFilenameMarksStatusLine = Builder.AddCheckbox(Msg::ConfigFilenameMarksStatusLine, &Opt.FilenameMarksInStatusBar);
	Builder.LinkFlags(CbShowFilenameMarks, CbShowFilenameMarksStatusLine, DIF_DISABLE);

	auto tShowFilenameMarksHint = Builder.AddText(Msg::ConfigFilenameMarksHint);
	tShowFilenameMarksHint->Flags = DIF_CENTERGROUP | DIF_DISABLE;
	auto tShowFilenameMarksInStatusLineHint = Builder.AddText(Msg::ConfigFilenameMarksStatusLineHint);
	tShowFilenameMarksInStatusLineHint->Flags = DIF_CENTERGROUP | DIF_DISABLE;

	auto edShowFilenameMarksIndent = Builder.AddIntEditField((int *)&Opt.MinFilenameIndentation, 2, 0, false);
	Builder.AddText(Msg::ConfigFilenameMinIndentation);

	Builder.AddIntEditField((int *)&Opt.MaxFilenameIndentation, 2, 0, false);
	Builder.AddText(Msg::ConfigFilenameMaxIndentation);

	Builder.LinkFlags(CbHighlight, CbShowFilenameMarks, DIF_DISABLE);
	Builder.LinkFlags(CbHighlight, edShowFilenameMarksIndent, DIF_DISABLE);

    /* column sizes section */

    Builder.AddSeparator(Msg::DirSettingsTitle);

    Builder.AddText(Msg::DirSettingsHint);
    Builder.AddText(Msg::DirSettingsShowAs, false);

	DialogBuilderListItemWide DSStyleListItems[4];
	wchar_t tmp1[48 * 4], tmp2[48*4];
	memset(tmp1, 0, sizeof(tmp1));
	for (size_t i = 0; i < 4; i++) {
		swprintf(tmp1 + i * 48, 48, L"%-10.10S | %-10.10S | %-10.10S", DirNames[i].CPtr(), DirUpNames[i].CPtr(), SymLinkNames[i].CPtr());
		DSStyleListItems[i].Text = tmp1 + i * 48;
		DSStyleListItems[i].ItemValue = i;
	}
	Builder.AddComboBox(&dircfg_data.DirNameStyle, 49, DSStyleListItems, ARRAYSIZE(DSStyleListItems));

	Builder.AddCheckbox(Msg::DirSettingsCenter, &dircfg_data.bCentered);
	auto cbSurr = Builder.AddCheckbox(Msg::DirSettingsSurround, &dircfg_data.bSurr, false);

	DialogBuilderListItemWide DSurListItems[4];
	memset(tmp2, 0, sizeof(tmp2));
	for (size_t i = 0; i < 4; i++) {
		swprintf(tmp2 + i * 16, 16, L"%C%S%C", surdircharleft[i], DirNames[dircfg_data.DirNameStyle].CPtr(), surdircharright[i]);
		DSurListItems[i].Text = tmp2 + i * 16;
		DSurListItems[i].ItemValue = i;
	}
	auto cSurr = Builder.AddComboBox(&dircfg_data.SurrIndex, 17, DSurListItems, ARRAYSIZE(DSurListItems));
	Builder.LinkFlags(cbSurr, cSurr, DIF_DISABLE);

	Builder.AddText(Msg::DirSettingsWidthText, false);
	FarLangMsg DirWidthOptions[] = {Msg::DirSettingsWidthCombo0, Msg::DirSettingsWidthCombo1};
	Builder.AddRadioButtonsHorz((int*)&Opt.DirNameStyleColumnWidthAlways, ARRAYSIZE(DirWidthOptions), DirWidthOptions);

	Builder.AddText(Msg::DirSettingsSymlinkText, false);
	FarLangMsg SymLinkOptions[] = {Msg::DirSettingsSymlinkRadio1, Msg::DirSettingsSymlinkRadio2};
	Builder.AddRadioButtonsHorz((int*)&Opt.ShowSymlinkSize, ARRAYSIZE(SymLinkOptions), SymLinkOptions);

	Builder.AddText(Msg::DirSettingsSymlinkSizeHint);

    /* command line settings */

	Builder.AddSeparator(Msg::ConfigCmdlineTitle);

	DialogBuilderListItem CMWListItems[] = {
			{Msg::ConfigCmdlineWaitKeypress_Never,   0},
			{Msg::ConfigCmdlineWaitKeypress_OnError, 1},
			{Msg::ConfigCmdlineWaitKeypress_Always,  2},
	};

	Builder.StartColumns();

	Builder.AddCheckbox(Msg::ConfigCmdlineEditBlock, &Opt.CmdLine.EditBlock);
	Builder.AddCheckbox(Msg::ConfigCmdlineSplitter, &Opt.CmdLine.Splitter);
	Builder.AddText(Msg::ConfigCmdlineVTLogLimit, false);
	Builder.AddIntEditField(&Opt.CmdLine.VTLogLimit, 8);

	auto UsePromptFormat = Builder.AddCheckbox(Msg::ConfigCmdlineUsePromptFormat, &Opt.CmdLine.UsePromptFormat, false);
	auto PromptFormat = Builder.AddEditField(&Opt.CmdLine.strPromptFormat, 19);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	Builder.ColumnBreak();

	Builder.AddCheckbox(Msg::ConfigCmdlineDelRemovesBlocks, &Opt.CmdLine.DelRemovesBlocks);
	Builder.AddCheckbox(Msg::ConfigShowStartupBanner, &Opt.ShowStartupBanner);
	Builder.AddText(Msg::ConfigCmdlineWaitKeypress, false);
	Builder.AddComboBox((int *)&Opt.CmdLine.WaitKeypress, 20, CMWListItems, ARRAYSIZE(CMWListItems),
		DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE);

	auto UseShell = Builder.AddCheckbox(Msg::ConfigCmdlineUseShell, &Opt.CmdLine.UseShell, false);
	auto Shell = Builder.AddEditField(&Opt.CmdLine.strShell, 19);
	Builder.LinkFlags(UseShell, Shell, DIF_DISABLE);

	Builder.EndColumns();

    /* viewer settings */

	Builder.AddSeparator(Msg::ViewConfigTitle);

	Builder.AddCheckbox(Msg::ViewConfigExternalF3, &Opt.ViOpt.UseExternalViewer);
	Builder.AddText(Msg::ViewConfigExternalCommand, false);
	Builder.AddEditField(&Opt.strExternalViewer, 40, L"ExternalViewer", DIF_EDITPATH);

	Builder.AddSeparator(Msg::ViewConfigInternal);

	Builder.AddIntEditField(&Opt.ViOpt.TabSize, 3, 0, false);
	Builder.AddText(Msg::ViewConfigTabSize);

	Builder.AddText(Msg::ViewConfigDefaultCodePage, false);
	Builder.AddCodePagesBox(&Opt.ViOpt.DefaultCodePage, 15, false, false);
	Builder.AddCheckbox(Msg::ViewAutoDetectCodePage, &Opt.ViOpt.AutoDetectCodePage);

	Builder.StartColumns();

	Builder.AddCheckbox(Msg::ViewConfigPersistentSelection, &Opt.ViOpt.PersistentBlocks);
	auto SavePos = Builder.AddCheckbox(Msg::ViewConfigSavePos, &Opt.ViOpt.SavePos, false);
	auto SaveShortPos = Builder.AddCheckbox(Msg::ViewConfigSaveShortPos, &Opt.ViOpt.SaveShortPos);
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(Msg::ViewClickableURLs, &Opt.ViOpt.ClickableURLs);

	Builder.ColumnBreak();

	Builder.AddCheckbox(Msg::ViewShowKeyBar, &Opt.ViOpt.ShowKeyBar);
	Builder.AddCheckbox(Msg::ViewConfigArrows, &Opt.ViOpt.ShowArrows);
	Builder.AddCheckbox(Msg::ViewConfigScrollbar, &Opt.ViOpt.ShowScrollbar);
	Builder.AddCheckbox(Msg::ViewShowTitleBar, &Opt.ViOpt.ShowTitleBar);
	Builder.AddCheckbox(Msg::ViewShowMenuBar, &Opt.ViOpt.ShowMenuBar);
	
	Builder.EndColumns();

	/* editor settings */

	Builder.AddSeparator(Msg::EditConfigTitle);

	Builder.AddCheckbox(Msg::EditConfigEditorF4, &Opt.EdOpt.UseExternalEditor);
	Builder.AddText(Msg::EditConfigEditorCommand, false);
	Builder.AddEditField(&Opt.strExternalEditor, 40, L"ExternalEditor", DIF_EDITPATH);
	
	Builder.AddSeparator(Msg::EditConfigInternal);

	Builder.AddIntEditField(&Opt.EdOpt.TabSize, 3,	0, false );
	Builder.AddText(Msg::EditConfigTabSize, false);
	Builder.AddText(Msg::EditConfigExpandTabsTitle, false);
	DialogBuilderListItem ExpandTabsItems[] = {
		{Msg::EditConfigDoNotExpandTabs,        EXPAND_NOTABS },
		{Msg::EditConfigExpandTabs,             EXPAND_NEWTABS},
		{Msg::EditConfigConvertAllTabsToSpaces, EXPAND_ALLTABS}
	};
	Builder.AddComboBox(&Opt.EdOpt.ExpandTabs, 24, ExpandTabsItems, 3,
		DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE );

	Builder.AddText(Msg::EditConfigDefaultCodePage, false);
	Builder.AddCodePagesBox(&Opt.EdOpt.DefaultCodePage, 15, false, false);
	Builder.AddCheckbox(Msg::EditAutoDetectCodePage, &Opt.EdOpt.AutoDetectCodePage);
	Builder.AddCheckbox(Msg::EditConfigAutoIndent, &Opt.EdOpt.AutoIndent);
	Builder.AddCheckbox(Msg::EditWordWrap, &Opt.EdOpt.WordWrap);

	Builder.AddEmptyLine();

	Builder.StartColumns();

	Builder.AddCheckbox(Msg::EditConfigPersistentBlocks, &Opt.EdOpt.PersistentBlocks);

	SavePos = Builder.AddCheckbox(Msg::EditConfigSavePos, &Opt.EdOpt.SavePos, false);
	SaveShortPos = Builder.AddCheckbox(Msg::EditConfigSaveShortPos, &Opt.EdOpt.SaveShortPos);
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	
	Builder.AddCheckbox(Msg::EditConfigDelRemovesBlocks, &Opt.EdOpt.DelRemovesBlocks);
	Builder.AddCheckbox(Msg::EditCursorBeyondEnd, &Opt.EdOpt.CursorBeyondEOL);
	Builder.AddCheckbox(Msg::EditConfigPickUpWord, &Opt.EdOpt.SearchPickUpWord);
	Builder.AddCheckbox(Msg::EditUseEditorConfigOrg, &Opt.EdOpt.UseEditorConfigOrg);
	Builder.AddCheckbox(Msg::EditLockROFileModification, &Opt.EdOpt.ReadOnlyLock, 1);
	Builder.AddCheckbox(Msg::EditWarningBeforeOpenROFile, &Opt.EdOpt.ReadOnlyLock, 2);

	Builder.ColumnBreak();

	Builder.AddCheckbox(Msg::EditShowWhiteSpace, &Opt.EdOpt.ShowWhiteSpace);
	Builder.AddCheckbox(Msg::EditShowKeyBar, &Opt.EdOpt.ShowKeyBar);
	Builder.AddCheckbox(Msg::EditShowLineNumbers, &Opt.EdOpt.ShowLineNumbers);
	Builder.AddCheckbox(Msg::EditConfigScrollbar, &Opt.EdOpt.ShowScrollBar);
	Builder.AddCheckbox(Msg::EditShowTitleBar, &Opt.EdOpt.ShowTitleBar);
	Builder.AddCheckbox(Msg::EditShowMenuBar, &Opt.EdOpt.ShowMenuBar);

	Builder.EndColumns();

    /* plugins settings */

	Builder.AddSeparator(Msg::PluginsManagerSettingsTitle);

	Builder.AddText(Msg::PluginsManagerPersonalPath, false);
	Builder.AddEditField(&Opt.LoadPlug.strPersonalPluginsPath, 45, L"PersPath", DIF_EDITPATH);
	Builder.AddCheckbox(Msg::PluginsManagerScanSymlinks, &Opt.LoadPlug.ScanSymlinks);

	Builder.AddSeparator(Msg::PluginConfirmationTitle);
	Builder.StartColumns();
	auto ConfirmOFP = Builder.AddCheckbox(Msg::PluginsManagerOFP, &Opt.PluginConfirm.OpenFilePlugin);
	ConfirmOFP->Flags|= DIF_3STATE;
	Builder.AddCheckbox(Msg::PluginsManagerSFL, &Opt.PluginConfirm.SetFindList);
	Builder.ColumnBreak();
	Builder.AddCheckbox(Msg::PluginsManagerStdAssoc, &Opt.PluginConfirm.StandardAssociation);
	Builder.AddCheckbox(Msg::PluginsManagerPF, &Opt.PluginConfirm.Prefix);
	Builder.ColumnBreak();
	Builder.AddCheckbox(Msg::PluginsManagerEvenOne, &Opt.PluginConfirm.EvenIfOnlyOnePlugin);
	Builder.EndColumns();

    /* folder file-id settings */
	Builder.AddSeparator(Msg::SetFolderInfoTitle);

	Builder.AddText(Msg::SetFolderInfoNames);
	Builder.AddEditField(&Opt.InfoPanel.strFolderInfoFiles, 65);

    /* file-id.diz settings */

	Builder.AddSeparator(Msg::CfgDizTitle);

	Builder.AddText(Msg::CfgDizListNames, false);
	Builder.AddEditField(&Opt.Diz.strListNames, 45);

	Builder.AddEmptyLine();

	Builder.AddCheckbox(Msg::CfgDizSetHidden, &Opt.Diz.SetHidden, false);
	Builder.AddCheckbox(Msg::CfgDizROUpdate, &Opt.Diz.ROUpdate);
	Builder.AddText(Msg::CfgDizStartPos, false);
	Builder.AddIntEditField(&Opt.Diz.StartPos, 2);

	Builder.AddEmptyLine();

	static FarLangMsg DizOptions[] = {Msg::CfgDizNotUpdate, Msg::CfgDizUpdateIfDisplayed,
			Msg::CfgDizAlwaysUpdate};
	Builder.AddRadioButtonsHorz(&Opt.Diz.UpdateMode, 3, DizOptions);
	Builder.AddEmptyLine();

	Builder.AddCheckbox(Msg::CfgDizAnsiByDefault, &Opt.Diz.AnsiByDefault, false);
	Builder.AddCheckbox(Msg::CfgDizSaveInUTF, &Opt.Diz.SaveInUTF);

	Builder.AddOKCancel(&OKButtonID, &CancelButtonID);

	int clicked_id = -1;
	bool cancelled = !Builder.ShowDialog(&clicked_id);
	return cancelled ? CancelButtonID : clicked_id;
}

void AllSystemSettings()
{
   	supported_tweaks = ApplyConsoleTweaks();
	DateFormatIndex = GetDateFormat(); //Opt.DateFormat
	strDateSeparator = GetDateSeparator();
	strTimeSeparator = GetTimeSeparator();
	strDecimalSeparator = GetDecimalSeparator();

	int oldUseShell = Opt.CmdLine.UseShell;
	FARString oldShell = FARString(Opt.CmdLine.strShell);

	cmdHist_optExecPanel = !(Opt.ExcludeCmdHistory & EXCLUDECMDHISTORY_NOTPANEL);
	cmdHist_optExecCmdLine = !(Opt.ExcludeCmdHistory & EXCLUDECMDHISTORY_NOTCMDLINE);

	AutoUpdate = (Opt.AutoUpdateLimit);
	TreeScanDepthEnabled = (Opt.Tree.ScanDepthEnabled);
	oldColorCC = Opt.Dialogs.EnforceColorCorrection;

	cmdHist_optAssSys = !(Opt.ExcludeCmdHistory & EXCLUDECMDHISTORY_NOTWINASS);
	cmdHist_optAssFar = !(Opt.ExcludeCmdHistory & EXCLUDECMDHISTORY_NOTFARASS);

	dircfg_data.DirNameStyle = Opt.DirNameStyle & 3;
	dircfg_data.SurrIndex = (Opt.DirNameStyle >> 2) & 3;
	dircfg_data.bCentered = (Opt.DirNameStyle & DIRNAME_STYLE_CENTERED);
	dircfg_data.bSurr = (Opt.DirNameStyle & DIRNAME_STYLE_SURR_CH);

	for (int i = 0; i < (int)xlats.size(); ++i) {
		if (Opt.XLat.XLat == xlats[i]) {
			SelectedXLat = i;
		}
		XLatItems.emplace_back(DialogBuilderListItem{FarLangMsg{::Lang.InternMsg(xlats[i].c_str())}, i});
	}

	for(;;) {
	
		int code = Do_AllSystemSettings();

		if (code == OKButtonID) {
    		ConfigOptSaveAutoOptions();
    		SanitizeHistoryCounts();
    		ApplySudoConfiguration();

    		if (size_t(SelectedXLat) < xlats.size()) {
    			Opt.XLat.XLat = xlats[SelectedXLat];
    		}
    		ApplyConsoleTweaks();
    		XlatReinit();

			if (!AutoUpdate)
				Opt.AutoUpdateLimit = 0;

			SanitizeIndentationCounts();

			if (Opt.CMOpt.CopyTimeRule)
				Opt.CMOpt.CopyTimeRule = 3;

			Opt.DateFormat = DateFormatIndex;
			Opt.strDateSeparator = strDateSeparator;
			Opt.strTimeSeparator = strTimeSeparator;
			Opt.strDecimalSeparator = strDecimalSeparator;
			ConvertDate_ResetInit();

			SetFarConsoleMode();

       		if (Opt.Dialogs.EnforceColorCorrection != oldColorCC) {
       			FarColors::FARColors.Set();
       		}

			WINPORT(SetConsoleCursorBlinkTime)(NULL, Opt.CursorBlinkTime);

    		Opt.ExcludeCmdHistory
    			= (cmdHist_optAssSys ? 0 : EXCLUDECMDHISTORY_NOTWINASS)
    			| (cmdHist_optAssFar ? 0 : EXCLUDECMDHISTORY_NOTFARASS)
    			| (cmdHist_optExecPanel ? 0 : EXCLUDECMDHISTORY_NOTPANEL)
    			| (cmdHist_optExecCmdLine ? 0 : EXCLUDECMDHISTORY_NOTCMDLINE);

    		if (Opt.Dialogs.MouseButton) Opt.Dialogs.MouseButton = 0xFFFF;

			CtrlObject->CmdLine->SetPersistentBlocks(Opt.CmdLine.EditBlock);
			CtrlObject->CmdLine->SetDelRemovesBlocks(Opt.CmdLine.DelRemovesBlocks);
			CtrlObject->CmdLine->SetAutoComplete(Opt.CmdLine.AutoComplete);

			if (Opt.CmdLine.UseShell != oldUseShell || Opt.CmdLine.strShell != oldShell) VTShell_Shutdown();

    		Opt.DirNameStyle = dircfg_data.DirNameStyle;
    		Opt.DirNameStyle |= (dircfg_data.SurrIndex << 2);
    		Opt.DirNameStyle |= DIRNAME_STYLE_CENTERED * dircfg_data.bCentered;
    		Opt.DirNameStyle |= DIRNAME_STYLE_SURR_CH * dircfg_data.bSurr;
    		UpdateDefaultColumnTypeWidths( );

			if (Opt.ViOpt.TabSize < 1 || Opt.ViOpt.TabSize > 512)
				Opt.ViOpt.TabSize = 8;
			if (Opt.EdOpt.TabSize < 1 || Opt.EdOpt.TabSize > 512)
				Opt.EdOpt.TabSize = 8;

			// FrameManager->RefreshFrame();
			CtrlObject->Cp()->ActiveTab().LeftPanel->Update(UPDATE_KEEP_SELECTION);
			CtrlObject->Cp()->ActiveTab().RightPanel->Update(UPDATE_KEEP_SELECTION);
			CtrlObject->Cp()->SetScreenPosition();
			CtrlObject->Cp()->Redraw();

			break;
		}
		else if (code == CancelButtonID)
			break;
		else if (code == ChangeFontID)
			WINPORT(ConsoleChangeFont)();
		else if (code == DateTimeDefaultID) {
			DateFormatIndex = GetDateFormatDefault();
			strDateSeparator = GetDateSeparatorDefault();
			strTimeSeparator = GetTimeSeparatorDefault();
			strDecimalSeparator = GetDecimalSeparatorDefault();
		}
		else if (code == DateTimeCurrentID) {
			DateFormatIndex = GetDateFormat();
			strDateSeparator = GetDateSeparator();
			strTimeSeparator = GetTimeSeparator();
			strDecimalSeparator = GetDecimalSeparator();
		}
		else if (code == DateTimeFromSystemID) {
			// parcing part of possible https://help.gnome.org/users/gthumb/stable/gthumb-date-formats.html
			std::string::size_type
					pos_date_2 = std::string::npos,
					pos_day = std::string::npos,
					pos_month = std::string::npos,
					pos_year = std::string::npos,
					pos_time_2 = std::string::npos;
			size_t length_decimal;
			std::string format_date = nl_langinfo(D_FMT);
			std::string format_time = nl_langinfo(T_FMT);
			std::string format_decimal = nl_langinfo(RADIXCHAR/*DECIMAL_POINT*/);
			if (format_date=="%D") { // %D Equivalent to %m/%d/%y
				DateFormatIndex = 0;
				strDateSeparator = "/";
				pos_date_2 = 0; // for not error in message
			}
			else if (format_date=="%F") { // %F Equivalent to %Y-%m-%d
				DateFormatIndex = 2;
				strDateSeparator = "-";
				pos_date_2 = 0; // for not error in message
			}
			else if (format_date.length() >= 8) {
				static const char *codes_day[] = { "%d", "%e", "%Ed", "%Ee", "%Od", "%Oe" };
				for (const auto &code : codes_day) {
					pos_day = format_date.find(code);
					if (pos_day != std::string::npos)
						break;
				}
				static const char *codes_month[] = {
					"%m", "%B", "%b", "%h", "%Em", "%EB", "%Eb", "%Eh", "%Om", "%OB", "%Ob", "%Oh" };
				for (const auto &code : codes_month) {
					pos_month = format_date.find(code);
					if (pos_month != std::string::npos)
						break;
				}
				static const char *codes_year[] = {
					"%Y", "%y", "%G", "%g", "%EY", "%Ey", "%EG", "%Eg", "%OY", "%Oy", "%OG", "%Og" };
				for (const auto &code : codes_year) {
					pos_year = format_date.find(code);
					if (pos_year != std::string::npos)
						break;
				}
				if (pos_day != std::string::npos && pos_month != std::string::npos && pos_year != std::string::npos) {
					if (pos_day < pos_month && pos_month < pos_year) // day-month-year
					{ DateFormatIndex = 1; pos_date_2 = pos_month; strDateSeparator = format_date[pos_date_2-1]; }
					else if (pos_year < pos_month && pos_month < pos_day) // year-month-day
					{ DateFormatIndex = 2; pos_date_2 = pos_month; strDateSeparator = format_date[pos_date_2-1]; }
					else if (pos_month < pos_day  && pos_month < pos_year) // month-day-year
					{ DateFormatIndex = 0; pos_date_2 = pos_day;   strDateSeparator = format_date[pos_date_2-1]; }
				}
			}

			if (format_time=="%T") { // %T The time in 24-hour notation (%H:%M:%S).
				strTimeSeparator = ":";
				pos_time_2 = 0; // for not error in message
			}
			else {
				pos_day = format_time.find('%');
				if (pos_day != std::string::npos) {
					pos_time_2 = format_time.find('%', pos_day+2);
					if (pos_time_2 != std::string::npos)
						strTimeSeparator = format_time[pos_time_2-1];
				}
			}

			length_decimal = format_decimal.length();
			if (length_decimal > 0)
				strDecimalSeparator = format_decimal[0];

			ExMessager em;
			em.Add(L"From system locale");
			em.AddFormat(L"Date format from locale:      \"%s\"", format_date.c_str());
			em.AddFormat(L"  Date order:        %s (order %d)",
				(pos_date_2 != std::string::npos) ? "imported" : "unchanged",
				DateFormatIndex);
			em.AddFormat(L"  Date separator:    %s (\'%ls\')",
				(pos_date_2 != std::string::npos) ? "imported" : "unchanged",
				strDateSeparator.CPtr());
			em.AddFormat(L"Time format from locale:      \"%s\"", format_time.c_str());
			em.AddFormat(L"  Time separator:    %s (\'%ls\')",
				(pos_time_2 != std::string::npos) ? "imported" : "unchanged",
				 strTimeSeparator.CPtr());
			em.AddFormat(L"DecimalSeparator from locale: \"%s\"", format_decimal.c_str());
			em.AddFormat(L"  Decimal separator: %s (\'%ls\')",
				length_decimal>0 ? "imported" : "unchanged",
				strDecimalSeparator.CPtr());
			em.Add(Msg::Ok);
			em.Show(MSG_LEFTALIGN |
					( (pos_date_2 == std::string::npos
						|| pos_time_2 == std::string::npos
						|| length_decimal<=0 )
					? MSG_WARNING : 0),
				1);
		}
	}
}
