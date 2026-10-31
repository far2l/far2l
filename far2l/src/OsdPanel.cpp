#include "headers.hpp"

#include "OsdPanel.hpp"
#include "interf.hpp"
#include "farcolors.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

#include <iostream>
#include <string>

OsdPanel::OsdPanel(int x1, int y1, int maxWidth, int maxHeight)
    : ScreenObject(),
      m_maxWidth(maxWidth),
      m_maxHeight(maxHeight) 
{
	X1 = x1;
	X2 = X1 + maxWidth;
	Y1 = y1;
	Y2 = y1 + maxHeight;

	m_defaultColor  = FarColorToReal(COL_PANELTEXT);
}

void OsdPanel::SetOrientation(Orientation orientation) {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_orientation = orientation;
    }
    TriggerRedrawIfEnabled();
}

OsdPanel::Orientation OsdPanel::GetOrientation() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_orientation;
}

void OsdPanel::SetMaxSize(int maxWidth, int maxHeight) {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_maxWidth = std::max(20, maxWidth);
        m_maxHeight = std::max(3, maxHeight);
        X2 = X1 + m_maxWidth;
        Y2 = Y1 + m_maxHeight;
    }
    TriggerRedrawIfEnabled();
}

int OsdPanel::GetMaxWidth() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_maxWidth;
}

int OsdPanel::GetMaxHeight() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_maxHeight;
}

void OsdPanel::SetAutoRedraw(bool enable) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_autoRedraw = enable;
}

bool OsdPanel::GetAutoRedraw() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_autoRedraw;
}

void OsdPanel::SetDefaultColors(uint64_t color) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_defaultColor = color;
}

void OsdPanel::TriggerRedrawIfEnabled() {
    if (m_autoRedraw && IsVisible())      Redraw();
}

// --- General Line API ---

int OsdPanel::AddLine(const std::wstring& text, uint64_t color) {
    int idx = 0;
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        RowItem item;
        item.type = RowType::PlainText;
        item.rawText = text;
        item.color = (color != 0) ? color : m_defaultColor;
        m_rows.push_back(std::move(item));
        idx = static_cast<int>(m_rows.size()) - 1;
        m_rows[idx].visible = true;

        fprintf(stderr, "\tOSD[%d] = `%ls`\n", idx, m_rows[idx].rawText.c_str());
    }
    TriggerRedrawIfEnabled();
    return idx;
}

void OsdPanel::UpdateLine(int rowNumber, const std::wstring& text) {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
            return;
        }
        m_rows[rowNumber].rawText = text;
        m_rows[rowNumber].type = RowType::PlainText;
    }
    TriggerRedrawIfEnabled();
}

bool OsdPanel::HasActiveLines() {
	bool hasSome = false;
    for(size_t i = 0; i < m_rows.size(); ++i) {
    	if (m_rows[i].visible){ 
        	hasSome = true;
    		break;
        }
    }
    return hasSome;
}


void OsdPanel::CompleteLine(int rowNumber) {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (rowNumber >= 0 && rowNumber < static_cast<int>(m_rows.size())) {
	        m_rows[rowNumber].visible = false;
		}
        if (!HasActiveLines()) { 
        	m_rows.clear();
        	if (IsVisible()) Hide();
        }
    }
    TriggerRedrawIfEnabled();
}

std::wstring OsdPanel::GetLine(int rowNumber) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
        return L"";
    }
    return m_rows[rowNumber].rawText;
}

void OsdPanel::UpdateField(int rowNumber, int colOffset, int width, const std::wstring& fieldText) {
    if (width <= 0 || colOffset < 0) return;
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
            return;
        }
        auto& row = m_rows[rowNumber];
        row.type = RowType::SubFieldText;

        // Ensure rawText is large enough
        if (static_cast<int>(row.rawText.length()) < colOffset + width) {
            row.rawText.resize(colOffset + width, L' ');
        }

        // Format fieldText to exact width
        std::wstring formatted = fieldText;
        if (static_cast<int>(formatted.length()) < width) {
            formatted.append(width - formatted.length(), L' ');
        } else if (static_cast<int>(formatted.length()) > width) {
            formatted = formatted.substr(0, width);
        }

        // Overlay onto rawText directly for fast lookup
        row.rawText.replace(colOffset, width, formatted);

        // Keep subfield record
        bool found = false;
        for (auto& sf : row.subFields) {
            if (sf.colOffset == colOffset && sf.width == width) {
                sf.value = formatted;
                found = true;
                break;
            }
        }
        if (!found) {
            row.subFields.push_back({colOffset, width, formatted});
        }
    }
    TriggerRedrawIfEnabled();
}

// --- Path API ---

int OsdPanel::AddPath(const std::wstring& prefix, const std::wstring& suffix, 
                      const std::wstring& pathToFile,
                      uint64_t color) {
    int idx = 0;
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        RowItem item;
        item.type = RowType::Path;
        item.prefix = prefix;
        item.suffix = suffix;
        item.pathToFile = pathToFile;
        item.color = (color != 0) ? color : m_defaultColor;
        m_rows.push_back(std::move(item));
        idx = static_cast<int>(m_rows.size()) - 1;
        m_rows[idx].visible = true;
    }
    TriggerRedrawIfEnabled();
    return idx;
}

void OsdPanel::UpdatePath(int rowNumber, const std::wstring& pathToFile) {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
            return;
        }
        m_rows[rowNumber].pathToFile = pathToFile;
        m_rows[rowNumber].type = RowType::Path;
    }
    TriggerRedrawIfEnabled();
}

std::wstring OsdPanel::GetPath(int rowNumber) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
        return L"";
    }
    return m_rows[rowNumber].pathToFile;
}

// --- Numeric API ---

int OsdPanel::AddNumeric(const std::wstring& prefix, const std::wstring& suffix, 
                         double value, int precision,
                         uint64_t color) {
    int idx = 0;
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        RowItem item;
        item.type = RowType::Numeric;
        item.prefix = prefix;
        item.suffix = suffix;
        item.numericValue = value;
        item.precision = precision;
        item.color = (color != 0) ? color : m_defaultColor;
        m_rows.push_back(std::move(item));
        idx = static_cast<int>(m_rows.size()) - 1;
        m_rows[idx].visible = true;
    }
    TriggerRedrawIfEnabled();
    return idx;
}

void OsdPanel::UpdateNumeric(int rowNumber, double value) {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
            return;
        }
        m_rows[rowNumber].numericValue = value;
        m_rows[rowNumber].type = RowType::Numeric;
    }
    TriggerRedrawIfEnabled();
}

double OsdPanel::GetNumeric(int rowNumber) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
        return 0.0;
    }
    return m_rows[rowNumber].numericValue;
}

// --- Progress Bar API ---

int OsdPanel::AddProgressBar(const std::wstring& prefix, const std::wstring& suffix,
                             int64_t current, int64_t total, int64_t minVal,
                             bool showPercentage,
                             uint64_t color) {
    int idx = 0;
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        RowItem item;
        item.type = RowType::ProgressBar;
        item.prefix = prefix;
        item.suffix = suffix;
        item.pbCurrent = current;
        item.pbTotal = (total > minVal) ? total : (minVal + 1);
        item.pbMin = minVal;
        item.showPercentage = showPercentage;
        item.color = (color != 0) ? color : m_defaultColor;
        m_rows.push_back(std::move(item));
        idx = static_cast<int>(m_rows.size()) - 1;
        m_rows[idx].visible = true;
    }
    TriggerRedrawIfEnabled();
    return idx;
}

void OsdPanel::UpdateProgressBar(int rowNumber, int64_t current, int64_t total) {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
            return;
        }
        m_rows[rowNumber].pbCurrent = current;
        if (total > m_rows[rowNumber].pbMin) {
            m_rows[rowNumber].pbTotal = total;
        }
        m_rows[rowNumber].type = RowType::ProgressBar;
    }
    TriggerRedrawIfEnabled();
}

int64_t OsdPanel::GetProgressBarCurrent(int rowNumber) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (rowNumber < 0 || rowNumber >= static_cast<int>(m_rows.size())) {
        return 0;
    }
    return m_rows[rowNumber].pbCurrent;
}

// --- Sizing & Utilities ---

std::pair<int, int> OsdPanel::GetPreferredSize() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_rows.empty()) {
        return {0, 0};
    }

    if (m_orientation == Orientation::Vertical) {
        // In vertical orientation: height is max row length, width is count of rows
        int maxHeight = 0;
        for (const auto& row : m_rows) {
        	if (!row.visible) continue;
            std::wstring formatted = FormatRowString(row, 120);
            maxHeight = std::max(maxHeight, static_cast<int>(formatted.length()));
        }
        return {static_cast<int>(m_rows.size()), maxHeight};
    }

    // Horizontal orientation: height is row count, width is maximum item length
    int maxWidth = 0;
    for (const auto& row : m_rows) {
    	if (!row.visible) continue;
        std::wstring formatted = FormatRowString(row, 120);
        maxWidth = std::max(maxWidth, static_cast<int>(formatted.length()));
    }
    return {maxWidth, static_cast<int>(m_rows.size())};
}

void OsdPanel::Clear() {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_rows.clear();
    }
    TriggerRedrawIfEnabled();
}

size_t OsdPanel::GetRowCount() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_rows.size();
}

void OsdPanel::RemoveRow(int rowNumber) {
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (rowNumber >= 0 && rowNumber < static_cast<int>(m_rows.size())) {
            m_rows.erase(m_rows.begin() + rowNumber);
        }

        if (!HasActiveLines()) { 
        	m_rows.clear();
        	if (IsVisible()) Hide();
        }
    }
    TriggerRedrawIfEnabled();
}

// --- Progress Bar String Builder ---

std::wstring OsdPanel::BuildProgressBarString(int64_t current, int64_t minVal, int64_t total,
                                             size_t barWidth, bool showPercentage,
                                             const std::wstring& fill,
                                             const std::wstring& empty) {
    if (barWidth == 0) return L"";

    int64_t range = (total > minVal) ? (total - minVal) : 1;
    double ratio = static_cast<double>(current - minVal) / static_cast<double>(range);
    ratio = std::clamp(ratio, 0.0, 1.0);
    int percent = static_cast<int>(std::round(ratio * 100.0));

    std::wstring percentStr = L"";
    if (showPercentage) {
        std::wostringstream oss;
        oss << std::setw(3) << percent << L"%";
        percentStr = oss.str();
    }

    if (barWidth <= percentStr.length() + 2) {
        // Tight space: render compact percentage
        if (barWidth >= 5) {
            std::wostringstream oss;
            oss << percent << L"%";
            return oss.str().substr(0, barWidth);
        }
        return fill;
    }

    size_t actualBarSlots = barWidth - percentStr.length();
    size_t filledSlots = static_cast<size_t>(std::round(ratio * actualBarSlots));
    filledSlots = std::min(filledSlots, actualBarSlots);

    std::wstring bar;
    bar.reserve(barWidth);
    for (size_t i = 0; i < filledSlots; ++i) {
        bar += fill;
    }
    for (size_t i = filledSlots; i < actualBarSlots; ++i) {
        bar += empty;
    }
    bar += percentStr;

    return bar;
}

// --- Format Helper ---

std::wstring OsdPanel::FormatRowString(const RowItem& item, size_t availableWidth) const {
    switch (item.type) {
        case RowType::PlainText:
        case RowType::SubFieldText:
            return item.rawText;

        case RowType::Path: {
            size_t fixLen = item.prefix.length() + item.suffix.length();
            if (availableWidth <= fixLen + 4) {
                // Extremely constrained
                return item.prefix + L"..." + item.suffix;
            }
            size_t pathAvail = availableWidth - fixLen;
            std::wstring truncated = TruncatePathMiddle(item.pathToFile, pathAvail);
            return item.prefix + truncated + item.suffix;
        }

        case RowType::Numeric: {
            std::wostringstream oss;
            oss << item.prefix;
            oss << std::fixed << std::setprecision(item.precision) << item.numericValue;
            oss << item.suffix;
            return oss.str();
        }

        case RowType::ProgressBar: {
            size_t fixLen = item.prefix.length() + item.suffix.length();
            size_t barSpace = (availableWidth > fixLen) ? (availableWidth - fixLen) : 10;
            std::wstring bar = BuildProgressBarString(item.pbCurrent, item.pbMin, item.pbTotal,
                                                    barSpace, item.showPercentage,
                                                    item.pbFilledChar, item.pbEmptyChar);
            return item.prefix + bar + item.suffix;
        }
    }
    return L"";
}

// --- Far2l ScreenObject Rendering Hook ---

void OsdPanel::DisplayObject() {
    if (!IsVisible()) return;

    // Snapshot state under shared lock to minimize critical section
    std::vector<RowItem> rowsSnapshot;
    Orientation orientation;
    int maxWidth = 0;
    int maxHeight = 0;


	SetScreen(X1, Y1, X2, Y2, L' ', FarColorToReal(COL_PANELTEXT));
	Hint(X1, Y1, X2, Y2, HintOSD, HintObjectNone);
    Box(X1, Y1, X2, Y2, FarColorToReal(COL_PANELBOX), SINGLE_BOX);

    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        for(size_t i = 0; i < m_rows.size(); ++i)
        	if (m_rows[i].visible) rowsSnapshot.push_back(m_rows[i]);
        orientation = m_orientation;
        maxWidth = m_maxWidth;
        maxHeight = m_maxHeight;
    }

    if (rowsSnapshot.empty()) {
        return;
    }

    // Determine target width for dynamic formatting
    size_t targetWidth = (maxWidth > 0) ? static_cast<size_t>(maxWidth) - 2 : 80;

    // Vertical line clipping: "we need to see the bottom of the lines"
    size_t startRowIdx = 0;
    //size_t visibleRowCount = rowsSnapshot.size();
    if (maxHeight > 0 && rowsSnapshot.size() > static_cast<size_t>(maxHeight) - 2) {
        startRowIdx = rowsSnapshot.size() - maxHeight - 2;
        //visibleRowCount = maxHeight;
    }

    if (orientation == Orientation::Horizontal) {
        // Standard horizontal line rendering
        int y = Y1 + 1;
        for (size_t i = startRowIdx; i < rowsSnapshot.size(); ++i, ++y) {
            const auto& row = rowsSnapshot[i];
            std::wstring formatted = FormatRowString(row, targetWidth);

            // Horizontal clipping: "and the beginning of the strings (e.g. clipping)"
            if (maxWidth > 0 && static_cast<int>(formatted.length()) > maxWidth - 2) {
                formatted = formatted.substr(0, maxWidth - 2);
            }

            GotoXY(X1 + 1, y);
            SetColor(row.color);
            Text(formatted.c_str());
        }
    } else {
        // Vertical column orientation
        // Each row rendered vertically as a column, or characters arranged downwards
        int x = X1 + 1;
        for (size_t i = startRowIdx; i < rowsSnapshot.size(); ++i, ++x) {
            const auto& row = rowsSnapshot[i];
            std::wstring formatted = FormatRowString(row, targetWidth);

            size_t maxChars = (maxHeight > 0) ? std::min(formatted.length(), static_cast<size_t>(maxHeight) - 2) : formatted.length();
            for (size_t charIdx = 0; charIdx < maxChars; ++charIdx) {
                GotoXY(x, Y1 + static_cast<int>(charIdx) + 1);
                std::wstring singleChar(1, formatted[charIdx]);
	            SetColor(row.color);
                Text(singleChar.c_str());
            }
        }
    }
}

