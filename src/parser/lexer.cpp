/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/lexer.hpp>


std::u32string kh::LexException::format() const {
    return this->what + U" at line " + kh::str((uint64_t)this->line) + U" column " +
           kh::str((uint64_t)this->column);
}

std::vector<kh::Token> kh::lex(const std::u32string& source) {
    std::vector<kh::LexException> exceptions;
    kh::LexerContext context{source, exceptions};
    std::vector<kh::Token> tokens = kh::lex(context);

    if (exceptions.empty())
        return tokens;
    else
        throw exceptions;
}

std::vector<kh::Token> kh::lex(KH_LEX_CTX) {
    return {};
}
