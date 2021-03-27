/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/parser/parser.hpp
* Declares the kh::Parser class and its recursive parse methods.
*/

#pragma once

#include <vector>

#include "parser/token.hpp"
#include "parser/ast.hpp"


namespace kh {
    struct ParseException {
        ParseException(const std::u32string _what, const size_t _index) :
            what(_what), index(_index) {}

        std::u32string what;
        size_t index;
    };

    /// <summary>
    /// Parses the given tokens and returns an AST module tree.
    /// </summary>
    /// <param name="tokens">Tokens to be parsed</param>
    /// <returns></returns>
    kh::AstModule* parse(const std::vector<kh::Token>& tokens);

    class Parser {
    public:
        Parser(const std::vector<kh::Token>& _tokens);
        ~Parser();

        /// <summary>
        /// Parses the provided tokens and returns an AST module tree.
        /// </summary>
        /// <returns></returns>
        kh::AstModule* parse();
    private:
        std::vector<kh::Token> tokens;
        size_t i = 0;

        kh::AstImport* parseImport();
        kh::AstFunction* parseFunction();
        kh::AstFunction* parseFunction(const bool is_static, const bool is_public);
        kh::AstDeclarationExpression* parseDeclaration();
        kh::AstDeclarationExpression* parseDeclaration(const bool is_static, const bool is_public);

        kh::AstClass* parseClass();
        kh::AstStruct* parseStruct();
        kh::AstEnum* parseEnum();

        /* These parse expressions below are ordered based from their precedence from lowest to highest */

        kh::AstExpression* parseExpression();     /* Parses an expression */

        kh::AstExpression* parseAssignOps();      /* Parses assignment operations and in-place operations `=`, `+=`, `-=`, `*=`, ... */
        kh::AstExpression* parseTernary();        /* Parses ternary expressions */
        kh::AstExpression* parseOr();             /* Parses logical OR `||` */
        kh::AstExpression* parseAnd();            /* Parses logical AND `&&` */
        kh::AstExpression* parseComparison();     /* Parses comparison `==` `!=` `<` `>` `<=` `>=` */
        kh::AstExpression* parseBitwiseOr();      /* Parses bitwise OR `|` */
        kh::AstExpression* parseBitwiseAnd();     /* Parses bitwise AND `&` */
        kh::AstExpression* parseBitwiseShift();   /* Parses bitwise shift `<<` `>>` */
        kh::AstExpression* parseAddSub();         /* Parses `+` and `-` */
        kh::AstExpression* parseMulDivMod();      /* Parses `*`, `/`, and `%` */
        kh::AstExpression* parseExponentiation(); /* Parses `^` */
        kh::AstExpression* parseUnLiteral();      /* Parses literals and unary operations */
        kh::AstExpression* parseIdentifiers();    /* Parses identifiers with scoping and templates */
        kh::AstExpression* parseTuple();          /* Parses tuples with parentheses expressions */
    };
}
