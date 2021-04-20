/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/runtime/vm.hpp
 * Declares the Kithare VM runtime environment class.
 */

#pragma once

#include <stdint.h>

#include "utility/string.hpp"


namespace kh {
    struct VMException {
        VMException(const std::u32string& _what) : what(_what) {}

        std::u32string what;
    };

    struct Block {
        bool free = true;
        uint64_t size = sizeof(Block);
    };

    class VM {
    public:
        VM(const size_t mem_size);
        VM(const kh::VM& copy);
        ~VM();

        void resizeMem(const size_t mem_size);
        inline size_t memSize() const {
            return this->vmemsize;
        }

    private:
        size_t vmemsize;
        uint8_t* vmem;
    };
}
