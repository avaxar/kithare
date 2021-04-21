/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/compiler/ir.hpp
 * Declares compiler intermediate representation types.
 */

#pragma once

#include <stdint.h>


namespace kh {
    class IrType;
    class IrFunction;
    class IrAction;

    class IrType {
    public:
        enum class Type {
            BUILTIN_CLASS, BUILTIN_STRUCT, TUPLE,
            USER_ENUM, USER_STRUCT, USER_CLASS
        } type;
        uint64_t size;
    };

    class IrFunction {

    };

    class IrAction {
    public:
        enum class Action {
            INT_CAST
        } type;
    };
}
