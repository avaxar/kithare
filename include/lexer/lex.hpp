/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/lexer/lex.hpp
* Declares the lexing function to be defined.
*/

#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"

#include "lexer/type.hpp"
#include "lexer/value.hpp"
#include "lexer/token.hpp"

/* Helper to raise error at a file */
#define KH_RAISE_ERROR(msg, n) throw kh::LexException(file_name, kh::toString(msg), line_n, char_line + n, i + n, n > 0 ? chAt(i + n) : ' ')


namespace kh {
    enum class TokenizeState {
        NONE, IDENTIFIER,
        INTEGER, FLOATING, HEX, OCTAL, BIN,
        IN_BUF, IN_STR, 
        IN_INLINE_COMMENT, IN_MULTIPLE_LINE_COMMENT
    };

    struct LexException {
        LexException(const kh::String& _file_name, const kh::String& _what, const size_t _line, const size_t _character_line,
                const size_t _index, const uint32 _character) :
            file_name(_file_name), what(_what), line(_line), character_line(_character_line), index(_index), character(_character) {}

        kh::String file_name;
        kh::String what;
        size_t line;
        size_t character_line;
        size_t index;
        uint32 character;
    };

    std::vector<kh::Token> lex(const kh::String& source, const kh::String& file_name);

    inline bool isDec(const uint32 chr) {
        return '0' <= chr && chr <= '9';
    }
    inline bool isBin(const uint32 chr) {
        return chr == '0' || chr == '1';
    }
    inline bool isOct(const uint32 chr) {
        return '0' <= chr && chr <= '7';
    }
    inline bool isHex(const uint32 chr) {
        return (('0' <= chr && chr <= '9') ||
                ('a' <= chr && chr <= 'f') ||
                ('A' <= chr && chr <= 'F'));
    }
}
