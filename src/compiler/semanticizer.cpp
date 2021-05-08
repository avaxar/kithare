/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * src/compiler/semanticizer.cpp
 * Defines include/compiler/semanticizer.hpp.
 */

#include "compiler/semanticizer.hpp"


kh::Nast* kh::semanticize(kh::Ast* ast) {
    kh::Semanticizer semanticizer(ast);
    kh::Nast* nast = semanticizer.semanticize();

    if (semanticizer.exceptions.empty())
        return nast;
    else {
        delete nast;
        throw semanticizer.exceptions;
    }
}

kh::Semanticizer::Semanticizer(kh::Ast* _ast) : ast(_ast) {}

kh::Semanticizer::~Semanticizer() {}

kh::Nast* kh::Semanticizer::semanticize() {
    this->exceptions.clear();
    
    /* Placeholder */
    return nullptr;
}
