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

#include "compiler/nast.hpp"
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

    /// <summary>
    /// Does semantic analysis from the given AST tree and returns an NAST tree.
    /// </summary>
    /// <param name="ast">AST tree to be analyzed</param>
    /// <returns></returns>
    kh::Nast* semanticize(kh::Ast* ast);

    class Semanticizer {
    public:
        std::vector<SemanticizeException> exceptions;

        Semanticizer(kh::Ast* ast);
        ~Semanticizer();

        /// <summary>
        /// Does semantic analysis from the provided AST tree and returns an NAST tree.
        /// </summary>
        /// <returns></returns>
        kh::Nast* semanticize();

    private:
        kh::Ast* ast_tree;

        std::u32string locateModule(const std::vector<std::u32string>& path, const bool is_relative);
        kh::NastModule* makeModule(kh::Ast* ast);

        kh::NastUserEnum* makeEnum(std::shared_ptr<kh::AstEnum>& ast);
        kh::NastUserStruct* makeStruct(std::shared_ptr<kh::AstStruct>& ast);
        kh::NastClassTemplate* makeClass(kh::NastModule* context,
                                         std::shared_ptr<kh::AstClass>& ast);
        kh::NastUserClass* templatizeClass(kh::NastClassTemplate* class_template,
                                           const std::vector<kh::NastType*>& arguments);
        kh::NastFunctionTemplate* makeFunction(kh::NastModule* context,
                                               std::shared_ptr<kh::AstFunctionExpression>& ast);
        kh::NastUserFunction* templatizeFunction(kh::NastFunctionTemplate* function_template,
                                                 const std::vector<kh::NastType*>& arguments);

        std::vector<kh::NastExpression*> resolve(kh::NastModule* gcontext, kh::NastScope* lcontext,
                                                 const std::vector<std::u32string>& identifiers);
        std::vector<kh::NastExpression*> resolve(kh::NastExpression* expr,
                                                 const std::u32string& identifier);
    };
}
