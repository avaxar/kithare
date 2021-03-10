/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* test/test_lexer.cpp
* Lexical analyzer (lexer) unittest.
*/

#include "parser/lexer.hpp"

#include "test.hpp"


bool testLexTokenType() {
    std::u32string source =
        U"import std;                            \n"
        U"int main() {                           \n"
        U"    // Inline comments                 \n"
        U"    float number = 6.9;                \n"
        U"    std.print(\"Hello, world!\");      \n"
        U"}                                      \n";


    try {
        auto tokens = kh::lex(source);

        KH_ASSERT_EQUAL(tokens.size(), 21);
        KH_ASSERT_EQUAL(tokens[0].type, kh::TokenType::IDENTIFIER);
        KH_ASSERT_EQUAL(tokens[1].type, kh::TokenType::IDENTIFIER);
        KH_ASSERT_EQUAL(tokens[2].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[3].type, kh::TokenType::IDENTIFIER);
        KH_ASSERT_EQUAL(tokens[4].type, kh::TokenType::IDENTIFIER);
        KH_ASSERT_EQUAL(tokens[5].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[6].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[7].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[8].type, kh::TokenType::IDENTIFIER);
        KH_ASSERT_EQUAL(tokens[9].type, kh::TokenType::IDENTIFIER);
        KH_ASSERT_EQUAL(tokens[10].type, kh::TokenType::OPERATOR);
        KH_ASSERT_EQUAL(tokens[11].type, kh::TokenType::FLOATING);
        KH_ASSERT_EQUAL(tokens[12].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[13].type, kh::TokenType::IDENTIFIER);
        KH_ASSERT_EQUAL(tokens[14].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[15].type, kh::TokenType::IDENTIFIER);
        KH_ASSERT_EQUAL(tokens[16].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[17].type, kh::TokenType::STRING);
        KH_ASSERT_EQUAL(tokens[18].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[19].type, kh::TokenType::SYMBOL);
        KH_ASSERT_EQUAL(tokens[20].type, kh::TokenType::SYMBOL);
        return false;
    }
    catch (...) { return true; }
}

/* TODO */
bool testLexNumeralValue() {
    std::u32string source =
        U"0 1 2 8 9  "  /* Single digit decimal integers */
        U"00 10 29U  "  /* Multi-digit + Unsigned */
        U"0.1 0.2    "  /* Floating point */
        U"11.1 .123  "  /* Several other cases */
        U"0xFFF 0x1  "  /* Hexadecimal */
        U"0o77 0o11  "  /* Octal */
        U"0b111 0b01 "  /* Binary */
        U"4i 2i 5.6i "; /* Imaginary */

    try {
        auto tokens = kh::lex(source);
        KH_ASSERT_EQUAL(tokens.size() , 21);

        KH_ASSERT_EQUAL(tokens[0].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[0].value.integer, 0);
        KH_ASSERT_EQUAL(tokens[1].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[1].value.integer, 1);
        KH_ASSERT_EQUAL(tokens[2].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[2].value.integer, 2);
        KH_ASSERT_EQUAL(tokens[3].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[3].value.integer, 8);
        KH_ASSERT_EQUAL(tokens[4].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[4].value.integer, 9);
        KH_ASSERT_EQUAL(tokens[5].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[5].value.integer, 0);
        KH_ASSERT_EQUAL(tokens[6].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[6].value.integer, 10);
        KH_ASSERT_EQUAL(tokens[7].type, kh::TokenType::UINTEGER);
        KH_ASSERT_EQUAL(tokens[7].value.uinteger, 29);
        KH_ASSERT_EQUAL(tokens[8].type, kh::TokenType::FLOATING);
        KH_ASSERT_EQUAL(tokens[8].value.floating, 0.1);
        KH_ASSERT_EQUAL(tokens[9].type, kh::TokenType::FLOATING);
        KH_ASSERT_EQUAL(tokens[9].value.floating, 0.2);
        KH_ASSERT_EQUAL(tokens[10].type, kh::TokenType::FLOATING);
        KH_ASSERT_EQUAL(tokens[10].value.floating, 11.1);
        KH_ASSERT_EQUAL(tokens[11].type, kh::TokenType::FLOATING);
        KH_ASSERT_EQUAL(tokens[11].value.floating, 0.123);
        KH_ASSERT_EQUAL(tokens[12].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[12].value.integer, 4095);
        KH_ASSERT_EQUAL(tokens[13].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[13].value.integer, 1);
        KH_ASSERT_EQUAL(tokens[14].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[14].value.integer, 63);
        KH_ASSERT_EQUAL(tokens[15].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[15].value.integer, 9);
        KH_ASSERT_EQUAL(tokens[16].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[16].value.integer, 7);
        KH_ASSERT_EQUAL(tokens[17].type, kh::TokenType::INTEGER);
        KH_ASSERT_EQUAL(tokens[17].value.integer, 1);
        KH_ASSERT_EQUAL(tokens[18].type, kh::TokenType::IMAGINARY);
        KH_ASSERT_EQUAL(tokens[18].value.imaginary, 4.0);
        KH_ASSERT_EQUAL(tokens[19].type, kh::TokenType::IMAGINARY);
        KH_ASSERT_EQUAL(tokens[19].value.imaginary, 2.0);
        KH_ASSERT_EQUAL(tokens[20].type, kh::TokenType::IMAGINARY);
        KH_ASSERT_EQUAL(tokens[20].value.imaginary, 5.6);
        return false;
    }
    catch (...) { return true; }
}

KH_TEST_BEGIN(Lexer)
KH_TEST_WITH_FUNC(testLexTokenType, "Lex Token Types")
KH_TEST_WITH_FUNC(testLexNumeralValue, "Lex Numeral Values")
KH_TEST_END
