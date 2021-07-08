/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <string>


namespace kh {
    class Exception {
    public:
        virtual ~Exception() {}
        virtual std::string format() const {
            return "";
        }
    };
}
