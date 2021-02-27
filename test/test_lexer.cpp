/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* test/test_string.cpp
* Source for lexer file tests
*/

#include "test/test_lexer.hpp"


bool testLexTokenType() {
    kh::String source = kh::toString(
        L"import std;                            \n"
        L"int main() {                           \n"
        L"    // Inline comments                 \n"
        L"    float number = 6.9;                \n"
        L"    std.print(\"Hello, world!\");      \n"
        L"}                                      \n"
    );


    try {
        auto tokens = kh::lex(source, kh::toString(L"<test>"));

        KH_TEST_TRUE(tokens.size() == 21);
        KH_TEST_TRUE(tokens[0].type == kh::TokenType::IDENTIFIER);
        KH_TEST_TRUE(tokens[1].type == kh::TokenType::IDENTIFIER);
        KH_TEST_TRUE(tokens[2].type == kh::TokenType::SYMBOL);
        KH_TEST_TRUE(tokens[3].type == kh::TokenType::IDENTIFIER);
        KH_TEST_TRUE(tokens[4].type == kh::TokenType::IDENTIFIER);
        KH_TEST_TRUE(tokens[5].type == kh::TokenType::SYMBOL);
        KH_TEST_TRUE(tokens[6].type == kh::TokenType::SYMBOL);
        KH_TEST_TRUE(tokens[7].type == kh::TokenType::SYMBOL);
        KH_TEST_TRUE(tokens[8].type == kh::TokenType::IDENTIFIER);
        KH_TEST_TRUE(tokens[9].type == kh::TokenType::IDENTIFIER);
        KH_TEST_TRUE(tokens[10].type == kh::TokenType::OPERATOR);
        KH_TEST_TRUE(tokens[11].type == kh::TokenType::FLOATING);
        KH_TEST_TRUE(tokens[12].type == kh::TokenType::SYMBOL);
        KH_TEST_TRUE(tokens[13].type == kh::TokenType::IDENTIFIER);
        KH_TEST_TRUE(tokens[14].type == kh::TokenType::OPERATOR);
        KH_TEST_TRUE(tokens[15].type == kh::TokenType::IDENTIFIER);
        KH_TEST_TRUE(tokens[16].type == kh::TokenType::SYMBOL);
        KH_TEST_TRUE(tokens[17].type == kh::TokenType::STRING);
        KH_TEST_TRUE(tokens[18].type == kh::TokenType::SYMBOL);
        KH_TEST_TRUE(tokens[19].type == kh::TokenType::SYMBOL);
        KH_TEST_TRUE(tokens[20].type == kh::TokenType::SYMBOL);
        return false;
    }
    catch (...) { return true; }
}

/* TODO */
bool testLexNumeralValue() {
    kh::String source = kh::toString(
        L"0 1 2 8 9  " /* Single digit decimal integers */
        L"00 10 29U  " /* Multi-digit + Unsigned */
        L"0.1 0.2    " /* Floating point */
        L"11.1 .123  " /* Several other cases */
        L"0xFFF 0x1  " /* Hexadecimal */
        L"0o77 0o11  " /* Octal */
        L"0b111 0b01 " /* Binary */
        L"4i 2i 5.6i " /* Imaginary */
    );

    try {
        auto tokens = kh::lex(source, kh::toString(L"<test>"));
        KH_TEST_TRUE(tokens.size() == 21);

        KH_TEST_TRUE(tokens[0].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[0].value.integer == 0);
        KH_TEST_TRUE(tokens[1].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[1].value.integer == 1);
        KH_TEST_TRUE(tokens[2].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[2].value.integer == 2);
        KH_TEST_TRUE(tokens[3].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[3].value.integer == 8);
        KH_TEST_TRUE(tokens[4].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[4].value.integer == 9);
        KH_TEST_TRUE(tokens[5].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[5].value.integer == 0);
        KH_TEST_TRUE(tokens[6].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[6].value.integer == 10);
        KH_TEST_TRUE(tokens[7].type == kh::TokenType::UNSIGNED_INTEGER);
        KH_TEST_TRUE(tokens[7].value.unsigned_integer == 29);
        KH_TEST_TRUE(tokens[8].type == kh::TokenType::FLOATING);
        KH_TEST_TRUE(tokens[8].value.floating == 0.1);
        KH_TEST_TRUE(tokens[9].type == kh::TokenType::FLOATING);
        KH_TEST_TRUE(tokens[9].value.floating == 0.2);
        KH_TEST_TRUE(tokens[10].type == kh::TokenType::FLOATING);
        KH_TEST_TRUE(tokens[10].value.floating == 11.1);
        KH_TEST_TRUE(tokens[11].type == kh::TokenType::FLOATING);
        KH_TEST_TRUE(tokens[11].value.floating == 0.123);
        KH_TEST_TRUE(tokens[12].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[12].value.integer == 4095);
        KH_TEST_TRUE(tokens[13].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[13].value.integer == 1);
        KH_TEST_TRUE(tokens[14].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[14].value.integer == 63);
        KH_TEST_TRUE(tokens[15].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[15].value.integer == 9);
        KH_TEST_TRUE(tokens[16].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[16].value.integer == 7);
        KH_TEST_TRUE(tokens[17].type == kh::TokenType::INTEGER);
        KH_TEST_TRUE(tokens[17].value.integer == 1);
        KH_TEST_TRUE(tokens[18].type == kh::TokenType::IMAGINARY);
        KH_TEST_TRUE(tokens[18].value.imaginary == 4.0);
        KH_TEST_TRUE(tokens[19].type == kh::TokenType::IMAGINARY);
        KH_TEST_TRUE(tokens[19].value.imaginary == 2.0);
        KH_TEST_TRUE(tokens[20].type == kh::TokenType::IMAGINARY);
        KH_TEST_TRUE(tokens[20].value.imaginary == 5.6);
        return false;
    }
    catch (...) { return true; }
}

KH_TEST_BEGIN(Lexer)
    KH_TEST_WITH_FUNC(testLexTokenType, "Lex Token Types")
    KH_TEST_WITH_FUNC(testLexNumeralValue, "Lex Numeral Values")
KH_TEST_END
