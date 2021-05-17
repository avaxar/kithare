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

    /// <summary>
    /// Does semantic analysis from the given AST tree and returns an Ir tree.
    /// </summary>
    /// <param name="ast">AST tree to be analyzed</param>
    /// <returns></returns>
    kh::Ir* semanticize(kh::Ast* ast);

    class Semanticizer {
    public:
        std::vector<SemanticizeException> exceptions;

        Semanticizer(kh::Ast* ast);
        ~Semanticizer();

        /// <summary>
        /// Does semantic analysis from the provided AST tree and returns an Ir tree.
        /// </summary>
        /// <returns></returns>
        kh::Ir* semanticize();

    private:
        kh::Ast* ast_tree;

        std::u32string locateModule(const std::vector<std::u32string>& path, const bool is_relative);
        kh::IrModule* makeModule(kh::Ast* ast);

        kh::IrEnum* makeEnum(std::shared_ptr<kh::AstEnum>& ast);
        kh::IrStruct* makeStruct(std::shared_ptr<kh::AstStruct>& ast);
        kh::IrClassTemplate* makeClass(kh::IrModule* context, std::shared_ptr<kh::AstClass>& ast);
        kh::IrClass* templatizeClass(kh::IrClassTemplate* class_template,
                                       const std::vector<kh::IrType*>& arguments);
        kh::IrFunctionTemplate* makeFunction(kh::IrModule* context,
                                               std::shared_ptr<kh::AstFunctionExpression>& ast);
        kh::IrFunction* templatizeFunction(kh::IrFunctionTemplate* function_template,
                                             const std::vector<kh::IrType*>& arguments);

        std::vector<kh::IrExpression*> resolve(kh::IrModule* gcontext, kh::IrScope* lcontext,
                                                 const std::vector<std::u32string>& identifiers);
        std::vector<kh::IrExpression*> resolve(kh::IrExpression* expr,
                                                 const std::u32string& identifier);
    };
}
