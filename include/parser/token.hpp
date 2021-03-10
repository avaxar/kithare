/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/parser/token.hpp
*/

#pragma once

#include <string>


namespace kh {
    enum class Operator;
    enum class Symbol;
    enum class TokenType;
    struct TokenValue;
    struct Token;

    std::u32string repr(const kh::Token& token);
    std::u32string repr(const kh::TokenType type);
    std::u32string repr(const kh::Operator op);
    std::u32string repr(const kh::Symbol sym);

    enum class Operator {
        ADD, SUB, MUL, DIV, MOD, POW,
        IADD, ISUB, IMUL, IDIV, IMOD, IPOW,

        EQUAL, NOT_EQUAL, LESS, MORE, LESS_EQUAL, MORE_EQUAL,

        BIT_AND, BIT_OR, BIT_NOT,
        BIT_LSHIFT, BIT_RSHIFT,
        AND, OR, NOT,

        ASSIGN, SIZEOF, ADDRESS
    };

    enum class Symbol {
        SEMICOLON, DOT, COMMA,
        QUESTION, COLON, DOLLAR,

        PARENTHESES_OPEN, PARENTHESES_CLOSE,
        CURLY_OPEN, CURLY_CLOSE,
        SQUARE_OPEN, SQUARE_CLOSE,

        TEMPLATE_OPEN, TEMPLATE_CLOSE
    };

    enum class TokenType {
        IDENTIFIER, OPERATOR, SYMBOL,
        CHARACTER, STRING, BUFFER,
        UINTEGER, INTEGER, FLOATING, IMAGINARY
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
        size_t column, line;

        kh::TokenType type;
        kh::TokenValue value;

        Token(const kh::TokenType _type, const kh::TokenValue& _value, const size_t _column, const size_t _line) :
            type(_type), value(_value), column(_column), line(_line) {}

        Token(const kh::Operator op, const size_t _column, const size_t _line) :
            type(kh::TokenType::OPERATOR), column(_column), line(_line) {
            this->value.operator_type = op;
        }

        Token(const kh::Symbol sym, const size_t _column, const size_t _line) :
            type(kh::TokenType::SYMBOL), column(_column), line(_line) {
            this->value.symbol_type = sym;
        }

        Token(const uint64_t uintg, const size_t _column, const size_t _line) :
            type(kh::TokenType::UINTEGER), column(_column), line(_line) {
            this->value.uinteger = uintg;
        }

        Token(const int64_t intg, const size_t _column, const size_t _line) :
            type(kh::TokenType::INTEGER), column(_column), line(_line) {
            this->value.integer = intg;
        }

        Token(const char32_t chr, const size_t _column, const size_t _line) :
            type(kh::TokenType::CHARACTER), column(_column), line(_line) {
            this->value.character = chr;
        }

        Token(const std::u32string str, const size_t _column, const size_t _line) :
            type(kh::TokenType::STRING), column(_column), line(_line) {
            this->value.string = str;
        }

        Token(const std::string buf, const size_t _column, const size_t _line) :
            type(kh::TokenType::BUFFER), column(_column), line(_line) {
            this->value.buffer = buf;
        }

        Token(const double fl, const kh::TokenType _type, const size_t _column, const size_t _line) :
            type(_type), column(_column), line(_line) {
            switch (_type) {
            case kh::TokenType::FLOATING:
                this->value.floating = fl; break;
            case kh::TokenType::IMAGINARY:
                this->value.imaginary = fl; break;
            }
        }

        Token(const std::u32string str, const kh::TokenType _type, const size_t _column, const size_t _line) :
            type(_type), column(_column), line(_line) {
            switch (_type) {
            case kh::TokenType::IDENTIFIER:
                this->value.identifier = str; break;
            case kh::TokenType::STRING:
                this->value.string = str; break;
            }
        }
    };
}
