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

#define KH_PARSE_CTX kh::ParserContext& context


namespace kh {
    using namespace std;

    class ParseException : public kh::Exception {
    public:
        string what;
        kh::Token token;

        ParseException(const string _what, const kh::Token& _token) : what(_what), token(_token) {}
        virtual ~ParseException() {}
        virtual string format() const;
    };

    struct ParserContext {
        const vector<kh::Token>& tokens;
        vector<kh::ParseException>& exceptions;

        /* Token iterator */
        size_t ti = 0;

        /* Gets token of the current iterator index */
        inline kh::Token& tok() const {
            return *(kh::Token*)(size_t) & this->tokens[this->ti];
        }
    };

    inline bool isReservedKeyword(const string& identifier) {
        return identifier == "public" || identifier == "private" || identifier == "static" ||
               identifier == "try" || identifier == "def" || identifier == "class" ||
               identifier == "struct" || identifier == "enum" || identifier == "import" ||
               identifier == "include" || identifier == "if" || identifier == "elif" ||
               identifier == "else" || identifier == "for" || identifier == "while" ||
               identifier == "do" || identifier == "break" || identifier == "continue" ||
               identifier == "return" || identifier == "ref";
    }

    kh::AstModule parse(const vector<kh::Token>& tokens);
    kh::AstExpression* parseExpression(const vector<kh::Token>& tokens);

    /* Most of these parses stuff such as imports, includes, classes, structs, enums, functions at the
     * top level scope */
    kh::AstModule parseWhole(KH_PARSE_CTX);
    void parseAccessAttribs(KH_PARSE_CTX, bool& is_public, bool& is_static);
    kh::AstImport parseImport(KH_PARSE_CTX, bool is_include);
    kh::AstFunction parseFunction(KH_PARSE_CTX, bool is_conditional);
    kh::AstDeclaration parseDeclaration(KH_PARSE_CTX);
    kh::AstUserType parseUserType(KH_PARSE_CTX, bool is_class);
    kh::AstEnumType parseEnum(KH_PARSE_CTX);
    vector<shared_ptr<kh::AstBody>> parseBody(KH_PARSE_CTX, size_t loop_count = 0);
    void parseTopScopeIdentifiersAndGenericArgs(KH_PARSE_CTX, vector<string>& identifiers,
                                                vector<string>& generic_args);

    /* These parse expressions below are ordered based from their precedence from lowest to
     * highest */
    kh::AstExpression* parseExpression(KH_PARSE_CTX);
    kh::AstExpression* parseAssignOps(KH_PARSE_CTX);
    kh::AstExpression* parseTernary(KH_PARSE_CTX);
    kh::AstExpression* parseOr(KH_PARSE_CTX);
    kh::AstExpression* parseAnd(KH_PARSE_CTX);
    kh::AstExpression* parseNot(KH_PARSE_CTX);
    kh::AstExpression* parseComparison(KH_PARSE_CTX);
    kh::AstExpression* parseBitwiseOr(KH_PARSE_CTX);
    kh::AstExpression* parseBitwiseAnd(KH_PARSE_CTX);
    kh::AstExpression* parseBitwiseShift(KH_PARSE_CTX);
    kh::AstExpression* parseAddSub(KH_PARSE_CTX);
    kh::AstExpression* parseMulDivMod(KH_PARSE_CTX);
    kh::AstExpression* parseUnary(KH_PARSE_CTX);
    kh::AstExpression* parseExponentiation(KH_PARSE_CTX);
    kh::AstExpression* parseRevUnary(KH_PARSE_CTX);
    kh::AstExpression* parseOthers(KH_PARSE_CTX);
    kh::AstIdentifiers parseIdentifiers(KH_PARSE_CTX);
    kh::AstExpression* parseTuple(KH_PARSE_CTX, kh::Symbol opening = kh::Symbol::PARENTHESES_OPEN,
                                  kh::Symbol closing = kh::Symbol::PARENTHESES_CLOSE,
                                  bool explicit_tuple = true);
    kh::AstExpression* parseList(KH_PARSE_CTX);
    kh::AstExpression* parseDict(KH_PARSE_CTX);
    vector<uint64_t> parseArrayDimension(KH_PARSE_CTX, kh::AstIdentifiers& type);
}
