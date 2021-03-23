/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/parser/parser.hpp
* Declares the kh::Parser class.
*/

#pragma once

#include <vector>

#include "parser/token.hpp"
#include "parser/ast.hpp"


namespace kh {
    kh::AstModule parse(const std::vector<kh::Token>& tokens);

    class Parser {
    public:
        Parser(const std::vector<kh::Token>& tokens);
        ~Parser();

        kh::AstModule parse();
    private:

    };
}
