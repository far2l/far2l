/**
 * @file WeatherCore.cpp
 * @brief Implementation of Far2l Utility Library Weather & Geolocation Services
 */

#include "WeatherCore.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <regex>

#ifdef USE_CURL
#include <curl/curl.h>
#endif

#include "farplug-wide.h"

#include <WideMB.h>
#include <utils.h>
#include <KeyFileHelper.h>

#include "weather.h"

namespace {
#ifdef USE_CURL
size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total);
    return total;
}

std::string HttpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "far2l-WeatherUtility/2.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK) ? response : "";
}
#else
std::string HttpGet(const std::string&) {
    // Fallback stub if curl is not linked
    return "";
}
#endif

std::string ExpandPath(const std::string& path) {
    if (path.empty() || path[0] != '~') return path;
    const char* home = std::getenv("HOME");
    if (!home) return path;
    return std::string(home) + path.substr(1);
}
} // anonymous namespace

// ============================================================================
// IpApiGeoLocationService Implementation
// ============================================================================

IpApiGeoLocationService::IpApiGeoLocationService(const std::string& endpoint)
    : m_endpoint(endpoint) {}

bool IpApiGeoLocationService::ResolveLocation(double& outLat, double& outLon, std::string& outLocationName) {
    std::string res = HttpGet(m_endpoint);
    if (res.empty()) return false;

    std::smatch m;
    if (std::regex_search(res, m, std::regex(R"(\"status\":\"success\")"))) {
        std::smatch latMatch, lonMatch, cityMatch;
        if (std::regex_search(res, latMatch, std::regex(R"(\"lat\":(-?\d+\.?\d*))")) &&
            std::regex_search(res, lonMatch, std::regex(R"(\"lon\":(-?\d+\.?\d*))"))) {
            outLat = std::stod(latMatch[1]);
            outLon = std::stod(lonMatch[1]);
            if (std::regex_search(res, cityMatch, std::regex(R"(\"city\":\"([^\"]+)\")"))) {
                outLocationName = cityMatch[1];
            } else {
                outLocationName = "Auto Detected";
            }
            return true;
        }
    }
    return false;
}

// ============================================================================
// ManualGeoLocationService Implementation
// ============================================================================

ManualGeoLocationService::ManualGeoLocationService(double lat, double lon, const std::string& name)
    : m_lat(lat), m_lon(lon), m_name(name) {}

bool ManualGeoLocationService::ResolveLocation(double& outLat, double& outLon, std::string& outLocationName) {
    outLat = m_lat;
    outLon = m_lon;
    outLocationName = m_name;
    return true;
}

void ManualGeoLocationService::SetCoordinates(double lat, double lon, const std::string& name) {
    m_lat = lat;
    m_lon = lon;
    m_name = name;
}

// ============================================================================
// OpenMeteoWeatherService Implementation
// ============================================================================

bool OpenMeteoWeatherService::QueryWeather(double lat, double lon, WeatherDataSnapshot& outData) {
    std::ostringstream url;
    url << "https://api.open-meteo.com/v1/forecast?latitude=" << lat << "&longitude=" << lon
        << "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m,precipitation,surface_pressure";

    std::string response = FetchHttp(url.str());
    if (response.empty()) return false;

    std::smatch m;
    if (std::regex_search(response, m, std::regex(R"(\"temperature_2m\":(-?\d+\.?\d*))"))) {
        outData.temperatureCelsius = static_cast<int>(std::round(std::stod(m[1])));
    }
    if (std::regex_search(response, m, std::regex(R"(\"weather_code\":(\d+))"))) {
        outData.weatherCode = std::stoi(m[1]);
        outData.condition = WeatherCore::CodeToCondition(outData.weatherCode);
        outData.conditionDescription = WeatherCore::ConditionToString(outData.condition);
    }
    if (std::regex_search(response, m, std::regex(R"(\"relative_humidity_2m\":(\d+))"))) {
        outData.humidityPercent = std::stoi(m[1]);
    }
    if (std::regex_search(response, m, std::regex(R"(\"wind_speed_10m\":(\d+\.?\d*))"))) {
        outData.windSpeedKmh = static_cast<int>(std::round(std::stod(m[1])));
    }
    if (std::regex_search(response, m, std::regex(R"(\"wind_direction_10m\":(\d+))"))) {
        outData.windDirectionDegrees = std::stoi(m[1]);
    }
    if (std::regex_search(response, m, std::regex(R"(\"precipitation\":(\d+\.?\d*))"))) {
        outData.precipitationMm = static_cast<float>(std::stod(m[1]));
    }
    if (std::regex_search(response, m, std::regex(R"(\"surface_pressure\":(\d+\.?\d*))"))) {
        outData.pressureHpa = static_cast<int>(std::round(std::stod(m[1])));
    }

    outData.latitude = lat;
    outData.longitude = lon;
    outData.timestamp = std::chrono::system_clock::now();
    outData.isCached = false;
    outData.isFallback = false;
    return true;
}

std::string OpenMeteoWeatherService::FetchHttp(const std::string& url) {
    return HttpGet(url);
}

// ============================================================================
// WeatherCore Implementation
// ============================================================================

WeatherCore::WeatherCore(
    std::shared_ptr<IGeoLocationService> geoService,
    std::shared_ptr<IWeatherService> weatherService,
    const WeatherCoreConfig& config
)
    : m_geoService(std::move(geoService))
    , m_weatherService(std::move(weatherService))
    , m_config(config)
{
}

WeatherCore::~WeatherCore() {
    Shutdown();
}

void WeatherCore::Initialize() {
    if (m_isRunning.exchange(true)) return;

    // Load offline cache on startup for immediate display
    LoadCacheFromDisk();

    // Start background polling thread
    m_workerThread = std::thread(&WeatherCore::WorkerLoop, this);
}

void WeatherCore::Shutdown() {
    if (!m_isRunning.exchange(false)) return;

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    SaveCacheToDisk();
}

void WeatherCore::QueryWeatherAsync() {
    if (m_isOffline.load() || m_queryInProgress.load()) return;

    std::thread([this]() {
        m_queryInProgress.store(true);
        double lat = 51.5074;
        double lon = -0.1278;
        std::string locName = "London";

        if (m_geoService) {
            m_geoService->ResolveLocation(lat, lon, locName);
        }

        WeatherDataSnapshot snapshot;
        snapshot.locationName = locName;

        bool success = false;
        if (m_weatherService) {
            success = m_weatherService->QueryWeather(lat, lon, snapshot);
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (success) {
                m_currentData = snapshot;
                m_currentData.locationName = locName;
            } else {
                m_currentData.isFallback = true;
            }
        }

        if (success) {
            SaveCacheToDisk();
            for (auto& cb : m_callbacks) {
                cb(snapshot);
            }
        }

        m_queryInProgress.store(false);
    }).detach();
}

WeatherDataSnapshot WeatherCore::GetData() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentData;
}

TimeOfDay WeatherCore::GetCurrentTimeOfDay() const {
    if (m_config.timeOfDayOverride != TOD_AUTO) {
        return m_config.timeOfDayOverride;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm{};
#if defined(_WIN32)
    localtime_s(&local_tm, &tt);
#else
    localtime_r(&tt, &local_tm);
#endif
    int hour = local_tm.tm_hour;
    if (hour >= 5 && hour < 8) return TOD_DAWN;
    if (hour >= 8 && hour < 18) return TOD_DAY;
    if (hour >= 18 && hour < 21) return TOD_DUSK;
    return TOD_NIGHT;
}

void WeatherCore::SetGeoLocationService(std::shared_ptr<IGeoLocationService> geoService) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_geoService = std::move(geoService);
}

void WeatherCore::SetWeatherService(std::shared_ptr<IWeatherService> weatherService) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_weatherService = std::move(weatherService);
}

void WeatherCore::SetOfflineMode(bool offline) {
    m_isOffline.store(offline);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentData.isOffline = offline;
    if (offline) {
        m_currentData.isCached = true;
    }
}

bool WeatherCore::IsOfflineMode() const {
    return m_isOffline.load();
}

bool WeatherCore::SaveCacheToDisk(const std::string& customPath) const {
    std::string path = customPath.empty() ? ExpandPath(m_config.cacheFilePath) : customPath;
    std::lock_guard<std::mutex> lock(m_mutex);

    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "{\n";
    file << "  \"temperature\": " << m_currentData.temperatureCelsius << ",\n";
    file << "  \"weatherCode\": " << m_currentData.weatherCode << ",\n";
    file << "  \"humidity\": " << m_currentData.humidityPercent << ",\n";
    file << "  \"windSpeed\": " << m_currentData.windSpeedKmh << ",\n";
    file << "  \"windDirection\": " << m_currentData.windDirectionDegrees << ",\n";
    file << "  \"precipitation\": " << m_currentData.precipitationMm << ",\n";
    file << "  \"pressure\": " << m_currentData.pressureHpa << ",\n";
    file << "  \"location\": \"" << m_currentData.locationName << "\"\n";
    file << "}\n";
    return true;
}

bool WeatherCore::LoadCacheFromDisk(const std::string& customPath) {
    std::string path = customPath.empty() ? ExpandPath(m_config.cacheFilePath) : customPath;
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    std::lock_guard<std::mutex> lock(m_mutex);
    std::smatch m;
    if (std::regex_search(content, m, std::regex(R"(\"temperature\":\s*(-?\d+))"))) {
        m_currentData.temperatureCelsius = std::stoi(m[1]);
    }
    if (std::regex_search(content, m, std::regex(R"(\"weatherCode\":\s*(\d+))"))) {
        m_currentData.weatherCode = std::stoi(m[1]);
        m_currentData.condition = CodeToCondition(m_currentData.weatherCode);
        m_currentData.conditionDescription = ConditionToString(m_currentData.condition);
    }
    if (std::regex_search(content, m, std::regex(R"(\"humidity\":\s*(\d+))"))) {
        m_currentData.humidityPercent = std::stoi(m[1]);
    }
    if (std::regex_search(content, m, std::regex(R"(\"windSpeed\":\s*(\d+))"))) {
        m_currentData.windSpeedKmh = std::stoi(m[1]);
    }
    if (std::regex_search(content, m, std::regex(R"(\"location\":\s*\"([^\"]+)\")"))) {
        m_currentData.locationName = m[1];
    }
    m_currentData.isCached = true;
    return true;
}

void WeatherCore::RegisterUpdateCallback(UpdateCallback cb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callbacks.push_back(cb);
}

void WeatherCore::WorkerLoop() {
    QueryWeatherAsync();

    while (m_isRunning.load()) {
        int intervalSeconds = 15 * 60;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            intervalSeconds = std::max(1, m_config.queryIntervalMinutes) * 60;
        }

        for (int i = 0; i < intervalSeconds && m_isRunning.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (m_isRunning.load() && !m_isOffline.load()) {
            QueryWeatherAsync();
        }
    }
}

std::string WeatherCore::ConditionToString(WeatherCondition cond) {
    switch (cond) {
        case WEATHER_CLEAR: return "Clear Sky";
        case WEATHER_CLOUDY: return "Partly Cloudy";
        case WEATHER_RAIN: return "Rain Showers";
        case WEATHER_SNOW: return "Snowfall";
        case WEATHER_THUNDERSTORM: return "Thunderstorm";
        case WEATHER_FOG: return "Fog & Mist";
        default: return "Unknown";
    }
}

WeatherCondition WeatherCore::CodeToCondition(int wmoCode) {
    if (wmoCode == 0 || wmoCode == 1) return WEATHER_CLEAR;
    if (wmoCode == 2 || wmoCode == 3) return WEATHER_CLOUDY;
    if (wmoCode == 45 || wmoCode == 48) return WEATHER_FOG;
    if ((wmoCode >= 51 && wmoCode <= 67) || (wmoCode >= 80 && wmoCode <= 82)) return WEATHER_RAIN;
    if ((wmoCode >= 71 && wmoCode <= 77) || (wmoCode >= 85 && wmoCode <= 86)) return WEATHER_SNOW;
    if (wmoCode >= 95 && wmoCode <= 99) return WEATHER_THUNDERSTORM;
    return WEATHER_CLOUDY;
}
