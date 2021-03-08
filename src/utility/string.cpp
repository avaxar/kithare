/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/utility/string.cpp
* Defines include/utility/string.hpp
*/

#include <iostream>

#include "utility/string.hpp"


/// <summary>
/// Encodes UTF-32 string to a UTF-8 string.
/// </summary>
/// <param name="str">UTF-32 string</param>
/// <returns></returns>
std::string kh::encodeUtf8(const std::u32string& str) {
    std::string str8;
    str8.reserve(str.size());

    return str8;
}

/// <summary>
/// Decodes a UTF-8 encoded string to a UTF-32 string.
/// </summary>
/// <param name="str">UTF-8 string</param>
/// <returns></returns>
std::u32string kh::decodeUtf8(const std::string& str) {
    std::u32string str32;
    str32.reserve(str.size());

    return str32;
}

std::u32string kh::repr(const std::u32string& str) {
    return str;
}

std::u32string kh::repr(const std::wstring& str) {
    std::u32string str32;
    str32.reserve(str.size());

    for (const wchar_t chr : str)
        str32 += (char32_t)chr;

    return str32;
}

std::u32string kh::repr(const std::string& str) {
    return kh::decodeUtf8(str);
}

std::u32string kh::repr(const char chr) {
    char32_t chr32 = chr;
    return std::u32string(&chr32, &chr32 + 1);
}

std::u32string kh::repr(const wchar_t chr) {
    char32_t chr32 = chr;
    return std::u32string(&chr32, &chr32 + 1);
}

std::u32string kh::repr(const char32_t chr) {
    return std::u32string(&chr, &chr + 1);
}

std::u32string kh::repr(const int64_t n) {
    return kh::repr(std::to_wstring(n));
}

std::u32string kh::repr(const uint64_t n) {
    return kh::repr(std::to_wstring(n));
}

std::u32string kh::repr(const float n) {
    return kh::repr(std::to_wstring(n));
}

std::u32string kh::repr(const double n) {
    return kh::repr(std::to_wstring(n));
}
