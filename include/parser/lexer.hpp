/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/parser/lexer.hpp
* Declares the lexical analyzer (lexer) function and an exception type.
*/

#pragma once

#include <cwctype>
#include <functional>
#include <vector>

#include "parser/token.hpp"
#include "utility/string.hpp"


namespace kh {
    struct LexException {
        LexException(const std::u32string& _what, const size_t _index) :
            what(_what), index(_index) {}

        std::u32string what;
        size_t index;
    };

    /// <summary>
    /// Lexicate/tokenize a source string into a list/std::vector of tokens.
    /// </summary>
    /// <param name="source">Input source string</param>
    /// <returns></returns>
    std::vector<kh::Token> lex(const std::u32string& source);
}
