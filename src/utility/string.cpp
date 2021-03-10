/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/utility/string.cpp
* Defines include/utility/string.hpp
*/

#include <iostream>
#include <string>
#include <sstream>

#include "utility/string.hpp"


/// <summary>
/// Encodes UTF-32 string to a UTF-8 string.
/// </summary>
/// <param name="str">UTF-32 string</param>
/// <returns></returns>
std::string kh::encodeUtf8(const std::u32string& str) {
    std::string str8;
    str8.reserve(str.size() + str.size() / 4);

    for (const char32_t chr : str) {
        if (chr > 0xFFFF) {
            str8 += 0b11110000 | (char)(0b00000111 & (chr >> 18));
            str8 += 0b10000000 | (char)(0b00111111 & (chr >> 12));
            str8 += 0b10000000 | (char)(0b00111111 & (chr >> 6));
            str8 += 0b10000000 | (char)(0b00111111 & chr);
        }
        else if (chr > 0x7FF) {
            str8 += 0b11100000 | (char)(0b00001111 & (chr >> 12));
            str8 += 0b10000000 | (char)(0b00111111 & (chr >> 6));
            str8 += 0b10000000 | (char)(0b00111111 & chr);
        }
        else if (chr > 0x7F) {
            str8 += 0b11000000 | (char)(0b00011111 & (chr >> 6));
            str8 += 0b10000000 | (char)(0b00111111 & chr);
        }
        else 
            str8 += (char)chr;
    }

    return str8;
}

/// <summary>
/// Decodes a UTF-8 encoded string to a UTF-32 string.
/// </summary>
/// <param name="str">UTF-8 string</param>
/// <returns></returns>
std::u32string kh::decodeUtf8(const std::string& str) {
    std::u32string str32;
    str32.reserve(str.size());

    uint8_t continuation = 0;
    uint32_t temp = 0;

    for (size_t i = 0; i < str.size(); i++) {
        const uint8_t chr = str[i];

        if (continuation) {
            if ((chr & 0b11000000) != 0b10000000)
                throw kh::UnicodeDecodeError(U"Expected continuation byte", i);

            temp = (temp << 6) + (chr & 0b00111111);
            continuation--;
        }
        else {
            if (temp) {
                str32 += (char32_t)temp;
                temp = 0;
                i++;
            }

            if (chr < 128)
                str32 += (char32_t)chr;
            else if ((chr & 0b11100000) == 0b11000000) {
                temp = chr & 0b00011111;
                continuation = 1;
            }
            else if ((chr & 0b11110000) == 0b11100000) {
                temp = chr & 0b00001111;
                continuation = 2;
            }
            else if ((chr & 0b11111000) == 0b11110000) {
                temp = chr & 0b00000111;
                continuation = 3;
            }
            else
                throw kh::UnicodeDecodeError(U"Ivalid start byte", i);
        }
    }

    if (continuation)
        throw kh::UnicodeDecodeError(U"Expected continuation byte near end", str.size() - 1);

    if (temp)
        str32 += (char32_t)temp;

    return str32;
}

/// <summary>
/// Quotes a string and handles escapes.
/// </summary>
/// <param name="str">String to be quoted</param>
/// <returns></returns>
std::u32string kh::quote(const std::u32string& str) {
    std::u32string repr_str = U"\"";
    repr_str.reserve(str.size() + str.size() / 8 + 2);

    for (const char32_t chr : str) {
        switch (chr) {
        case U'"': repr_str += U"\\\""; break;
        case U'\\': repr_str += U"\\\\"; break;
        case U'\t': repr_str += U"\\t"; break;
        case U'\v': repr_str += U"\\v"; break;
        case U'\n': repr_str += U"\\n"; break;
        case U'\r': repr_str += U"\\r"; break;
        default: repr_str += chr;
        }
    }

    repr_str += U'"';
    return repr_str;
}

/// <summary>
/// Quotes a string and handles escapes.
/// </summary>
/// <param name="str">String to be quoted</param>
/// <returns></returns>
std::u32string kh::quote(const std::string& str) {
    std::u32string repr_str = U"\"";
    repr_str.reserve(str.size() + str.size() / 8 + 2);

    for (const char chr : str) {
        switch (chr) {
        case '"': repr_str += U"\\\""; break;
        case '\\': repr_str += U"\\\\"; break;
        case ' ': repr_str += U" "; break;
        case '\t': repr_str += U"\\t"; break;
        case '\v': repr_str += U"\\v"; break;
        case '\n': repr_str += U"\\n"; break;
        case '\r': repr_str += U"\\r"; break;
        default:
            if (chr > 32 && chr < 127)
                repr_str += (char32_t)chr;
            else {
                std::stringstream sstream;
                sstream << std::hex << (int)((uint8_t)chr);
                repr_str += U"\\x" + kh::repr((sstream.str().size() == 1 ? "0" : "") + sstream.str());
            }
        }
    }

    repr_str += U'"';
    return repr_str;
}

std::u32string kh::repr(const std::u32string& str) {
    return str;
}

std::u32string kh::repr(const std::wstring& str) {
    std::u32string str32;
    str32.reserve(str.size());

    for (const wchar_t chr : str)
        str32 += (char32_t)chr;

    return str32;
}

std::u32string kh::repr(const std::string& str) {
    return kh::decodeUtf8(str);
}

std::u32string kh::repr(const char chr) {
    char32_t chr32 = chr;
    return std::u32string(&chr32, &chr32 + 1);
}

std::u32string kh::repr(const wchar_t chr) {
    char32_t chr32 = chr;
    return std::u32string(&chr32, &chr32 + 1);
}

std::u32string kh::repr(const char32_t chr) {
    return std::u32string(&chr, &chr + 1);
}

std::u32string kh::repr(const int64_t n) {
    return kh::repr(std::to_string(n));
}

std::u32string kh::repr(const uint64_t n) {
    return kh::repr(std::to_string(n));
}

std::u32string kh::repr(const float n) {
    return kh::repr(std::to_string(n));
}

std::u32string kh::repr(const double n) {
    return kh::repr(std::to_string(n));
}
