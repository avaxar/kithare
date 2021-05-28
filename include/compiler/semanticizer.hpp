/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/compiler/semanticizer.hpp.
 * Declares the kh::Semanticizer class and its methods which analyzes the source AST tree,
 * and outputs an IR.
 */

#pragma once

#include <string>
#include <vector>

#include "compiler/ir.hpp"
#include "parser/ast.hpp"
#include "utility/string.hpp"


namespace kh {
    struct SemanticizeException {
        SemanticizeException(const std::u32string& _file_name, const std::u32string& _what,
                             const size_t _index)
            : file_name(_file_name), what(_what), index(_index) {}

        std::u32string file_name;
        std::u32string what;
        size_t index;
    };
}
