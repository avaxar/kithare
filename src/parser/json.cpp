/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * src/parser/json.hpp
 * Defines include/parser/json.hpp.
 */

#include "parser/json.hpp"


std::u32string kh::json(const std::vector<kh::Token>& tokens) {
    std::u32string json = U"[";

    for (const kh::Token& token : tokens) {
        json += U"[";
        json += kh::quote(kh::repr(token.type)) + U',';

        switch (token.type) {
            case kh::TokenType::IDENTIFIER:
                json += kh::quote(token.value.identifier);
                break;
            case kh::TokenType::OPERATOR:
                json += kh::quote(kh::repr(token.value.operator_type));
                break;
            case kh::TokenType::SYMBOL:
                json += kh::quote(kh::repr(token.value.symbol_type));
                break;

            case kh::TokenType::CHARACTER:
                json += kh::quote(std::u32string() + token.value.character);
                break;
            case kh::TokenType::STRING:
                json += kh::quote(token.value.string);
                break;
            case kh::TokenType::BUFFER:
                json += kh::quote(token.value.buffer);
                break;

            case kh::TokenType::UINTEGER:
                json += kh::repr(token.value.uinteger);
                break;
            case kh::TokenType::INTEGER:
                json += kh::repr(token.value.integer);
                break;
            case kh::TokenType::FLOATING:
                json += kh::repr(token.value.floating);
                break;
            case kh::TokenType::IMAGINARY:
                json += kh::repr(token.value.imaginary);
                break;

            case kh::TokenType::COMMENT:
                json += U"69";
                break;

            default:
                json = U"unknown";
        }

        json += U',';
        json += kh::repr((uint64_t)token.index) + U',';
        json += kh::repr((uint64_t)token.length);
        json += U"],";
    }

    if (tokens.size())
        json.pop_back();

    json += U']';
    return json;
}
