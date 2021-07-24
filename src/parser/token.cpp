/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/token.hpp>


using namespace kh;

Token::Token() : begin(0), end(0), type(), value(), column(0), line(0) {}

Token::Token(size_t _begin, size_t _end, TokenType _type, const TokenValue& _value)
    : begin(_begin), end(_end), type(_type), value(_value), column(0), line(0) {}

std::string kh::strfy(const Token& token, bool show_token_type) {
    std::string str;
    if (show_token_type) {
        str = strfy(token.type) + ' ';
    }

    switch (token.type) {
        case TokenType::IDENTIFIER:
            str += token.value.identifier;
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
            str += strfy(token.value.imaginary) + 'i';
            break;

        default:
            str = "unknown";
    }

    return str;
}

std::string kh::strfy(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER:
            return "identifier";
        case TokenType::OPERATOR:
            return "operator";
        case TokenType::SYMBOL:
            return "symbol";

        case TokenType::CHARACTER:
            return "character";
        case TokenType::STRING:
            return "string";
        case TokenType::BUFFER:
            return "buffer";

        case TokenType::UINTEGER:
            return "uinteger";
        case TokenType::INTEGER:
            return "integer";
        case TokenType::FLOATING:
            return "floating";
        case TokenType::IMAGINARY:
            return "imaginary";

        default:
            return "unknown";
    }
}

std::string kh::strfy(Operator op) {
    switch (op) {
        case Operator::ADD:
            return "+";
        case Operator::SUB:
            return "-";
        case Operator::MUL:
            return "*";
        case Operator::DIV:
            return "/";
        case Operator::MOD:
            return "%";
        case Operator::POW:
            return "^";

        case Operator::IADD:
            return "+=";
        case Operator::ISUB:
            return "-=";
        case Operator::IMUL:
            return "*=";
        case Operator::IDIV:
            return "/=";
        case Operator::IMOD:
            return "%=";
        case Operator::IPOW:
            return "^=";

        case Operator::INCREMENT:
            return "++";
        case Operator::DECREMENT:
            return "--";

        case Operator::EQUAL:
            return "==";
        case Operator::NOT_EQUAL:
            return "!=";
        case Operator::LESS:
            return "<";
        case Operator::MORE:
            return ">";
        case Operator::LESS_EQUAL:
            return "<=";
        case Operator::MORE_EQUAL:
            return ">=";

        case Operator::BIT_AND:
            return "&";
        case Operator::BIT_OR:
            return "|";
        case Operator::BIT_NOT:
            return "~";

        case Operator::BIT_LSHIFT:
            return "<<";
        case Operator::BIT_RSHIFT:
            return ">>";

        case Operator::AND:
            return "and";
        case Operator::OR:
            return "or";
        case Operator::NOT:
            return "not";

        case Operator::ASSIGN:
            return "=";
        case Operator::SIZEOF:
            return "#";
        case Operator::ADDRESS:
            return "@";

        default:
            return "unknown";
    }
}

std::string kh::strfy(Symbol sym) {
    switch (sym) {
        case Symbol::SEMICOLON:
            return ";";
        case Symbol::DOT:
            return ".";
        case Symbol::COMMA:
            return ",";
        case Symbol::COLON:
            return ":";

        case Symbol::PARENTHESES_OPEN:
            return "(";
        case Symbol::PARENTHESES_CLOSE:
            return ")";

        case Symbol::CURLY_OPEN:
            return "{";
        case Symbol::CURLY_CLOSE:
            return "}";

        case Symbol::SQUARE_OPEN:
            return "[";
        case Symbol::SQUARE_CLOSE:
            return "]";

        default:
            return "unknown";
    }
}
