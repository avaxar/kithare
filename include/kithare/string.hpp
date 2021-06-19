/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <complex>
#include <string>


namespace kh {
    void getLineColumn(const std::u32string& str, const size_t index, size_t& column, size_t& line);

    std::u32string quote(const std::u32string& str);
    std::u32string quote(const std::string& str);

    std::u32string str(const std::wstring& str);
    std::u32string str(const std::string& str);

    std::u32string str(const char chr);
    std::u32string str(const wchar_t chr);
    std::u32string str(const char32_t chr);

    std::u32string str(const int64_t n);
    std::u32string str(const uint64_t n);
    std::u32string str(const float n);
    std::u32string str(const double n);

    std::u32string str(const std::complex<float>& n);
    std::u32string str(const std::complex<double>& n);
}
