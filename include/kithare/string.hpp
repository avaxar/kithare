/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <complex>
#include <string>


namespace kh {
    using namespace std;

    void getLineColumn(const u32string& str, size_t index, size_t& column, size_t& line);

    u32string quote(const u32string& str);
    u32string quote(const string& str);

    u32string str(const wstring& str);
    u32string str(const string& str);

    u32string str(char chr);
    u32string str(wchar_t chr);
    u32string str(char32_t chr);

    u32string str(int64_t n);
    u32string str(uint64_t n);
    u32string str(float n);
    u32string str(double n);

    u32string str(const complex<float>& n);
    u32string str(const complex<double>& n);
}
