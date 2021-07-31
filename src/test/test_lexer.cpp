/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/lexer.hpp>
#include <kithare/test.hpp>


using namespace kh;

static std::vector<std::string>* errors_ptr;

static void lexerTypeTest() {
    std::vector<LexException> lex_exceptions;
    Lexer lexer{U"import std;                            \n"
                U"def main() {                           \n"
                U"    // Inline comments                 \n"
                U"    float number = 6.9;                \n"
                U"    std.print(\"Hello, world!\");      \n"
                U"}                                      \n",
                lex_exceptions};
    std::vector<Token> tokens = lexer.lex();

    KH_TEST_ASSERT(lex_exceptions.empty());
    KH_TEST_ASSERT(tokens.size() == 21);
    KH_TEST_ASSERT(tokens[0].type == TokenType::IDENTIFIER);
    KH_TEST_ASSERT(tokens[1].type == TokenType::IDENTIFIER);
    KH_TEST_ASSERT(tokens[2].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[3].type == TokenType::IDENTIFIER);
    KH_TEST_ASSERT(tokens[4].type == TokenType::IDENTIFIER);
    KH_TEST_ASSERT(tokens[5].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[6].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[7].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[8].type == TokenType::IDENTIFIER);
    KH_TEST_ASSERT(tokens[9].type == TokenType::IDENTIFIER);
    KH_TEST_ASSERT(tokens[10].type == TokenType::OPERATOR);
    KH_TEST_ASSERT(tokens[11].type == TokenType::FLOATING);
    KH_TEST_ASSERT(tokens[12].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[13].type == TokenType::IDENTIFIER);
    KH_TEST_ASSERT(tokens[14].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[15].type == TokenType::IDENTIFIER);
    KH_TEST_ASSERT(tokens[16].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[17].type == TokenType::STRING);
    KH_TEST_ASSERT(tokens[18].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[19].type == TokenType::SYMBOL);
    KH_TEST_ASSERT(tokens[20].type == TokenType::SYMBOL);
    return;
error:
    errors_ptr->back() += "lexerTypeTest";
}

static void lexerNumeralTest() {
    std::vector<LexException> lex_exceptions;
    Lexer lexer{U"0 1 2 8 9  " /* Single digit decimal integers */
                U"00 10 29U  " /* Multi-digit + Unsigned */
                U"0.1 0.2    " /* Floating point */
                U"11.1 .123  " /* Several other cases */
                U"0xFFF 0x1  " /* Hexadecimal */
                U"0o77 0o11  " /* Octal */
                U"0b111 0b01 " /* Binary */
                U"4i 2i 5.6i " /* Imaginary */,
                lex_exceptions};
    std::vector<Token> tokens = lexer.lex();

    KH_TEST_ASSERT(lex_exceptions.empty());
    KH_TEST_ASSERT(tokens.size() == 21);
    KH_TEST_ASSERT(tokens[0].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[0].value.integer == 0);
    KH_TEST_ASSERT(tokens[1].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[1].value.integer == 1);
    KH_TEST_ASSERT(tokens[2].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[2].value.integer == 2);
    KH_TEST_ASSERT(tokens[3].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[3].value.integer == 8);
    KH_TEST_ASSERT(tokens[4].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[4].value.integer == 9);
    KH_TEST_ASSERT(tokens[5].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[5].value.integer == 0);
    KH_TEST_ASSERT(tokens[6].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[6].value.integer == 10);
    KH_TEST_ASSERT(tokens[7].type == TokenType::UINTEGER);
    KH_TEST_ASSERT(tokens[7].value.uinteger == 29);
    KH_TEST_ASSERT(tokens[8].type == TokenType::FLOATING);
    KH_TEST_ASSERT(tokens[8].value.floating == 0.1);
    KH_TEST_ASSERT(tokens[9].type == TokenType::FLOATING);
    KH_TEST_ASSERT(tokens[9].value.floating == 0.2);
    KH_TEST_ASSERT(tokens[10].type == TokenType::FLOATING);
    KH_TEST_ASSERT(tokens[10].value.floating == 11.1);
    KH_TEST_ASSERT(tokens[11].type == TokenType::FLOATING);
    KH_TEST_ASSERT(tokens[11].value.floating == 0.123);
    KH_TEST_ASSERT(tokens[12].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[12].value.integer == 4095);
    KH_TEST_ASSERT(tokens[13].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[13].value.integer == 1);
    KH_TEST_ASSERT(tokens[14].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[14].value.integer == 63);
    KH_TEST_ASSERT(tokens[15].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[15].value.integer == 9);
    KH_TEST_ASSERT(tokens[16].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[16].value.integer == 7);
    KH_TEST_ASSERT(tokens[17].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[17].value.integer == 1);
    KH_TEST_ASSERT(tokens[18].type == TokenType::IMAGINARY);
    KH_TEST_ASSERT(tokens[18].value.imaginary == 4.0);
    KH_TEST_ASSERT(tokens[19].type == TokenType::IMAGINARY);
    KH_TEST_ASSERT(tokens[19].value.imaginary == 2.0);
    KH_TEST_ASSERT(tokens[20].type == TokenType::IMAGINARY);
    KH_TEST_ASSERT(tokens[20].value.imaginary == 5.6);
    return;
error:
    errors_ptr->back() += "lexerNumeralTest";
}

static void lexerStringTest() {
    std::vector<LexException> lex_exceptions;
    Lexer lexer{U"\"AB\\x42\\x88\\u1234\\u9876\\v\\U00001234\\U00010000\\\"\\n\"" /* Escape tests */
                U"b'' '' b\"aFd\\x87\\x90\\xff\" 'K' b'\\b' b'\\x34''\\U0001AF21' '\\r' "
                U"\"Hello, world!\" "  /* String */
                U"b\"Hello, world!\" " /* Buffer / byte-string */
                U"\"\"\"Hello,\n"
                U"world!\"\"\" " /* Multiline string */
                U"b\"\"\"Hello,\n"
                U"world!\"\"\" " /* Multiline buffer */,
                lex_exceptions};
    std::vector<Token> tokens = lexer.lex();

    KH_TEST_ASSERT(lex_exceptions.empty());
    KH_TEST_ASSERT(tokens.size() == 13);
    KH_TEST_ASSERT(tokens[0].type == TokenType::STRING);
    KH_TEST_ASSERT(tokens[0].value.string == U"AB\x42\x88\u1234\u9876\v\U00001234\U00010000\"\n");
    KH_TEST_ASSERT(tokens[1].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[1].value.integer == '\0');
    KH_TEST_ASSERT(tokens[2].type == TokenType::CHARACTER);
    KH_TEST_ASSERT(tokens[2].value.character == U'\0');
    KH_TEST_ASSERT(tokens[3].type == TokenType::BUFFER);
    KH_TEST_ASSERT(tokens[3].value.buffer == "aFd\x87\x90\xff");
    KH_TEST_ASSERT(tokens[4].type == TokenType::CHARACTER);
    KH_TEST_ASSERT(tokens[4].value.character == U'K');
    KH_TEST_ASSERT(tokens[5].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[5].value.integer == '\b');
    KH_TEST_ASSERT(tokens[6].type == TokenType::INTEGER);
    KH_TEST_ASSERT(tokens[6].value.integer == '\x34');
    KH_TEST_ASSERT(tokens[7].type == TokenType::CHARACTER);
    KH_TEST_ASSERT(tokens[7].value.character == U'\U0001AF21');
    KH_TEST_ASSERT(tokens[8].type == TokenType::CHARACTER);
    KH_TEST_ASSERT(tokens[8].value.character == U'\r');
    KH_TEST_ASSERT(tokens[9].type == TokenType::STRING);
    KH_TEST_ASSERT(tokens[9].value.string == U"Hello, world!");
    KH_TEST_ASSERT(tokens[10].type == TokenType::BUFFER);
    KH_TEST_ASSERT(tokens[10].value.buffer == "Hello, world!");
    KH_TEST_ASSERT(tokens[11].type == TokenType::STRING);
    KH_TEST_ASSERT(tokens[11].value.string == U"Hello,\nworld!");
    KH_TEST_ASSERT(tokens[12].type == TokenType::BUFFER);
    KH_TEST_ASSERT(tokens[12].value.buffer == "Hello,\nworld!");
    return;
error:
    errors_ptr->back() += "lexerStringTest";
}

void kh_test::lexerTest(std::vector<std::string>& errors) {
    errors_ptr = &errors;
    lexerTypeTest();
    lexerNumeralTest();
    lexerStringTest();
}
