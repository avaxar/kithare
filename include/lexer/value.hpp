/*
* This file is a part of the Kithare programming language source code.
* The Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/lexer/value.hpp
* Declares the token value struct/union.
*/

#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"
#include "lexer/type.hpp"


namespace kh {
    struct TokenValue {
        union {
            kh::Operator operator_type;
            kh::Symbol symbol_type;

            uint64 unsigned_integer;
            int64 integer;
            double floating;
            double imaginary;

            uint32 character;
        };

        kh::String identifier_name;
        kh::String string;
        std::vector<uint8> buffer;
    };
}
