/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/string.hpp>


void kh::getLineColumn(const std::u32string& str, size_t index, size_t& column, size_t& line) {
    column = 0;
    line = 1;

    for (size_t i = 0; i < index + 1; i++) {
        if (str[i] == '\n') {
            column = 1;
            line++;
        }
        else {
            column++;
        }
    }
}

std::string kh::quote(const std::u32string& str) {
    std::string quoted_str = "\"";
    quoted_str.reserve(str.size() + str.size() / 8 + 2);

    for (char32_t chr : str) {
        switch (chr) {
            case '"':
                quoted_str += "\\\"";
                break;
            case '\\':
                quoted_str += "\\\\";
                break;
            case ' ':
                quoted_str += " ";
                break;
            case '\t':
                quoted_str += "\\t";
                break;
            case '\v':
                quoted_str += "\\v";
                break;
            case '\n':
                quoted_str += "\\n";
                break;
            case '\r':
                quoted_str += "\\r";
                break;
            case '\b':
                quoted_str += "\\b";
                break;
            case '\f':
                quoted_str += "\\f";
                break;
            case '\a':
                quoted_str += "\\a";
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
                            "\\x" + std::string(sstream.str().size() == 1 ? "0" : "") + sstream.str();
                    else if (chr <= 0xfff) {
                        quoted_str += "\\u0" + sstream.str();
                    }
                    else if (chr <= 0xfffff) {
                        quoted_str += "\\U000" + sstream.str();
                    }
                    else if (chr <= 0xffffff) {
                        quoted_str += "\\U00" + sstream.str();
                    }
                    else if (chr <= 0xfffffff) {
                        quoted_str += "\\U0" + sstream.str();
                    }
                    else
                        quoted_str += "\\" + sstream.str();
                }
        }
    }

    quoted_str += '"';
    return quoted_str;
}

std::string kh::quote(const std::string& str) {
    std::string quoted_str = "\"";
    quoted_str.reserve(str.size() + str.size() / 8 + 2);

    for (char chr : str) {
        switch (chr) {
            case '"':
                quoted_str += "\\\"";
                break;
            case '\\':
                quoted_str += "\\\\";
                break;
            case ' ':
                quoted_str += " ";
                break;
            case '\t':
                quoted_str += "\\t";
                break;
            case '\v':
                quoted_str += "\\v";
                break;
            case '\n':
                quoted_str += "\\n";
                break;
            case '\r':
                quoted_str += "\\r";
                break;
            case '\b':
                quoted_str += "\\b";
                break;
            case '\f':
                quoted_str += "\\f";
                break;
            case '\a':
                quoted_str += "\\a";
                break;
            default:
                if (chr > 32 && chr < 127) {
                    quoted_str += (char32_t)chr;
                }
                else {
                    std::stringstream sstream;
                    sstream << std::hex << (int)(uint8_t)chr;
                    quoted_str +=
                        "\\x" + std::string(sstream.str().size() == 1 ? "0" : "") + sstream.str();
                }
        }
    }

    quoted_str += '"';
    return quoted_str;
}

std::string kh::strfy(const std::wstring& str) {
    std::u32string u32str;
    u32str.reserve(str.size());
    for (wchar_t chr : str) {
        u32str += chr;
    }

    return kh::utf8Encode(u32str);
}

std::string kh::strfy(char chr) {
    return std::string(&chr, &chr + 1);
}

std::string kh::strfy(wchar_t chr) {
    char32_t chr32 = chr;
    return kh::utf8Encode(std::u32string(&chr32, &chr32 + 1));
}

std::string kh::strfy(char32_t chr) {
    return kh::utf8Encode(std::u32string(&chr, &chr + 1));
}

#define KH_REPR_TO_STRING(TYPE)     \
    std::string kh::strfy(TYPE n) { \
        return std::to_string(n);   \
    }

KH_REPR_TO_STRING(int64_t);
KH_REPR_TO_STRING(uint64_t);
KH_REPR_TO_STRING(float);
KH_REPR_TO_STRING(double);

std::string kh::strfy(const std::complex<float>& n) {
    return strfy(n.real()) + " + " + strfy(n.imag()) + 'i';
}

std::string kh::strfy(const std::complex<double>& n) {
    return strfy(n.real()) + " + " + strfy(n.imag()) + 'i';
}
