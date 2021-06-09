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


namespace kh {
    class LexException : public kh::Exception {
    public:
        std::u32string what;
        size_t column;
        size_t line;
        size_t index;

        LexException(const std::u32string _what, const size_t _index) : what(_what), index(_index) {}
        virtual ~LexException() {}
        virtual std::u32string format() const;
    };

    class ParseException : public kh::Exception {
    public:
        std::u32string what;
        kh::Token token;

        ParseException(const std::u32string _what, const kh::Token& _token)
            : what(_what), token(_token) {}
        virtual ~ParseException() {}
        virtual std::u32string format() const;
    };

    class Parser {
    public:
        std::u32string source;
        std::vector<kh::Token> tokens;
        std::shared_ptr<kh::Ast> ast;

        Parser();
        Parser(const std::u32string& _source);
        Parser(const std::vector<kh::Token>& _tokens);
        ~Parser();

        inline bool ok() {
            return this->lex_exceptions.empty() && this->parse_exceptions.empty();
        }

#///lexer:
        double lex_time = 0.0;
        std::vector<kh::LexException> lex_exceptions;

        void lex();

#///parser:
        double parse_time = 0.0;
        size_t ti = 0; /* Token iterator */
        std::vector<kh::ParseException> parse_exceptions;

        /* Most of these parses stuff at the top level scope */
        void parse();
        kh::AstImport* parseImport(const bool is_include);
        void parseAccessAttribs(bool& is_static, bool& is_public);
        kh::AstFunctionExpression* parseFunction(const bool is_static, const bool is_public);
        kh::AstDeclarationExpression* parseDeclaration(const bool is_static, const bool is_public);
        kh::AstClass* parseClass();
        kh::AstStruct* parseStruct();
        kh::AstEnum* parseEnum();
        std::vector<std::shared_ptr<kh::AstBody>> parseBody(const size_t loop_count = 0);

#///expression parser:
        /* These parse expressions below are ordered based from their precedence from lowest to
         * highest */
        kh::AstExpression* parseExpression();     /* Parses an expression */
        kh::AstExpression* parseAssignOps();      /* Parses assignment operations and in-place
                                                   * operations `=`, `+=`, `-=`, `*=`, ... */
        kh::AstExpression* parseTernary();        /* Parses ternary expressions */
        kh::AstExpression* parseOr();             /* Parses logical `or` */
        kh::AstExpression* parseAnd();            /* Parses logical `and` */
        kh::AstExpression* parseNot();            /* Parses logical `not` */
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
        std::vector<uint64_t> parseArrayDimension(std::shared_ptr<kh::AstIdentifierExpression>& type);

    private:
        /* Gets token of the current iterator index */
        inline kh::Token& to() {
            return this->tokens[this->ti];
        }

        inline kh::Token& tokFromIndex(const size_t index) {
            for (kh::Token& token : this->tokens)
                if (token.index == index)
                    return token;

            /* placeholder */
            return *(new kh::Token());
        }
    };

    inline bool isReservedKeyword(const std::u32string& identifier) {
        return identifier == U"public" || identifier == U"private" || identifier == U"static" ||
               identifier == U"def" || identifier == U"class" || identifier == U"struct" ||
               identifier == U"enum" || identifier == U"import" || identifier == U"include" ||
               identifier == U"if" || identifier == U"for" || identifier == U"while" ||
               identifier == U"do" || identifier == U"break" || identifier == U"continue" ||
               identifier == U"return" || identifier == U"ref";
    }

    inline bool isDec(const char32_t chr) {
        return U'0' <= chr && chr <= U'9';
    }

    inline bool isBin(const char32_t chr) {
        return chr == U'0' || chr == U'1';
    }

    inline bool isOct(const char32_t chr) {
        return U'0' <= chr && chr <= U'7';
    }

    inline bool isHex(const char32_t chr) {
        return (U'0' <= chr && chr <= U'9') || (U'a' <= chr && chr <= U'f') ||
               (U'A' <= chr && chr <= U'F');
    }
}

#define KH_PARSE_GUARD()                                                              \
    do {                                                                              \
        if (this->ti >= this->tokens.size()) {                                        \
            this->parse_exceptions.emplace_back(U"Was expecting a token but hit EOF", \
                                                this->tokens.back());                 \
            goto end;                                                                 \
        }                                                                             \
    } while (false)
