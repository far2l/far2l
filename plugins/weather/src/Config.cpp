#include "farplug-wide.h"

#include <WideMB.h>
#include <utils.h>
#include <KeyFileHelper.h>

#include "weather.h"

#define INI_LOCATION InMyConfig("plugins/weather/config.ini")
#define INI_SECTION  "Settings"

static const char sKeyEnabled[] = ("Enabled");
static const char sKeyLat[] = ("Latitude");
static const char sKeyLon[] = ("Longtitude");
static const char sKeyCity[] = ("Name");

static double GetDouble(KeyFileReadSection& kfh, const char* key, double dfl) {
	wchar_t dflStr[32];
	swprintf(dflStr, 32, L"%lf", dfl);
	double z = dfl;
	std::wstring x = kfh.GetString(key, dflStr);
	swscanf(x.c_str(), L"%ld", &z);
	return z;
}

static void SetDouble(KeyFileHelper& kfh, const char* section, const char* key, double v) 
{
	char vstr[32];
	snprintf(vstr, 32, "%lf", v);
	kfh.SetString(section, key, std::string(vstr));
}

void RestoreConfig(Options *options)
{
	KeyFileReadSection kfh(INI_LOCATION, INI_SECTION);
	options->enabled = !!kfh.GetInt(sKeyEnabled, options->enabled);
	options->lat = GetDouble(kfh, sKeyLat, 0.0);
	options->lon = GetDouble(kfh, sKeyLon, 0.0);
	options->name = kfh.GetString(sKeyCity, "");
}

void SaveConfig(const Options *options)
{
	KeyFileHelper kfh(INI_LOCATION);
	kfh.SetInt(INI_SECTION, sKeyEnabled, options->enabled);
	SetDouble(kfh, INI_SECTION, sKeyLat, options->lat);
	SetDouble(kfh, INI_SECTION, sKeyLon, options->lon);
	kfh.SetString(INI_SECTION, sKeyCity, options->name);
}
