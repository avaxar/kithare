/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/string.hpp>
#include <kithare/utf8.hpp>


std::string kh::Utf8DecodingException::format() const {
    return this->what + " at index " + std::to_string(this->index);
}

std::string kh::encodeUtf8(const std::u32string& str) {
    std::string str8;
    str8.reserve(str.size());

    for (char32_t chr : str) {
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
        else {
            str8 += (char)chr;
        }
    }

    return str8;
}

std::u32string kh::decodeUtf8(const std::string& str) {
    std::u32string str32;
    str32.reserve(str.size());

    uint8_t continuation = 0;
    uint32_t temp = 0;

    for (size_t i = 0; i < str.size(); i++) {
        uint8_t chr = str[i];

        if (continuation) {
            if ((chr & 0b11000000) != 0b10000000) {
                throw kh::Utf8DecodingException("expected continuation byte", i);
            }

            temp = (temp << 6) + (chr & 0b00111111);
            continuation--;
        }
        else {
            if (temp) {
                str32 += (char32_t)temp;
                temp = 0;
            }

            if (chr < 128) {
                str32 += (char32_t)chr;
            }
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
            else {
                throw kh::Utf8DecodingException("invalid start byte", i);
            }
        }
    }

    if (continuation) {
        throw kh::Utf8DecodingException("expected continuation byte but hit end of file",
                                        str.size() - 1);
    }

    if (temp) {
        str32 += (char32_t)temp;
    }

    return str32;
}
