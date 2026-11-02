#pragma once

#include <string>
#include <stdint.h>

#include "formats.hpp"

/**
 * @brief Intelligent path middle-truncation preserving head (root) and tail (filename).
 * @param fullPath Path string to truncate.
 * @param maxLen Maximum character width allowed.
 */
std::wstring TruncatePathMiddle(const std::wstring& fullPath, size_t maxLen);


std::wstring BuildPercentString(double v, size_t maxLen);
std::wstring BuildSizeString(int64_t v, size_t maxLen);
std::wstring BuildRealString(double v, size_t maxLen);

std::wstring BuildDateTime(int64_t timestamp, bool shortForm = true);
