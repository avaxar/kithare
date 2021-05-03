/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/parser/parser.hpp
 * Declares the kh::Parser class and its recursive parse methods.
 */

#pragma once

#include "parser/ast.hpp"


namespace kh {
    struct ParseException {
        ParseException(const std::u32string _what, const size_t _index) : what(_what), index(_index) {}

        std::u32string what;
        size_t index;
    };

    struct ParseExceptions {
        ParseExceptions(const std::vector<kh::ParseException>& _exceptions) : exceptions(_exceptions) {}

        std::vector<kh::ParseException> exceptions;
    };

    /// <summary>
    /// Parses the given tokens and returns an AST module tree.
    /// </summary>
    /// <param name="tokens">Tokens to be parsed</param>
    /// <returns></returns>
    kh::Ast* parse(const std::vector<kh::Token>& tokens);

    class Parser {
    public:
        std::vector<kh::ParseException> exceptions;

        Parser(const std::vector<kh::Token>& _tokens);
        ~Parser();

        /// <summary>
        /// Parses the provided tokens and returns an AST module tree.
        /// </summary>
        /// <returns></returns>
        kh::Ast* parse();

    private:
        std::vector<kh::Token> tokens;
        size_t ti = 0; /* Token iterator */

        /* Get token of the current iterator index */
        inline kh::Token& to(const size_t offset = 0) {
            return this->tokens[this->ti + offset];
        }

        kh::AstImport* parseImport(const bool is_include);
        void parseAccessAttribs(bool& is_static, bool& is_public);
        kh::AstFunctionExpression* parseFunction(const bool is_static, const bool is_public);
        kh::AstDeclarationExpression* parseDeclaration(const bool is_static, const bool is_public);

        kh::AstClass* parseClass();
        kh::AstStruct* parseStruct();
        kh::AstEnum* parseEnum();

        std::vector<std::shared_ptr<kh::AstBody>> parseBody(const bool break_continue_allowed = false);

        /* These parse expressions below are ordered based from their precedence from lowest to
         * highest */

        kh::AstExpression* parseExpression(); /* Parses an expression */

        kh::AstExpression* parseAssignOps();      /* Parses assignment operations and in-place
                                                   * operations `=`, `+=`, `-=`, `*=`, ... */
        kh::AstExpression* parseTernary();        /* Parses ternary expressions */
        kh::AstExpression* parseOr();             /* Parses logical OR `||` */
        kh::AstExpression* parseAnd();            /* Parses logical AND `&&` */
        kh::AstExpression* parseComparison();     /* Parses `==` `!=` `<` `>` `<=` `>=` */
        kh::AstExpression* parseBitwiseOr();      /* Parses bitwise OR `|` */
        kh::AstExpression* parseBitwiseAnd();     /* Parses bitwise AND `&` */
        kh::AstExpression* parseBitwiseShift();   /* Parses bitwise shift `<<` `>>` */
        kh::AstExpression* parseAddSub();         /* Parses `+` and `-` */
        kh::AstExpression* parseMulDivMod();      /* Parses `*`, `/`, and `%` */
        kh::AstExpression* parseUnary();          /* Parses unary operations */
        kh::AstExpression* parseExponentiation(); /* Parses `^` */
        kh::AstExpression* parseLiteral();        /* Parses literals */
        kh::AstExpression* parseIdentifiers();    /* Parses identifiers with scoping and
                                                   * templates */
        kh::AstExpression* parseTuple(            /* Parses tuples with parentheses expressions */
                                      const kh::Symbol opening = kh::Symbol::PARENTHESES_OPEN,
                                      const kh::Symbol closing = kh::Symbol::PARENTHESES_CLOSE,
                                      const bool can_contain_one_element = true);

        std::vector<uint64_t>
        parseArrayDimensionList(std::shared_ptr<kh::AstIdentifierExpression>& type);
    };
}
