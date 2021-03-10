/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/utility/string.hpp
* Implements string utilities and specialization for Kithare's generic string which is a UTF-32 string
*/

#pragma once

#include <iostream>
#include <string>

#define print(val) {                    \
    std::u32string str = kh::repr(val); \
    for (const char32_t chr : str)      \
        std::wcout << (wchar_t)chr; }
#define println(val) { print(val); std::wcout << L'\n'; }


namespace kh {
    std::string encodeUtf8(const std::u32string& str);
    std::u32string decodeUtf8(const std::string& str);

    std::u32string quote(const std::u32string& str);
    std::u32string quote(const std::string& str);

    std::u32string repr(const std::u32string& str);
    std::u32string repr(const std::wstring& str);
    std::u32string repr(const std::string& str);

    std::u32string repr(const char chr);
    std::u32string repr(const wchar_t chr);
    std::u32string repr(const char32_t chr);

    std::u32string repr(const int64_t n);
    std::u32string repr(const uint64_t n);
    std::u32string repr(const float n);
    std::u32string repr(const double n);
}
