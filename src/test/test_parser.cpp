/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/parser.hpp>
#include <kithare/test.hpp>


static std::vector<std::string>* errors_ptr;

static void parserImportTest() {
    kh::Parser parser(U"import stuff;          \n"
                      U"import stuff as other; \n"
                      U"import stuff.other;    \n"
                      U"include this;          \n");
    std::shared_ptr<kh::Ast>& ast = parser.ast;
    parser.lex();
    parser.parse();

    KH_TEST_ASSERT(parser.ok());
    KH_TEST_ASSERT(ast->imports.size() == 4);
    KH_TEST_ASSERT(ast->imports[0]->identifier == U"stuff");
    KH_TEST_ASSERT(ast->imports[1]->identifier == U"other");
    KH_TEST_ASSERT(ast->imports[2]->identifier == U"other");
    KH_TEST_ASSERT(ast->imports[0]->is_include == false);
    KH_TEST_ASSERT(ast->imports[1]->is_include == false);
    KH_TEST_ASSERT(ast->imports[2]->is_include == false);
    KH_TEST_ASSERT(ast->imports[3]->is_include == true);
    return;
error:
    errors_ptr->back() += "parserImportTest";
}

void kh_test::parserTest(std::vector<std::string>& errors) {
    errors_ptr = &errors;
    parserImportTest();
}
