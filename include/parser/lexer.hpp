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
        LexException(const std::u32string& _what, const size_t _index) :
            what(_what), index(_index) {}

        std::u32string what;
        size_t index;
    };

    std::vector<kh::Token> lex(const std::u32string& source);
}
