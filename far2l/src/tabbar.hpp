#pragma once

/*
tabbar.hpp

Показ горизонтального меню при включенном "Always show menu bar"
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

#include "scrobj.hpp"

#include <string>
#include <vector>

class TabBar : public ScreenObject
{
public:
	TabBar() {}
	virtual ~TabBar() {}

	struct TabNameAndPos {
		FARString name;

		int x { -1 };
		int w { 0 };

		FARString p_name;
		FARString display;
		FARString left;
		FARString right;

		int delX { -1 };
		int leftPinX { - 1 };
		int rightPinX { - 1 };

		size_t cells() {
			size_t k = left.CellsCount();
			if(!right.IsEmpty()) k += 1 + right.CellsCount();
			return k;
		}
	};

	void SetTabs(const std::vector<TabNameAndPos>& v, int activeTab = -1);
	void SetTexts(const std::vector<std::wstring>& v, int activeTab = -1);
	const std::vector<std::wstring>& GetTabNames(){ return tabs; }
	int ActiveTab(){ return activeTab; }
	int GetActive(){ return activeTab; }
	const std::vector<TabNameAndPos>& GetTabPositions(){ return tabPos; }
	int HoveredTab(){ return hoveredTab; }

	void SetActive(int x){ activeTab = x; }
	void SetHovered(int x){ hoveredTab = x; }
	void Clear(){ tabs.clear(); tabPos.clear(); }
	void AddTab(const FARString& name){ tabPos.push_back({ name, 0, (int)name.CellsCount() }); }
	void AddTab(const FARString& left, const FARString& right){ tabPos.push_back({ left, 0, (int)(left.CellsCount() + right.CellsCount() + 1), right }); }

	void EnsureActiveVisible();

	int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
	int moreContextMenu();
	int pathContextMenu(bool left, int x, int fromTab);
	FARString getFormattedTitle(size_t j, int width);

protected:
	virtual void DisplayObject();

	int render();
	void layout(size_t& maxWidth, int& gap, std::vector<size_t>& leafWidths);
	void setHoverMask(int tabNo, 
		bool tab, bool del, bool leftPin, bool rightPin, 
		bool more, bool menu, bool console,
		bool leftScroll = false, bool rightScroll = false);

	std::vector<std::wstring> tabs;
	int activeTab { 0 };
	int hoveredTab { -1 };
	
	int visibleTab { 0 };
	int leftAX  { -1 };
	int rightAX { -1 };
	bool leftAHovered { false };
	bool rightAHovered { false };

	int plusX { 0 };
	bool plusHovered { false };
	int moreX { -1 };
	bool moreHovered { false };
	int menuX { - 1 };
	bool menuHovered { false };
	int delHovered {-1 };
	int leftPinHovered { - 1 };
	int rightPinHovered { -1 };
	int consoleX { -1 };
	bool consoleHovered { false };

	std::vector<TabNameAndPos> tabPos;
};

std::wstring widestCommonPrefix(const std::vector<std::wstring>& paths);
std::wstring widestCommonDirectory(const std::vector<std::wstring>& paths);
std::wstring makeLeaf(const std::wstring& fullPath, const std::wstring& prefix);
std::vector<std::wstring> compactifyFileNamesUpTo(std::vector<std::wstring> v);
std::wstring shortenLeaf(const std::wstring& leaf, size_t maxLeafWidth);
