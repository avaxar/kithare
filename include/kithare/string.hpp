/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <complex>

#include <kithare/utf8.hpp>


namespace kh {
    void getLineColumn(const std::u32string& str, size_t index, size_t& column, size_t& line);

    std::u32string quote(const std::u32string& str);
    std::u32string quote(const std::string& str);

    std::u32string strfy(const std::wstring& str);
    std::u32string strfy(const std::string& str);

    std::u32string strfy(char chr);
    std::u32string strfy(wchar_t chr);
    std::u32string strfy(char32_t chr);

    std::u32string strfy(int64_t n);
    std::u32string strfy(uint64_t n);
    std::u32string strfy(float n);
    std::u32string strfy(double n);

    std::u32string strfy(const std::complex<float>& n);
    std::u32string strfy(const std::complex<double>& n);
}
