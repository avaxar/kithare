/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <string>

#include <kithare/exception.hpp>


namespace kh {
    class Utf8DecodingException : public Exception {
    public:
        std::string what;
        size_t index;

        Utf8DecodingException(const std::string _what, size_t _index) : what(_what), index(_index) {}
        virtual ~Utf8DecodingException() {}
        virtual std::string format() const;
    };

    std::string utf8Encode(const std::u32string& str);
    std::u32string utf8Decode(const std::string& str);
}
