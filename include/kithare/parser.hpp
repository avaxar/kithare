/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <kithare/ast.hpp>
#include <kithare/exception.hpp>
#include <kithare/string.hpp>
#include <kithare/token.hpp>

#define KH_PARSE_GUARD()                                                                    \
    do {                                                                                    \
        if (context.ti >= context.tokens.size()) {                                          \
            context.exceptions.emplace_back("expected a token but reached the end of file", \
                                            context.tokens.back());                         \
            goto end;                                                                       \
        }                                                                                   \
    } while (false)

#define KH_PARSE_CTX ParserContext& context


namespace kh {
    class ParseException : public Exception {
    public:
        std::string what;
        Token token;

        ParseException(const std::string _what, const Token& _token) : what(_what), token(_token) {}
        virtual ~ParseException() {}
        virtual std::string format() const;
    };

    struct ParserContext {
        const std::vector<Token>& tokens;
        std::vector<ParseException>& exceptions;

        /* Token iterator */
        size_t ti = 0;

        /* Gets token of the current iterator index */
        inline Token& tok() const {
            return *(Token*)(size_t) & this->tokens[this->ti];
        }
    };

    inline bool isReservedKeyword(const std::string& identifier) {
        return identifier == "public" || identifier == "private" || identifier == "static" ||
               identifier == "try" || identifier == "def" || identifier == "class" ||
               identifier == "struct" || identifier == "enum" || identifier == "import" ||
               identifier == "include" || identifier == "if" || identifier == "elif" ||
               identifier == "else" || identifier == "for" || identifier == "while" ||
               identifier == "do" || identifier == "break" || identifier == "continue" ||
               identifier == "return" || identifier == "ref";
    }

    AstModule parse(const std::vector<Token>& tokens);
    AstExpression* parseExpression(const std::vector<Token>& tokens);

    /* Most of these parses stuff such as imports, includes, classes, structs, enums, functions at the
     * top level scope */
    AstModule parseWhole(KH_PARSE_CTX);
    void parseAccessAttribs(KH_PARSE_CTX, bool& is_public, bool& is_static);
    AstImport parseImport(KH_PARSE_CTX, bool is_include);
    AstFunction parseFunction(KH_PARSE_CTX, bool is_conditional);
    AstDeclaration parseDeclaration(KH_PARSE_CTX);
    AstUserType parseUserType(KH_PARSE_CTX, bool is_class);
    AstEnumType parseEnum(KH_PARSE_CTX);
    std::vector<std::shared_ptr<AstBody>> parseBody(KH_PARSE_CTX, size_t loop_count = 0);
    void parseTopScopeIdentifiersAndGenericArgs(KH_PARSE_CTX, std::vector<std::string>& identifiers,
                                                std::vector<std::string>& generic_args);

    /* These parse expressions below are ordered based from their precedence from lowest to
     * highest */
    AstExpression* parseExpression(KH_PARSE_CTX);
    AstExpression* parseAssignOps(KH_PARSE_CTX);
    AstExpression* parseTernary(KH_PARSE_CTX);
    AstExpression* parseOr(KH_PARSE_CTX);
    AstExpression* parseAnd(KH_PARSE_CTX);
    AstExpression* parseNot(KH_PARSE_CTX);
    AstExpression* parseComparison(KH_PARSE_CTX);
    AstExpression* parseBitwiseOr(KH_PARSE_CTX);
    AstExpression* parseBitwiseAnd(KH_PARSE_CTX);
    AstExpression* parseBitwiseShift(KH_PARSE_CTX);
    AstExpression* parseAddSub(KH_PARSE_CTX);
    AstExpression* parseMulDivMod(KH_PARSE_CTX);
    AstExpression* parseUnary(KH_PARSE_CTX);
    AstExpression* parseExponentiation(KH_PARSE_CTX);
    AstExpression* parseRevUnary(KH_PARSE_CTX);
    AstExpression* parseOthers(KH_PARSE_CTX);
    AstIdentifiers parseIdentifiers(KH_PARSE_CTX);
    AstExpression* parseTuple(KH_PARSE_CTX, Symbol opening = Symbol::PARENTHESES_OPEN,
                              Symbol closing = Symbol::PARENTHESES_CLOSE, bool explicit_tuple = true);
    AstExpression* parseList(KH_PARSE_CTX);
    AstExpression* parseDict(KH_PARSE_CTX);
    std::vector<uint64_t> parseArrayDimension(KH_PARSE_CTX, AstIdentifiers& type);
}
