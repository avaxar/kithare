/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/utility/string.hpp
* Implements string utilities and specialization for Kithare's generic string which is a UTF-32 string
*/

#pragma once

#include <clocale>
#include <complex>
#include <iostream>
#include <string>

#ifdef _WIN32
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <codecvt>
#endif

#ifndef KH_PRINT_TAB_SIZE
#define KH_PRINT_TAB_SIZE 4
#endif

#define print(val)                          \
    do {                                    \
        std::u32string str = kh::repr(val); \
        for (const char32_t chr : str) {    \
            if (chr == U'\t')               \
                for (size_t i = 0; i < KH_PRINT_TAB_SIZE; i++) \
                    std::putwchar(L' ');    \
            else                            \
                std::putwchar((wchar_t)chr);\
        }                                   \
    } while (false)                         \

#define println(val) do { print(val); std::putwchar(L'\n'); } while (false)


/* Sets the locale. These below are sorta' automatically run once the program starts if this header is included */
static const auto _locale_set = std::setlocale(LC_ALL, "en_US.utf8");

#ifdef _WIN32
/* Sets up std::wcout and std::wcin on Windows */
static std::locale _utf8_locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
static const auto _imbue_wcout_utf8 = std::wcout.imbue(_utf8_locale);
static const auto _imbue_wcin_utf8 = std::wcin.imbue(_utf8_locale);
#endif

namespace kh {
    struct UnicodeDecodeError {
        UnicodeDecodeError(const std::u32string& _what, const size_t _index) :
            what(_what), index(_index) {}

        std::u32string what;
        size_t index;
    };

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

    std::u32string repr(const std::complex<float> n);
    std::u32string repr(const std::complex<double> n);
}
