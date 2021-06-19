/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <complex>
#include <string>


namespace kh {
    void getLineColumn(const std::u32string& str, size_t index, size_t& column, size_t& line);

    std::u32string quote(const std::u32string& str);
    std::u32string quote(const std::string& str);

    std::u32string str(const std::wstring& str);
    std::u32string str(const std::string& str);

    std::u32string str(char chr);
    std::u32string str(wchar_t chr);
    std::u32string str(char32_t chr);

    std::u32string str(int64_t n);
    std::u32string str(uint64_t n);
    std::u32string str(float n);
    std::u32string str(double n);

    std::u32string str(const std::complex<float>& n);
    std::u32string str(const std::complex<double>& n);
}
