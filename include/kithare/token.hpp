/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <kithare/string.hpp>


namespace kh {
    struct Token;
    struct TokenValue;
    enum class Operator;
    enum class Symbol;
    enum class TokenType;

    std::u32string repr(const kh::Token& token);
    std::u32string repr(const kh::TokenType type);
    std::u32string repr(const kh::Operator op);
    std::u32string repr(const kh::Symbol sym);

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
        IMAGINARY,
        COMMENT
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

        std::u32string identifier;
        std::u32string string;

        std::string buffer;
    };

    struct Token {
        size_t column;
        size_t line;
        size_t index;
        size_t length;
        kh::TokenType type;
        kh::TokenValue value;

        Token() {}
        Token(const size_t _index, const size_t _end, const kh::TokenType _type,
              const kh::TokenValue& _value)
            : index(_index), length(_end - index), type(_type), value(_value) {}
    };
}
