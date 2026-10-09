#include "headers.hpp"

#include "OsdPanel.hpp"
#include "interf.hpp"
#include "farcolors.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

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
    if (m_autoRedraw && IsVisible()) {
        Redraw();
    }
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
            std::wstring formatted = FormatRowString(row, 120);
            maxHeight = std::max(maxHeight, static_cast<int>(formatted.length()));
        }
        return {static_cast<int>(m_rows.size()), maxHeight};
    }

    // Horizontal orientation: height is row count, width is maximum item length
    int maxWidth = 0;
    for (const auto& row : m_rows) {
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

// --- Middle-Truncation Algorithm ---

std::wstring OsdPanel::TruncatePathMiddle(const std::wstring& fullPath, size_t maxLen) {
    if (fullPath.length() <= maxLen) {
        return fullPath;
    }
    if (maxLen <= 3) {
        return fullPath.substr(0, maxLen);
    }
    if (maxLen <= 6) {
        return L"..." + fullPath.substr(fullPath.length() - (maxLen - 3));
    }

    // Determine path separator
    wchar_t sep = L'/';
    if (fullPath.find(L'\\') != std::wstring::npos && fullPath.find(L'/') == std::wstring::npos) {
        sep = L'\\';
    }

    // Split path into segments
    std::vector<std::wstring> segments;
    std::wstring root;
    size_t startPos = 0;

    // Detect root: Windows "C:\" or POSIX "/" or network "\\share\"
    if (fullPath.length() >= 2 && fullPath[1] == L':') {
        if (fullPath.length() >= 3 && (fullPath[2] == L'\\' || fullPath[2] == L'/')) {
            root = fullPath.substr(0, 3);
            startPos = 3;
        } else {
            root = fullPath.substr(0, 2);
            startPos = 2;
        }
    } else if (!fullPath.empty() && (fullPath[0] == L'/' || fullPath[0] == L'\\')) {
        root = fullPath.substr(0, 1);
        startPos = 1;
    }

    size_t current = startPos;
    while (current < fullPath.length()) {
        size_t nextSep = fullPath.find_first_of(L"/\\", current);
        if (nextSep == std::wstring::npos) {
            segments.push_back(fullPath.substr(current));
            break;
        }
        segments.push_back(fullPath.substr(current, nextSep - current));
        current = nextSep + 1;
    }

    if (segments.empty()) {
        return fullPath.substr(0, maxLen);
    }

    std::wstring tailFilename = segments.back();
    segments.pop_back(); // remaining are intermediate directories

    const std::wstring ellipsis = L"...";

    // If filename alone + root + ellipsis exceeds maxLen, truncate inside filename
    size_t minBaseLen = root.length() + ellipsis.length() + 1 + tailFilename.length();
    if (minBaseLen > maxLen) {
        // Need to truncate tailFilename itself
        size_t availForTail = (maxLen > root.length() + ellipsis.length() + 1)
                                  ? (maxLen - root.length() - ellipsis.length() - 1)
                                  : 3;
        if (tailFilename.length() > availForTail) {
            // Keep extension if possible
            size_t dotPos = tailFilename.find_last_of(L'.');
            if (dotPos != std::wstring::npos && dotPos > 0 && (tailFilename.length() - dotPos) < availForTail) {
                size_t extLen = tailFilename.length() - dotPos;
                size_t nameKeep = availForTail > (extLen + 1) ? (availForTail - extLen - 1) : 1;
                tailFilename = tailFilename.substr(0, nameKeep) + L"~" + tailFilename.substr(dotPos);
            } else {
                tailFilename = tailFilename.substr(tailFilename.length() - availForTail);
            }
        }
        return root + ellipsis + sep + tailFilename;
    }

    // Try keeping as many head segments and tail segments as possible
    size_t leftIdx = 0;
    int rightIdx = static_cast<int>(segments.size()) - 1;

    std::vector<std::wstring> leftSegs;
    std::vector<std::wstring> rightSegs;

    while (leftIdx <= static_cast<size_t>(rightIdx)) {
        // Alternate trying to add left segment (more context from top) or right segment (immediate parent folder)
        // Check if adding left segment fits
        size_t candidateLen = root.length() + tailFilename.length() + ellipsis.length() + 2; // separators
        for (const auto& s : leftSegs) candidateLen += s.length() + 1;
        for (const auto& s : rightSegs) candidateLen += s.length() + 1;

        bool added = false;
        // Prioritize parent folder (right side)
        if (rightIdx >= static_cast<int>(leftIdx)) {
            if (candidateLen + segments[rightIdx].length() + 1 <= maxLen) {
                rightSegs.insert(rightSegs.begin(), segments[rightIdx]);
                candidateLen += segments[rightIdx].length() + 1;
                rightIdx--;
                added = true;
            }
        }
        // Then try left segment
        if (leftIdx <= static_cast<size_t>(rightIdx)) {
            if (candidateLen + segments[leftIdx].length() + 1 <= maxLen) {
                leftSegs.push_back(segments[leftIdx]);
                leftIdx++;
                added = true;
            }
        }

        if (!added) break;
    }

    // Assemble final path: root + leftSegs + /.../ + rightSegs + /tail
    std::wstring result = root;
    for (size_t i = 0; i < leftSegs.size(); ++i) {
        result += leftSegs[i] + sep;
    }
    result += ellipsis + sep;
    for (size_t i = 0; i < rightSegs.size(); ++i) {
        result += rightSegs[i] + sep;
    }
    result += tailFilename;

    if (result.length() > maxLen) {
        // Fallback safety
        return result.substr(0, maxLen);
    }
    return result;
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
        oss << L" [" << std::setw(3) << percent << L"%]";
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
