/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * test/test_parser.cpp
 * Parser unittest.
 */

#include <memory>

#include "parser/lexer.hpp"
#include "parser/parser.hpp"

#include "test.hpp"

bool testImportParse() {
    std::u32string source = U"import stuff;          \n"
                            U"import stuff as other; \n"
                            U"import stuff.other;    \n"
                            U"include this;          \n";

    try {
        std::shared_ptr<kh::Ast> ast(kh::parse(kh::lex(source)));

        KH_ASSERT_EQUAL(ast->imports.size(), 4);

        KH_ASSERT_EQUAL(ast->imports[0]->identifier, U"stuff");
        KH_ASSERT_EQUAL(ast->imports[1]->identifier, U"other");
        KH_ASSERT_EQUAL(ast->imports[2]->identifier, U"other");
        
        KH_ASSERT_EQUAL(ast->imports[0]->is_include, false);
        KH_ASSERT_EQUAL(ast->imports[1]->is_include, false);
        KH_ASSERT_EQUAL(ast->imports[2]->is_include, false);
        KH_ASSERT_EQUAL(ast->imports[3]->is_include, true);

        return false;
    }
    catch (...) {
        return true;
    }
}

KH_TEST_BEGIN(Parser)
KH_TEST_WITH_FUNC(testImportParse, "Parse Imports and Includes")
KH_TEST_END
