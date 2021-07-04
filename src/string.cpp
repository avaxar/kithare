/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/string.hpp>
#include <kithare/utf8.hpp>


std::u32string kh::quote(const std::u32string& str) {
    std::u32string quoted_str = U"\"";
    quoted_str.reserve(str.size() + str.size() / 8 + 2);

    for (char32_t chr : str) {
        switch (chr) {
            case U'"':
                quoted_str += U"\\\"";
                break;
            case U'\\':
                quoted_str += U"\\\\";
                break;
            case U' ':
                quoted_str += U" ";
                break;
            case U'\t':
                quoted_str += U"\\t";
                break;
            case U'\v':
                quoted_str += U"\\v";
                break;
            case U'\n':
                quoted_str += U"\\n";
                break;
            case U'\r':
                quoted_str += U"\\r";
                break;
            case U'\b':
                quoted_str += U"\\b";
                break;
            case U'\f':
                quoted_str += U"\\f";
                break;
            case U'\a':
                quoted_str += U"\\a";
                break;
            default:
                if (chr > 32 && chr < 127) {
                    quoted_str += (char32_t)chr;
                }
                else {
                    std::stringstream sstream;
                    sstream << std::hex << (uint32_t)chr;

                    if (chr <= 0xff)
                        quoted_str +=
                            U"\\x" + kh::str((sstream.str().size() == 1 ? "0" : "") + sstream.str());
                    else if (chr <= 0xfff) {
                        quoted_str += U"\\u0" + kh::str(sstream.str());
                    }
                    else if (chr <= 0xfffff) {
                        quoted_str += U"\\U000" + kh::str(sstream.str());
                    }
                    else if (chr <= 0xffffff) {
                        quoted_str += U"\\U00" + kh::str(sstream.str());
                    }
                    else if (chr <= 0xfffffff) {
                        quoted_str += U"\\U0" + kh::str(sstream.str());
                    }
                    else
                        quoted_str += U"\\U" + kh::str(sstream.str());
                }
        }
    }

    quoted_str += U'"';
    return quoted_str;
}

std::u32string kh::quote(const std::string& str) {
    std::u32string quoted_str = U"\"";
    quoted_str.reserve(str.size() + str.size() / 8 + 2);

    for (char chr : str) {
        switch (chr) {
            case '"':
                quoted_str += U"\\\"";
                break;
            case '\\':
                quoted_str += U"\\\\";
                break;
            case ' ':
                quoted_str += U" ";
                break;
            case '\t':
                quoted_str += U"\\t";
                break;
            case '\v':
                quoted_str += U"\\v";
                break;
            case '\n':
                quoted_str += U"\\n";
                break;
            case '\r':
                quoted_str += U"\\r";
                break;
            case '\b':
                quoted_str += U"\\b";
                break;
            case '\f':
                quoted_str += U"\\f";
                break;
            case '\a':
                quoted_str += U"\\a";
                break;
            default:
                if (chr > 32 && chr < 127) {
                    quoted_str += (char32_t)chr;
                }
                else {
                    std::stringstream sstream;
                    sstream << std::hex << (int)(uint8_t)chr;
                    quoted_str +=
                        U"\\x" + kh::str((sstream.str().size() == 1 ? "0" : "") + sstream.str());
                }
        }
    }

    quoted_str += U'"';
    return quoted_str;
}

void kh::getLineColumn(const std::u32string& str, size_t index, size_t& column, size_t& line) {
    column = 0;
    line = 1;

    for (size_t i = 0; i < index + 1; i++) {
        if (str[i] == U'\n') {
            column = 1;
            line++;
        }
        else
            column++;
    }
}

std::u32string kh::str(const std::wstring& str) {
    std::u32string str32;
    str32.reserve(str.size());

    for (wchar_t chr : str) {
        str32 += (char32_t)chr;
    }

    return str32;
}

std::u32string kh::str(const std::string& str) {
    std::u32string str32;
    str32.reserve(str.size());

    for (char chr : str) {
        str32 += (char32_t)chr;
    }

    return str32;
}

std::u32string kh::str(char chr) {
    char32_t chr32 = chr;
    return std::u32string(&chr32, &chr32 + 1);
}

std::u32string kh::str(wchar_t chr) {
    char32_t chr32 = chr;
    return std::u32string(&chr32, &chr32 + 1);
}

std::u32string kh::str(char32_t chr) {
    return std::u32string(&chr, &chr + 1);
}

#define KH_REPR_TO_STRING(TYPE)            \
    std::u32string kh::str(TYPE n) {       \
        return kh::str(std::to_string(n)); \
    }

KH_REPR_TO_STRING(int64_t);
KH_REPR_TO_STRING(uint64_t);
KH_REPR_TO_STRING(float);
KH_REPR_TO_STRING(double);

std::u32string kh::str(const std::complex<float>& n) {
    return kh::str(n.real()) + U" + " + kh::str(n.imag()) + U"i";
}

std::u32string kh::str(const std::complex<double>& n) {
    return kh::str(n.real()) + U" + " + kh::str(n.imag()) + U"i";
}
