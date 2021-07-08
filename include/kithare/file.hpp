/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <string>

#include <kithare/exception.hpp>


namespace kh {
    class FileReadError : public kh::Exception {
    public:
        std::string fname;

        FileReadError(const std::string& _fname) : fname(_fname) {}
        virtual ~FileReadError() {}
        virtual std::string format() const;
    };

    std::u32string readFile(const std::u32string& path);
    std::string readFileBinary(const std::u32string& path);
}
