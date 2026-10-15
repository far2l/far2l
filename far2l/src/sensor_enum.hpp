#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace NetstatFar2l {

// Type of hardware sensor
enum class SensorType {
    ALL,
    TEMPERATURE, // °C
    FAN,         // RPM
    VOLTAGE,     // V
    POWER,       // W
    CURRENT      // A
};

// Hardware category
enum class SensorCategory {
    CPU,
    GPU,
    MOTHERBOARD,
    STORAGE,
    CHASSIS,
    OTHER
};

// Single hardware sensor measurement
struct SensorEntry {
    std::string chip_name;    // e.g. "coretemp", "k10temp", "amdgpu", "nouveau", "nct6775", "nvme"
    std::string sensor_name;  // e.g. "Package id 0", "Core 0", "GPU Edge", "CPU Fan", "Vcore"
    SensorType type = SensorType::TEMPERATURE;
    std::string type_str;     // "TEMP", "FAN", "VOLT", "POWER"
    SensorCategory category = SensorCategory::OTHER;
    std::string category_str; // "CPU", "GPU", "MB", "DRIVE", "OTHER"

    double current_value = 0.0; // Current reading (e.g. 48.5 for °C, 1450.0 for RPM, 1.21 for V)
    std::string unit;           // "°C", "RPM", "V", "W"
    
    std::optional<double> min_value;  // Minimum threshold
    std::optional<double> max_value;  // High/Warning threshold
    std::optional<double> crit_value; // Critical threshold

    std::string status = "OK";        // "OK", "HIGH", "CRIT", "LOW"
    std::string sysfs_path;           // Path in /sys/class/hwmon/...

    // Convenient formatting helpers
    std::string GetFormattedValue() const;   // e.g. "48.5 °C", "1450 RPM"
    std::string GetFormattedLimits() const;  // e.g. "crit: 100.0°C" or "min: 600 RPM"
    std::string GetStatusDisplay() const;    // e.g. "[OK]", "[HIGH]", "[CRIT]"
};

// Column width specifications for far2l VMenu sensor dialog
struct Far2lSensorColumnSizes {
    int cat_width    = 5;   // "CPU  ", "GPU  ", "MB   "
    int chip_width   = 11;  // "coretemp   ", "amdgpu     "
    int sensor_width = 18;  // "Package id 0      ", "CPU Fan           "
    int value_width  = 14;  // "48.5 °C     ", "1450 RPM    "
    int status_width = 6;   // "OK    ", "HIGH  ", "CRIT  "
    int limits_width = 16;  // "crit: 100.0°C   "

    // Compact format (fits strictly within standard 80-column terminal dialog with frame):
    // 4 + 1 + 10 + 1 + 17 + 1 + 11 + 1 + 6 + 1 + 14 = 67 chars + borders = 78 chars!
    static Far2lSensorColumnSizes Compact() {
        return Far2lSensorColumnSizes{4, 10, 17, 11, 6, 14};
    }

    static Far2lSensorColumnSizes Standard() {
        return Far2lSensorColumnSizes{5, 11, 18, 14, 6, 16};
    }

    static Far2lSensorColumnSizes Wide() {
        return Far2lSensorColumnSizes{6, 14, 24, 14, 8, 20};
    }
};

// Core enumeration API
// Zero external dependencies. Reads /sys/class/hwmon and /sys/class/thermal on Linux,
// SMC / sysctl on macOS, and sysctl on BSD.
std::vector<SensorEntry> EnumerateSensors(SensorType filter = SensorType::ALL, 
                                        bool include_synthetic_if_empty = true);

// Format helpers tailored for far2l VMenu items
std::string FormatVMenuSensorHeader(const Far2lSensorColumnSizes& sizes = Far2lSensorColumnSizes::Standard(), 
                                   bool use_box_chars = true);
std::string FormatVMenuSensorSeparator(const Far2lSensorColumnSizes& sizes = Far2lSensorColumnSizes::Standard(), 
                                      bool use_box_chars = true);
std::string FormatVMenuSensorRow(const SensorEntry& entry, 
                                const Far2lSensorColumnSizes& sizes = Far2lSensorColumnSizes::Standard(), 
                                bool use_box_chars = true);

// Direct printf table output to stdout
void PrintSensors(const std::vector<SensorEntry>& entries, 
                  const Far2lSensorColumnSizes& sizes = Far2lSensorColumnSizes::Standard(), 
                  bool use_box_chars = true);

const char* SensorTypeToString(SensorType type);
const char* SensorCategoryToString(SensorCategory cat);

std::vector<std::wstring> FormatSensors(const std::vector<SensorEntry>& entries, 
                      const Far2lSensorColumnSizes& sizes = Far2lSensorColumnSizes::Standard(),
                      bool use_box_chars = true);

} // namespace NetstatFar2l
