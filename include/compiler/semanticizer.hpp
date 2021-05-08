/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/compiler/semanticizer.hpp.
 * Declares the kh::Semanticizer class and its methods which semanticizes the source AST
 * making it into a NAST.
 */

#pragma once

#include <string>
#include <vector>

#include "utility/string.hpp"
#include "parser/ast.hpp"
#include "compiler/nast.hpp"


namespace kh {
    struct SemanticizeException {
        std::u32string file_name;
        std::u32string what;
        size_t index;
    };

    /// <summary>
    /// Does semantic analysis from the given AST tree and returns an NAST tree.
    /// </summary>
    /// <param name="ast">AST tree to be analyzed</param>
    /// <returns></returns>
    kh::Nast* semanticize(kh::Ast* ast);

    class Semanticizer {
    public:
        std::vector<SemanticizeException> exceptions;

        Semanticizer(kh::Ast* _ast);
        ~Semanticizer();

        /// <summary>
        /// Does semantic analysis from the provided AST tree and returns an NAST tree.
        /// </summary>
        /// <returns></returns>
        kh::Nast* semanticize();
    private:
        kh::Ast* ast;
    };
}
