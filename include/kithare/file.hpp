/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once


#include <kithare/utf8.hpp>


namespace kh {
    class FileError : public Exception {
    public:
        FileError() {}
        virtual ~FileError() {}
        virtual std::string format() const;
    };

    std::u32string fileRead(const std::string& path);
    std::string fileReadBinary(const std::string& path);
}
