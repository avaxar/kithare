/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/parser/lexer.hpp
*/

#pragma once

#include <string>

#include "parser/token.hpp"


namespace kh {
	class Lexer {
	private:
		std::u32string src;
		size_t it;
	public:
		Lexer(const std::u32string& source);
		~Lexer();
	};
}
