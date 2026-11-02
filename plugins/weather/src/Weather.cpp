#include "weather.h"

#include <farplug-wide.h>
#include <utils.h>

#include <fardlgbuilder.hpp>
#include <farosdpanel.hpp>

#include "WeatherCore.hpp"

static struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
struct Options options;

static std::shared_ptr<WeatherCore> weatherCore;
static int osdRowNumber = -1;
static OsdPanelInterface* osd = nullptr;
static int last_min = -1;

const TCHAR *GetMsg(int MsgId) {
    return (Info.GetMsg(Info.ModuleNumber, MsgId));
}

static void init() {
	std::shared_ptr<IGeoLocationService> geo;
	if(options.lat != 0.0 && options.lon != 0.0)
		geo = std::make_shared<ManualGeoLocationService>(options.lat, options.lon, options.name);
	else
		geo = std::make_shared<IpApiGeoLocationService>();
    auto weather = std::make_shared<OpenMeteoWeatherService>();

    weatherCore = std::make_shared<WeatherCore>(geo, weather);
    weatherCore->Initialize();

    ProcessSynchroEventW(-1, nullptr);
}

static void shutdown() {
	if(osd && osdRowNumber > 0)
		osd->CompleteLine(osdRowNumber);
	osd = nullptr;
	osdRowNumber = -1;
	last_min = -1;
	weatherCore->Shutdown();
}

SHAREDSYMBOL void WINAPI ExitFARW(void)
{
	weatherCore->Shutdown();
}

#include <string_view>
#include <cmath>

const char* CompassDirection(double degrees)
{
    static const char* dirs[] = {
        "N", "NNE", "NE", "ENE",
        "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW",
        "W", "WNW", "NW", "NNW"
    };

    degrees = std::fmod(degrees, 360.0);
    if (degrees < 0)
        degrees += 360.0;

    const int index = static_cast<int>((degrees + 11.25) / 22.5) % 16;
    return dirs[index];
}

std::wstring format(const WeatherDataSnapshot& m_data, const TimeOfDay& m_tod) 
{
    static wchar_t buf[256];

    wchar_t symbol[6] = L"⛅☀️";
    switch (m_data.condition) {
        case WEATHER_RAIN: symbol[0] = L'🌧️'; break;
        case WEATHER_SNOW: symbol[0] = L'❄️'; break;
        case WEATHER_THUNDERSTORM: symbol[0] = L'⚡'; break;
        case WEATHER_FOG: symbol[0] = L'🌫️'; break;
        case WEATHER_CLOUDY: symbol[0] = L'⛅'; break;
        case WEATHER_CLEAR: symbol[0] = L'🌞';  break;
        default: symbol[0] = L'⛅'; break;
    }

    switch(m_tod) {
    case TOD_DUSK: symbol[1] = L'🌆'; break;
    case TOD_NIGHT: symbol[1] = L'🌃'; break;
    case TOD_DAWN: symbol[1] = L'🌇'; break;
    case TOD_DAY: 
    default:
    	symbol[1] = L'☀️'; break;
    }
    // symbol[1] = (m_tod == TOD_NIGHT) ? L"🌙" : L"☀️";

    swprintf(buf, sizeof(buf)/sizeof(wchar_t), 
    	L"%-12.12hs %ls %d°C Hum %d%% Wind ↖%hs %d km/h   ", 
    	m_data.locationName.c_str(), symbol, m_data.temperatureCelsius,
    	m_data.humidityPercent, CompassDirection(m_data.windDirectionDegrees), m_data.windSpeedKmh);
    return std::wstring(buf);
}

static int cur_min() {
    using namespace std::chrono;

    // get local time
    auto now = system_clock::now();
    time_t tt = system_clock::to_time_t(now);
    tm local{};
    localtime_r(&tt, &local);   // or localtime_s on Windows

    int minute = local.tm_min;
    return minute;
}

SHAREDSYMBOL int WINAPI ProcessSynchroEventW(int Event, void *Param)
{
	(void)Param;
	// if (Event != SE_COMMONSYNCHRO) return 0;

	fprintf(stderr, "ProcessSynchroEventW: %d\n", Event);

	if (osdRowNumber < 0 || !osd || last_min != cur_min()) {

		fprintf(stderr, "ProcessSynchroEventW: %d -> up\n", Event);

		if (!osd) osd = (OsdPanelInterface*)(*Info.OsdAPI)();
		fprintf(stderr, "ProcessSynchroEventW: %d -> up, osd api = %p\n", Event, osd);

		last_min = cur_min();

		WeatherDataSnapshot data = weatherCore->GetData();
		TimeOfDay tod = weatherCore->GetCurrentTimeOfDay();
		std::wstring row = format(data, tod);
		if (osdRowNumber < 0){ 
			fprintf(stderr, "ProcessSynchroEventW: %d -> up, osd api = %p, row = %d, NEW `%ls`\n", Event, osd, osdRowNumber, row.c_str());
			osdRowNumber = osd->AddLine(row);
		}
		else {
			fprintf(stderr, "ProcessSynchroEventW: %d -> up, osd api = %p, row = %d, UPD\n", Event, osd, osdRowNumber);
			osd->UpdateLine(osdRowNumber, format(data, tod));
		}
	}
	return 1;
}

SHAREDSYMBOL int WINAPI EXP_NAME(GetMinFarVersion)() {
    return FARMANAGERVERSION;
}

SHAREDSYMBOL void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info) {
    ::Info     = *Info;
    ::FSF      = *Info->FSF;
    ::Info.FSF = &::FSF;

    RestoreConfig(&options);
    if (options.enabled) init();
	fprintf(stderr, "Weather: started\n");
}

SHAREDSYMBOL void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info) {

    memset(Info, 0, sizeof(*Info));

    Info->StructSize            = sizeof(*Info);
    Info->Flags                 = PF_PRELOAD;
    Info->DiskMenuStringsNumber = 0;

    static const TCHAR *PluginMenuStrings[1];
    PluginMenuStrings[0]            = GetMsg(LNG_MENU);
    Info->PluginMenuStringsNumber   = 0;
    Info->PluginConfigStrings         = PluginMenuStrings;
    Info->PluginConfigStringsNumber = ARRAYSIZE(PluginMenuStrings);
}

SHAREDSYMBOL int WINAPI EXP_NAME(Configure)(int) {
	PluginDialogBuilder Builder(Info, LNG_SETTINGS_TITLE, NULL);

	std::wstring wname = StrMB2Wide(options.name);
	wchar_t buf[64];
	wcscpy(buf, wname.c_str());

	Builder.AddCheckbox(LNG_SETTINGS_ENABLED, &options.enabled);
	Builder.StartColumns();
	Builder.AddText(LNG_SETTINGS_LAT_T);
	Builder.AddText(LNG_SETTINGS_LON_T);
	Builder.AddText(LNG_SETTINGS_CITY_T);
	Builder.ColumnBreak();
	Builder.AddDoubleEditField(&options.lat, 10);
	Builder.AddDoubleEditField(&options.lon, 10);
	Builder.AddEditField(buf, 12, 16);
	Builder.EndColumns();

	Builder.AddOKCancel(LNG_SETTINGS_OK, LNG_SETTINGS_CANCEL);
	Options old = options;
	if (Builder.ShowDialog()) {
		wname = buf;
		options.name = StrWide2MB(wname);
		SaveConfig(&options);

		if (old.enabled != options.enabled) {
			if (options.enabled) 
				init();
			else
				shutdown();
		}
		else if (old.lat != options.lat || old.lon != options.lon) {
			if (options.enabled) {
				shutdown();
				init();
			}
		}
	}
	return 0;
}
