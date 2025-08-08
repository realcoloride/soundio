#ifndef WCHARCONVERT_H
#define WCHARCONVERT_H
#pragma once

#include "include.h"

/// <summary>
/// Converts a wchar to a string with a specified max buffer size
/// </summary>
static std::string convertWideCharToString(const wchar_t* wideString, size_t maxBufferSize = 512) {
    if (!wideString) return ""; // handle null input

    std::string result;
    result.resize(maxBufferSize);

    size_t converted = 0;
    if (!wideString || wcstombs_s(&converted, &result[0], result.size(), wideString, _TRUNCATE) != 0)
        return ""; // conversion failed

    // trim back to actual size
    result.resize(converted);
    return result;
}

#endif // WCHARCONVERT_H