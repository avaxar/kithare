/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * src/compiler/semanticizer.cpp
 * Defines include/compiler/semanticizer.hpp.
 */

#include "compiler/semanticizer.hpp"


kh::Ir* kh::semanticize(kh::Ast* ast) {
    kh::Semanticizer semanticizer(ast);
    kh::Ir* ir = semanticizer.semanticize();

    if (semanticizer.exceptions.empty())
        return ir;
    else {
        delete ir;
        throw semanticizer.exceptions;
    }
}

kh::Semanticizer::Semanticizer(kh::Ast* ast) : ast_tree(ast) {}

kh::Semanticizer::~Semanticizer() {}

kh::Ir* kh::Semanticizer::semanticize() {
    this->exceptions.clear();

    /* Placeholder */
    return nullptr;
}
