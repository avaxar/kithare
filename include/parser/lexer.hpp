/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/parser/lexer.hpp
* Declares the lexical analyzer (lexer) function and an exception type.
*/

#pragma once

#include <string>
#include <vector>

#include "parser/token.hpp"


namespace kh {
    struct LexException {
        LexException(const std::u32string& _what, const size_t _line, const size_t _column,
            const size_t _index, const char32_t _character) :
            what(_what), line(_line), column(_column), index(_index), character(_character) {}

        std::u32string what;
        size_t line;
        size_t column;
        size_t index;
        char32_t character;
    };

    std::vector<kh::Token> lex(const std::u32string& source);
}
