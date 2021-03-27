/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/parser/parser.cpp
* Defines include/parser/parser.hpp.
*/

#include "parser/parser.hpp"


kh::AstModule* kh::parse(const std::vector<kh::Token>& tokens) {
    kh::Parser parser(tokens);
    return parser.parse();
}

kh::Parser::Parser(const std::vector<kh::Token>& _tokens) {
    this->tokens = _tokens;
}

kh::Parser::~Parser() {

}

kh::AstModule* kh::Parser::parse() {
    std::vector<kh::AstImport*> imports;
    std::vector<kh::AstFunction*> functions;
    std::vector<kh::AstClass*> classes;
    std::vector<kh::AstStruct*> structs;
    std::vector<kh::AstEnum*> enums;
    std::vector<kh::AstDeclarationExpression*> variables;

    for (this->i = 0; this->i < this->tokens.size(); this->i++) {

    }

    return new kh::AstModule(imports, functions, classes, structs, enums, variables);
}

kh::AstImport* kh::Parser::parseImport() {
    
}

kh::AstFunction* kh::Parser::parseFunction() {

}

kh::AstFunction* kh::Parser::parseFunction(const bool is_static, const bool is_public) {

}

kh::AstDeclarationExpression* kh::Parser::parseDeclaration() {

}

kh::AstDeclarationExpression* kh::Parser::parseDeclaration(const bool is_static, const bool is_private) {

}

kh::AstClass* kh::Parser::parseClass() {

}

kh::AstStruct* kh::Parser::parseStruct() {

}

kh::AstEnum* kh::Parser::parseEnum() {

}
