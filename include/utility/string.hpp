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


namespace kh {
    std::string encodeUtf8(const std::u32string& str);
    std::u32string decodeUtf8(const std::string& str);

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

    /// <summary>
    /// Prints the value to the terminal.
    /// </summary>
    /// <typeparam name="T">Value type</typeparam>
    /// <param name="val">Value to be printed</param>
    template <typename T> 
    void print(const T& val) {
        std::u32string str = kh::repr(val);

        for (const char32_t chr : str)
            std::wcout << (wchar_t)chr;
    }

    /// <summary>
    /// Prints the value to the terminal with an additional newline character at the end.
    /// </summary>
    /// <typeparam name="T">Value type</typeparam>
    /// <param name="val">Value to be printed</param>
    template <typename T> 
    void println(const T& val) {
        kh::print<T>(val);
        std::wcout << '\n';
    }
}
