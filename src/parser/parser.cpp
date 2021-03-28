/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/parser/parser.cpp
* Defines include/parser/parser.hpp.
*/

#include "parser/parser.hpp"


kh::Ast* kh::parse(const std::vector<kh::Token>& tokens) {
    kh::Parser parser(tokens);
    kh::Ast* ast = parser.parse();

    if (parser.exceptions.empty())
        return ast;
    else
        throw kh::ParseExceptions(parser.exceptions);

}

kh::Parser::Parser(const std::vector<kh::Token>& _tokens) {
    this->tokens = _tokens;
}

kh::Parser::~Parser() {

}

kh::Ast* kh::Parser::parse() {
    this->exceptions.clear();

    std::vector<kh::AstImport*> imports;
    std::vector<kh::AstFunction*> functions;
    std::vector<kh::AstClass*> classes;
    std::vector<kh::AstStruct*> structs;
    std::vector<kh::AstEnum*> enums;
    std::vector<kh::AstDeclarationExpression*> variables;

    for (this->ti = 0; this->ti < this->tokens.size(); this->ti++) {

    }

    return new kh::Ast(imports, functions, classes, structs, enums, variables);
}

/* // This part of the code is commented as these methods haven't been implemented plus so that the CI doesn't fail, hopefully.

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

kh::AstExpression* kh::Parser::parseExpression() {

}

kh::AstExpression* kh::Parser::parseAssignOps() {

}

kh::AstExpression* kh::Parser::parseTernary() {

}

kh::AstExpression* kh::Parser::parseOr() {

}

kh::AstExpression* kh::Parser::parseAnd() {

}

kh::AstExpression* kh::Parser::parseComparison() {

}

kh::AstExpression* kh::Parser::parseBitwiseOr() {

}

kh::AstExpression* kh::Parser::parseBitwiseAnd() {

}

kh::AstExpression* kh::Parser::parseBitwiseShift() {

}

kh::AstExpression* kh::Parser::parseAddSub() {

}

kh::AstExpression* kh::Parser::parseMulDivMod() {

}

kh::AstExpression* kh::Parser::parseExponentiation() {

}

kh::AstExpression* kh::Parser::parseUnLiteral() {

}

kh::AstExpression* kh::Parser::parseIdentifiers() {

}

kh::AstExpression* kh::Parser::parseTuple() {

}

*/
