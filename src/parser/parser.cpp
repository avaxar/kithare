/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/parser/parser.cpp
* Defines include/parser/parser.hpp.
*/

#include "parser/parser.hpp"


kh::AstModule kh::parse(const std::vector<kh::Token>& tokens) {
    kh::Parser parser(tokens);
    return parser.parse();
}

kh::Parser::Parser(const std::vector<kh::Token>& tokens) {

}

kh::Parser::~Parser() {

}

kh::AstModule kh::Parser::parse() {
    return *(kh::AstModule*)nullptr;
}
