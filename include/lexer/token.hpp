/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/lexer/token.hpp
* Declares the token struct.
*/

#pragma once

#include "utility/utils.hpp"
#include "lexer/type.hpp"
#include "lexer/value.hpp"


namespace kh {
    struct Token {
        Token(const kh::TokenType _type, const kh::TokenValue _value, const size_t _character, const size_t _line) :
            type(_type), value(_value), character(_character), line(_line) {}

        kh::TokenType type;
        kh::TokenValue value;

        size_t character, line;
    };

    kh::String repr(const kh::Token& token);
}

inline std::ostream& operator<<(std::ostream& out_stream, const kh::Token& token) {
    out_stream << kh::repr(token);
    return out_stream;
}

inline std::wostream& operator<<(std::wostream& wout_stream, const kh::Token& token) {
    wout_stream << kh::repr(token);
    return wout_stream;
}
