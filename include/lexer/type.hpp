/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/lexer/type.hpp
* Declares the token type enum and several sub-attribute enums.
*/

#pragma once

#include "utility/utils.hpp"


namespace kh {
    enum class TokenType {
        IDENTIFIER, OPERATOR, SYMBOL,
        CHARACTER, STRING, BUFFER, 
        UNSIGNED_INTEGER, INTEGER, FLOATING, IMAGINARY
    };

    enum class Operator {
        ADD, SUB, MUL, DIV, MOD, POW,
        IADD, ISUB, IMUL, IDIV, IMOD, IPOW,

        EQUAL, NOT_EQUAL, LESS, MORE, LESS_EQUAL, MORE_EQUAL,

        BIT_AND, BIT_OR, BIT_NOT,
        BIT_LSHIFT, BIT_RSHIFT,
        AND, OR, NOT,

        ASSIGN, SIZEOF, ADDRESS,
        STACK_INST
    };

    enum class Symbol {
        SEMICOLON, DOT, COMMA, QUESTION, COLON,

        PARENTHESES_OPEN, PARENTHESES_CLOSE,
        CURLY_OPEN, CURLY_CLOSE,
        SQUARE_OPEN, SQUARE_CLOSE,
        
        TEMPLATE_OPEN, TEMPLATE_CLOSE
    };
}
