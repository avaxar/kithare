#pragma once

#include <clocale>
#include <complex>
#include <iostream>
#include <string>


namespace kh {
    void getLineColumn(const std::u32string& str, const size_t index, size_t& column, size_t& line);

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
    std::u32string repr(const size_t n);
    std::u32string repr(const float n);
    std::u32string repr(const double n);

    std::u32string repr(const std::complex<float>& n);
    std::u32string repr(const std::complex<double>& n);
}
