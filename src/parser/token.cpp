/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/parser/token.cpp
* Defines include/parser/token.hpp
*/

#include "utility/string.hpp"

#include "parser/token.hpp"


std::u32string kh::repr(const kh::Token& token) {
    std::u32string str = kh::repr(token.type) + U" ";

    switch (token.type) {
    case kh::TokenType::IDENTIFIER:
        str += token.value.identifier; break;
    case kh::TokenType::OPERATOR:
        str += kh::repr(token.value.operator_type); break;
    case kh::TokenType::SYMBOL:
        str += kh::repr(token.value.symbol_type); break;

    case kh::TokenType::CHARACTER:
        str += kh::repr(token.value.character); break;
    case kh::TokenType::STRING:
        str += kh::quote(token.value.string); break;
    case kh::TokenType::BUFFER:
        str += kh::quote(token.value.buffer); break;

    case kh::TokenType::UINTEGER: 
        str += kh::repr(token.value.uinteger); break;
    case kh::TokenType::INTEGER: 
        str += kh::repr(token.value.integer); break;
    case kh::TokenType::FLOATING: 
        str += kh::repr(token.value.floating); break;
    case kh::TokenType::IMAGINARY: 
        str += kh::repr(token.value.imaginary) + U"i"; break;

    default: str = U"UNKNOWN";
    }

    return str;
}

std::u32string kh::repr(const kh::TokenType type) {
    switch (type) {
    case kh::TokenType::IDENTIFIER: return U"IDENTIFIER";
    case kh::TokenType::OPERATOR: return U"OPERATOR";
    case kh::TokenType::SYMBOL: return U"SYMBOL";

    case kh::TokenType::CHARACTER: return U"CHARACTER";
    case kh::TokenType::STRING: return U"STRING";
    case kh::TokenType::BUFFER: return U"BUFFER";

    case kh::TokenType::UINTEGER: return U"UINTEGER";
    case kh::TokenType::INTEGER: return U"INTEGER";
    case kh::TokenType::FLOATING: return U"FLOATING";
    case kh::TokenType::IMAGINARY: return U"IMAGINARY";

    default: return U"UNKNOWN";
    }
}

std::u32string kh::repr(const kh::Operator op) {
    switch (op) {
    case kh::Operator::ADD: return U"+";
    case kh::Operator::SUB: return U"-";
    case kh::Operator::MUL: return U"*";
    case kh::Operator::DIV: return U"/";
    case kh::Operator::MOD: return U"%";
    case kh::Operator::POW: return U"^";

    case kh::Operator::IADD: return U"+=";
    case kh::Operator::ISUB: return U"-=";
    case kh::Operator::IMUL: return U"*=";
    case kh::Operator::IDIV: return U"/=";
    case kh::Operator::IMOD: return U"%=";
    case kh::Operator::IPOW: return U"^=";

    case kh::Operator::EQUAL: return U"==";
    case kh::Operator::NOT_EQUAL: return U"!=";
    case kh::Operator::LESS: return U"<";
    case kh::Operator::MORE: return U">";
    case kh::Operator::LESS_EQUAL: return U"<=";
    case kh::Operator::MORE_EQUAL: return U">=";

    case kh::Operator::BIT_AND: return U"&";
    case kh::Operator::BIT_OR: return U"|";
    case kh::Operator::BIT_NOT: return U"~";

    case kh::Operator::BIT_LSHIFT: return U"<<";
    case kh::Operator::BIT_RSHIFT: return U">>";

    case kh::Operator::AND: return U"&&";
    case kh::Operator::OR: return U"||";
    case kh::Operator::NOT: return U"!";

    case kh::Operator::ASSIGN: return U"=";
    case kh::Operator::SIZEOF: return U"#";
    case kh::Operator::ADDRESS: return U"@";

    default: return U"UNKNOWN";
    }
}

std::u32string kh::repr(const kh::Symbol sym) {
    switch (sym) {
    case kh::Symbol::SEMICOLON: return U";";
    case kh::Symbol::DOT: return U".";
    case kh::Symbol::COMMA: return U",";

    case kh::Symbol::QUESTION: return U"?";
    case kh::Symbol::COLON: return U":";
    case kh::Symbol::DOLLAR: return U"$";

    case kh::Symbol::PARENTHESES_OPEN: return U"(";
    case kh::Symbol::PARENTHESES_CLOSE: return U")";

    case kh::Symbol::CURLY_OPEN: return U"{";
    case kh::Symbol::CURLY_CLOSE: return U"}";

    case kh::Symbol::SQUARE_OPEN: return U"[";
    case kh::Symbol::SQUARE_CLOSE: return U"]";

    case kh::Symbol::TEMPLATE_OPEN: return U"[<";
    case kh::Symbol::TEMPLATE_CLOSE: return U">]";
    }
}
