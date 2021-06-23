/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/lexer.hpp>
#include <kithare/parser.hpp>
#include <kithare/test.hpp>


static std::vector<std::string>* errors_ptr;

static void parserImportTest() {
    std::vector<kh::LexException> lex_exceptions;
    kh::LexerContext lexer_context{U"import stuff;          \n"
                                   U"import stuff as other; \n"
                                   U"import stuff.other;    \n"
                                   U"include this;          \n",
                                   lex_exceptions};
    std::vector<kh::Token> tokens = kh::lex(lexer_context);
    std::vector<kh::ParseException> parse_exceptions;
    kh::ParserContext parser_context{tokens, parse_exceptions};
    kh::Ast ast = kh::parseWhole(parser_context);

    KH_TEST_ASSERT(lex_exceptions.empty());
    KH_TEST_ASSERT(parse_exceptions.empty());
    KH_TEST_ASSERT(ast.imports.size() == 4);
    KH_TEST_ASSERT(ast.imports[0].identifier == U"stuff");
    KH_TEST_ASSERT(ast.imports[1].identifier == U"other");
    KH_TEST_ASSERT(ast.imports[2].identifier == U"other");
    KH_TEST_ASSERT(ast.imports[0].is_include == false);
    KH_TEST_ASSERT(ast.imports[1].is_include == false);
    KH_TEST_ASSERT(ast.imports[2].is_include == false);
    KH_TEST_ASSERT(ast.imports[3].is_include == true);
    return;
error:
    errors_ptr->back() += "parserImportTest";
}

void kh_test::parserTest(std::vector<std::string>& errors) {
    errors_ptr = &errors;
    parserImportTest();
}
