#pragma once

#include <string>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include <optional>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "scrobj.hpp"

/**
 * @brief Dynamic OSD (On-Screen Display) text UI widget for Far2l.
 * 
 * Supports dynamic horizontal and vertical rendering, multi-threaded row/field updates,
 * intelligent path middle-truncation, automatic progress bars, formatted numerics,
 * and dynamic max-dimension clipping with preferred size calculations.
 */
class OsdPanel : public ScreenObject {
public:
    enum class Orientation {
        Horizontal, // Standard row-by-row rendering
        Vertical    // Vertical column / stacked character rendering
    };

    enum class RowType {
        PlainText,      // Plain string
        SubFieldText,   // Base string with dynamic character field overlays
        Path,           // Dynamic path with middle-truncation
        Numeric,        // Dynamic floating-point or integer value
        ProgressBar     // Dynamic progress bar with min, total, and current
    };

    struct SubField {
        int colOffset{0};
        int width{0};
        std::wstring value;
    };

    struct RowItem {
        RowType type{RowType::PlainText};
        std::wstring prefix;
        std::wstring suffix;
        std::wstring rawText;
        std::vector<SubField> subFields;

        // Path specific
        std::wstring pathToFile;

        // Numeric specific
        double numericValue{0.0};
        int precision{1};

        // Progress bar specific
        int64_t pbCurrent{0};
        int64_t pbTotal{100};
        int64_t pbMin{0};
        std::wstring pbFilledChar{L"█"};
        std::wstring pbEmptyChar{L"░"};
        bool showPercentage{true};
        bool visible {true};

        // Styling
        uint64_t color{0x00FFFFFF00000080}; // Far2l Navy Blue
    };

private:
    // Concurrency guard: mutable so const query methods can acquire read locks
    mutable std::shared_mutex m_mutex;

    std::vector<RowItem> m_rows;

    Orientation m_orientation{Orientation::Horizontal};

    // Dynamic constraints (0 = unlimited / dynamic)
    int m_maxWidth{0};
    int m_maxHeight{0};

    // Default palette
    uint64_t m_defaultColor{0x00E0E0E000000080};
    uint64_t m_accentColor{0x0000FFFF000000080}; // Cyan

    // Whether changes should auto-trigger Redraw()
    bool m_autoRedraw{true};

public:
    OsdPanel(int x1 = 2, int y1 = 2, int maxWidth = 60, int maxHeight = 15);
    virtual ~OsdPanel() override = default;

    // --- Configuration API ---

    void SetOrientation(Orientation orientation);
    Orientation GetOrientation() const;

    void SetMaxSize(int maxWidth, int maxHeight);
    int GetMaxWidth() const;
    int GetMaxHeight() const;

    void SetAutoRedraw(bool enable);
    bool GetAutoRedraw() const;

    void SetDefaultColors(uint64_t _color);

    bool HasActiveLines();

    // --- General Line API (Thread-Safe) ---

    int AddLine(const std::wstring& text, uint64_t color = 0);
    void UpdateLine(int rowNumber, const std::wstring& text);
    void CompleteLine(int rowNumber);
    std::wstring GetLine(int rowNumber) const;

    /**
     * @brief Update a sub-slice of an existing line at a fixed character position and size.
     * E.g. to update only the fan speed at columns 12..17 without recreating the entire line.
     * 
     * @param rowNumber Zero-based index of row.
     * @param colOffset Zero-based starting character column.
     * @param width Fixed width of the field in characters.
     * @param fieldText Text to insert (will be padded or truncated to match width).
     */
    void UpdateField(int rowNumber, int colOffset, int width, const std::wstring& fieldText);

    // --- Path API (Intelligent Middle-Truncation) ---

    int AddPath(const std::wstring& prefix, const std::wstring& suffix, 
                const std::wstring& pathToFile,
                uint64_t color = 0);

    void UpdatePath(int rowNumber, const std::wstring& pathToFile);

    std::wstring GetPath(int rowNumber) const;

    // --- Numeric API ---

    int AddNumeric(const std::wstring& prefix, const std::wstring& suffix, 
                   double value, int precision = 1,
                   uint64_t color = 0);

    void UpdateNumeric(int rowNumber, double value);

    double GetNumeric(int rowNumber) const;

    // --- Progress Bar API ---

    int AddProgressBar(const std::wstring& prefix, const std::wstring& suffix,
                       int64_t current, int64_t total, int64_t minVal = 0,
                       bool showPercentage = true,
                       uint64_t color = 0);

    void UpdateProgressBar(int rowNumber, int64_t current, int64_t total = -1);

    int64_t GetProgressBarCurrent(int rowNumber) const;

    // --- Sizing & Utilities ---

    /**
     * @brief Computes preferred dimensions that the panel would like to occupy.
     * @return std::pair<int, int> { preferredWidth, preferredHeight }
     */
    std::pair<int, int> GetPreferredSize() const;

    void Clear();
    size_t GetRowCount() const;
    void RemoveRow(int rowNumber);

    // --- Far2l ScreenObject Overrides ---

    /**
     * @brief Main rendering method invoked during repaint cycles.
     * Takes a fast snapshot under a shared read-lock to eliminate UI tearing,
     * arranges templated items, applies middle path truncation, and renders via GotoXY / Text.
     */
    virtual void DisplayObject() override;

    // --- String & Path Algorithm Helpers (Static & Pure) ---

    /**
     * @brief Intelligent path middle-truncation preserving head (root) and tail (filename).
     * @param fullPath Path string to truncate.
     * @param maxLen Maximum character width allowed.
     */
    static std::wstring TruncatePathMiddle(const std::wstring& fullPath, size_t maxLen);

    /**
     * @brief Formats a progress bar string using block characters.
     */
    static std::wstring BuildProgressBarString(int64_t current, int64_t minVal, int64_t total,
                                              size_t barWidth, bool showPercentage,
                                              const std::wstring& fill = L"█",
                                              const std::wstring& empty = L"░");

private:
    std::wstring FormatRowString(const RowItem& item, size_t availableWidth) const;
    void TriggerRedrawIfEnabled();
};
