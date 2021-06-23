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

#define KH_PARSE_GUARD()                                                          \
    do {                                                                          \
        if (context.ti >= context.tokens.size()) {                                \
            context.exceptions.emplace_back(U"Was expecting a token but hit EOF", \
                                            context.tokens.back());               \
            goto end;                                                             \
        }                                                                         \
    } while (false)

#define KH_PARSE_CTX kh::ParserContext& context


namespace kh {
    class ParseException : public kh::Exception {
    public:
        std::u32string what;
        kh::Token token;

        ParseException(const std::u32string _what, const kh::Token& _token)
            : what(_what), token(_token) {}
        virtual ~ParseException() {}
        virtual std::u32string format() const;
    };

    struct ParserContext {
        const std::vector<kh::Token>& tokens;
        std::vector<kh::ParseException>& exceptions;

        /* Token iterator */
        size_t ti = 0;

        /* Gets token of the current iterator index */
        inline kh::Token& tok() const {
            return *(kh::Token*)(size_t) & this->tokens[this->ti];
        }

        inline kh::Token& tokFromIndex(const size_t index) const {
            for (const kh::Token& token : this->tokens)
                if (token.index == index)
                    return *(kh::Token*)(size_t)&token;

            /* placeholder */
            return *(kh::Token*)nullptr;
        }
    };

    inline bool isReservedKeyword(const std::u32string& identifier) {
        return identifier == U"public" || identifier == U"private" || identifier == U"static" ||
               identifier == U"try" || identifier == U"def" || identifier == U"class" ||
               identifier == U"struct" || identifier == U"enum" || identifier == U"import" ||
               identifier == U"include" || identifier == U"if" || identifier == U"for" ||
               identifier == U"while" || identifier == U"do" || identifier == U"break" ||
               identifier == U"continue" || identifier == U"return" || identifier == U"ref";
    }

    kh::Ast parse(const std::vector<kh::Token>& tokens);
    kh::AstExpression* parseExpression(const std::vector<kh::Token>& tokens);

    /* Most of these parses stuff such as imports, includes, classes, structs, enums, functions at the
     * top level scope */
    kh::Ast parseWhole(KH_PARSE_CTX);
    kh::AstImport parseImport(KH_PARSE_CTX, bool is_include);
    void parseAccessAttribs(KH_PARSE_CTX, bool& is_static, bool& is_public);
    kh::AstFunction parseFunction(KH_PARSE_CTX, bool is_static, bool is_public, bool is_conditional);
    kh::AstDeclaration parseDeclaration(KH_PARSE_CTX, bool is_static, bool is_public);
    kh::AstUserType parseUserType(KH_PARSE_CTX, bool is_class);
    kh::AstEnumType parseEnum(KH_PARSE_CTX);
    std::vector<std::shared_ptr<kh::AstBody>> parseBody(KH_PARSE_CTX, size_t loop_count = 0);
    void parseTopScopeIdentifiersAndGenericArgs(KH_PARSE_CTX, std::vector<std::u32string>& identifiers,
                                                std::vector<std::u32string>& generic_args);

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
    kh::AstExpression* parseLiteral(KH_PARSE_CTX);
    kh::AstIdentifiers parseIdentifiers(KH_PARSE_CTX);
    kh::AstExpression* parseTuple(KH_PARSE_CTX, kh::Symbol opening = kh::Symbol::PARENTHESES_OPEN,
                                  kh::Symbol closing = kh::Symbol::PARENTHESES_CLOSE,
                                  bool explicit_tuple = true);
    kh::AstExpression* parseList(KH_PARSE_CTX);
    kh::AstExpression* parseDict(KH_PARSE_CTX);
    std::vector<uint64_t> parseArrayDimension(KH_PARSE_CTX, kh::AstIdentifiers& type);
}
