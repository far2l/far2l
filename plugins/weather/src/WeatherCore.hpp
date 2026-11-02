/**
 * @file WeatherCore.hpp
 * @brief Far2l Utility Library - Extensible Weather & Geolocation Services
 * 
 * Standalone utility classes without GUI dependencies.
 * Provides pluggable interfaces for geolocation and weather providers,
 * persistent offline caching, and thread-safe background updates.
 */

#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>

// Weather condition classification
enum WeatherCondition {
    WEATHER_CLEAR,
    WEATHER_CLOUDY,
    WEATHER_RAIN,
    WEATHER_SNOW,
    WEATHER_THUNDERSTORM,
    WEATHER_FOG,
    WEATHER_UNKNOWN
};

// Time of day classification
enum TimeOfDay {
    TOD_DAWN,
    TOD_DAY,
    TOD_DUSK,
    TOD_NIGHT,
    TOD_AUTO
};

// Pure snapshot data structure passed around
struct WeatherDataSnapshot {
    int temperatureCelsius{ 18 };
    WeatherCondition condition{ WEATHER_CLOUDY };
    std::string conditionDescription{ "Partly Cloudy" };
    int weatherCode{ 2 };
    int humidityPercent{ 65 };
    int windSpeedKmh{ 14 };
    int windDirectionDegrees{ 315 };  // NW
    float precipitationMm{ 0.0f };
    int pressureHpa{ 1013 };
    std::string locationName{ "London" };
    double latitude{ 51.5074 };
    double longitude{ -0.1278 };
    std::chrono::system_clock::time_point timestamp{ std::chrono::system_clock::now() };
    bool isCached{ false };
    bool isOffline{ false };
    bool isFallback{ false };
};

// ============================================================================
// 1. Geolocation Service Interface & Implementations
// ============================================================================

/**
 * @brief Pluggable interface for resolving geographic coordinates.
 * Allows independent implementations (IP-based, GPS, system API, manual config).
 */
class IGeoLocationService {
public:
    virtual ~IGeoLocationService() = default;
    virtual bool ResolveLocation(double& outLat, double& outLon, std::string& outLocationName) = 0;
};

/**
 * @brief Geolocation provider via IP lookup REST API.
 */
class IpApiGeoLocationService : public IGeoLocationService {
public:
    explicit IpApiGeoLocationService(const std::string& endpoint = "http://ip-api.com/json/?fields=lat,lon,city,status");
    bool ResolveLocation(double& outLat, double& outLon, std::string& outLocationName) override;

private:
    std::string m_endpoint;
};

/**
 * @brief Fallback / static geolocation provider configured by user settings.
 */
class ManualGeoLocationService : public IGeoLocationService {
public:
    ManualGeoLocationService(double lat, double lon, const std::string& name);
    bool ResolveLocation(double& outLat, double& outLon, std::string& outLocationName) override;

    void SetCoordinates(double lat, double lon, const std::string& name);

private:
    double m_lat;
    double m_lon;
    std::string m_name;
};

// ============================================================================
// 2. Weather Service Interface & Implementations
// ============================================================================

/**
 * @brief Pluggable interface for querying weather conditions.
 * Allows independent providers (Open-Meteo, MET Norway, custom sensor socket, etc.).
 */
class IWeatherService {
public:
    virtual ~IWeatherService() = default;
    virtual bool QueryWeather(double lat, double lon, WeatherDataSnapshot& outData) = 0;
};

/**
 * @brief Standard implementation querying the Open-Meteo REST API.
 */
class OpenMeteoWeatherService : public IWeatherService {
public:
    OpenMeteoWeatherService() = default;
    bool QueryWeather(double lat, double lon, WeatherDataSnapshot& outData) override;

private:
    std::string FetchHttp(const std::string& url);
};

// ============================================================================
// 3. Central WeatherCore Coordinator
// ============================================================================

struct WeatherCoreConfig {
    int queryIntervalMinutes{ 15 };
    std::string cacheFilePath{ "~/.config/far2l/weather_cache.json" };
    TimeOfDay timeOfDayOverride{ TOD_AUTO };
};

/**
 * @brief Utility class coordinating geolocation, weather queries, caching, and timer.
 * Thread-safe, non-blocking, zero GUI dependencies.
 */
class WeatherCore {
public:
    using UpdateCallback = std::function<void(const WeatherDataSnapshot&)>;

    WeatherCore(
        std::shared_ptr<IGeoLocationService> geoService,
        std::shared_ptr<IWeatherService> weatherService,
        const WeatherCoreConfig& config = WeatherCoreConfig{}
    );
    ~WeatherCore();

    void Initialize();
    void Shutdown();

    // Query triggers & data access
    void QueryWeatherAsync();
    WeatherDataSnapshot GetData() const;
    TimeOfDay GetCurrentTimeOfDay() const;

    // Pluggable service replacement at runtime
    void SetGeoLocationService(std::shared_ptr<IGeoLocationService> geoService);
    void SetWeatherService(std::shared_ptr<IWeatherService> weatherService);

    // Offline mode & disk caching
    void SetOfflineMode(bool offline);
    bool IsOfflineMode() const;
    bool SaveCacheToDisk(const std::string& customPath = "") const;
    bool LoadCacheFromDisk(const std::string& customPath = "");

    void RegisterUpdateCallback(UpdateCallback cb);

    static std::string ConditionToString(WeatherCondition cond);
    static WeatherCondition CodeToCondition(int wmoCode);

private:
    void WorkerLoop();

    std::shared_ptr<IGeoLocationService> m_geoService;
    std::shared_ptr<IWeatherService> m_weatherService;
    WeatherCoreConfig m_config;

    mutable std::mutex m_mutex;
    WeatherDataSnapshot m_currentData;

    std::atomic<bool> m_isOffline{ false };
    std::atomic<bool> m_isRunning{ false };
    std::atomic<bool> m_queryInProgress{ false };

    std::thread m_workerThread;
    std::vector<UpdateCallback> m_callbacks;
};
