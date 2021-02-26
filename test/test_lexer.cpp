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

		#define T(i, t) tokens[i].type != kh::TokenType::t

		return 
			tokens.size() != 21 ||
			T(0, IDENTIFIER) ||
			T(1, IDENTIFIER) ||
			T(2, SYMBOL)     ||
			T(3, IDENTIFIER) ||
			T(4, IDENTIFIER) ||
			T(5, SYMBOL)     ||
			T(6, SYMBOL)     ||
			T(7, SYMBOL)     ||
			T(8, IDENTIFIER) ||
			T(9, IDENTIFIER) ||
			T(10, OPERATOR)  ||
			T(11, FLOATING)  ||
			T(12, SYMBOL)    ||
			T(13, IDENTIFIER)||
			T(14, OPERATOR)  ||
			T(15, IDENTIFIER)||
			T(16, SYMBOL)    ||
			T(17, STRING)    ||
			T(18, SYMBOL)    ||
			T(19, SYMBOL)    ||
			T(20, SYMBOL);
	}
	catch (...) { return true; }
}

/* TODO */
bool testLexNumeralValue() {
	kh::String source = kh::toString(
		L"0 1 2 8 9 " /* Single digit decimal integers */
		L"00 10 29U " /* Multi-digit + Unsigned */
		L"0.1 0.2   " /* Floating point */
		L"11.1 .123 " /* Several other cases */
		L"0xFFF 0x1 " /* Hexadecimal */
		L"0o77 0o11 " /* Octal */
		L"0b111 0b01" /* Binary */
	);

	try {
		
	}
	catch (...) { return true; }
}

KH_TEST_BEGIN(Lexer)
	KH_TEST_WITH_FUNC(testLexTokenType, "'lex' tested to recognize different token types")
KH_TEST_END
