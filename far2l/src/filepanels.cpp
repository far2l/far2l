/*
filepanels.cpp

Файловые панели
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

#include "filepanels.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "lang.hpp"
#include "ctrlobj.hpp"
#include "filelist.hpp"
#include "rdrwdsk.hpp"
#include "cmdline.hpp"
#include "treelist.hpp"
#include "qview.hpp"
#include "infolist.hpp"
#include "help.hpp"
#include "filefilter.hpp"
#include "filediff.hpp"
#include "findfile.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "syslog.hpp"
#include "options.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "interf.hpp"
#include "scrbuf.hpp"

FilePanels::FilePanels()
{
	_OT(SysLog(L"[%p] FilePanels::FilePanels()", this));
	MacroMode = MACRO_SHELL;
	KeyBarVisible = Opt.ShowKeyBar;

	tabs.push_back(DoublePanel());

	ActiveTab().LeftPanel  = CreatePanel(Opt.LeftPanel.Type);
	ActiveTab().RightPanel = CreatePanel(Opt.RightPanel.Type);

	// SetKeyBar(&MainKeyBar);
	//_D(SysLog(L"MainKeyBar=0x%p",&MainKeyBar));
}

static void PrepareOptFolder(FARString &strSrc)
{
	if (strSrc.IsEmpty()) {
		strSrc = DefaultPanelInitialDirectory();
	} else {
		apiExpandEnvironmentStrings(strSrc, strSrc);
	}
}

void FilePanels::deactivatePanelsInTab(DoublePanel& activeTab)
{
	activeTab.ActiveVisible = activeTab.ActivePanel->IsVisible();
	activeTab.PassiveVisible = activeTab.PassivePanel()->IsVisible();

	activeTab.PassivePanel()->Hide();
	activeTab.ActivePanel->Hide();
}

void FilePanels::activatePanelsInTab(DoublePanel& activeTab)
{
	if (activeTab.ActiveVisible || activeTab.PassiveVisible)
		SetPanelPositions(activeTab,
			FileList::IsModeFullScreen(Opt.LeftPanel.ViewMode),
			FileList::IsModeFullScreen(Opt.RightPanel.ViewMode),
			Opt.PanelsDisposition);

	// show means right order -> active first
	if(activeTab.ActiveVisible)  activeTab.ActivePanel->Show();
	if(activeTab.PassiveVisible) activeTab.PassivePanel()->Show();

	if(activeTab.ActiveVisible) {
		activeTab.ActivePanel->Update(UPDATE_KEEP_SELECTION | UPDATE_CAN_BE_ANNOYING);
	}

	if(activeTab.PassiveVisible) {
		activeTab.PassivePanel()->Update(UPDATE_KEEP_SELECTION | UPDATE_CAN_BE_ANNOYING);
	}

	FARString dir;
	activeTab.ActivePanel->GetCurDir(dir);
	CtrlObject->CmdLine->SetCurDir(dir);

    /*
	if (!activeTab.ActivePanel->IsVisible() || !activeTab.ActivePanel->GetFocus()) {
		if (activeTab.ActivePanel == activeTab.RightPanel)
			activeTab.LeftPanel->SetFocus();
		else
			activeTab.RightPanel->SetFocus();
	}*/
	UpdateCmdLineVisibility(activeTab);

	// FrameManager->RefreshFrame();
}

void FilePanels::Init(DoublePanel& activeTab)
{
	SetPanelPositions(activeTab,
		FileList::IsModeFullScreen(Opt.LeftPanel.ViewMode),
		FileList::IsModeFullScreen(Opt.RightPanel.ViewMode),
		Opt.PanelsDisposition);


	activeTab.LeftPanel->SetViewMode(Opt.LeftPanel.ViewMode);
	activeTab.RightPanel->SetViewMode(Opt.RightPanel.ViewMode);
	activeTab.LeftPanel->SetSortMode(std::min(std::max(Opt.LeftPanel.SortMode, 0), (int)MAX_PANEL_SORT_MODE));
	activeTab.RightPanel->SetSortMode(std::min(std::max(Opt.RightPanel.SortMode, 0), (int)MAX_PANEL_SORT_MODE));
	activeTab.LeftPanel->SetNumericSort(Opt.LeftPanel.NumericSort);
	activeTab.RightPanel->SetNumericSort(Opt.RightPanel.NumericSort);
	activeTab.LeftPanel->SetCaseSensitiveSort(Opt.LeftPanel.CaseSensitiveSort);
	activeTab.RightPanel->SetCaseSensitiveSort(Opt.RightPanel.CaseSensitiveSort);
	activeTab.LeftPanel->SetSortOrder(Opt.LeftPanel.SortOrder);
	activeTab.RightPanel->SetSortOrder(Opt.RightPanel.SortOrder);
	activeTab.LeftPanel->SetSortGroups(Opt.LeftPanel.SortGroups);
	activeTab.RightPanel->SetSortGroups(Opt.RightPanel.SortGroups);
	activeTab.LeftPanel->SetSelectedFirstMode(Opt.LeftSelectedFirst);
	activeTab.RightPanel->SetSelectedFirstMode(Opt.RightSelectedFirst);
	activeTab.LeftPanel->SetDirectoriesFirst(Opt.LeftPanel.DirectoriesFirst);
	activeTab.RightPanel->SetDirectoriesFirst(Opt.RightPanel.DirectoriesFirst);
	activeTab.LeftPanel->SetExecutablesFirst(Opt.LeftPanel.ExecutablesFirst);
	activeTab.RightPanel->SetExecutablesFirst(Opt.RightPanel.ExecutablesFirst);

	// if(&activeTab == &ActiveTab()) SetCanLoseFocus(TRUE);
	Panel *PassivePanel = nullptr;
	int PassiveIsLeftFlag = TRUE;

	if (Opt.LeftPanel.Focus) {
		activeTab.ActivePanel = activeTab.LeftPanel;
		PassivePanel = activeTab.RightPanel;
		PassiveIsLeftFlag = FALSE;
	} else {
		activeTab.ActivePanel = activeTab.RightPanel;
		PassivePanel = activeTab.LeftPanel;
		PassiveIsLeftFlag = TRUE;
	}

	// activeTab.ActivePanel->SetFocus();

	PrepareOptFolder(Opt.strLeftFolder);
	PrepareOptFolder(Opt.strRightFolder);

	if (Opt.AutoSaveSetup || !Opt.SetupArgv) {
		activeTab.LeftPanel->InitCurDir(Opt.strLeftFolder);
		activeTab.RightPanel->InitCurDir(Opt.strRightFolder);
	}

	if (!Opt.AutoSaveSetup) {
		if (Opt.SetupArgv >= 1) {
			if (activeTab.ActivePanel == activeTab.RightPanel) {
				activeTab.RightPanel->InitCurDir(Opt.strRightFolder);
			} else {
				activeTab.LeftPanel->InitCurDir(Opt.strLeftFolder);
			}

			if (Opt.SetupArgv == 2) {
				if (activeTab.ActivePanel == activeTab.LeftPanel) {
					activeTab.RightPanel->InitCurDir(Opt.strRightFolder);
				} else {
					activeTab.LeftPanel->InitCurDir(Opt.strLeftFolder);
				}
			}
		}

		const wchar_t *PassiveFolder = PassiveIsLeftFlag ? Opt.strLeftFolder : Opt.strRightFolder;

		if (Opt.SetupArgv < 2 && *PassiveFolder) {
			PassivePanel->InitCurDir(PassiveFolder);
		}
	}

#if 1

	//! Вначале "показываем" пассивную панель
	if (PassiveIsLeftFlag) {
		if (Opt.LeftPanel.Visible) {
			activeTab.LeftPanel->Show();
		}

		if (Opt.RightPanel.Visible) {
			activeTab.RightPanel->Show();
		}
	} else {
		if (Opt.RightPanel.Visible) {
			activeTab.RightPanel->Show();
		}

		if (Opt.LeftPanel.Visible) {
			activeTab.LeftPanel->Show();
		}
	}

#endif

	// при понашенных панелях не забыть бы выставить корректно каталог в CmdLine
	if (!Opt.RightPanel.Visible && !Opt.LeftPanel.Visible) {
		CtrlObject->CmdLine->SetCurDir(PassiveIsLeftFlag ? Opt.strRightFolder : Opt.strLeftFolder);
	}

}

void FilePanels::Init()
{
	Init(ActiveTab());
	// todo: if many tabs were saved earler, it is a good place to restore

	if (!Opt.strLeftFolderList.IsEmpty() && !Opt.strRightFolderList.IsEmpty()) {
		wchar_t* ldup = wcsdup(Opt.strLeftFolderList.CPtr());  // separator is `|`
		wchar_t* rdup = wcsdup(Opt.strRightFolderList.CPtr());
		int switchTo = Opt.activeTabNo;

		int foundTabs = 0;
		wchar_t* left = ldup;
		wchar_t* right = rdup;
		for(;;) {
			wchar_t* ql = wcschr(left, '|');
			wchar_t* qr = wcschr(right, '|');

			if(ql) *ql = 0;
			if(qr) *qr = 0;

			if(foundTabs > 0) {
				AppendNewTab();
			}

			tabs[TabActive].LeftPanel->InitCurDir(left);
			tabs[TabActive].RightPanel->InitCurDir(right);

			if (!ql || !qr) break;

			left = ql + 1;
			right = qr + 1;
            ++foundTabs;
		}

		SwitchActiveTabTo(switchTo);

		free(ldup);
		free(rdup);
	}

	ActiveTab().ActivePanel->SetFocus();

	SetKeyBar(&MainKeyBar);
	MainKeyBar.SetOwner(this);
	SetCanLoseFocus(TRUE);
}

void FilePanels::destroyPanelsGracefully(DoublePanel& activeTab) {
	if (activeTab.LastLeftFilePanel != activeTab.LeftPanel && activeTab.LastLeftFilePanel != activeTab.RightPanel)
		DeletePanel(activeTab, activeTab.LastLeftFilePanel);

	if (activeTab.LastRightFilePanel != activeTab.LeftPanel && activeTab.LastRightFilePanel != activeTab.RightPanel)
		DeletePanel(activeTab, activeTab.LastRightFilePanel);

	DeletePanel(activeTab, activeTab.LeftPanel);
	activeTab.LeftPanel = nullptr;
	DeletePanel(activeTab, activeTab.RightPanel);
	activeTab.RightPanel = nullptr;
}

FilePanels::~FilePanels()
{
	_OT(SysLog(L"[%p] FilePanels::~FilePanels()", this));

	for(size_t i = 0; i < tabs.size(); ++i) {
		DoublePanel& activeTab = tabs[i];
		destroyPanelsGracefully(activeTab);
	}
}

void FilePanels::UpdateCmdLineVisibility(bool repos)
{
	UpdateCmdLineVisibility(ActiveTab(), repos);
}

void FilePanels::UpdateCmdLineVisibility(DoublePanel& activeTab, bool repos)
{
	int left_x1, left_x2, left_y1, left_y2;
	int right_x1, right_x2, right_y1, right_y2;
	int cl_x1, cl_x2, cl_y1, cl_y2;
	bool cl_visible = CtrlObject->CmdLine->IsVisible(), new_cl_visible;

	ActiveTab().LeftPanel->GetPosition(left_x1, left_y1, left_x2, left_y2);
	ActiveTab().RightPanel->GetPosition(right_x1, right_y1, right_x2, right_y2);
	CtrlObject->CmdLine->GetPosition(cl_x1, cl_y1, cl_x2, cl_y2);

	const bool left_overlap = ActiveTab().LeftPanel->IsVisible() && left_y2 + Opt.ShowKeyBar >= ScrY;
	const bool right_overlap = ActiveTab().RightPanel->IsVisible() && right_y2 + Opt.ShowKeyBar >= ScrY;

	if (!Opt.PanelsDisposition)
		new_cl_visible = !left_overlap || !right_overlap;
	else
		new_cl_visible = !left_overlap && !right_overlap;

	const int extra = CtrlObject->CmdLine->GetExtraLines();
	int new_cl_x1 = 0, new_cl_x2 = ScrX - 1;
	int new_cl_y2 = ScrY - (Opt.ShowKeyBar);
	int new_cl_y1 = new_cl_y2 - extra;
	if (new_cl_visible) {
		if (left_overlap) {
			new_cl_x1 = right_x1;
		} else if (right_overlap) {
			new_cl_x2 = left_x2 - 1;
		}
	}
	bool cl_repos = (new_cl_x1 != cl_x1 || new_cl_x2 != cl_x2 || new_cl_y1 != cl_y1 || new_cl_y2 != cl_y2);
	if (cl_visible != new_cl_visible) {
		CtrlObject->CmdLine->SetVisible(new_cl_visible);
	}
	if (cl_repos || repos) {
		CtrlObject->CmdLine->SetPosition(new_cl_x1, new_cl_y1, new_cl_x2, new_cl_y2);
	}

	if (cl_visible != new_cl_visible || cl_repos || repos) {
		if (new_cl_visible) {
			CtrlObject->CmdLine->Redraw();
		}
		if (cl_visible != new_cl_visible || cl_repos) {
			if (ActiveTab().LeftPanel->IsVisible()) {
				ActiveTab().LeftPanel->Redraw();
			}
			if (ActiveTab().RightPanel->IsVisible()) {
				ActiveTab().RightPanel->Redraw();
			}
		}
	}
}

void FilePanels::SetPanelPositions(int LeftFullScreen, int RightFullScreen, int Disposition)
{
	SetPanelPositions(ActiveTab(), LeftFullScreen, RightFullScreen, Disposition);
}

void FilePanels::SetPanelPositions(DoublePanel& activeTab, int LeftFullScreen, int RightFullScreen, int Disposition)
{
	if (Disposition == 0) { /// vertical panels

		Opt.LeftHeightDecrement = Max(-1, Min(Opt.LeftHeightDecrement, ScrY - 7));
		Opt.RightHeightDecrement = Max(-1, Min(Opt.RightHeightDecrement, ScrY - 7));

		if (Opt.WidthDecrement < -(ScrX / 2 - 10))
			Opt.WidthDecrement = -(ScrX / 2 - 10);

		if (Opt.WidthDecrement > (ScrX / 2 - 10))
			Opt.WidthDecrement = (ScrX / 2 - 10);

		const int extra = CtrlObject && CtrlObject->CmdLine ? CtrlObject->CmdLine->GetExtraLines() : 0;
		const int LeftDecrement = extra ? std::max(Opt.LeftHeightDecrement, extra) : Opt.LeftHeightDecrement;
		const int RightDecrement = extra ? std::max(Opt.RightHeightDecrement, extra) : Opt.RightHeightDecrement;
		const int LeftY2 = ScrY - 1 - (Opt.ShowKeyBar) - LeftDecrement;
		const int RightY2 = ScrY - 1 - (Opt.ShowKeyBar) - RightDecrement;

		if (LeftFullScreen) {
			activeTab.LeftPanel->SetPosition(0, Opt.ShowMenuBar ? 2 : 1, ScrX, LeftY2);
			activeTab.LeftPanel->ViewSettings.FullScreen = 1;
		} else {
			activeTab.LeftPanel->SetPosition(0, Opt.ShowMenuBar ? 2 : 1, ScrX / 2 - Opt.WidthDecrement, LeftY2);
		}

		if (RightFullScreen) {
			activeTab.RightPanel->SetPosition(0, Opt.ShowMenuBar ? 2 : 1, ScrX, RightY2);
			activeTab.RightPanel->ViewSettings.FullScreen = 1;
		} else {
			activeTab.RightPanel->SetPosition(ScrX / 2 + 1 - Opt.WidthDecrement, Opt.ShowMenuBar ? 2 : 1, ScrX, RightY2);
		}
	}
	else if (Disposition == 1) { /// horizontal panels

		activeTab.LeftPanel->ViewSettings.FullScreen = 0;
		activeTab.RightPanel->ViewSettings.FullScreen = 0;

		Opt.LeftHeightDecrement = Max(-1, Min(Opt.LeftHeightDecrement, ScrY - 13));
		Opt.RightHeightDecrement = Max(-1, Min(Opt.RightHeightDecrement, ScrY - 13));
#if 0
		const bool bRightPanelVisible = activeTab.RightPanel->IsVisible();
		const bool bLeftPanelVisible = activeTab.LeftPanel->IsVisible();

		if (bRightPanelVisible)  {
			if (Opt.WidthDecrement < -((ScrY - Opt.LeftHeightDecrement) / 2 - 6))
				Opt.WidthDecrement = -((ScrY - Opt.LeftHeightDecrement) / 2 - 6);
		}
		else {
			if (Opt.WidthDecrement < -((ScrY - Opt.LeftHeightDecrement) / 2))
				Opt.WidthDecrement = -((ScrY - Opt.LeftHeightDecrement) / 2);
		}

		if (bLeftPanelVisible)  {
			if (Opt.WidthDecrement > ((ScrY - Opt.LeftHeightDecrement) / 2 - 6))
				Opt.WidthDecrement = ((ScrY - Opt.LeftHeightDecrement) / 2 - 6);
		}
		else {
			if (Opt.WidthDecrement > ((ScrY - Opt.LeftHeightDecrement) / 2))
				Opt.WidthDecrement = ((ScrY - Opt.LeftHeightDecrement) / 2);
		}
#else
		if (Opt.WidthDecrement < -((ScrY - Opt.LeftHeightDecrement) / 2 - 6))
			Opt.WidthDecrement = -((ScrY - Opt.LeftHeightDecrement) / 2 - 6);

		if (Opt.WidthDecrement > ((ScrY - Opt.LeftHeightDecrement) / 2 - 6))
			Opt.WidthDecrement = ((ScrY - Opt.LeftHeightDecrement) / 2 - 6);
#endif

		const int extra = CtrlObject && CtrlObject->CmdLine ? CtrlObject->CmdLine->GetExtraLines() : 0;
		const int LeftY2 = (ScrY - Opt.ShowMenuBar) / 2 - (Opt.ShowKeyBar) - Opt.LeftHeightDecrement / 2;
		const int RightDecrement = extra ? std::max(Opt.RightHeightDecrement, extra) : Opt.RightHeightDecrement;
		int RightY2 = ScrY - (Opt.ShowKeyBar) - 1 - RightDecrement;

#if 0
		if (LeftFullScreen) {
			activeTab.LeftPanel->SetPosition(0, Opt.ShowMenuBar ? 2 : 1, ScrX, LeftY2);
			activeTab.LeftPanel->ViewSettings.FullScreen = 1;
		} else {
			activeTab.LeftPanel->SetPosition(0, Opt.ShowMenuBar ? 2 : 1, ScrX, LeftY2 - Opt.WidthDecrement );
		}

		if (RightFullScreen) {
			activeTab.RightPanel->SetPosition(0, Opt.ShowMenuBar ? 2 : 1, ScrX, RightY2);
			activeTab.RightPanel->ViewSettings.FullScreen = 1;
		} else {
			if (RightY2 - 5 < LeftY2 + 1 - Opt.WidthDecrement)
				RightY2 = LeftY2 + 1 - Opt.WidthDecrement + 5;
			activeTab.RightPanel->SetPosition(0, LeftY2 + 1 - Opt.WidthDecrement, ScrX, RightY2);
		}
#else
		activeTab.LeftPanel->SetPosition(0, Opt.ShowMenuBar ? 2 : 1, ScrX, LeftY2 - Opt.WidthDecrement );

		if (RightY2 - 5 < LeftY2 + 1 - Opt.WidthDecrement)
			RightY2 = LeftY2 + 1 - Opt.WidthDecrement + 5;
		activeTab.RightPanel->SetPosition(0, LeftY2 + 1 - Opt.WidthDecrement, ScrX, RightY2);
#endif
	}

	UpdateCmdLineVisibility(activeTab, true);
}

void FilePanels::SetScreenPosition()
{
	SetScreenPosition(ActiveTab());
}

void FilePanels::SetScreenPosition(DoublePanel& activeTab)
{
	_OT(SysLog(L"[%p] FilePanels::SetScreenPosition() {%d, %d - %d, %d}", this, X1, Y1, X2, Y2));
	TopMenuBar.SetPosition(0, 0, ScrX, 0);
	TopTabBar.SetPosition(0, Opt.ShowMenuBar ? 1 : 0, ScrX, Opt.ShowMenuBar ? 1 : 0);
	MainKeyBar.SetPosition(0, ScrY, ScrX, ScrY);
	SetPanelPositions(activeTab, activeTab.LeftPanel->IsFullScreen(), activeTab.RightPanel->IsFullScreen(), Opt.PanelsDisposition);
	SetPosition(0, 0, ScrX, ScrY);
}

void FilePanels::RedrawKeyBar()
{
	ActiveTab().ActivePanel->UpdateKeyBar();
	MainKeyBar.Redraw();
}

Panel *FilePanels::CreatePanel(int Type)
{
	Panel *pResult = nullptr;

	switch (Type) {
		case FILE_PANEL:
			pResult = new FileList;
			break;
		case TREE_PANEL:
			pResult = new TreeList;
			break;
		case QVIEW_PANEL:
			pResult = new QuickView;
			break;
		case INFO_PANEL:
			pResult = new InfoList;
			break;
	}

	if (pResult)
		pResult->SetOwner(this);

	return pResult;
}

void FilePanels::DeletePanel(Panel *Deleted)
{
	DeletePanel(ActiveTab(), Deleted);
}

void FilePanels::DeletePanel(DoublePanel& activeTab, Panel *Deleted)
{
	if (!Deleted)
		return;

	if (Deleted == activeTab.LastLeftFilePanel)
		activeTab.LastLeftFilePanel = nullptr;

	if (Deleted == activeTab.LastRightFilePanel)
		activeTab.LastRightFilePanel = nullptr;

	delete Deleted;
}

int FilePanels::SetAnotherPanelFocus()
{
	int Ret = FALSE;

	if (ActiveTab().ActivePanel == ActiveTab().LeftPanel) {
		if (ActiveTab().RightPanel->IsVisible()) {
			ActiveTab().RightPanel->SetFocus();
			Ret = TRUE;
		}
	} else {
		if (ActiveTab().LeftPanel->IsVisible()) {
			ActiveTab().LeftPanel->SetFocus();
			Ret = TRUE;
		}
	}

	if (Ret) {
		SetTabNames();
		TopTabBar.Show();
	}

	return Ret;
}

int FilePanels::SwapPanels()
{
	int Ret = FALSE;	// это значит ни одна из панелей не видна

	if (ActiveTab().LeftPanel->IsVisible() || ActiveTab().RightPanel->IsVisible()) {
		int XL1, YL1, XL2, YL2;
		int XR1, YR1, XR2, YR2;
		ActiveTab().LeftPanel->GetPosition(XL1, YL1, XL2, YL2);
		ActiveTab().RightPanel->GetPosition(XR1, YR1, XR2, YR2);

		if (!ActiveTab().LeftPanel->ViewSettings.FullScreen || !ActiveTab().RightPanel->ViewSettings.FullScreen) {
			Opt.WidthDecrement = -Opt.WidthDecrement;

			Opt.LeftHeightDecrement^= Opt.RightHeightDecrement;
			Opt.RightHeightDecrement = Opt.LeftHeightDecrement ^ Opt.RightHeightDecrement;
			Opt.LeftHeightDecrement^= Opt.RightHeightDecrement;
		}

		Panel *Swap;
		int SwapType;
		Swap = ActiveTab().LeftPanel;
		ActiveTab().LeftPanel = ActiveTab().RightPanel;
		ActiveTab().RightPanel = Swap;
		Swap = ActiveTab().LastLeftFilePanel;
		ActiveTab().LastLeftFilePanel = ActiveTab().LastRightFilePanel;
		ActiveTab().LastRightFilePanel = Swap;
		SwapType = ActiveTab().LastLeftType;
		ActiveTab().LastLeftType = ActiveTab().LastRightType;
		ActiveTab().LastRightType = SwapType;
		FileFilter::SwapFilter();
		Ret = TRUE;
	}
	SetScreenPosition();
	FrameManager->RefreshFrame();
	return Ret;
}

int64_t FilePanels::VMProcess(MacroOpcode OpCode, void *vParam, int64_t iParam)
{
	return ActiveTab().ActivePanel->VMProcess(OpCode, vParam, iParam);
}

void FilePanels::RetryActivePanelRead()
{
	if (ActiveTab().ActivePanel->GetType() == FILE_PANEL)
		static_cast<FileList *>(ActiveTab().ActivePanel)->RetryFailedRead();
}

int FilePanels::ProcessKey(FarKey Key)
{
	if (!Key)
		return TRUE;

	if (CtrlObject->CmdLine->IsMultiline()
			&& (Key == KEY_UP || Key == KEY_NUMPAD8 || Key == KEY_DOWN || Key == KEY_NUMPAD2
					|| Key == KEY_LEFT || Key == KEY_NUMPAD4 || Key == KEY_RIGHT || Key == KEY_NUMPAD6)) {
		CtrlObject->CmdLine->ProcessKey(Key);
		return TRUE;
	}

	if ((Key == KEY_CTRLLEFT || Key == KEY_CTRLRIGHT || Key == KEY_CTRLNUMPAD4 || Key == KEY_CTRLNUMPAD6
				/* || Key==KEY_CTRLUP || Key==KEY_CTRLDOWN || Key==KEY_CTRLNUMPAD8 || Key==KEY_CTRLNUMPAD2 */)
			&& (CtrlObject->CmdLine->IsNotEmpty() || (!ActiveTab().LeftPanel->IsVisible() && !ActiveTab().RightPanel->IsVisible()))) {
		CtrlObject->CmdLine->ProcessKey(Key);
		return TRUE;
	}
	SudoClientRegion scr;
	switch (Key) {
		case KEY_F1: {
			if (!ActiveTab().LeftPanel->IsVisible() && !ActiveTab().RightPanel->IsVisible()) {
				Help::Present(L"Terminal");
			} else if (!ActiveTab().ActivePanel->ProcessKey(KEY_F1)) {
				Help::Present(L"Contents");
			}

			return TRUE;
		}
		case KEY_TAB: {
			if (!SetAnotherPanelFocus()) {
				CtrlObject->CmdLine->ProcessKey(Key);
			}
			break;
		}
		case KEY_CTRLF1: {
			if (ActiveTab().LeftPanel->IsVisible()) {
				ActiveTab().LeftPanel->Hide();

				if (ActiveTab().RightPanel->IsVisible())
					ActiveTab().RightPanel->SetFocus();
			} else {
				if (!ActiveTab().RightPanel->IsVisible())
					ActiveTab().LeftPanel->SetFocus();

				ActiveTab().LeftPanel->Show();
			}
			UpdateCmdLineVisibility(ActiveTab());
			Redraw();
			break;
		}
		case KEY_CTRLF2: {
			if (ActiveTab().RightPanel->IsVisible()) {
				ActiveTab().RightPanel->Hide();

				if (ActiveTab().LeftPanel->IsVisible())
					ActiveTab().LeftPanel->SetFocus();
			} else {
				if (!ActiveTab().LeftPanel->IsVisible())
					ActiveTab().RightPanel->SetFocus();

				ActiveTab().RightPanel->Show();
			}
			UpdateCmdLineVisibility(ActiveTab());
			Redraw();
			break;
		}
		case KEY_CTRLB: {
			Opt.ShowKeyBar = !Opt.ShowKeyBar;
			KeyBarVisible = Opt.ShowKeyBar;

			if (!KeyBarVisible)
				MainKeyBar.Hide();

			SetScreenPosition();
			FrameManager->RefreshFrame();
			break;
		}
		case KEY_CTRLL:
		case KEY_CTRLQ:
		case KEY_CTRLT: {
			if (ActiveTab().ActivePanel->IsVisible()) {
				Panel *AnotherPanel = GetAnotherPanel(ActiveTab(), ActiveTab().ActivePanel);
				int NewType;

				if (Key == KEY_CTRLL)
					NewType = INFO_PANEL;
				else if (Key == KEY_CTRLQ)
					NewType = QVIEW_PANEL;
				else
					NewType = TREE_PANEL;

				if (ActiveTab().ActivePanel->GetType() == NewType)
					AnotherPanel = ActiveTab().ActivePanel;

				if (!AnotherPanel->ProcessPluginEvent(FE_CLOSE, nullptr)) {
					if (AnotherPanel->GetType() == NewType)
						/*
							$ 19.09.2000 IS
							Повторное нажатие на ctrl-l|q|t всегда включает файловую панель
						*/
						AnotherPanel = ChangePanel(AnotherPanel, FILE_PANEL, FALSE, FALSE);
					else
						AnotherPanel = ChangePanel(AnotherPanel, NewType, FALSE, FALSE);

					/*
						$ 07.09.2001 VVM
						! При возврате из CTRL+Q, CTRL+L восстановим каталог, если активная панель - дерево.
					*/
					if (ActiveTab().ActivePanel->GetType() == TREE_PANEL) {
						FARString strCurDir;
						ActiveTab().ActivePanel->GetCurDir(strCurDir);
						AnotherPanel->SetCurDir(strCurDir, TRUE);
						AnotherPanel->Update(0);
					} else
						AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					AnotherPanel->Show();
				}

				ActiveTab().ActivePanel->SetFocus();
			}

			break;
		}
		case KEY_CTRLO: {
			{
				int LeftVisible = ActiveTab().LeftPanel->IsVisible();
				int RightVisible = ActiveTab().RightPanel->IsVisible();
				int HideState = !LeftVisible && !RightVisible;

				if (!HideState) {
					ActiveTab().LeftStateBeforeHide = LeftVisible;
					ActiveTab().RightStateBeforeHide = RightVisible;
					ActiveTab().LeftPanel->Hide();
					ActiveTab().RightPanel->Hide();
					FrameManager->RefreshFrame();
				} else {
					if (!ActiveTab().LeftStateBeforeHide && !ActiveTab().RightStateBeforeHide)
						ActiveTab().LeftStateBeforeHide = ActiveTab().RightStateBeforeHide = TRUE;

					if (ActiveTab().LeftStateBeforeHide)
						ActiveTab().LeftPanel->Show();

					if (ActiveTab().RightStateBeforeHide)
						ActiveTab().RightPanel->Show();

					if (!ActiveTab().ActivePanel->IsVisible()) {
						if (ActiveTab().ActivePanel == ActiveTab().RightPanel)
							ActiveTab().LeftPanel->SetFocus();
						else
							ActiveTab().RightPanel->SetFocus();
					}
				}
				UpdateCmdLineVisibility(ActiveTab());
			}
			break;
		}
		case KEY_CTRLP: {
			if (ActiveTab().ActivePanel->IsVisible()) {
				Panel *AnotherPanel = GetAnotherPanel(ActiveTab(), ActiveTab().ActivePanel);

				if (AnotherPanel->IsVisible())
					AnotherPanel->Hide();
				else
					AnotherPanel->Show();
				UpdateCmdLineVisibility(ActiveTab());
				CtrlObject->CmdLine->Redraw();
			}

			FrameManager->RefreshFrame();
			break;
		}
		case KEY_CTRLI: {
			ActiveTab().ActivePanel->EditFilter();
			return TRUE;
		}
		case KEY_CTRLU: {
			if (!ActiveTab().LeftPanel->IsVisible() && !ActiveTab().RightPanel->IsVisible()) {
				CtrlObject->CmdLine->ProcessKey(Key);
			} else
				SwapPanels();

			break;
		}
//		case (KEY_CTRL + KEY_COMMA) | KEY_ALT: {
		case (KEY_CTRL + KEY_COMMA): {
			Opt.PanelsDisposition ^= 1;
			SetScreenPosition();
			FrameManager->RefreshFrame();
			break;
		}

		/*
			$ 08.04.2002 IS
			При смене диска установим принудительно текущий каталог на активной
			панели, т.к. система не знает ничего о том, что у Фара две панели, и
			текущим для системы после смены диска может быть каталог и на пассивной
			панели
		*/
		case KEY_ALTF1: {
			ActiveTab().LeftPanel->ChangeDisk();

			if (ActiveTab().ActivePanel != ActiveTab().LeftPanel)
				ActiveTab().ActivePanel->SetCurPath();

			break;
		}
		case KEY_ALTF2: {
			ActiveTab().RightPanel->ChangeDisk();

			if (ActiveTab().ActivePanel != ActiveTab().RightPanel)
				ActiveTab().ActivePanel->SetCurPath();

			break;
		}
		case KEY_ALTF7: {
			FindFiles::Present();
			break;
		}
		case KEY_CTRLD: {
			if (!CtrlObject->CmdLine->IsNotEmpty())
				PresentFileDiff();
			else
				CtrlObject->CmdLine->ProcessKey(Key);

			break;
		}
		case KEY_ALTDOT: {
			// bash-like yank-last-arg, but only when command line is not
			// empty; with empty command line Alt+. keeps its old behavior -
			// panel fast-find for names starting with dot
			if (CtrlObject->CmdLine->IsVisible() && CtrlObject->CmdLine->IsNotEmpty()) {
				CtrlObject->CmdLine->ProcessKey(Key);
				return TRUE;
			}

			if (!ActiveTab().ActivePanel->ProcessKey(Key))
				CtrlObject->CmdLine->ProcessKey(Key);

			break;
		}
		case KEY_CTRLSHIFTD: {
			PresentFileDiff(true);
			break;
		}
		case KEY_CTRLUP:
		case KEY_CTRLNUMPAD8: {
			bool Set = false;
			if (Opt.LeftHeightDecrement < ScrY - 7) {
				Opt.LeftHeightDecrement++;
				Set = true;
			}
			if (Opt.RightHeightDecrement < ScrY - 7) {
				Opt.RightHeightDecrement++;
				Set = true;
			}
			if (Set) {
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLDOWN:
		case KEY_CTRLNUMPAD2: {
			bool Set = false;
			const int min_decrement = CtrlObject->CmdLine->IsNotEmpty() ? 0 : -1;
			if (Opt.LeftHeightDecrement > min_decrement) {
				Opt.LeftHeightDecrement--;
				Set = true;
			}
			if (Opt.RightHeightDecrement > min_decrement) {
				Opt.RightHeightDecrement--;
				Set = true;
			}
			if (Set) {
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}

//		case KEY_CTRLALTUP:
		case KEY_CTRLSHIFTUP:
		case KEY_CTRLSHIFTNUMPAD8: {
			int &HeightDecrement =
					(ActiveTab().ActivePanel == ActiveTab().LeftPanel) ? Opt.LeftHeightDecrement : Opt.RightHeightDecrement;
			if (HeightDecrement < ScrY - 7) {
				HeightDecrement++;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}
			break;
		}

//		case KEY_CTRLALTDOWN:
		case KEY_CTRLSHIFTDOWN:
		case KEY_CTRLSHIFTNUMPAD2: {
			int &HeightDecrement =
					(ActiveTab().ActivePanel == ActiveTab().LeftPanel) ? Opt.LeftHeightDecrement : Opt.RightHeightDecrement;
			const int min_decrement = CtrlObject->CmdLine->IsNotEmpty() ? 0 : -1;
			if (HeightDecrement > min_decrement) {
				HeightDecrement--;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}
			break;
		}

		case KEY_CTRLLEFT:
		case KEY_CTRLNUMPAD4: {
			if (Opt.WidthDecrement < ScrX / 2 - 10) {
				Opt.WidthDecrement++;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLRIGHT:
		case KEY_CTRLNUMPAD6: {
			if (Opt.WidthDecrement > -(ScrX / 2 - 10)) {
				Opt.WidthDecrement--;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLCLEAR: {
			if (Opt.WidthDecrement) {
				Opt.WidthDecrement = 0;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLALTCLEAR: {
			bool Set = false;
			if (Opt.LeftHeightDecrement) {
				Opt.LeftHeightDecrement = 0;
				Set = true;
			}
			if (Opt.RightHeightDecrement) {
				Opt.RightHeightDecrement = 0;
				Set = true;
			}
			if (Set) {
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_F9: {
			ShellOptions(0, nullptr);
			return TRUE;
		}
		case KEY_SHIFTF10: {
			ShellOptions(1, nullptr);
			return TRUE;
		}
		default: {
			if (Key >= KEY_CTRL0 && Key <= KEY_CTRL9 && !(ActiveTab().ActivePanel->GetType() == TREE_PANEL))
				ChangePanelViewMode(ActiveTab().ActivePanel, Key - KEY_CTRL0, TRUE);
			else if (!ActiveTab().ActivePanel->ProcessKey(Key))
				CtrlObject->CmdLine->ProcessKey(Key);

			break;
		}
	}

	RetryActivePanelRead();
	return TRUE;
}

int FilePanels::ChangePanelViewMode(Panel *Current, int Mode, BOOL RefreshFrame)
{
	if (Current && Mode >= VIEW_0 && Mode <= VIEW_9) {
		Current->SetViewMode(Mode);
		Current = ChangePanelToFilled(Current, FILE_PANEL);
		Current->SetViewMode(Mode);
		// ВНИМАНИЕ! Костыль! Но Работает!
		SetScreenPosition();

		if (RefreshFrame)
			FrameManager->RefreshFrame();

		return TRUE;
	}

	return FALSE;
}

Panel *FilePanels::ChangePanelToFilled(Panel *Current, int NewType)
{
	if (Current->GetType() != NewType && !Current->ProcessPluginEvent(FE_CLOSE, nullptr)) {
		Current->Hide();
		Current = ChangePanel(Current, NewType, FALSE, FALSE);
		Current->Update(0);
		Current->Show();

		if (!GetAnotherPanel(ActiveTab(), Current)->GetFocus())
			Current->SetFocus();
	}

	return (Current);
}

Panel *FilePanels::GetAnotherPanel(DoublePanel& activeTab, Panel *Current)
{
	if (Current == activeTab.LeftPanel)
		return activeTab.RightPanel;
	else
		return activeTab.LeftPanel;
}

Panel *FilePanels::GetAnotherPanel(Panel *Current)
{
	return GetAnotherPanel(ActiveTab(), Current);
}

Panel *FilePanels::ChangePanel(Panel *Current, int NewType, int CreateNew, int Force)
{
	Panel *NewPanel;
	SaveScreen *SaveScr = nullptr;
	// OldType не инициализировался...
	int OldType = Current->GetType(), X1, Y1, X2, Y2;
	int OldViewMode, OldSortMode, OldSortOrder, OldSortGroups, OldSelectedFirst, OldDirectoriesFirst, OldExecutablesFirst;
	int OldPanelMode, LeftPosition, ChangePosition, OldNumericSort, OldCaseSensitiveSort;
	int OldFullScreen, OldFocus, UseLastPanel = 0;
	OldPanelMode = Current->GetMode();

	if (!Force && NewType == OldType && OldPanelMode == NORMAL_PANEL)
		return (Current);

	OldViewMode = Current->GetPrevViewMode();
	OldFullScreen = Current->IsFullScreen();
	OldSortMode = Current->GetPrevSortMode();
	OldSortOrder = Current->GetPrevSortOrder();
	OldNumericSort = Current->GetPrevNumericSort();
	OldCaseSensitiveSort = Current->GetPrevCaseSensitiveSort();
	OldSortGroups = Current->GetSortGroups();
	OldFocus = Current->GetFocus();
	OldSelectedFirst = Current->GetSelectedFirstMode();
	OldDirectoriesFirst = Current->GetPrevDirectoriesFirst();
	OldExecutablesFirst = Current->GetPrevExecutablesFirst();
	LeftPosition = (Current == ActiveTab().LeftPanel);
	Panel *&LastFilePanel = LeftPosition ? ActiveTab().LastLeftFilePanel : ActiveTab().LastRightFilePanel;
	Current->GetPosition(X1, Y1, X2, Y2);
	ChangePosition = ((OldType == FILE_PANEL && NewType != FILE_PANEL && OldFullScreen)
			|| (NewType == FILE_PANEL
					&& ((OldFullScreen && !FileList::IsModeFullScreen(OldViewMode))
							|| (!OldFullScreen && FileList::IsModeFullScreen(OldViewMode)))));

	if (!ChangePosition) {
		SaveScr = Current->SaveScr;
		Current->SaveScr = nullptr;
	}

	if (OldType == FILE_PANEL && NewType != FILE_PANEL) {
		delete Current->SaveScr;
		Current->SaveScr = nullptr;

		if (LastFilePanel != Current) {
			DeletePanel(LastFilePanel);
			LastFilePanel = Current;
		}

		LastFilePanel->Hide();

		if (LastFilePanel->SaveScr) {
			LastFilePanel->SaveScr->Discard();
			delete LastFilePanel->SaveScr;
			LastFilePanel->SaveScr = nullptr;
		}
	} else {
		Current->Hide();
		DeletePanel(Current);

		if (OldType == FILE_PANEL && NewType == FILE_PANEL) {
			DeletePanel(LastFilePanel);
			LastFilePanel = nullptr;
		}
	}

	if (!CreateNew && NewType == FILE_PANEL && LastFilePanel) {
		int LastX1, LastY1, LastX2, LastY2;
		LastFilePanel->GetPosition(LastX1, LastY1, LastX2, LastY2);

		if (LastFilePanel->IsFullScreen())
			LastFilePanel->SetPosition(LastX1, Y1, LastX2, Y2);
		else
			LastFilePanel->SetPosition(X1, Y1, X2, Y2);

		NewPanel = LastFilePanel;

		if (!ChangePosition) {
			if ((NewPanel->IsFullScreen() && !OldFullScreen)
					|| (!NewPanel->IsFullScreen() && OldFullScreen)) {
				Panel *AnotherPanel = GetAnotherPanel(ActiveTab(), Current);

				if (SaveScr && AnotherPanel->IsVisible() && AnotherPanel->GetType() == FILE_PANEL
						&& AnotherPanel->IsFullScreen())
					SaveScr->Discard();

				delete SaveScr;
			} else
				NewPanel->SaveScr = SaveScr;
		}

		if (!OldFocus && NewPanel->GetFocus())
			NewPanel->KillFocus();

		UseLastPanel = TRUE;
	} else
		NewPanel = CreatePanel(NewType);

	if (Current == ActiveTab().ActivePanel)
		ActiveTab().ActivePanel = NewPanel;

	if (LeftPosition) {
		ActiveTab().LeftPanel = NewPanel;
		ActiveTab().LastLeftType = OldType;
	} else {
		ActiveTab().RightPanel = NewPanel;
		ActiveTab().LastRightType = OldType;
	}

	if (!UseLastPanel) {
		if (ChangePosition) {
			if (LeftPosition) {
				NewPanel->SetPosition(0, Y1, ScrX / 2 - Opt.WidthDecrement, Y2);
				ActiveTab().RightPanel->Redraw();
			} else {
				NewPanel->SetPosition(ScrX / 2 + 1 - Opt.WidthDecrement, Y1, ScrX, Y2);
				ActiveTab().LeftPanel->Redraw();
			}
		} else {
			NewPanel->SaveScr = SaveScr;
			NewPanel->SetPosition(X1, Y1, X2, Y2);
		}

		NewPanel->SetSortMode(OldSortMode);
		NewPanel->SetSortOrder(OldSortOrder);
		NewPanel->SetNumericSort(OldNumericSort);
		NewPanel->SetCaseSensitiveSort(OldCaseSensitiveSort);
		NewPanel->SetSortGroups(OldSortGroups);
		NewPanel->SetPrevViewMode(OldViewMode);
		NewPanel->SetViewMode(OldViewMode);
		NewPanel->SetSelectedFirstMode(OldSelectedFirst);
		NewPanel->SetDirectoriesFirst(OldDirectoriesFirst);
		NewPanel->SetExecutablesFirst(OldExecutablesFirst);
	}

	return (NewPanel);
}

int FilePanels::GetSubpanelCount(){ return (int)tabs.size(); }

int FilePanels::GetSubpanelTypeAndName(int index, FARString &strType, FARString &strName, int maxLen)
{
	strType = Msg::ScreensPanels;
	FARString strFullName;

	switch (tabs[index].LeftPanel->GetType()) {
	case TREE_PANEL:
	case QVIEW_PANEL:
	case INFO_PANEL:
		tabs[index].ActivePanel->GetCurName(strFullName);
		ConvertNameToFull(strFullName, strFullName);
		break;
	case FILE_PANEL:
		strFullName = TopTabBar.getFormattedTitle(index, maxLen < 0 ? 60 : maxLen);
		break;
	}

	strName = strFullName;
	return (MODALTYPE_PANELS);
}

int FilePanels::GetTypeAndName(FARString &strType, FARString &strName){
	return GetSubpanelTypeAndName(TabActive, strType, strName);
}

void FilePanels::OnChangeFocus(int f)
{
	_OT(SysLog(L"FilePanels::OnChangeFocus(%i)", f));

	/*
		$ 20.06.2001 tran
		баг с отрисовкой при копировании и удалении
		не учитывался LockRefreshCount
	*/
	if (f) {
		/*
			$ 22.06.2001 SKV
			+ update панелей при получении фокуса
		*/
		CtrlObject->Cp()->GetAnotherPanel(ActiveTab().ActivePanel)->UpdateIfChanged(UIC_UPDATE_FORCE_NOTIFICATION);
		ActiveTab().ActivePanel->UpdateIfChanged(UIC_UPDATE_FORCE_NOTIFICATION);
		/*
			$ 13.04.2002 KM
			! ??? Я не понял зачем здесь Redraw, если
			Redraw вызывается следом во Frame::OnChangeFocus.
		*/
		// Redraw();
		ActiveTab().ActivePanel->SetCurPath();
		UpdateTabBar();
		Frame::OnChangeFocus(1);
	}
}

void FilePanels::DisplayObject()
{
	// if ( !Focus )
	// return;
	_OT(SysLog(L"[%p] FilePanels::Redraw() {%d, %d - %d, %d}", this, X1, Y1, X2, Y2));
	CtrlObject->CmdLine->ShowBackground( (bool)(Opt.PanelsDisposition) );

	if (Opt.ShowMenuBar)
		CtrlObject->TopMenuBar->Show();

	SetTabNames();
	TopTabBar.Show();
	CtrlObject->CmdLine->Show();

	MainKeyBar.Refresh(Opt.ShowKeyBar);

	KeyBarVisible = Opt.ShowKeyBar;
#if 1

	if (ActiveTab().LeftPanel->IsVisible())
		ActiveTab().LeftPanel->Show();

	if (ActiveTab().RightPanel->IsVisible())
		ActiveTab().RightPanel->Show();

#else
	/*
	Panel *PassivePanel = nullptr;
	int PassiveIsLeftFlag = TRUE;

	if (Opt.LeftPanel.Focus) {
		ActiveTab().ActivePanel = ActiveTab().LeftPanel;
		PassivePanel = ActiveTab().RightPanel;
		PassiveIsLeftFlag = FALSE;
	} else {
		ActiveTab().ActivePanel = ActiveTab().RightPanel;
		PassivePanel = ActiveTab().LeftPanel;
		PassiveIsLeftFlag = TRUE;
	}

	//! Вначале "показываем" пассивную панель
	if (PassiveIsLeftFlag) {
		if (Opt.LeftPanel.Visible) {
			ActiveTab().LeftPanel->Show();
		}

		if (Opt.RightPanel.Visible) {
			ActiveTab().RightPanel->Show();
		}
	} else {
		if (Opt.RightPanel.Visible) {
			ActiveTab().RightPanel->Show();
		}

		if (Opt.LeftPanel.Visible) {
			ActiveTab().LeftPanel->Show();
		}
	}*/
#endif
}

int FilePanels::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	int MsX = MouseEvent->dwMousePosition.X;
	int MsY = MouseEvent->dwMousePosition.Y;
	if (MsX >= X1 && MsX <= X2 && MsY == Y1 + (Opt.ShowMenuBar ? 1 : 0 ))
		if (TopTabBar.ProcessMouse(MouseEvent))
			return TRUE;

	if (MouseEvent->dwEventFlags == MOUSE_MOVED && MsY == Y1)
		MainKeyBar.ProcessMouse(MouseEvent);

	if (MouseEvent->dwEventFlags == MOUSE_MOVED) {
		TabHovered = -1;
		TopTabBar.SetHovered(TabHovered);
		TopTabBar.Redraw();
	}

	if (!ActiveTab().ActivePanel->ProcessMouse(MouseEvent))
		if (!GetAnotherPanel(ActiveTab(), ActiveTab().ActivePanel)->ProcessMouse(MouseEvent))
			if (!MainKeyBar.ProcessMouse(MouseEvent))
				CtrlObject->CmdLine->ProcessMouse(MouseEvent);
	return TRUE;
}

void FilePanels::ShowConsoleTitle()
{
	if (ActiveTab().ActivePanel && ActiveTab().ActivePanel->IsVisible())
		ActiveTab().ActivePanel->SetTitle();
}

void FilePanels::ResizeConsole()
{
	Frame::ResizeConsole();
	MainKeyBar.ResizeConsole();
	TopMenuBar.ResizeConsole();
	TopTabBar.ResizeConsole();
	SetScreenPosition();
	CtrlObject->CmdLine->ResizeConsole();
	_OT(SysLog(L"[%p] FilePanels::ResizeConsole() {%d, %d - %d, %d}", this, X1, Y1, X2, Y2));
}

int FilePanels::FastHide()
{
	return Opt.AllCtrlAltShiftRule & CASR_PANEL;
}

void FilePanels::Refresh()
{
	/*
		$ 31.07.2001 SKV
		Вызовем так, а не Frame::OnChangeFocus,
		который из этого и позовётся.
	*/
	// Frame::OnChangeFocus(1);
	OnChangeFocus(1);
}

void FilePanels::GoToFile(const wchar_t *FileName)
{
	if (FirstSlash(FileName)) {
		FARString ADir, PDir;
		Panel *PassivePanel = GetAnotherPanel(ActiveTab(), ActiveTab().ActivePanel);
		int PassiveMode = PassivePanel->GetMode();

		if (PassiveMode == NORMAL_PANEL) {
			PassivePanel->GetCurDir(PDir);
			AddEndSlash(PDir);
		}

		int ActiveMode = ActiveTab().ActivePanel->GetMode();

		if (ActiveMode == NORMAL_PANEL) {
			ActiveTab().ActivePanel->GetCurDir(ADir);
			AddEndSlash(ADir);
		}

		FARString strNameFile = PointToName(FileName);
		FARString strNameDir = FileName;
		CutToSlash(strNameDir);
		/*
			$ 10.04.2001 IS
			Не делаем SetCurDir, если нужный путь уже есть на открытых
			панелях, тем самым добиваемся того, что выделение с элементов
			панелей не сбрасывается.
		*/
		BOOL AExist = (ActiveMode == NORMAL_PANEL) && !StrCmp(ADir, strNameDir);
		BOOL PExist = (PassiveMode == NORMAL_PANEL) && !StrCmp(PDir, strNameDir);

		// если нужный путь есть на пассивной панели
		if (!AExist && PExist)
			ProcessKey(KEY_TAB);

		if (!AExist && !PExist)
			ActiveTab().ActivePanel->SetCurDir(strNameDir, TRUE);

		ActiveTab().ActivePanel->GoToFile(strNameFile);
		// всегда обновим заголовок панели, чтобы дать обратную связь, что
		// Ctrl-F10 обработан
		ActiveTab().ActivePanel->SetTitle();
	}
}

int FilePanels::GetMacroMode()
{
	switch (ActiveTab().ActivePanel->GetType()) {
		case TREE_PANEL:
			return MACRO_TREEPANEL;
		case QVIEW_PANEL:
			return MACRO_QVIEWPANEL;
		case INFO_PANEL:
			return MACRO_INFOPANEL;
		default:
			return MACRO_SHELL;
	}
}

FARString StrTrim(const FARString& x) {
	std::wstring y = x.GetWide();
	StrTrim(y);
	return y;
}

int FilePanels::SetTabNames()
{
    TopTabBar.Clear();
	for(size_t i = 0; i < tabs.size(); ++i){
		tabs[i].LeftPanel->GetTitle(tabs[i].a_name, 128, 2);
		tabs[i].RightPanel->GetTitle(tabs[i].p_name, 128, 2);
		tabs[i].a_name = StrTrim(tabs[i].a_name);
		tabs[i].p_name = StrTrim(tabs[i].p_name);
		TopTabBar.AddTab(tabs[i].a_name, tabs[i].p_name);
	}

	TopTabBar.SetActive(TabActive);
	TopTabBar.SetHovered(TabHovered);
	TopTabBar.EnsureActiveVisible();
	TopTabBar.Redraw();
	return 0;
}

void FilePanels::UpdateTabBar() {
	SetTabNames();
}

void FilePanels::Update() 
{
	//if(ActiveTab().ActivePanel->IsVisible()) 	 ActiveTab().ActivePanel->Update(UPDATE_KEEP_SELECTION);
	//if(ActiveTab().PassivePanel()->IsVisible())  ActiveTab().PassivePanel()->Update(UPDATE_KEEP_SELECTION);
	FrameManager->RefreshFrame();
	//Redraw();
}

void FilePanels::SwitchActiveTabTo(int tabNo)
{
	int tabA = TabActive;

	if(tabA == tabNo) return;

	deactivatePanelsInTab(tabs[tabA]);
	TabActive = tabNo;
	activatePanelsInTab(tabs[tabNo]);

	ActiveTab().ActivePanel->SetFocus();
}

int FilePanels::AppendNewTab() {
	int tabNo = (int)tabs.size();
	tabs.push_back(DoublePanel());

	for(size_t i = 0; i < tabs.size(); ++i) {
		fprintf(stderr, "\tappendNewTab prior to add: [%d] active=%p left=%p right=%p\n", 
			(int)i, tabs[i].ActivePanel, tabs[i].LeftPanel, tabs[i].RightPanel);
	}

	tabs[tabNo].LeftPanel  = CreatePanel(Opt.LeftPanel.Type);
	tabs[tabNo].RightPanel = CreatePanel(Opt.RightPanel.Type);

	Init(tabs[tabNo]);

	SwitchActiveTabTo(tabNo);

	for(size_t i = 0; i < tabs.size(); ++i) {
		fprintf(stderr, "\tappendNewTab after switch: [%d] active=%p left=%p right=%p\n", 
			(int)i, tabs[i].ActivePanel, tabs[i].LeftPanel, tabs[i].RightPanel);
	}
	return tabNo;
}

void FilePanels::DeleteTab(int tabNo) {
	if (tabs.size() == 1 || tabNo < 0 || tabNo >= (int)tabs.size()) return; // last panels cannot be removed

	int oldActive = TabActive;

	if (TabActive == tabNo) { // deleting active tab -> need to switch active to other first
		int switchTo = TabActive == 0 ? 1 : TabActive - 1;
		// SwitchActiveTabTo(switchTo);
		TabActive = switchTo;
	}
	// if we're deleting tab on the left, the active pointer needs to be shifted
	else if (TabActive > tabNo) {
		--TabActive; 
		// no changes 
	}

	deactivatePanelsInTab(tabs[tabNo]);

	DoublePanel panel = tabs[tabNo];
	tabs.erase(tabs.begin() + tabNo);
	if (TabActive != oldActive)
		activatePanelsInTab(tabs[TabActive]);
	destroyPanelsGracefully(panel);

	Redraw();

	for(size_t i = 0; i < tabs.size(); ++i) {
		fprintf(stderr, "\tdeleteTab: [%d] active=%p left=%p right=%p\n", 
			(int)i, tabs[i].ActivePanel, tabs[i].LeftPanel, tabs[i].RightPanel);
	}
}

void FilePanels::EnlistAllPaths(std::vector<std::wstring>& holder, bool left, bool exceptActive)
{
	for(size_t i = 0; i < tabs.size(); ++i){
		if (exceptActive && (int)i == TabActive){
			holder.push_back(L"-");
			continue;
		}

		FARString x;
		if (left) tabs[i].LeftPanel->GetTitle(x, 128, 2);
		else tabs[i].RightPanel->GetTitle(x, 128, 2);
		x = StrTrim(x);
		holder.push_back(x.GetWide());
	}
}

void FilePanels::SwapTo(int srcTab, int dstTab, bool isLeft) {
	DoublePanel& src = tabs[srcTab];
	DoublePanel& dst = tabs[dstTab];

	Panel *Swap;
	int SwapType;
	if (srcTab == TabActive) deactivatePanelsInTab(src);
	if (dstTab == TabActive) deactivatePanelsInTab(dst);

	if (isLeft) {

		Swap = dst.LeftPanel;
		dst.LeftPanel = src.LeftPanel;
		src.LeftPanel = Swap;

		Swap = dst.LastLeftFilePanel;
		dst.LastLeftFilePanel = src.LastLeftFilePanel;
		src.LastLeftFilePanel = Swap;

		SwapType = dst.LastLeftType;
		dst.LastLeftType = src.LastLeftType;
		src.LastLeftType = SwapType;
	}
	else {
		Swap = dst.RightPanel;
		dst.RightPanel = src.RightPanel;
		src.RightPanel = Swap;

		Swap = dst.LastRightFilePanel;
		dst.LastRightFilePanel = src.LastRightFilePanel;
		src.LastRightFilePanel = Swap;

		SwapType = dst.LastRightType;
		dst.LastRightType = src.LastRightType;
		src.LastRightType = SwapType;
	}

	if (srcTab == TabActive) activatePanelsInTab(src);
	if (dstTab == TabActive) activatePanelsInTab(dst);

	FrameManager->RefreshFrame();
}

void FilePanels::GetActiveTabPaths(FARString& leftFolderList, FARString& rightFolderList, int& activeTab) 
{
	leftFolderList = L"";
	rightFolderList = L"";
	activeTab = TabActive;
	for(size_t i = 0; i < tabs.size(); ++i) {
		if(i > 0) leftFolderList += L"|";
		if(i > 0) rightFolderList += L"|";
		leftFolderList += tabs[i].a_name;
		rightFolderList += tabs[i].p_name;
	}
}
