/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <vector>

#include <kithare/exception.hpp>
#include <kithare/string.hpp>
#include <kithare/token.hpp>

#define KH_LEX_CTX kh::LexerContext& context


namespace kh {
    using namespace std;

    class LexException : public kh::Exception {
    public:
        string what;
        size_t column;
        size_t line;
        size_t index;

        LexException(const string& _what, /* size_t _column, size_t _line, */ size_t _index)
            : what(_what), /* column(_column), line(_line), */ index(_index) {}
        virtual ~LexException() {}
        virtual string format() const;
    };

    struct LexerContext {
        const u32string& source;
        vector<kh::LexException>& exceptions;

        /* Character iterator */
        size_t ci = 0;

        /* Gets character of the current iterator index */
        inline char32_t chr() const {
            return this->source[this->ci];
        }
    };

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

    vector<kh::Token> lex(const u32string& source);

    vector<kh::Token> lex(KH_LEX_CTX);
}
