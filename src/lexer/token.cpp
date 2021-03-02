/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/lexer/token.cpp
* Defines include/lexer/token.hpp
*/

#include "lexer/token.hpp"


kh::String kh::repr(const kh::Token& token) {
    kh::String str;

    std::function<void()> loc = [&]() { str += kh::toString(L" at ") + kh::toString(std::to_wstring(token.line)) + kh::toString(L", ") + kh::toString(std::to_wstring(token.character)); };

    switch (token.type) {
    case kh::TokenType::IDENTIFIER:
        str = kh::toString(L"IDENTIFIER \"") + token.value.identifier_name + kh::toString(L"\"");
        break;

    case kh::TokenType::OPERATOR:
        str = kh::toString(L"OPERATOR '");

        switch (token.value.operator_type) {
        case kh::Operator::ADD:
            str += kh::toString(L"+"); break;
        case kh::Operator::SUB:
            str += kh::toString(L"-"); break;
        case kh::Operator::MUL:
            str += kh::toString(L"*"); break;
        case kh::Operator::DIV:
            str += kh::toString(L"/"); break;
        case kh::Operator::MOD:
            str += kh::toString(L"%"); break;
        case kh::Operator::POW:
            str += kh::toString(L"^"); break;

        case kh::Operator::IADD:
            str += kh::toString(L"+="); break;
        case kh::Operator::ISUB:
            str += kh::toString(L"-="); break;
        case kh::Operator::IMUL:
            str += kh::toString(L"*="); break;
        case kh::Operator::IDIV:
            str += kh::toString(L"/="); break;
        case kh::Operator::IMOD:
            str += kh::toString(L"%="); break;
        case kh::Operator::IPOW:
            str += kh::toString(L"^="); break;

        case kh::Operator::EQUAL:
            str += kh::toString(L"=="); break;
        case kh::Operator::NOT_EQUAL:
            str += kh::toString(L"!="); break;
        case kh::Operator::LESS:
            str += kh::toString(L"<"); break;
        case kh::Operator::MORE:
            str += kh::toString(L">"); break;
        case kh::Operator::LESS_EQUAL:
            str += kh::toString(L"<="); break;
        case kh::Operator::MORE_EQUAL:
            str += kh::toString(L">="); break;

        case kh::Operator::BIT_AND:
            str += kh::toString(L"&"); break;
        case kh::Operator::BIT_OR:
            str += kh::toString(L"|"); break;
        case kh::Operator::BIT_NOT:
            str += kh::toString(L"~"); break;

        case kh::Operator::BIT_LSHIFT:
            str += kh::toString(L"<<"); break;
        case kh::Operator::BIT_RSHIFT:
            str += kh::toString(L">>"); break;

        case kh::Operator::AND:
            str += kh::toString(L"&&"); break;
        case kh::Operator::OR:
            str += kh::toString(L"||"); break;
        case kh::Operator::NOT:
            str += kh::toString(L"!"); break;

        case kh::Operator::ASSIGN:
            str += kh::toString(L"="); break;
        case kh::Operator::SIZEOF:
            str += kh::toString(L"#"); break;
        case kh::Operator::ADDRESS:
            str += kh::toString(L"@"); break;

        default:
            str += kh::toString(L"UNKNOWN");
        }

        str += kh::toString(L"'");
        break;

    case kh::TokenType::SYMBOL:
        str = kh::toString(L"SYMBOL '");

        switch (token.value.symbol_type) {
        case kh::Symbol::SEMICOLON:
            str += kh::toString(L";"); break;
        case kh::Symbol::DOT:
            str += kh::toString(L"."); break;
        case kh::Symbol::COMMA:
            str += kh::toString(L","); break;

        case kh::Symbol::QUESTION:
            str += kh::toString(L"?"); break;
        case kh::Symbol::COLON:
            str += kh::toString(L":"); break;
        case kh::Symbol::DOLLAR:
            str += kh::toString(L"$"); break;

        case kh::Symbol::PARENTHESES_OPEN:
            str += kh::toString(L"("); break;
        case kh::Symbol::PARENTHESES_CLOSE:
            str += kh::toString(L")"); break;

        case kh::Symbol::CURLY_OPEN:
            str += kh::toString(L"{"); break;
        case kh::Symbol::CURLY_CLOSE:
            str += kh::toString(L"}"); break;

        case kh::Symbol::SQUARE_OPEN:
            str += kh::toString(L"["); break;
        case kh::Symbol::SQUARE_CLOSE:
            str += kh::toString(L"]"); break;

        case kh::Symbol::TEMPLATE_OPEN:
            str += kh::toString(L"[<"); break;
        case kh::Symbol::TEMPLATE_CLOSE:
            str += kh::toString(L">]"); break;
        }

        str += kh::toString(L"'");
        break;

    case kh::TokenType::CHARACTER:
        str = kh::toString(L"CHARACTER '") + token.value.character + kh::toString(L"'");
        break;

    case kh::TokenType::STRING:
        str = kh::toString(L"STRING \"") + token.value.string + kh::toString(L"\"");
        break;

    case kh::TokenType::BUFFER:
        str = kh::toString(L"BUFFER ");
        for (const uint8 byte : token.value.buffer)
            str += kh::toString(std::to_wstring((int)byte)) + kh::toString(L", ");
        break;

    case kh::TokenType::UNSIGNED_INTEGER:
        str = kh::toString(L"UNSIGNED_INTEGER ") + kh::toString(std::to_wstring(token.value.unsigned_integer));
        break;

    case kh::TokenType::INTEGER:
        str = kh::toString(L"INTEGER ") + kh::toString(std::to_wstring(token.value.integer));
        break;

    case kh::TokenType::FLOATING:
        str = kh::toString(L"FLOATING ") + kh::toString(std::to_wstring(token.value.floating));
        break;

    case kh::TokenType::IMAGINARY:
        str = kh::toString(L"IMAGINARY ") + kh::toString(std::to_wstring(token.value.imaginary));
        break;

    default:
        str = kh::toString(L"UNKNOWN");
    }

    loc();

    return str;
}
