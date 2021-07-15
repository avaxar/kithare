/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <string>

#include <kithare/exception.hpp>


namespace kh {
    using namespace std;

    class Utf8DecodingException : public kh::Exception {
    public:
        string what;
        size_t index;

        Utf8DecodingException(const string _what, size_t _index) : what(_what), index(_index) {}
        virtual ~Utf8DecodingException() {}
        virtual string format() const;
    };

    string encodeUtf8(const u32string& str);
    u32string decodeUtf8(const string& str);
}
