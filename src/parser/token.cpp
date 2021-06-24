/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/string.hpp>
#include <kithare/token.hpp>


kh::Token::Token() : column(0), line(0), index(0), length(0), type(), value() {}

kh::Token::Token(size_t _index, size_t _end, kh::TokenType _type, const kh::TokenValue& _value)
    : column(0), line(0), index(_index), length(_end - index), type(_type), value(_value) {}

std::u32string kh::str(const kh::Token& token, bool show_token_type) {
    std::u32string str;
    if (show_token_type)
        str = kh::str(token.type) + U' ';

    switch (token.type) {
        case kh::TokenType::IDENTIFIER:
            str += token.value.identifier;
            break;
        case kh::TokenType::OPERATOR:
            str += kh::str(token.value.operator_type);
            break;
        case kh::TokenType::SYMBOL:
            str += kh::str(token.value.symbol_type);
            break;

        case kh::TokenType::CHARACTER:
            str += kh::str(token.value.character);
            break;
        case kh::TokenType::STRING:
            str += kh::quote(token.value.string);
            break;
        case kh::TokenType::BUFFER:
            str += kh::quote(token.value.buffer);
            break;

        case kh::TokenType::UINTEGER:
            str += kh::str(token.value.uinteger);
            break;
        case kh::TokenType::INTEGER:
            str += kh::str(token.value.integer);
            break;
        case kh::TokenType::FLOATING:
            str += kh::str(token.value.floating);
            break;
        case kh::TokenType::IMAGINARY:
            str += kh::str(token.value.imaginary) + U"i";
            break;

        default:
            str = U"unknown";
    }

    return str;
}

std::u32string kh::str(kh::TokenType type) {
    switch (type) {
        case kh::TokenType::IDENTIFIER:
            return U"identifier";
        case kh::TokenType::OPERATOR:
            return U"operator";
        case kh::TokenType::SYMBOL:
            return U"symbol";

        case kh::TokenType::CHARACTER:
            return U"character";
        case kh::TokenType::STRING:
            return U"string";
        case kh::TokenType::BUFFER:
            return U"buffer";

        case kh::TokenType::UINTEGER:
            return U"uinteger";
        case kh::TokenType::INTEGER:
            return U"integer";
        case kh::TokenType::FLOATING:
            return U"floating";
        case kh::TokenType::IMAGINARY:
            return U"imaginary";

        default:
            return U"unknown";
    }
}

std::u32string kh::str(kh::Operator op) {
    switch (op) {
        case kh::Operator::ADD:
            return U"+";
        case kh::Operator::SUB:
            return U"-";
        case kh::Operator::MUL:
            return U"*";
        case kh::Operator::DIV:
            return U"/";
        case kh::Operator::MOD:
            return U"%";
        case kh::Operator::POW:
            return U"^";

        case kh::Operator::IADD:
            return U"+=";
        case kh::Operator::ISUB:
            return U"-=";
        case kh::Operator::IMUL:
            return U"*=";
        case kh::Operator::IDIV:
            return U"/=";
        case kh::Operator::IMOD:
            return U"%=";
        case kh::Operator::IPOW:
            return U"^=";

        case kh::Operator::INCREMENT:
            return U"++";
        case kh::Operator::DECREMENT:
            return U"--";

        case kh::Operator::EQUAL:
            return U"==";
        case kh::Operator::NOT_EQUAL:
            return U"!=";
        case kh::Operator::LESS:
            return U"<";
        case kh::Operator::MORE:
            return U">";
        case kh::Operator::LESS_EQUAL:
            return U"<=";
        case kh::Operator::MORE_EQUAL:
            return U">=";

        case kh::Operator::BIT_AND:
            return U"&";
        case kh::Operator::BIT_OR:
            return U"|";
        case kh::Operator::BIT_NOT:
            return U"~";

        case kh::Operator::BIT_LSHIFT:
            return U"<<";
        case kh::Operator::BIT_RSHIFT:
            return U">>";

        case kh::Operator::AND:
            return U"and";
        case kh::Operator::OR:
            return U"or";
        case kh::Operator::NOT:
            return U"not";

        case kh::Operator::ASSIGN:
            return U"=";
        case kh::Operator::SIZEOF:
            return U"#";
        case kh::Operator::ADDRESS:
            return U"@";

        default:
            return U"unknown";
    }
}

std::u32string kh::str(kh::Symbol sym) {
    switch (sym) {
        case kh::Symbol::SEMICOLON:
            return U";";
        case kh::Symbol::DOT:
            return U".";
        case kh::Symbol::COMMA:
            return U",";
        case kh::Symbol::COLON:
            return U":";

        case kh::Symbol::PARENTHESES_OPEN:
            return U"(";
        case kh::Symbol::PARENTHESES_CLOSE:
            return U")";

        case kh::Symbol::CURLY_OPEN:
            return U"{";
        case kh::Symbol::CURLY_CLOSE:
            return U"}";

        case kh::Symbol::SQUARE_OPEN:
            return U"[";
        case kh::Symbol::SQUARE_CLOSE:
            return U"]";

        default:
            return U"unknown";
    }
}
