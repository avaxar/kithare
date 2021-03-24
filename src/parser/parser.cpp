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

kh::Parser::Parser(const std::vector<kh::Token>& _tokens) {
    this->tokens = _tokens;
}

kh::Parser::~Parser() {

}

kh::AstModule kh::Parser::parse() {
    std::vector<kh::AstImport*> imports;
    std::vector<kh::AstFunction*> functions;
    std::vector<kh::AstClass*> classes;
    std::vector<kh::AstStruct*> structs;
    std::vector<kh::AstEnum*> enums;
    std::vector<kh::AstDeclarationExpression*> variables;

    for (this->i = 0; this->i < this->tokens.size(); this->i++) {

    }

    return kh::AstModule{ imports, functions, classes, structs, enums, variables };
}

kh::AstImport* kh::Parser::parseImport() {
    this->i++; /* Consume `import` */
}

kh::AstFunction* kh::Parser::parseFunction() {
    this->i++; /* Consume `def` */
}

kh::AstDeclarationExpression* kh::Parser::parseDeclaration() {
    
}

kh::AstClass* kh::Parser::parseClass() {
    this->i++; /* Consume `class` */
}

kh::AstStruct* kh::Parser::parseStruct() {
    this->i++; /* Consume `struct` */
}

kh::AstEnum* kh::Parser::parseEnum() {
    this->i++; /* Consume `enum` */
}
