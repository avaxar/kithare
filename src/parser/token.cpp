/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/string.hpp>
#include <kithare/token.hpp>
#include <kithare/utf8.hpp>


using namespace kh;

Token::Token() : column(0), line(0), index(0), length(0), type(), value() {}

Token::Token(size_t _index, size_t _end, TokenType _type, const TokenValue& _value)
    : column(0), line(0), index(_index), length(_end - index), type(_type), value(_value) {}

std::u32string kh::strfy(const Token& token, bool show_token_type) {
    std::u32string str;
    if (show_token_type) {
        str = strfy(token.type) + U' ';
    }

    switch (token.type) {
        case TokenType::IDENTIFIER:
            str += utf8Decode(token.value.identifier);
            break;
        case TokenType::OPERATOR:
            str += strfy(token.value.operator_type);
            break;
        case TokenType::SYMBOL:
            str += strfy(token.value.symbol_type);
            break;

        case TokenType::CHARACTER:
            str += strfy(token.value.character);
            break;
        case TokenType::STRING:
            str += quote(token.value.string);
            break;
        case TokenType::BUFFER:
            str += quote(token.value.buffer);
            break;

        case TokenType::UINTEGER:
            str += strfy(token.value.uinteger);
            break;
        case TokenType::INTEGER:
            str += strfy(token.value.integer);
            break;
        case TokenType::FLOATING:
            str += strfy(token.value.floating);
            break;
        case TokenType::IMAGINARY:
            str += strfy(token.value.imaginary) + U"i";
            break;

        default:
            str = U"unknown";
    }

    return str;
}

std::u32string kh::strfy(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER:
            return U"identifier";
        case TokenType::OPERATOR:
            return U"operator";
        case TokenType::SYMBOL:
            return U"symbol";

        case TokenType::CHARACTER:
            return U"character";
        case TokenType::STRING:
            return U"string";
        case TokenType::BUFFER:
            return U"buffer";

        case TokenType::UINTEGER:
            return U"uinteger";
        case TokenType::INTEGER:
            return U"integer";
        case TokenType::FLOATING:
            return U"floating";
        case TokenType::IMAGINARY:
            return U"imaginary";

        default:
            return U"unknown";
    }
}

std::u32string kh::strfy(Operator op) {
    switch (op) {
        case Operator::ADD:
            return U"+";
        case Operator::SUB:
            return U"-";
        case Operator::MUL:
            return U"*";
        case Operator::DIV:
            return U"/";
        case Operator::MOD:
            return U"%";
        case Operator::POW:
            return U"^";

        case Operator::IADD:
            return U"+=";
        case Operator::ISUB:
            return U"-=";
        case Operator::IMUL:
            return U"*=";
        case Operator::IDIV:
            return U"/=";
        case Operator::IMOD:
            return U"%=";
        case Operator::IPOW:
            return U"^=";

        case Operator::INCREMENT:
            return U"++";
        case Operator::DECREMENT:
            return U"--";

        case Operator::EQUAL:
            return U"==";
        case Operator::NOT_EQUAL:
            return U"!=";
        case Operator::LESS:
            return U"<";
        case Operator::MORE:
            return U">";
        case Operator::LESS_EQUAL:
            return U"<=";
        case Operator::MORE_EQUAL:
            return U">=";

        case Operator::BIT_AND:
            return U"&";
        case Operator::BIT_OR:
            return U"|";
        case Operator::BIT_NOT:
            return U"~";

        case Operator::BIT_LSHIFT:
            return U"<<";
        case Operator::BIT_RSHIFT:
            return U">>";

        case Operator::AND:
            return U"and";
        case Operator::OR:
            return U"or";
        case Operator::NOT:
            return U"not";

        case Operator::ASSIGN:
            return U"=";
        case Operator::SIZEOF:
            return U"#";
        case Operator::ADDRESS:
            return U"@";

        default:
            return U"unknown";
    }
}

std::u32string kh::strfy(Symbol sym) {
    switch (sym) {
        case Symbol::SEMICOLON:
            return U";";
        case Symbol::DOT:
            return U".";
        case Symbol::COMMA:
            return U",";
        case Symbol::COLON:
            return U":";

        case Symbol::PARENTHESES_OPEN:
            return U"(";
        case Symbol::PARENTHESES_CLOSE:
            return U")";

        case Symbol::CURLY_OPEN:
            return U"{";
        case Symbol::CURLY_CLOSE:
            return U"}";

        case Symbol::SQUARE_OPEN:
            return U"[";
        case Symbol::SQUARE_CLOSE:
            return U"]";

        default:
            return U"unknown";
    }
}
