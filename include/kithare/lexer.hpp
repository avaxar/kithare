/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <cwctype>
#include <functional>
#include <vector>

#include <kithare/token.hpp>


namespace kh {
    class LexException : public Exception {
    public:
        std::string what;
        size_t column;
        size_t line;
        size_t index;

        LexException(const std::string& _what, /* size_t _column, size_t _line, */ size_t _index)
            : what(_what), /* column(_column), line(_line), */ index(_index) {}
        virtual ~LexException() {}
        virtual std::string format() const;
    };

    struct Lexer {
        const std::u32string& source;
        std::vector<LexException>& exceptions;

        /* Character iterator */
        size_t ci = 0;

        /* Gets character of the current iterator index */
        inline char32_t chr() const {
            return this->source[this->ci];
        }

        std::vector<Token> lex();
    };

    std::vector<Token> lex(const std::u32string& source);

    inline bool isDec(char32_t chr) {
        return U'0' <= chr && chr <= U'9';
    }

    inline bool isBin(char32_t chr) {
        return chr == U'0' || chr == U'1';
    }

    inline bool isOct(char32_t chr) {
        return U'0' <= chr && chr <= U'7';
    }

    inline bool isHex(char32_t chr) {
        return (U'0' <= chr && chr <= U'9') || (U'a' <= chr && chr <= U'f') ||
               (U'A' <= chr && chr <= U'F');
    }
}
