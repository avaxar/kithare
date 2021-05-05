/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/parser/repr.hpp
 * Declares kh::repr overloads for the AST types.
 */

#pragma once

#include <string>

#include "utility/string.hpp"
#include "parser/ast.hpp"


namespace kh {
    std::u32string repr(const kh::Ast& module_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstImport& import_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstClass& class_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstStruct& struct_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstEnum& enum_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstBody& ast, const size_t indent = 0);
    std::u32string repr(const kh::AstExpression& expr, const size_t indent = 0);
}
