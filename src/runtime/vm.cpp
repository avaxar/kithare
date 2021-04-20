/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * src/runtime/vm.cpp
 * Defines include/runtime/vm.hpp
 */

#include <string.h>

#include "runtime/vm.hpp"

#ifdef _WIN32
#define aligned_alloc _aligned_malloc
#endif


kh::VM::VM(const size_t mem_size) {
    this->vmem = (uint8_t*)aligned_alloc(mem_size, sizeof(size_t));
    this->vmemsize = mem_size;

    if (!this->vmem)
        throw kh::VMException(U"Unable to allocate virtual memory");
}

kh::VM::VM(const kh::VM& copy) {
    this->vmem = (uint8_t*)aligned_alloc(copy.vmemsize, sizeof(size_t));
    this->vmemsize = copy.vmemsize;

    if (this->vmem)
        memcpy(this->vmem, copy.vmem, copy.vmemsize);
    else
        throw kh::VMException(U"Unable to allocate virtual memory");
}

kh::VM::~VM() {
    if (this->vmem)
        std::free(this->vmem);
}

void kh::VM::resizeMem(const size_t mem_size) {
#ifdef _WIN32
    uint8_t* resized_mem = this->vmem ? (uint8_t*)_aligned_realloc(this->vmem, mem_size, sizeof(size_t))
                                      : (uint8_t*)aligned_alloc(mem_size, sizeof(size_t));
#else
    uint8_t* resized_mem = (uint8_t*)aligned_alloc(mem_size, sizeof(size_t));
#endif

    if (!resized_mem)
        throw kh::VMException(U"Unable to resize virtual memory");

    if (this->vmem)
        memcpy(resized_mem, this->vmem, this->vmemsize);

    this->vmem = resized_mem;
}
