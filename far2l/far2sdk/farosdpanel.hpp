#pragma once

#include <string>
#include <stdint.h>

class OsdPanelInterface {
public:
    virtual bool HasActiveLines() = 0;

    virtual int AddLine(const std::wstring& text, uint64_t color = 0) = 0;
    virtual void UpdateLine(int rowNumber, const std::wstring& text) = 0;
    virtual void CompleteLine(int rowNumber) = 0;
    virtual std::wstring GetLine(int rowNumber) const = 0;

    /**
     * @brief Update a sub-slice of an existing line at a fixed character position and size.
     * E.g. to update only the fan speed at columns 12..17 without recreating the entire line.
     * 
     * @param rowNumber Zero-based index of row.
     * @param colOffset Zero-based starting character column.
     * @param width Fixed width of the field in characters.
     * @param fieldText Text to insert (will be padded or truncated to match width).
     */
    virtual void UpdateField(int rowNumber, int colOffset, int width, const std::wstring& fieldText) = 0;
    virtual int AddPath(const std::wstring& prefix, const std::wstring& suffix, const std::wstring& pathToFile, uint64_t color = 0) = 0;
    virtual void UpdatePath(int rowNumber, const std::wstring& pathToFile) = 0;
    virtual std::wstring GetPath(int rowNumber) const = 0;

    // --- Numeric API ---
    virtual int AddNumeric(const std::wstring& prefix, const std::wstring& suffix, double value, int precision = 1, uint64_t color = 0) = 0;
    virtual void UpdateNumeric(int rowNumber, double value) = 0;
    virtual double GetNumeric(int rowNumber) const = 0;

    // --- Progress Bar API ---

    virtual int AddProgressBar(const std::wstring& prefix, const std::wstring& suffix, 
    	int64_t current, int64_t total, int64_t minVal = 0, bool showPercentage = true, uint64_t color = 0) = 0;
    virtual void UpdateProgressBar(int rowNumber, int64_t current, int64_t total = -1) = 0;
    virtual int64_t GetProgressBarCurrent(int rowNumber) const = 0;

    virtual size_t GetRowCount() const = 0;
    virtual void RemoveRow(int rowNumber) = 0;

	virtual ~OsdPanelInterface() = default;

    virtual bool IsActive(){ return GetRowCount() > 0 && HasActiveLines(); }
};
