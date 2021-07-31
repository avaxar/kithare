/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <kithare/ast.hpp>

#define KH_PARSE_GUARD()                                                                  \
    do {                                                                                  \
        if (this->ti >= this->tokens.size()) {                                            \
            this->exceptions.emplace_back("expected a token but reached the end of file", \
                                          this->tokens.back());                           \
            goto end;                                                                     \
        }                                                                                 \
    } while (false)


namespace kh {
    class ParseException : public Exception {
    public:
        std::string what;
        Token token;

        ParseException(const std::string& _what, const Token& _token) : what(_what), token(_token) {}
        virtual ~ParseException() {}
        virtual std::string format() const;
    };

    struct Parser {
        const std::vector<Token>& tokens;
        std::vector<ParseException>& exceptions;

        /* Token iterator */
        size_t ti = 0;

        /* Gets token of the current iterator index */
        inline Token& tok() const {
            return *(Token*)(size_t) & this->tokens[this->ti];
        }

        /* Most of these parses stuff such as imports, includes, classes, structs, enums, functions at
         * the top level scope */
        AstModule parseWhole();
        void parseAccessAttribs(bool& is_public, bool& is_static);
        AstImport parseImport(bool is_include);
        AstFunction parseFunction(bool is_conditional);
        AstDeclaration parseDeclaration();
        AstUserType parseUserType(bool is_class);
        AstEnumType parseEnum();
        std::vector<std::shared_ptr<AstBody>> parseBody(size_t loop_count = 0);
        void parseTopScopeIdentifiersAndGenericArgs(std::vector<std::string>& identifiers,
                                                    std::vector<std::string>& generic_args);

        /* These parse expressions below are ordered based from their precedence from lowest to
         * highest */
        AstExpression* parseExpression();
        AstExpression* parseAssignOps();
        AstExpression* parseTernary();
        AstExpression* parseOr();
        AstExpression* parseAnd();
        AstExpression* parseNot();
        AstExpression* parseComparison();
        AstExpression* parseBitwiseOr();
        AstExpression* parseBitwiseAnd();
        AstExpression* parseBitwiseShift();
        AstExpression* parseAddSub();
        AstExpression* parseMulDivMod();
        AstExpression* parseUnary();
        AstExpression* parseExponentiation();
        AstExpression* parseRevUnary();
        AstExpression* parseOthers();
        AstIdentifiers parseIdentifiers();
        AstExpression* parseTuple(Symbol opening = Symbol::PARENTHESES_OPEN,
                                  Symbol closing = Symbol::PARENTHESES_CLOSE,
                                  bool explicit_tuple = true);
        AstExpression* parseList();
        AstExpression* parseDict();
        std::vector<uint64_t> parseArrayDimension(AstIdentifiers& type);
    };

    AstModule parse(const std::vector<Token>& tokens);
    AstExpression* parseExpression(const std::vector<Token>& tokens);

    inline bool isReservedKeyword(const std::string& identifier) {
        return identifier == "public" || identifier == "private" || identifier == "static" ||
               identifier == "try" || identifier == "def" || identifier == "class" ||
               identifier == "struct" || identifier == "enum" || identifier == "import" ||
               identifier == "include" || identifier == "if" || identifier == "elif" ||
               identifier == "else" || identifier == "for" || identifier == "while" ||
               identifier == "do" || identifier == "break" || identifier == "continue" ||
               identifier == "return" || identifier == "ref";
    }
}
