#pragma once

/*
filepanels.hpp

файловые панели
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

#include "frame.hpp"
#include "keybar.hpp"
#include "menubar.hpp"
#include "tabbar.hpp"

class Panel;
class CommandLine;


class FilePanels : public Frame
{
private:
	virtual void DisplayObject();
	void RetryActivePanelRead();
	typedef class Frame inherited;

public:

	struct DoublePanel {
		Panel *LastLeftFilePanel {nullptr};
		Panel *LastRightFilePanel {nullptr};
        Panel *LeftPanel {nullptr};
        Panel *RightPanel {nullptr};
        Panel *ActivePanel {nullptr};

		int LastLeftType {0}, LastRightType {0};
		int LeftStateBeforeHide {0}, RightStateBeforeHide {0};

		FARString a_name, p_name;
		int x;
		int w;

		bool ActiveVisible { true };
		bool PassiveVisible { true };

		Panel* PassivePanel() {
			return (LeftPanel == ActivePanel) ? RightPanel : LeftPanel ;
		}
	};

	KeyBar MainKeyBar;
	MenuBar TopMenuBar;
	TabBar TopTabBar;

	int TabHovered {1};
	int TabActive  {0};
	std::vector<DoublePanel> tabs;

	DoublePanel& ActiveTab() { return tabs[TabActive]; }

public:
	FilePanels();
	virtual ~FilePanels();
	void UpdateCmdLineVisibility(bool repos = false);
	void UpdateTabBar();

private:
	void Init(DoublePanel& tab);
	void SetPanelPositions(DoublePanel& tab, int LeftFullScreen, int RightFullScreen, int Disposition);
	void UpdateCmdLineVisibility(DoublePanel& tab, bool repos = false);
	void DeletePanel(DoublePanel& tab, Panel *Deleted);
	Panel *GetAnotherPanel(DoublePanel& tab, Panel *Current);
	void SetScreenPosition(DoublePanel& tab);

	int SetTabNames();
	void destroyPanelsGracefully(DoublePanel& tab);
	void activatePanelsInTab(DoublePanel& tab);
	void deactivatePanelsInTab(DoublePanel& tab);

public:
	void Init();
	void SetPanelPositions(int LeftFullScreen, int RightFullScreen, int Disposition);

	Panel *CreatePanel(int Type);
	void DeletePanel(Panel *Deleted);
	Panel *GetAnotherPanel(Panel *Current);
	Panel *ChangePanelToFilled(Panel *Current, int NewType);
	Panel *ChangePanel(Panel *Current, int NewType, int CreateNew, int Force);
	// void   SetPanelPositions();

	void SetupKeyBar();

	virtual int ProcessKey(FarKey Key);
	virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
	virtual int64_t VMProcess(MacroOpcode OpCode, void *vParam = nullptr, int64_t iParam = 0);

	int SetAnotherPanelFocus();
	int SwapPanels();
	int ChangePanelViewMode(Panel *Current, int Mode, BOOL RefreshFrame);

	virtual void SetScreenPosition();

	void Update();

	virtual int GetTypeAndName(FARString &strType, FARString &strName);
	virtual int GetType() { return MODALTYPE_PANELS; }
	virtual const wchar_t *GetTypeName() { return L"[FilePanels]"; };

	virtual void OnChangeFocus(int focus);

	virtual void RedrawKeyBar();	// virtual
	virtual void ShowConsoleTitle();
	virtual void ResizeConsole();
	virtual int FastHide();
	virtual void Refresh();
	void GoToFile(const wchar_t *FileName);

	virtual int GetMacroMode();

	void SwitchActiveTabTo(int tabNo);
	void SwitchHoveredTabTo(int tabNo) { TabHovered = tabNo; }
	int AppendNewTab();
	void DeleteTab(int tabNo);

	void EnlistAllPaths(std::vector<std::wstring>& holder, bool left, bool exceptActive = true);
	void SwapTo(int srcTab, int dstTab, bool isLeft);

	virtual int GetSubpanelCount();
	virtual int GetSubpanelTypeAndName(int index, FARString &strType, FARString &strName, int maxLen = 60);
	virtual int GetSelectedSubpanel(){ return TabActive; }
	virtual void ActivateSubpanel(int i){ SwitchActiveTabTo(i); }

	void GetActiveTabPaths(FARString& leftFolderList, FARString& rightFolderList, int& activeTab);
};
