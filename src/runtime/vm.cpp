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

#define ALIGNMENT 256


kh::VM::VM(const size_t mem_size) {
    this->origin_block = (kh::Block*)aligned_alloc(mem_size, ALIGNMENT);
    this->free_block = this->origin_block;
    this->last_block = this->origin_block;
    this->mem_size = mem_size;

    if (!this->origin_block)
        throw kh::VMException(U"Unable to allocate virtual memory");
}

kh::VM::~VM() {
    if (this->origin_block)
        std::free(this->origin_block);
}

void kh::VM::resizeMemory(const size_t size) {
#ifdef _WIN32
    kh::Block* resized_mem = this->origin_block
                                 ? (kh::Block*)_aligned_realloc(this->origin_block, size, ALIGNMENT)
                                 : (kh::Block*)aligned_alloc(size, ALIGNMENT);
#else
    kh::Block* resized_mem = (kh::Block*)aligned_alloc(mem_size, ALIGNMENT);
#endif

    /*
    if (!resized_mem)
        throw kh::VMException(U"Unable to resize virtual memory");

    if (this->origin_block)
        memcpy(resized_mem, this->origin_block, this->mem_size);

    this->mem_size = size;
    this->origin_block = resized_mem;

    if (this->last_block) {

    }
    else
        this->last_block = resized_mem;
    */
}

uint64_t kh::VM::malloc(const uint64_t size) {
    return 0; /* Placeholder */
}

void kh::VM::free(const uint64_t addr) {}
