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
    kh::AstModule parse(const std::vector<kh::Token>& tokens);

    class Parser {
    public:
        Parser(const std::vector<kh::Token>& _tokens);
        ~Parser();

        /// <summary>
        /// Parses the provided tokens and returns an AST module tree.
        /// </summary>
        /// <returns></returns>
        kh::AstModule parse();
    private:
        std::vector<kh::Token> tokens;
        size_t i = 0;

        kh::AstImport* parseImport();
        kh::AstFunction* parseFunction();
        kh::AstDeclarationExpression* parseDeclaration();

        kh::AstClass* parseClass();
        kh::AstStruct* parseStruct();
        kh::AstEnum* parseEnum();
    };
}
