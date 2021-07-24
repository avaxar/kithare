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

    std::string quote(const std::u32string& str);
    std::string quote(const std::string& str);

    std::string strfy(const std::wstring& str);

    std::string strfy(char chr);
    std::string strfy(wchar_t chr);
    std::string strfy(char32_t chr);

    std::string strfy(int64_t n);
    std::string strfy(uint64_t n);
    std::string strfy(float n);
    std::string strfy(double n);

    std::string strfy(const std::complex<float>& n);
    std::string strfy(const std::complex<double>& n);
}
