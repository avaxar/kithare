/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <string>

#include <kithare/exception.hpp>


namespace kh {
    class Utf8DecodingException : public kh::Exception {
    public:
        std::u32string what;
        size_t index;

        Utf8DecodingException(const std::u32string _what, const size_t _index)
            : what(_what), index(_index) {}
        virtual ~Utf8DecodingException() {}
        virtual std::u32string format() const;
    };

    std::string encodeUtf8(const std::u32string& str);
    std::u32string decodeUtf8(const std::string& str);
}
