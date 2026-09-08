/*
tabbar.cpp
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

#include "tabbar.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "interf.hpp"
#include "farcolors.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "vmenu.hpp"
#include "config.hpp"

void TabBar::DisplayObject()
{
	//FARString strMsg = L"Tab bar is here" + text;
	GotoXY(X1, Y1);
	SetFarColor(COL_HMENUTEXT);

	if (tabs.size() > 0) {
		tabPos.clear();
		tabPos.reserve(tabs.size());
		for(size_t i = 0; i < tabs.size(); ++i) {
			tabPos.push_back( { tabs[i] } );
		}
		tabs.clear();
	}
	render();
}

std::wstring widestCommonPrefix(const std::vector<std::wstring>& paths) {
    if (paths.empty())
        return L"";

    // Start with the first path as the candidate prefix
    std::wstring prefix = paths[0];

    for (size_t i = 1; i < paths.size(); ++i) {
        const std::wstring& p = paths[i];
        size_t j = 0;

        // Walk until mismatch or end of one of the strings
        while (j < prefix.size() && j < p.size() && prefix[j] == p[j])
            ++j;
        prefix.resize(j); // shrink to the matched part
        if (prefix.empty())
            break;
    }
    return prefix;
}

std::wstring widestCommonDirectory(const std::vector<std::wstring>& paths) {
    std::wstring prefix = widestCommonPrefix(paths);
    if (prefix.empty())
        return L"";

    // Find last slash or backslash
    size_t pos = prefix.find_last_of(L"/");
    if (pos == std::wstring::npos)
        return L"";

    return prefix.substr(0, pos + 1); // include the slash
}

std::wstring makeLeaf(const std::wstring& fullPath, const std::wstring& prefix) {
    if (fullPath.size() <= prefix.size())
        return fullPath;

    std::wstring leaf = fullPath.substr(prefix.size());

    // Remove leading slash/backslash if present
    if (!leaf.empty() && (leaf[0] == L'/' || leaf[0] == L'\\'))
        leaf.erase(0, 1);

    return leaf;
}

void makeLeaf(TabBar::TabNameAndPos& x, const std::wstring& prefix) {
	x.left = makeLeaf(x.left.GetWide(), prefix);
	if (!x.right.IsEmpty()) x.right = makeLeaf(x.right.GetWide(), prefix);
}

std::vector<std::wstring> compactifyFileNamesUpTo(std::vector<std::wstring> v) {
	if (v.size() < 2) return v;

	std::vector<std::wstring> r;
	r.reserve(v.size());

	std::wstring commonPrefix = widestCommonDirectory(v);
	for(size_t i = 0; i < v.size(); ++i) {
    	r.push_back(makeLeaf(v[i], commonPrefix));
	}

	return r;
}

void compactifyFileNamesUpTo(std::vector<TabBar::TabNameAndPos>& v) {
	if (v.size() < 1) return;

	size_t i;

	std::vector<std::wstring> paths;
	paths.reserve(v.size());
	for(i = 0; i < v.size(); ++i) {
		paths.push_back(v[i].name.CPtr());
		if (!v[i].p_name.IsEmpty())
			paths.push_back(v[i].p_name.CPtr());
	}

	std::wstring commonPrefix = widestCommonDirectory(paths);
	//fprintf(stderr, "... common dir=`%ls`\n", commonPrefix.c_str());
	for(i = 0; i < v.size(); ++i) {
		v[i].left = v[i].name;
		v[i].right = v[i].p_name;
		makeLeaf(v[i], commonPrefix);
		//fprintf(stderr, "... left=`%ls` right=`%ls`\n", v[i].left.CPtr(), v[i].right.CPtr());
	}
}

FARString shortenLeaf(TabBar::TabNameAndPos& leaf, size_t maxLeafWidth) {
    const wchar_t ell = L'…'; // L'\u2026' -- Unicode ellipsis

    if (leaf.cells() <= maxLeafWidth) 
        return leaf.right.IsEmpty() ? leaf.left : leaf.left + L" " + leaf.right;
    if (maxLeafWidth <= 1)  return std::wstring(1, ell);
    if (leaf.right.IsEmpty()) return shortenLeaf(leaf.left.GetWide(), maxLeafWidth);

    // now we have two parts and need to count them proportionally
    size_t left = leaf.left.CellsCount();
    size_t right = leaf.right.CellsCount();
    size_t leftMax = left * maxLeafWidth / (left + right + 1);
    return shortenLeaf(leaf.left.GetWide(), leftMax) + L" " + shortenLeaf(leaf.right.GetWide(), maxLeafWidth - leftMax);
}

std::wstring shortenLeaf(const std::wstring& leaf, size_t maxLeafWidth) {
    const wchar_t ell = L'…'; // L'\u2026' -- Unicode ellipsis

    if (leaf.size() <= maxLeafWidth)
        return leaf;

    if (maxLeafWidth <= 1)
        return std::wstring(1, ell);

    // Split prefix/suffix
    size_t prefixLen = maxLeafWidth / 4;
    if (prefixLen < 1) prefixLen = 1;
    size_t suffixLen = maxLeafWidth - prefixLen - 1; // -1 for ellipsis

    std::wstring prefix = leaf.substr(0, prefixLen);
    std::wstring suffix = leaf.substr(leaf.size() - suffixLen);

    return prefix + ell + suffix;
}

void TabBar::SetTabs(const std::vector<TabNameAndPos>& v, int activeTab) {
	tabs.clear();
	tabPos = v;
	if (activeTab != GetActive()) SetActive(activeTab);
}

static size_t cells(const wchar_t* s) {
	FARString x = s;
	return x.CellsCount();
}

void TabBar::EnsureActiveVisible() 
{
    if (tabPos.size() < 2 || activeTab < 0 || activeTab >= (int)tabPos.size()) return;

	// left case is simple
	if (activeTab < visibleTab) {
		visibleTab = activeTab;
		return;
	}

	size_t maxWidth;
	int gap;

    size_t N = tabPos.size();
	std::vector<size_t> leafWidths(N);

	layout(maxWidth, gap, leafWidths);
	int CurX = 0;
	int sepw = cells(L"│🧷📲📎 ✘");

    for (size_t i = visibleTab; i < N; ++i) {
        CurX += tabPos[i].w + sepw;
		if (CurX < (int)maxWidth && (int)i == activeTab) // active tab is visible
			return;
    }

    if (activeTab < 0 || activeTab >= (int)tabPos.size()) return;

    // Now we have found the active tab is not visible, we need to scroll tab bar horizontally
    CurX = 0;
    for (size_t i = activeTab; i >= 0; --i) {
        if (CurX + tabPos[i].w + sepw > (int)maxWidth) {
        	visibleTab = i + 1;
            return;
        }
    	CurX += tabPos[i].w + sepw;
    }

    visibleTab = activeTab; // fallback
}

void TabBar::layout(size_t& maxWidth, int& gap, std::vector<size_t>& leafWidths) 
{
	maxWidth = X2 - X1 + 1;
	gap = 1 + 3 + 4 + 4; /* console + new tab + menu + more + arrows */
    if (tabPos.empty()) return;

    if (!Opt.ShowMenuBar) gap += 3;
    if (!Opt.ShowMenuBar && Opt.Clock) maxWidth -= 5;
    maxWidth -= gap;

    compactifyFileNamesUpTo(tabPos);

    std::wstring separator = tabPos.size() > 1 ? L"║🧷📜📎 ✘" :  L"║📜";
    maxWidth -= tabPos.size() == 1 ? 1 : 2; // first file has icon too + arrows
    size_t N = tabPos.size();
    size_t sepWidth = separator.size();

    // Total width consumed by separators
    size_t totalSepWidth = sepWidth * (N - 1);

    for(size_t i = 0; i < N; ++i) tabPos[i].x = -1;

    if (maxWidth <= totalSepWidth) {
        // Not enough space even for separators → one button here
        return;
    }

    // Minimum width per leaf
    size_t minLeafWidth = std::max(15UL, (maxWidth - totalSepWidth) / N); // we have scroll so we can make tab visible all time

    // First pass: compute actual widths for each leaf
    for (size_t i = 0; i < N; ++i) {
        size_t full = tabPos[i].cells();
        leafWidths[i] = std::max(minLeafWidth, std::min(full, minLeafWidth));
    }

    // Second pass: distribute remaining space
    size_t usedWidth = totalSepWidth;
    for (size_t w : leafWidths)
        usedWidth += w;

    size_t remaining = (maxWidth > usedWidth ? maxWidth - usedWidth : 0);

    // Distribute remaining space greedily
    for (size_t i = 0; i < N && remaining > 0; ++i) {
        size_t full = tabPos[i].cells();
        size_t current = leafWidths[i];

        if (current < full) {
            size_t add = std::min(full - current, remaining);
            leafWidths[i] += add;
            remaining -= add;
        }
    }

    for (size_t i = 0; i < N; ++i) {
		tabPos[i].display = shortenLeaf(tabPos[i], leafWidths[i]);
		tabPos[i].w = (int)tabPos[i].display.CellsCount();
	}
}

int TabBar::render() 
{
    if (tabPos.empty()) return 0;

	size_t maxWidth;
	int gap;

    size_t N = tabPos.size();
	std::vector<size_t> leafWidths(N);

	layout(maxWidth, gap, leafWidths);
	 
	int activeIndex = activeTab;

	uint64_t color2 = 0;

   	SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, consoleHovered ? 1 : 0, 0, 0));
    consoleX = WhereX();
    FS << L"🢩💻 ";

    leftAX = rightAX = -1;
    if (tabPos.size() > 1) {
	   	SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, leftAHovered ? 1 : 0, 0, 0));
    	leftAX = WhereX();
	    FS << L"⏴ ";

		SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, 0, 0, 0));
	    FS << L"│";
    }

    bool more = false;
    for (size_t i = 0; i < N; ++i) {
    	if (WhereX() > X2 - gap) {
        	tabPos[i].x = -1;
        	more = true;
            break;
    	}

        if ((int)i < visibleTab) {
        	tabPos[i].x = -1;
            continue;
        }

		bool active = i == (size_t)activeIndex;
		bool hover  = i == (size_t)hoveredTab;

        /*
		fprintf(stderr, "... [%d]: `%ls` active=%c hover=%c left=`%ls` right=`%ls`\n", 
			(int)i, 
			tabPos[i].display.CPtr(), 
			active ? 'Y' : 'n', hover ? 'Y' : 'n',
			tabPos[i].left.CPtr(),
			tabPos[i].right.CPtr());
        */
		tabPos[i].x = WhereX() - X1;

		if (X1 + tabPos[i].x + tabPos[i].w > X2 - gap) {
			more = true;
			tabPos[i].x = -1;
			break;
		}

		SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, 0, 0, 0));
		if(i - visibleTab > 0) FS << L"│";

		if (tabPos.size() > 1){
			SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, leftPinHovered == (int)i ? 1 : 0, 0, 0));
			FS << L"🧷";
			tabPos[i].leftPinX = WhereX() - X1 - 1;
			tabPos[i].x = WhereX() - X1;
		} 

		color2 = SoftenItemColor(FarColorToReal(active ? COL_HMENUSELECTEDTEXT : COL_HMENUTEXT), // COL_EDITORSTATUS
			/*active ? 1 :*/ 0, 
			hover ? 1 : 0, 0, 0);
		SetColor(color2);
		FS << (active ? L"📲" /* L"📜" */ : L"📱" /* L"📝" */);
		FS << tabPos[i].display;
		if (tabPos.size() > 1){ 
			SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, rightPinHovered == (int)i ? 1 : 0, 0, 0));
			FS << L"📎";
			tabPos[i].rightPinX = WhereX() - X1 - 1;

			SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, delHovered == (int)i ? 1 : 0, 0, 0));
			FS << L" ✘";
			tabPos[i].delX = WhereX() - X1 - 2;
		}
    }

   	SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, 0, 0, 0));
    FS << L"|";

    if(tabPos.size() > 1) {
	   	SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, rightAHovered ? 1 : 0, 0, 0));
    	rightAX = WhereX();
	    FS << L"⏵ ";
    }

	SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, plusHovered ? 1 : 0, 0, 0));
    plusX = WhereX();
    FS << L" 🞧 ";

    moreX = -1;
    more = true;
    if (more) {
		SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, moreHovered ? 1 : 0, 0, 0));
    	moreX = WhereX();
	    FS << L" ⏷ ";
    }

    if (!Opt.ShowMenuBar) {
		SetColor(SoftenItemColor(FarColorToReal(COL_HMENUTEXT), 0, menuHovered ? 1 : 0, 0, 0));
   		menuX = WhereX();
	    FS << L" ☰";
    }
    else {
    	menuX = -1;
    }

	SetColor(FarColorToReal(COL_HMENUTEXT));
	int clockWidth = !Opt.ShowMenuBar &&Opt.Clock ? 5 : 0;
    if (WhereX() < X2 - clockWidth) 
    	FS << fmt::LeftAlign() << fmt::Cells() << fmt::Expand(X2 - WhereX() + 1 - clockWidth) << L" ";
    return 0;
}

void TabBar::SetTexts(const std::vector<std::wstring>& v, int _activeTab) {
	tabPos.clear();
	tabs = v;
	activeTab = _activeTab;
	hoveredTab = -1;
}

void TabBar::setHoverMask(int tabNo, bool plus, bool del, bool leftPin, bool rightPin, bool more, bool menu, bool console,
	bool leftScroll, bool rightScroll) 
{
	plusHovered = plus;
	moreHovered = more;
	menuHovered = menu;
	consoleHovered = console;
	leftAHovered = leftScroll;
	rightAHovered = rightScroll;
	delHovered = del ? tabNo : -1;
	leftPinHovered = leftPin ? tabNo : -1;
	rightPinHovered = rightPin ? tabNo : -1;

	SetHovered(plus || del || leftPin || rightPin || more || menu || console || leftScroll || rightScroll ? -1 : tabNo);
	CtrlObject->Cp()->SwitchHoveredTabTo(hoveredTab);
	Redraw();
}

int TabBar::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent) 
{
	int MsX = MouseEvent->dwMousePosition.X;
	int MsY = MouseEvent->dwMousePosition.Y;

	if (MsX < X1 || MsX > X2 || MsY != Y1)
		return FALSE;

	int pos = MsX - X1;
	plusHovered = moreHovered = menuHovered = consoleHovered = false;
	delHovered = leftPinHovered = rightPinHovered = -1;
	leftAHovered = rightAHovered = false;

	for(size_t i = 0; i < tabPos.size(); ++i) {
		if(tabPos[i].x < 0 || tabPos[i].w <= 0)
			continue;
		if (pos >= tabPos[i].x && pos <= tabPos[i].x + tabPos[i].w) { // tab
			if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
				if((int)i != hoveredTab){ 
					setHoverMask(i, false, false, false, false, false, false, false, false, false);
				}
				return TRUE;
			}
			else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
				SetActive(i);
				CtrlObject->Cp()->SwitchActiveTabTo(i);
				Redraw();
				return TRUE;
			}

			break;
		}
		else if (tabPos[i].leftPinX >= 0 && pos >= tabPos[i].leftPinX && pos <= tabPos[i].leftPinX + 1) { // left pin
			if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
				setHoverMask(i, false, false, true, false, false, false, false, false, false);
				return TRUE;
			}
			else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
				int k = pathContextMenu(true, tabPos[i].leftPinX, i);
				if (k >= 0 && k != ActiveTab()) {
					CtrlObject->Cp()->SwapTo(i, k, true);
					Redraw();
				}
				return TRUE;
			}
		}
		else if (tabPos[i].rightPinX > 0 && pos >= tabPos[i].rightPinX && pos <= tabPos[i].rightPinX + 1) { // left pin
			if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
				setHoverMask(i, false, false, false, true, false, false, false, false, false);
				return TRUE;
			}
			else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
				int k = pathContextMenu(false, tabPos[i].rightPinX, i);
				if (k >= 0 && k != ActiveTab()) {
					CtrlObject->Cp()->SwapTo(i, k, false);
					Redraw();
				}
				return TRUE;
			}
		}
		else if (tabPos[i].delX > 0 && pos >= tabPos[i].delX && pos <= tabPos[i].delX + 3) { // delete sign
			if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
				setHoverMask(i, false, true, false, false, false, false, false, false, false);
				return TRUE;
			}
			else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
				CtrlObject->Cp()->DeleteTab(i);
				return TRUE;
			}
		}
	}

	if (MsX >= plusX && MsX < plusX + 4) { // plus sign
		if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
			setHoverMask(-1, true, false, false, false, false, false, false, false, false);
			return TRUE;
		}
		else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
			CtrlObject->Cp()->AppendNewTab();
			return TRUE;
		}
	}
	else if (moreX > 0 && MsX >= moreX && MsX < moreX + 2) { // more sign
		if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
			setHoverMask(-1, false, false, false, false, true, false, false, false, false);
			return TRUE;
		}
		else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
			// show menu with all pairs and then choose 
			int tab = moreContextMenu();
			if (tab >= 0) {
				SetActive(tab);
				CtrlObject->Cp()->SwitchActiveTabTo(tab);
				CtrlObject->Cp()->Update();
				Redraw();
			}
			return TRUE;
		}
	}
	else if (menuX > 0 && MsX >= menuX && MsX < menuX + 2) { // menu sign
		if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
			setHoverMask(-1, false, false, false, false, false, true, false, false, false);
			return TRUE;
		}
		else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
			// enforce F9 key
			FrameManager->ProcessKey(KEY_F9);
			return TRUE;
		}
	}
	else if (consoleX >= 0 && MsX >= consoleX && MsX < consoleX + 3) { // console sign
		if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
			setHoverMask(-1, false, false, false, false, false, false, true, false, false);
			return TRUE;
		}
		else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
			FrameManager->ProcessKey(KEY_CTRLO);
			return TRUE;
		}
	}
	else if (leftAX > 0 && MsX >= leftAX && MsX < leftAX + 2) { // console sign
		if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
			setHoverMask(-1, false, false, false, false, false, false, false, true, false);
			return TRUE;
		}
		else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
			// FrameManager->ProcessKey(KEY_CTRLO);
			if (visibleTab > 0) --visibleTab;
			Redraw();
			return TRUE;
		}
	}
	else if (rightAX > 0 && MsX >= rightAX && MsX < rightAX + 2) { // console sign
		if ((MouseEvent->dwEventFlags & MOUSE_MOVED)) {
			setHoverMask(-1, false, false, false, false, false, false, false, false, true);
			return TRUE;
		}
		else if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
			if (visibleTab < (int)tabPos.size() - 2) ++visibleTab;
			Redraw();
			return TRUE;
		}
	}

	return FALSE;
}

int TabBar::pathContextMenu(bool left, int x, int tabNo) 
{
	std::vector<std::wstring> v, s;
	CtrlObject->Cp()->EnlistAllPaths(s, left, false);
	int width = 10;

	for (size_t j = 0; j < s.size(); ++j) {
		width = std::max(width, (int)s[j].size() + 5);
	}

	for (size_t j = 0; j < s.size(); ++j) {
		FARString x;
		auto left = shortenLeaf(s[j], width - 5);
		x.Format(L"%-*.*ls", width - 5, width - 5, left.c_str());
		v.push_back(x.CPtr());
	}

	// now we ready to fill menus
	MenuDataEx Groups[v.size()];
	for (size_t j = 0; j < s.size(); ++j) 
		Groups[j] = { v[j].c_str(), (DWORD)((int)j == tabNo ? MIF_DISABLE : 0), 0 };
	int GroupsLen = (int)v.size();

	{
		int GroupsCode;
		VMenu GroupsMenu(L"", Groups, GroupsLen, 0);

		for (;;) {
			GroupsMenu.SetPosition(x + 1, 1 + Opt.ShowMenuBar, 0, 0);
			GroupsMenu.SetFlags(VMENU_WRAPMODE | VMENU_NOTCHANGE);
			GroupsMenu.SetBoxType(SHORT_DOUBLE_BOX);
			GroupsMenu.ClearDone();
			GroupsMenu.Process();

			GroupsCode = GroupsMenu.Modal::GetExitCode();

			if (GroupsCode < 0 || GroupsCode >= GroupsLen) break;

			return GroupsCode;
		}
	}
	return -1;
}

FARString TabBar::getFormattedTitle(size_t j, int width) 
{
	FARString x;
	if (tabPos[j].right.IsEmpty()) {
		auto left = shortenLeaf(tabPos[j].left.CPtr(), width - 5);
		x.Format(L"%-*.*ls", width - 5, width - 5, left.c_str());
	}
	else {
		auto left = shortenLeaf(tabPos[j].left.CPtr(), (width - 3)/2);
		auto right= shortenLeaf(tabPos[j].right.CPtr(), (width - 3)/2);
		x.Format(L"%-*.*ls - %-*.*ls", 
			(width - 3)/2, (width - 3)/2, left.c_str(), 
			(width - 3)/2, (width - 3)/2, right.c_str());
	}
	return x;
}

int TabBar::moreContextMenu() 
{
	int width = 10;

	// count list size and widths
	std::vector<std::wstring> v;
	v.reserve(tabPos.size());
	for (size_t j = 0; j < tabPos.size(); ++j) {
		width = std::max(width, (int)(tabPos[j].left.CellsCount() + tabPos[j].right.CellsCount() + 3 + 5));
	}

	for (size_t j = 0; j < tabPos.size(); ++j) {
		FARString x = getFormattedTitle(j, width);
		v.push_back(x.CPtr());
	}

	// now we ready to fill menus
	MenuDataEx Groups[tabPos.size()];
	for (size_t j = 0; j < tabPos.size(); ++j) 
		Groups[j] = { v[j].c_str(), 0, 0 };
	int GroupsLen = (int)tabPos.size();

	{
		int GroupsCode;
		VMenu GroupsMenu(L"", Groups, GroupsLen, 0);

		for (;;) {
			GroupsMenu.SetPosition(moreX + 1 - width / 2, 1 + Opt.ShowMenuBar, 0, 0);
			GroupsMenu.SetFlags(VMENU_WRAPMODE | VMENU_NOTCHANGE);
			GroupsMenu.SetBoxType(SHORT_DOUBLE_BOX);
			GroupsMenu.ClearDone();
			GroupsMenu.Process();

			GroupsCode = GroupsMenu.Modal::GetExitCode();

			if (GroupsCode < 0 || GroupsCode >= GroupsLen) break;

			return GroupsCode;
		}
	}
	return -1;
}
