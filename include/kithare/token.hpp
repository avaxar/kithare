/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <kithare/string.hpp>


namespace kh {
    using namespace std;

    struct Token;
    struct TokenValue;
    enum class Operator;
    enum class Symbol;
    enum class TokenType;

    u32string str(const kh::Token& token, bool show_token_type = false);
    u32string str(kh::TokenType type);
    u32string str(kh::Operator op);
    u32string str(kh::Symbol sym);

    enum class Operator {
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        POW,

        IADD,
        ISUB,
        IMUL,
        IDIV,
        IMOD,
        IPOW,

        INCREMENT,
        DECREMENT,

        EQUAL,
        NOT_EQUAL,
        LESS,
        MORE,
        LESS_EQUAL,
        MORE_EQUAL,

        BIT_AND,
        BIT_OR,
        BIT_NOT,
        BIT_LSHIFT,
        BIT_RSHIFT,
        AND,
        OR,
        NOT,

        ASSIGN,
        SIZEOF,
        ADDRESS
    };

    enum class Symbol {
        SEMICOLON,
        DOT,
        COMMA,
        COLON,

        PARENTHESES_OPEN,
        PARENTHESES_CLOSE,
        CURLY_OPEN,
        CURLY_CLOSE,
        SQUARE_OPEN,
        SQUARE_CLOSE
    };

    enum class TokenType {
        IDENTIFIER,
        OPERATOR,
        SYMBOL,
        CHARACTER,
        STRING,
        BUFFER,
        UINTEGER,
        INTEGER,
        FLOATING,
        IMAGINARY
    };

    struct TokenValue {
        union {
            kh::Operator operator_type;
            kh::Symbol symbol_type;

            uint64_t uinteger;
            int64_t integer;
            char32_t character;

            double floating;
            double imaginary;
        };

        u32string ustring;

        string identifier;
        string buffer;
    };

    struct Token {
        size_t column;
        size_t line;
        size_t index;
        size_t length;
        kh::TokenType type;
        kh::TokenValue value;

        Token();
        Token(size_t _index, size_t _end, kh::TokenType _type, const kh::TokenValue& _value);
    };
}
