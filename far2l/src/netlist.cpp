/*
netlist.cpp

Список сокетов (Ctrl-Shift-W)
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

#include "netlist.hpp"
#include "netstat_enum.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "message.hpp"
#include "config.hpp"
#include "interf.hpp"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <string>

#define AUTOREFRESH_MSEC 5000

inline static void ShowSocketListTitle(VMenu &NetList, bool autorefresh, int sort_key)
{
	FARString title;
	title.Format(Msg::SocketListTitle, autorefresh ? "on" : "off", (char) sort_key);
	NetList.SetTitle(title);
}

void ShowSocketList(Panel *ActivePanel)
{
	MenuDataEx dummy{L"", 0, 0}; // will refresh immediately
	VMenu NetList(nullptr/*Msg::SocketListTitle*/, &dummy, 1, ScrY - 4);

	NetList.SetPosition(-1, -1, 0, 0);
	NetList.SetFlags(VMENU_WRAPMODE | VMENU_NOTCHANGE);
	NetList.SetBottomTitle(Msg::SocketListBottom);
	NetList.ClearDone();

	int sort_key = 'T';
	clock_t last_refresh = 0, schedule_refresh = 0;
	bool autorefresh = true;
	enum
	{
		AT_CHOSEN,
		AT_TOP,
		AT_BOTTOM,
	} keep{AT_BOTTOM};

	ShowSocketListTitle(NetList, autorefresh, sort_key);
	NetList.Show();
	NetList.SetRegularIdle(true);

   	NetstatFar2l::SocketFilter filter = NetstatFar2l::SocketFilter::OPEN_AND_CONNECTED;
    NetstatFar2l::Far2lColumnSizes sizes = NetstatFar2l::Far2lColumnSizes::Standard();

	std::vector<NetstatFar2l::ConnectionEntry> entries;

	for (unsigned int loop_id = 1; !NetList.Done(); ++loop_id) {
		const auto now = GetProcessUptimeMSec();
		if (last_refresh == 0 || (schedule_refresh && (now >= schedule_refresh || now < last_refresh))) {
			int selected_pos = NetList.GetSelectPos();
			int selected_pid = selected_pos < (int)entries.size() ? entries[selected_pos].pid : getpid();
			NetList.Hide();
			NetList.DeleteItems();

			NetList.SetPosition(-1,-1,0,0);

    		// Enumerate connections
    		entries = NetstatFar2l::EnumerateConnections(filter);

			if (sort_key == 't' || sort_key == 'T') // proTocol
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::ConnectionEntry& a, const NetstatFar2l::ConnectionEntry& b) {
						return sort_key == 'T' ? b.protocol < a.protocol : a.protocol < b.protocol;
					});
			else if (sort_key == 'n' || sort_key == 'N') // process Name
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::ConnectionEntry& a, const NetstatFar2l::ConnectionEntry& b) {
						return sort_key == 'n' ? b.process_name < a.process_name : a.process_name < b.process_name;
					});
			else if (sort_key == 'i' || sort_key == 'I') // state Info
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::ConnectionEntry& a, const NetstatFar2l::ConnectionEntry& b) {
						return sort_key == 'i' ? a.state_name < b.state_name : b.state_name < a.state_name;
					});
			else if (sort_key == 'c' || sort_key == 'C') // loCal addr + port
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::ConnectionEntry& a, const NetstatFar2l::ConnectionEntry& b) {
						auto a_str = a.local_addr + ":" + std::to_string(a.local_port);
						auto b_str = b.local_addr + ":" + std::to_string(b.local_port);
						return sort_key == 'C' ? b_str < a_str : a_str < b_str;
					});
			else if (sort_key == 'm' || sort_key == 'M') // reMote addr + port
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::ConnectionEntry& a, const NetstatFar2l::ConnectionEntry& b) {
						auto a_str = a.remote_addr + ":" + std::to_string(a.remote_port);
						auto b_str = b.remote_addr + ":" + std::to_string(b.remote_port);
						return sort_key == 'M' ? b_str < a_str : a_str < b_str;
					});

            auto v = NetstatFar2l::FormatConnections(entries, sizes, true);

			for (size_t j = 0; j < v.size(); ++j) {
				MenuItemEx item;
				item.strName = v[j];
				item.AccelKey = 0;
				if (j < entries.size() && entries[j].pid == selected_pid && keep == AT_CHOSEN) {
					item.Flags = LIF_SELECTED;
					selected_pos = -1;
				}
				NetList.AddItem(&item);
			}
			if (keep == AT_TOP) {
				NetList.SetSelectPos(0, 1);
			} else if (keep == AT_BOTTOM) {
				NetList.SetSelectPos(NetList.GetItemCount() - 1, -1);
			} else if (selected_pos != -1) {
				selected_pos = std::min(selected_pos, NetList.GetItemCount() - 1);
				if (selected_pos >= 0) {
					NetList.SetSelectPos(selected_pos, -1);
				}
			}

			NetList.Show();
			last_refresh = GetProcessUptimeMSec();
			schedule_refresh = autorefresh ? last_refresh + AUTOREFRESH_MSEC : 0;
		}
		FarKey key = NetList.ReadInput();
		switch (key) {
		case KEY_F1:
			Help::Present(L"TaskList");
			break;
		case 't': case 'T':
		case 'n': case 'N':
		case 'i': case 'I':
		case 'c': case 'C':
		case 'm': case 'M':
			sort_key = key;
			schedule_refresh = GetProcessUptimeMSec();
			ShowSocketListTitle(NetList, autorefresh, sort_key);
			break;
		case KEY_CTRLR:
			autorefresh = !autorefresh;
			schedule_refresh = autorefresh ? GetProcessUptimeMSec() : 0;
			ShowSocketListTitle(NetList, autorefresh, sort_key);
			NetList.Show();
			break;
		case KEY_HOME:
			keep = AT_TOP;
			NetList.ProcessInput();
			break;
		case KEY_END:
			keep = AT_BOTTOM;
			NetList.ProcessInput();
			break;
		case KEY_NONE: case KEY_IDLE:
			break;
		default:
			keep = AT_CHOSEN;
			if (schedule_refresh) { // postpone autorefresh while user typing something
				schedule_refresh = GetProcessUptimeMSec() + AUTOREFRESH_MSEC;
			}
			NetList.ProcessInput();
		}
	}
}
