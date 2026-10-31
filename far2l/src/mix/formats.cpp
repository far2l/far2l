#include "headers.hpp"

#include "formats.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

#include <iostream>
#include <stdint.h>

// --- Middle-Truncation Algorithm ---

std::wstring TruncatePathMiddle(const std::wstring& fullPath, size_t maxLen) {
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

/* text formats engine */

static std::wstring Fit(const std::vector<std::wstring>& candidates, std::size_t maxWidth) {
    auto it = std::find_if(candidates.begin(), candidates.end(),
        [maxWidth](const auto s) {
            return s.size() <= maxWidth;
        });

    if (it != candidates.end()) return *it;
    return candidates.empty() ? std::wstring {} : candidates.back();
}

static std::vector<std::wstring> GenerateFloatVariants(double value,  std::wstring suffix = {}) {
    std::vector<std::wstring> result;
    for (int precision = 3; precision >= 0; --precision) {
        std::wostringstream ss;
        ss << std::fixed << std::setprecision(precision) << value << suffix;
        result.push_back(ss.str());
    }
    return result;
}

std::wstring BuildRealString(double v, size_t maxWidth) {
	return Fit(GenerateFloatVariants(v, L""), maxWidth);
}

std::wstring BuildPercentString(double value, size_t maxWidth) {
	return Fit(GenerateFloatVariants(value, L"%"), maxWidth);
}

std::wstring BuildSizeString(int64_t bytes, size_t maxWidth) {
    static constexpr const wchar_t* units[] = {L" ", L"K", L"M", L"G", L"T", L"P", L"E"}; // small numbers need no modifier suffix
    std::vector<std::wstring> candidates;

    double value = static_cast<double>(bytes);
    std::size_t unit = 0;

    while (value >= 1024.0 && unit + 1 < std::size(units)) {
        value /= 1024.0;
        ++unit;
    }
    for (int precision = 3; precision >= 0; --precision) {
        std::wostringstream ss;
        ss << std::fixed << std::setprecision(precision) << value << units[unit];
        candidates.push_back(ss.str());
    }

    if (unit > 0) {
        std::wostringstream ss;
        ss << static_cast<std::uint64_t>(std::round(value)) << units[unit];
        candidates.push_back(ss.str());
    }
    return Fit(candidates, maxWidth);
}

/*
std::wstring OsdPanel::BuildSizeString(int64_t bytes, size_t maxWidth) {
    static constexpr const char* units[] = { "B", "K", "M", "G", "T", "P", "E" };

    double value = static_cast<double>(bytes);
    size_t unit = 0;

    while (value >= 1024.0 && unit + 1 < std::size(units)) {
        value /= 1024.0;
        ++unit;
    }

    for (;;) {
        for (int decimals = 2; decimals >= 0; --decimals) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(decimals) << value << units[unit];
            if (ss.str().size() <= maxWidth) return ss.str();
        }

        if (value < 1024.0 || unit + 1 >= std::size(units)) {
            std::ostringstream ss;
            ss << static_cast<uint64_t>(std::round(value)) << units[unit];
            return ss.str();
        }

        value /= 1024.0;
        ++unit;
    }
} */

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::wstring BuildDateTime(int64_t timestamp, bool shortForm)
{
    std::time_t t = static_cast<std::time_t>(timestamp);

    std::tm tm {};
    localtime_r(&t, &tm);

    std::wostringstream ss;
    if (shortForm) 
		ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");
	else
		ss << std::put_time(&tm, L"%c");  // locale-dependent date/time
    return ss.str();
}
