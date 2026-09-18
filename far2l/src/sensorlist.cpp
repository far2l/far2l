/*
sensorlist.cpp

Список сенсоров
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

#include "sensorlist.hpp"
#include "sensor_enum.hpp"
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

inline static void ShowSensorListTitle(VMenu &SensorList, bool autorefresh, int sort_key)
{
	FARString title;
	title.Format(Msg::SensorListTitle, autorefresh ? "on" : "off", (char) sort_key);
	SensorList.SetTitle(title);
}

void ShowSensorList(Panel *ActivePanel)
{
	MenuDataEx dummy{L"", 0, 0}; // will refresh immediately
	VMenu SensorList(nullptr/*Msg::SensorListTitle*/, &dummy, 1, ScrY - 4);

	SensorList.SetPosition(-1, -1, 0, 0);
	SensorList.SetFlags(VMENU_WRAPMODE | VMENU_NOTCHANGE);
	SensorList.SetBottomTitle(Msg::SensorListBottom);
	SensorList.ClearDone();

	int sort_key = 'T';
	clock_t last_refresh = 0, schedule_refresh = 0;
	bool autorefresh = true;
	enum
	{
		AT_CHOSEN,
		AT_TOP,
		AT_BOTTOM,
	} keep{AT_BOTTOM};

	ShowSensorListTitle(SensorList, autorefresh, sort_key);
	SensorList.Show();
	SensorList.SetRegularIdle(true);

   	NetstatFar2l::SensorType filter = NetstatFar2l::SensorType::ALL;
    NetstatFar2l::Far2lSensorColumnSizes sizes = NetstatFar2l::Far2lSensorColumnSizes::Standard();

	std::vector<NetstatFar2l::SensorEntry> entries;

	for (unsigned int loop_id = 1; !SensorList.Done(); ++loop_id) {
		const auto now = GetProcessUptimeMSec();
		if (last_refresh == 0 || (schedule_refresh && (now >= schedule_refresh || now < last_refresh))) {
			int selected_pos = SensorList.GetSelectPos();
			SensorList.Hide();
			SensorList.DeleteItems();

			SensorList.SetPosition(-1,-1,0,0);

    		// Enumerate sensors
    		entries = NetstatFar2l::EnumerateSensors(filter);

			if (sort_key == 't' || sort_key == 'T') // Type
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::SensorEntry& a, const NetstatFar2l::SensorEntry& b) {
						return sort_key == 'T' ? b.type_str < a.type_str : a.type_str < b.type_str;
					});
			else if (sort_key == 'n' || sort_key == 'N') // Name
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::SensorEntry& a, const NetstatFar2l::SensorEntry& b) {
						return sort_key == 'n' ? b.sensor_name < a.sensor_name : a.sensor_name < b.sensor_name;
					});
			else if (sort_key == 'i' || sort_key == 'I') // chIp
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::SensorEntry& a, const NetstatFar2l::SensorEntry& b) {
						return sort_key == 'i' ? a.chip_name < b.chip_name : b.chip_name < a.chip_name;
					});
			else if (sort_key == 'c' || sort_key == 'C') // current_value
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::SensorEntry& a, const NetstatFar2l::SensorEntry& b) {
						return sort_key == 'C' ? b.current_value < a.current_value : a.current_value < b.current_value;
					});
			else if (sort_key == 'm' || sort_key == 'M') // alarMed
				std::sort(entries.begin(), entries.end(),
					[sort_key](const NetstatFar2l::SensorEntry& a, const NetstatFar2l::SensorEntry& b) {
						return sort_key == 'M' ? b.status < a.status : a.status < b.status;
					});

            auto v = NetstatFar2l::FormatSensors(entries, sizes, true);

			for (size_t j = 0; j < v.size(); ++j) {
				MenuItemEx item;
				item.strName = v[j];
				item.AccelKey = 0;
				if (j < entries.size() && (int)j == selected_pos && keep == AT_CHOSEN) {
					item.Flags = LIF_SELECTED;
					selected_pos = -1;
				}
				SensorList.AddItem(&item);
			}
			if (keep == AT_TOP) {
				SensorList.SetSelectPos(0, 1);
			} else if (keep == AT_BOTTOM) {
				SensorList.SetSelectPos(SensorList.GetItemCount() - 1, -1);
			} else if (selected_pos != -1) {
				selected_pos = std::min(selected_pos, SensorList.GetItemCount() - 1);
				if (selected_pos >= 0) {
					SensorList.SetSelectPos(selected_pos, -1);
				}
			}

			SensorList.Show();
			last_refresh = GetProcessUptimeMSec();
			schedule_refresh = autorefresh ? last_refresh + AUTOREFRESH_MSEC : 0;
		}
		FarKey key = SensorList.ReadInput();
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
			ShowSensorListTitle(SensorList, autorefresh, sort_key);
			break;
		case KEY_CTRLR:
			autorefresh = !autorefresh;
			schedule_refresh = autorefresh ? GetProcessUptimeMSec() : 0;
			ShowSensorListTitle(SensorList, autorefresh, sort_key);
			SensorList.Show();
			break;
		case KEY_HOME:
			keep = AT_TOP;
			SensorList.ProcessInput();
			break;
		case KEY_END:
			keep = AT_BOTTOM;
			SensorList.ProcessInput();
			break;
		case KEY_NONE: case KEY_IDLE:
			break;
		default:
			keep = AT_CHOSEN;
			if (schedule_refresh) { // postpone autorefresh while user typing something
				schedule_refresh = GetProcessUptimeMSec() + AUTOREFRESH_MSEC;
			}
			SensorList.ProcessInput();
		}
	}
}
