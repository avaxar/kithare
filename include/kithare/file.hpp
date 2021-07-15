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

    class FileError : public kh::Exception {
    public:
        FileError() {}
        virtual ~FileError() {}
        virtual string format() const;
    };

    u32string readFile(const u32string& path);
    string readFileBinary(const u32string& path);
}
