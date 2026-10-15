#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

/**
 * @brief Fast, thread-safe, locale-independent UTF-8 <-> std::wstring (wchar_t) conversion.
 * Conforms to far2l internal wide-character string handling convention.
 */
inline std::string WideToNarrow(const std::wstring& wstr) {
    std::string out;
    out.reserve(wstr.size() * 3);
    for (wchar_t wc : wstr) {
        uint32_t cp = static_cast<uint32_t>(wc);
        if (cp <= 0x7F) {
            out.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0x10FFFF) {
            out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back('?');
        }
    }
    return out;
}

inline std::wstring NarrowToWide(const std::string& str) {
    std::wstring out;
    out.reserve(str.size());
    size_t i = 0;
    while (i < str.size()) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (c <= 0x7F) {
            out.push_back(static_cast<wchar_t>(c));
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 < str.size()) {
                uint32_t cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(str[i + 1]) & 0x3F);
                out.push_back(static_cast<wchar_t>(cp));
                i += 2;
            } else {
                out.push_back(static_cast<wchar_t>(c));
                i += 1;
            }
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 < str.size()) {
                uint32_t cp = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(str[i + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(str[i + 2]) & 0x3F);
                out.push_back(static_cast<wchar_t>(cp));
                i += 3;
            } else {
                out.push_back(static_cast<wchar_t>(c));
                i += 1;
            }
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 < str.size()) {
                uint32_t cp = ((c & 0x07) << 18) | ((static_cast<unsigned char>(str[i + 1]) & 0x3F) << 12) | ((static_cast<unsigned char>(str[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(str[i + 3]) & 0x3F);
                out.push_back(static_cast<wchar_t>(cp));
                i += 4;
            } else {
                out.push_back(static_cast<wchar_t>(c));
                i += 1;
            }
        } else {
            out.push_back(static_cast<wchar_t>(c));
            i += 1;
        }
    }
    return out;
}

// far2l-compatible naming aliases
inline std::string Wide2MB(const std::wstring& wstr) { return WideToNarrow(wstr); }
inline std::wstring MB2Wide(const std::string& str) { return NarrowToWide(str); }

inline std::wstring PathCombine(const std::wstring& base, const std::wstring& rel) {
    if (base.empty()) return rel;
    if (rel.empty()) return base;
    if (rel.front() == L'/') return rel;
    if (base.back() == L'/') return base + rel;
    return base + L'/' + rel;
}

inline std::wstring PathDirname(const std::wstring& path) {
    if (path.empty()) return L".";
    size_t lastSlash = path.find_last_of(L'/');
    if (lastSlash == std::wstring::npos) return L".";
    if (lastSlash == 0) return L"/";
    return path.substr(0, lastSlash);
}

inline std::wstring PathBasename(const std::wstring& path) {
    if (path.empty()) return L"";
    size_t lastSlash = path.find_last_of(L'/');
    if (lastSlash == std::wstring::npos) return path;
    return path.substr(lastSlash + 1);
}

inline bool PathStartsWith(const std::wstring& path, const std::wstring& prefix) {
    if (prefix.empty()) return true;
    if (path.size() < prefix.size()) return false;
    return (path.compare(0, prefix.size(), prefix) == 0);
}
