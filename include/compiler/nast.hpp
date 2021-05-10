/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/compiler/nast.hpp
 * Declares non-abstract syntax tree node types
 * (Compiler's second stage AST. Just can't think of a better name -.-).
 */

#pragma once

#include <memory>
#include <vector>
#include <stdint.h>


namespace kh {
    class Nast;

    class NastModule;
    class NastScope;

    class NastBody;
    class NastIf;
    class NastWhile;
    class NastStatement;

    class NastExpression;
    class NastConst;
    class NastVariable;
    class NastAssignExpression;
    class NastCallExpression;
    class NastReinterpret;
    class NastScopeExpression;
    class NastPointerScopeExpression;

    class NastFunction;
    class NastUserFunction;
    class NastBuiltinFunction;
    class NastFunctionTemplate;

    class NastType;
    class NastRefType;
    class NastUserEnum;
    class NastUserStruct;
    class NastUserClass;
    class NastClassTemplate;
    class NastPrimitiveType;
    class NastBuiltinType;
}
