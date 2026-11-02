#pragma once

#include <string>
#include <stdint.h>

struct Options{
	int   enabled;

	double lat;
	double lon;
	std::string name;
};

enum {
	LNG_MENU, 
	LNG_SETTINGS_TITLE, LNG_SETTINGS_ENABLED, LNG_SETTINGS_LAT_T, LNG_SETTINGS_LON_T, LNG_SETTINGS_CITY_T,
	LNG_SETTINGS_OK, LNG_SETTINGS_CANCEL,
	LNG_LAST 
};

void RestoreConfig(Options *options);
void SaveConfig(const Options *options);
