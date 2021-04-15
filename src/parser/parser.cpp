/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * src/parser/parser.cpp
 * Defines include/parser/parser.hpp.
 */

#include "parser/parser.hpp"


#define GUARD(offset)                                                                   \
    do {                                                                                \
        if (this->ti + offset >= this->tokens.size()) {                                 \
            this->exceptions.emplace_back(                                              \
                U"Was expecting a token but hit EOF",                                   \
                this->tokens.size() ? this->tokens[this->tokens.size() - 1].index : 0); \
            goto end;                                                                   \
        }                                                                               \
    } while (false)

kh::Ast* kh::parse(const std::vector<kh::Token>& tokens) {
    kh::Parser parser(tokens);
    kh::Ast* ast;

    ast = parser.parse();
    if (parser.exceptions.empty())
        return ast;
    else {
        delete ast;
        throw kh::ParseExceptions(parser.exceptions);
    }
}

kh::Parser::Parser(const std::vector<kh::Token>& _tokens) {
    this->tokens = _tokens;
}

kh::Parser::~Parser() {}

kh::Ast* kh::Parser::parse() {
    this->exceptions.clear();

    std::vector<std::shared_ptr<kh::AstImport>> imports;
    std::vector<std::shared_ptr<kh::AstFunctionExpression>> functions;
    std::vector<std::shared_ptr<kh::AstClass>> classes;
    std::vector<std::shared_ptr<kh::AstStruct>> structs;
    std::vector<std::shared_ptr<kh::AstEnum>> enums;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> variables;

    for (this->ti = 0; this->ti < this->tokens.size();) {
        kh::Token& token = this->to();

        switch (token.type) {
            case kh::TokenType::IDENTIFIER: {
                const std::u32string& identifier = token.value.identifier;

                /* Function declaration identifier keyword */
                if (identifier == U"def") {
                    /* Skips initial keyword */
                    this->ti++;
                    GUARD(0);

                    /* Parses access type */
                    bool is_static, is_public;
                    this->parseAccessAttribs(is_static, is_public);
                    GUARD(0);
                    /* Parses return type, name, arguments, and body */
                    functions.emplace_back(this->parseFunction(is_static, is_public));
                }
                /* Parses class declaration */
                else if (identifier == U"class") {
                    this->ti++;
                    GUARD(0);
                    classes.emplace_back(this->parseClass());
                }
                /* Parses struct declaration */
                else if (identifier == U"struct") {
                    this->ti++;
                    GUARD(0);
                    structs.emplace_back(this->parseStruct());
                }
                /* Parses enum declaration */
                else if (identifier == U"enum") {
                    this->ti++;
                    GUARD(0);
                    enums.emplace_back(this->parseEnum());
                }
                /* Parses import statement */
                else if (identifier == U"import") {
                    this->ti++;
                    GUARD(0);
                    imports.emplace_back(this->parseImport(false)); /* is_include = false */
                }
                /* Parses include statement */
                else if (identifier == U"include") {
                    this->ti++;
                    GUARD(0);
                    imports.emplace_back(this->parseImport(true)); /* is_include = true */
                }
                /* If it was none of those above, it's probably a variable declaration */
                else {
                    /* Gets variable's declaration access type */
                    bool is_static, is_public;
                    this->parseAccessAttribs(is_static, is_public);
                    GUARD(0);

                    /* Parses the variable's return type, name, and assignment value */
                    variables.emplace_back(this->parseDeclaration(is_static, is_public));

                    /* Makes sure it ends with a semicolon */
                    GUARD(0);
                    token = this->to();
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->exceptions.emplace_back(
                            U"Was expecting a semicolon after a variable declaration", token.index);
                }
            } break;

            case kh::TokenType::SYMBOL:
                switch (token.value.symbol_type) {
                    /* Placeholder semicolon, ignore */
                    case kh::Symbol::SEMICOLON: {
                        this->ti++;
                    } break;

                    /* Unknown symbol */
                    default:
                        this->ti++;
                        this->exceptions.emplace_back(U"Unexpected token while parsing the top scope",
                                                      token.index);
                }
                break;

            /* Unknown token */
            default:
                this->ti++;
                this->exceptions.emplace_back(U"Unexpected token while parsing the top scope",
                                              token.index);
        }
    }

end:
    return new kh::Ast(imports, functions, classes, structs, enums, variables);
}

kh::AstImport* kh::Parser::parseImport(const bool is_include) {
    std::vector<std::u32string> path;
    std::u32string identifier;
    kh::Token token = this->to();
    size_t index = token.index;

    /* Making sure that it starts with an identifier (an import/include statement must has at least
     * one identifier to be imported) */
    if (token.type == kh::TokenType::IDENTIFIER) {
        path.push_back(token.value.identifier);
        this->ti++;
    }
    else {
        this->exceptions.emplace_back(U"Was expecting an identifier in the import/include statement",
                                      token.index);
        this->ti++;
    }

    GUARD(0);
    token = this->to();

    /* Parses each identifier after a dot `import a.b.c.d ...` */
    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        this->ti++;
        GUARD(0);
        token = this->to();

        /* Appends the identifier */
        if (token.type == kh::TokenType::IDENTIFIER) {
            path.push_back(token.value.identifier);
            this->ti++;
            GUARD(0);
            token = this->to();
        }
        else {
            this->exceptions.emplace_back(
                U"Was expecting an identifier after the top in the import/include statement",
                token.index);
            break;
        }
    }

    /* An optional `as` for changing the namespace name in import statements */
    if ((!is_include) && token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"as") {
        this->ti++;
        GUARD(0);
        token = this->to();

        /* Gets the set namespace identifier */
        if (token.type == kh::TokenType::IDENTIFIER) {
            identifier = token.value.identifier;

            this->ti++;
            GUARD(0);
            token = this->to();
        }
        else {
            this->exceptions.emplace_back(
                U"Was expecting an identifier after `as` in the import statement", token.index);
            this->ti++;
        }
    }

    /* Ensure that it ends with a semicolon */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
        this->ti++;
    else
        this->exceptions.emplace_back(U"Was expecting a semicolon after the import/include statement",
                                      token.index);
end:
    return new kh::AstImport(index, path, is_include,
                             path.empty() ? U"" : (identifier.empty() ? path.back() : identifier));
}

void kh::Parser::parseAccessAttribs(bool& is_static, bool& is_public) {
    is_static = false;
    is_public = true;

    /* It parses these kinds of access types: `[static | private/public] int x = 3` */

    kh::Token token = this->to();
    while (token.type == kh::TokenType::IDENTIFIER) {
        if (token.value.identifier == U"static")
            is_static = true;
        else if (token.value.identifier == U"public")
            is_public = true;
        else if (token.value.identifier == U"private")
            is_public = false;
        else
            break;

        this->ti++;
        GUARD(0);
        token = this->to();
    }

end:
    return;
}

kh::AstFunctionExpression* kh::Parser::parseFunction(const bool is_static, const bool is_public) {
    std::vector<std::u32string> identifiers;
    std::vector<std::u32string> generic_args;
    std::shared_ptr<kh::AstIdentifierExpression> return_type;
    size_t return_ref_depth = 0;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> arguments;
    std::vector<std::shared_ptr<kh::AstBody>> body;

    std::shared_ptr<kh::AstIdentifierExpression> return_type_or_identifiers;
    kh::Token token = this->to();
    size_t index = token.index;

    /* Checks for the return type's ref depth: `def ref ref int a() {}` which returns a reference of
     * a reference of an int */
    while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
        return_ref_depth++;
        this->ti++;
        GUARD(0);
        token = this->to();
    }

    /* No return type anonymous/no-name functions/lambdas `def &() {}` */
    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::BIT_AND) {
        this->ti++;
        return_type.reset(new kh::AstIdentifierExpression(token.index, {U"void"}, {}));
        goto parseArgs;
    }

    /* `return_type_or_identifiers`' name is just that questionable to implement later on,
     * making return types be optional (makes them return `void` by default) */
    return_type_or_identifiers.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());

    GUARD(0);
    token = this->to();

    /* The case where it doesn't specify a return type */
    if (token.type == kh::TokenType::SYMBOL &&
        token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        /* Sets the return type to `void` */
        return_type.reset(
            new kh::AstIdentifierExpression(return_type_or_identifiers->index, {U"void"}, {}));
        /* Passes the `return_type_or_identifiers` as the function identifier */
        identifiers = return_type_or_identifiers->identifiers;

        /* The generic arguments were funnelled from the parsing of the identifier */
        for (auto& generic_ : return_type_or_identifiers->generics) {
            if (generic_->identifiers.size() != 1)
                this->exceptions.emplace_back(U"Could not have multiple identifiers as a generic "
                                              U"argument name in the function declaration",
                                              generic_->index);
            if (!generic_->generics.empty())
                this->exceptions.emplace_back(U"Could not have generic arguments in a generic "
                                              U"argument in the function declaration",
                                              generic_->generics[0]->index);

            generic_args.push_back(generic_->identifiers.empty() ? U"" : generic_->identifiers[0]);
        }
    }
    /* The case where it does specify a return type */
    else if (token.type == kh::TokenType::IDENTIFIER) {
        /* Passes `return_type_or_identifiers` as the return type */
        return_type = return_type_or_identifiers;

        /* Parses the identifier */
        std::shared_ptr<kh::AstIdentifierExpression> id_generic_args(
            (kh::AstIdentifierExpression*)this->parseIdentifiers());
        identifiers = id_generic_args->identifiers;

        /* The generic arguments were funnelled from the parsing of the identifier */
        for (auto& generic_ : id_generic_args->generics) {
            if (generic_->identifiers.size() != 1)
                this->exceptions.emplace_back(U"Could not have multiple identifiers as a generic "
                                              U"argument name in the function declaration",
                                              generic_->index);
            if (!generic_->generics.empty())
                this->exceptions.emplace_back(U"Could not have generic arguments in a generic "
                                              U"argument in the function declaration",
                                              generic_->generics[0]->index);

            generic_args.push_back(generic_->identifiers.empty() ? U"" : generic_->identifiers[0]);
        }
    }
    /* The case where it's a returning anonymous/no-name functions/lambdas `def int &() {}` */
    else if (token.type == kh::TokenType::OPERATOR &&
             token.value.operator_type == kh::Operator::BIT_AND) {
        return_type = return_type_or_identifiers;
        this->ti++;
    }
    else {
        this->exceptions.emplace_back(U"Unexpected token in the function declaration", token.index);
        goto end;
    }

parseArgs:
    GUARD(0);
    token = this->to();

    /* Ensures it has an opening parentheses */
    if (!(token.type == kh::TokenType::SYMBOL &&
          token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)) {
        this->exceptions.emplace_back(
            U"Was expecting an opening parentheses of the argument(s) in the function declaration",
            token.index);
        goto end;
    }

    this->ti++;
    GUARD(0);
    token = this->to();

    /* Loops until it reaches a closing parentheses */
    while (true) {
        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            break;

        /* Parses the argument */
        arguments.emplace_back(this->parseDeclaration(false, false));

        GUARD(0);
        token = this->to();

        if (token.type == kh::TokenType::SYMBOL) {
            /* Continues on parsing an argument if there's a comma */
            if (token.value.symbol_type == kh::Symbol::COMMA) {
                this->ti++;
                GUARD(0);
                continue;
            }
            /* Stops parsing arguments */
            else if (token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
                break;
            else {
                this->exceptions.emplace_back(U"Was expecting a closing parentheses or a comma in "
                                              U"the function declaration's argument(s)",
                                              token.index);
                goto end;
            }
        }
        else {
            this->exceptions.emplace_back(U"Was expecting a closing parentheses or a comma in the "
                                          U"function declaration's argument(s)",
                                          token.index);
            goto end;
        }
    }

    this->ti++;
    GUARD(0);

    /* Parses the function's body */
    body = this->parseBody();
end:
    return new kh::AstFunctionExpression(index, identifiers, generic_args, return_type,
                                         return_ref_depth, arguments, body, is_static, is_public);
}

kh::AstDeclarationExpression* kh::Parser::parseDeclaration(const bool is_static, const bool is_public) {
    std::shared_ptr<kh::AstIdentifierExpression> var_type;
    std::u32string var_name;
    std::shared_ptr<kh::AstExpression> expression = nullptr;
    size_t ref_depth = 0;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Counts the variable's type ref depth: `ref ref int x = 3` Declares a reference of a reference
     * of an int variable */
    while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
        ref_depth++;
        this->ti++;
        GUARD(0);
        token = this->to();
    }

    /* Parses the variable's type */
    var_type.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());

    /* Gets the variable's name */
    GUARD(0);
    token = this->to();
    if (token.type != kh::TokenType::IDENTIFIER) {
        this->exceptions.emplace_back(U"Was expecting an identifier of the variable declaration's name",
                                      token.index);
        goto end;
    }

    var_name = token.value.identifier;
    this->ti++;
    GUARD(0);
    token = this->to();

    /* The case where: `SomeClass x(1, 2, 3)` */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)
        expression.reset(this->parseTuple());
    /* The case where: `int x = 3` */
    else if (token.type == kh::TokenType::OPERATOR &&
             token.value.operator_type == kh::Operator::ASSIGN) {
        this->ti++;
        GUARD(0);
        expression.reset(this->parseExpression());
    }
    else
        goto end;
end:
    return new kh::AstDeclarationExpression(index, var_type, var_name, expression, ref_depth, is_static,
                                            is_public);
}

kh::AstClass* kh::Parser::parseClass() {
    std::u32string name;
    std::shared_ptr<kh::AstIdentifierExpression> base;
    std::vector<std::u32string> generic_args;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> members;
    std::vector<std::shared_ptr<kh::AstFunctionExpression>> methods;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Gets the class name */
    if (token.type == kh::TokenType::IDENTIFIER) {
        name = token.value.identifier;
        this->ti++;
    }
    else
        this->exceptions.emplace_back(
            U"Was expecting an identifier of the class name in the class declaration", token.index);

    GUARD(0);
    token = this->to();

    /* Optional generic arguments */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        /* Parses the generic argument(s') identifier */
        while (true) {
            /* Appends generic argument's identifier name */
            if (token.type == kh::TokenType::IDENTIFIER)
                generic_args.push_back(token.value.identifier);
            /* Stops parsing generic argument(s) once there's a generic close */
            else if (token.type == kh::TokenType::SYMBOL &&
                     token.value.symbol_type == kh::Symbol::GENERIC_CLOSE) {
                this->ti++;
                break;
            }

            this->ti++;
            GUARD(0);
            token = this->to();

            /* Stops parsing generic argument(s) once there's a generic close */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::GENERIC_CLOSE) {
                this->ti++;
                break;
            }
            /* Continues parsing generic argument(s) when there's a comma (The statement below is
             * negated) */
            else if (!(token.type == kh::TokenType::SYMBOL &&
                       token.value.symbol_type == kh::Symbol::COMMA)) {
                this->exceptions.emplace_back(U"Was expecting a comma or a generic close in the "
                                              U"generic argument(s') class declaration",
                                              token.index);
                break;
            }

            this->ti++;
            GUARD(0);
            token = this->to();
        }
    }

    GUARD(0);
    token = this->to();

    /* Optional inheriting */
    if (token.type == kh::TokenType::SYMBOL &&
        token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        this->ti++;
        GUARD(0);

        /* Parses base class' identifier */
        base.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());
        GUARD(0);
        token = this->to();

        /* Expects a closing parentheses */
        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            this->ti++;
        else
            this->exceptions.emplace_back(U"Was expecting a closing parentheses after inheritment "
                                          U"argument in the class declaration",
                                          token.index);
    }

    GUARD(0);
    token = this->to();

    /* Parses the body */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        while (true) {
            GUARD(0);
            kh::Token& token = this->to();

            switch (token.type) {
                case kh::TokenType::IDENTIFIER: {
                    /* Methods */
                    if (token.value.identifier == U"def") {
                        this->ti++;
                        GUARD(0);

                        bool is_static, is_public;
                        /* Parse access types */
                        this->parseAccessAttribs(is_static, is_public);
                        GUARD(0);
                        /* Parse function declaration */
                        methods.emplace_back(this->parseFunction(is_static, is_public));
                    }
                    /* Member/class variables */
                    else {
                        bool is_static, is_public;
                        /* Parse access types */
                        this->parseAccessAttribs(is_static, is_public);
                        GUARD(0);
                        /* Parse variable declaration */
                        members.emplace_back(this->parseDeclaration(is_static, is_public));

                        GUARD(0);
                        token = this->to();
                        /* Expects semicolon */
                        if (token.type == kh::TokenType::SYMBOL &&
                            token.value.symbol_type == kh::Symbol::SEMICOLON)
                            this->ti++;
                        else {
                            this->ti++;
                            this->exceptions.emplace_back(U"Was expecting a semicolon after a "
                                                          U"variable declaration in the class body",
                                                          token.index);
                        }
                    }
                } break;

                case kh::TokenType::SYMBOL: {
                    switch (token.value.symbol_type) {
                        /* Placeholder "does nothing" semicolon */
                        case kh::Symbol::SEMICOLON: {
                            this->ti++;
                        } break;

                        /* Closing curly bracket where it ends the class body */
                        case kh::Symbol::CURLY_CLOSE: {
                            this->ti++;
                            goto end;
                        } break;

                        default:
                            this->ti++;
                            this->exceptions.emplace_back(
                                U"Unexpected symbol while parsing the class body", token.index);
                    }
                } break;

                default:
                    this->ti++;
                    this->exceptions.emplace_back(U"Unexpected token while parsing the class body",
                                                  token.index);
            }
        }
    }
    else
        this->exceptions.emplace_back(
            U"Was expecting an opening curly bracket after the class declaration", token.index);
end:
    return new kh::AstClass(index, name, base, generic_args, members, methods);
}

kh::AstStruct* kh::Parser::parseStruct() {
    std::u32string name;
    std::shared_ptr<kh::AstIdentifierExpression> base;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> members;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Gets the struct name */
    if (token.type == kh::TokenType::IDENTIFIER) {
        name = token.value.identifier;
        this->ti++;
    }
    else
        this->exceptions.emplace_back(
            U"Was expecting an identifier of the struct name in the struct declaration", token.index);

    GUARD(0);
    token = this->to();

    /* Optional struct inheritment */
    if (token.type == kh::TokenType::SYMBOL &&
        token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        this->ti++;
        GUARD(0);

        /* Parses the base class' identifier */
        base.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());
        GUARD(0);
        token = this->to();
        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            this->ti++;
        else
            this->exceptions.emplace_back(U"Was expecting a closing parentheses after the "
                                          U"inheritment argument in the struct declaration",
                                          token.index);
    }

    GUARD(0);
    token = this->to();

    /* Ensures opening curly bracket */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        /* Parses the struct body which continuously parse variables */
        while (true) {
            /* Stops parsing the body */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            /* Passes through placeholder semicolon */
            else if (token.type == kh::TokenType::SYMBOL &&
                     token.value.symbol_type == kh::Symbol::SEMICOLON) {
                this->ti++;
                GUARD(0);
                token = this->to();
                continue;
            }
            /* Parses the member/static variable */
            else {
                bool is_static, is_public;
                this->parseAccessAttribs(is_static, is_public);
                members.emplace_back(this->parseDeclaration(is_static, is_public));
            }

            GUARD(0);
            token = this->to();

            /* Stops parsing the body */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            /* Ensures there's a semicolon ending each variable declaration */
            else if (!(token.type == kh::TokenType::SYMBOL &&
                       token.value.symbol_type == kh::Symbol::SEMICOLON))
                this->exceptions.emplace_back(
                    U"Was expecting a semicolon after the variable declaration in the struct body",
                    token.index);

            this->ti++;
            GUARD(0);
            token = this->to();
        }
    }
    else
        this->exceptions.emplace_back(
            U"Was expecting an opening curly bracket after the struct declaration", token.index);
end:
    return new kh::AstStruct(index, name, base, members);
}

kh::AstEnum* kh::Parser::parseEnum() {
    std::u32string name;
    std::shared_ptr<kh::AstIdentifierExpression> base;
    std::vector<std::u32string> members;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Gets the enum name */
    if (token.type == kh::TokenType::IDENTIFIER) {
        name = token.value.identifier;
        this->ti++;
    }
    else
        this->exceptions.emplace_back(
            U"Was expecting an identifier of the enum name in the enum declaration", token.index);

    GUARD(0);
    token = this->to();

    /* Optional enum inheritment */
    if (token.type == kh::TokenType::SYMBOL &&
        token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        this->ti++;
        GUARD(0);

        /* Parses the base enum identifier */
        base.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());
        GUARD(0);
        token = this->to();
        /* Ensures it's ended with a closing parentheses */
        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            this->ti++;
        else
            this->exceptions.emplace_back(U"Was expecting a closing parentheses after the "
                                          U"inheritment argument in the enum declaration",
                                          token.index);
    }

    GUARD(0);
    token = this->to();

    /* Opens with a curly bracket */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        /* Parses the enum content */
        while (true) {
            /* Stops parsing enum body */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            /* Appends member */
            else if (token.type == kh::TokenType::IDENTIFIER)
                members.push_back(token.value.identifier);

            this->ti++;
            GUARD(0);
            token = this->to();

            /* Stops parsing enum body */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            /* Ensures a comma after an enum member */
            else if (!(token.type == kh::TokenType::SYMBOL &&
                       token.value.symbol_type == kh::Symbol::COMMA))
                this->exceptions.emplace_back(U"Was expecting a closing curly bracket or a comma "
                                              U"after an enum member in the enum body",
                                              token.index);

            this->ti++;
            GUARD(0);
            token = this->to();
        }
    }
    else
        this->exceptions.emplace_back(
            U"Was expecting an opening curly bracket after the enum declaration", token.index);
end:
    return new kh::AstEnum(index, name, base, members);
}

std::vector<std::shared_ptr<kh::AstBody>> kh::Parser::parseBody() {
    std::vector<std::shared_ptr<kh::AstBody>> body;
    kh::Token token = this->to();

    /* Expects an opening curly bracket */
    if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN)) {
        this->exceptions.emplace_back(U"Was expecting an opening curly bracket", token.index);
        goto end;
    }

    this->ti++;

    /* Parses the body */
    while (true) {
        GUARD(0);
        token = this->to();
        size_t index = token.index;

        switch (token.type) {
            case kh::TokenType::IDENTIFIER: {
                /* Nested function */
                if (token.value.identifier == U"def") {
                    this->ti++;
                    GUARD(0);

                    bool is_static, is_public;
                    this->parseAccessAttribs(is_static, is_public);
                    GUARD(0);
                    body.emplace_back(this->parseFunction(is_static, is_public));
                }
                /* If statement */
                else if (token.value.identifier == U"if") {
                    std::vector<std::shared_ptr<kh::AstExpression>> conditions;
                    std::vector<std::vector<std::shared_ptr<kh::AstBody>>> bodies;
                    std::vector<std::shared_ptr<kh::AstBody>> else_body;

                    do {
                        /* Parses the expression and if body */
                        this->ti++;
                        token = this->to();
                        GUARD(0);
                        conditions.emplace_back(this->parseExpression());
                        GUARD(0);
                        bodies.emplace_back(this->parseBody());
                        GUARD(0);
                        token = this->to();

                        /* Recontinues if there's an else if (`elif`) clause */
                    } while (token.type == kh::TokenType::IDENTIFIER &&
                             token.value.identifier == U"elif");

                    /* Parses the body if there's an `else` clause */
                    if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"else") {
                        this->ti++;
                        GUARD(0);
                        else_body = this->parseBody();
                    }

                    body.emplace_back(new kh::AstIf(index, conditions, bodies, else_body));
                }
                /* While statement */
                else if (token.value.identifier == U"while") {
                    this->ti++;
                    GUARD(0);

                    /* Parses the expression and body */
                    std::shared_ptr<kh::AstExpression> condition(this->parseExpression());
                    std::vector<std::shared_ptr<kh::AstBody>> while_body = this->parseBody();

                    body.emplace_back(new kh::AstWhile(index, condition, while_body));
                }
                /* Do while statement */
                else if (token.value.identifier == U"do") {
                    this->ti++;
                    GUARD(0);

                    /* Parses the body */
                    std::vector<std::shared_ptr<kh::AstBody>> do_while_body = this->parseBody();
                    std::shared_ptr<kh::AstExpression> condition;

                    GUARD(0);
                    token = this->to();

                    /* Expects `while` and then parses the condition expression */
                    if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"while") {
                        this->ti++;
                        condition.reset(this->parseExpression());
                    }
                    else
                        this->exceptions.emplace_back(
                            U"Was expecting `while` after the `do {...}` body", token.index);

                    GUARD(0);
                    token = this->to();

                    /* Expects a semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->exceptions.emplace_back(
                            U"Was expecting a semicolon after the `do {...} while (...)` statement",
                            token.index);

                    body.emplace_back(new kh::AstDoWhile(index, condition, do_while_body));
                }
                /* For statement */
                else if (token.value.identifier == U"for") {
                    this->ti++;
                    GUARD(0);

                    std::vector<std::shared_ptr<kh::AstExpression>> targets;
                    std::shared_ptr<kh::AstExpression> iterator;
                    std::vector<std::shared_ptr<kh::AstBody>> for_body;

                    /* Parses the iterator target(s) */
                    while (true) {
                        /* Parses target expression */
                        targets.emplace_back(this->parseExpression());

                        GUARD(0);
                        kh::Token token = this->to();
                        if (token.type == kh::TokenType::SYMBOL) {
                            switch (token.value.symbol_type) {
                                /* Another target */
                                case kh::Symbol::COMMA:
                                    this->ti++;
                                    GUARD(0);
                                    break;

                                /* Now expecting iterator expression */
                                case kh::Symbol::COLON:
                                    this->ti++;
                                    GUARD(0);
                                    goto _break_while;

                                default:
                                    this->exceptions.emplace_back(U"Was expecting a colon after "
                                                                  U"the iterator target expression",
                                                                  token.index);
                            }
                        }
                        else
                            this->exceptions.emplace_back(
                                U"Was expecting a colon after the iterator target expression",
                                token.index);
                    }
                _break_while:

                    /* Parses the iterator expression and for body */
                    iterator.reset(this->parseExpression());
                    for_body = this->parseBody();

                    body.emplace_back(new kh::AstFor(index, targets, iterator, for_body));
                }
                /* `continue` statement */
                else if (token.value.identifier == U"continue") {
                    this->ti++;
                    GUARD(0);

                    /* Placeholder expression */
                    std::shared_ptr<kh::AstExpression> expression((kh::AstExpression*)nullptr);
                    token = this->to();

                    /* Expects semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->exceptions.emplace_back(U"Was expecting a semicolon after `continue`",
                                                      token.index);

                    body.emplace_back(
                        new kh::AstStatement(index, kh::AstStatement::Type::CONTINUE, expression));
                }
                /* `break` statement */
                else if (token.value.identifier == U"break") {
                    this->ti++;
                    GUARD(0);

                    /* Placeholder expression */
                    std::shared_ptr<kh::AstExpression> expression((kh::AstExpression*)nullptr);
                    token = this->to();

                    /* Expects semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->exceptions.emplace_back(U"Was expecting a semicolon after `break`",
                                                      token.index);

                    body.emplace_back(
                        new kh::AstStatement(index, kh::AstStatement::Type::BREAK, expression));
                }
                /* `return` statement */
                else if (token.value.identifier == U"return") {
                    this->ti++;
                    GUARD(0);
                    token = this->to();

                    std::shared_ptr<kh::AstExpression> expression((kh::AstExpression*)nullptr);

                    /* No expression given */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    /* If there's a provided return value expression */
                    else {
                        expression.reset(this->parseExpression());
                        GUARD(0);
                        token = this->to();

                        /* Expects semicolon */
                        if (token.type == kh::TokenType::SYMBOL &&
                            token.value.symbol_type == kh::Symbol::SEMICOLON)
                            this->ti++;
                        else
                            this->exceptions.emplace_back(
                                U"Was expecting a semicolon after the `return` statement", token.index);
                    }

                    body.emplace_back(
                        new kh::AstStatement(index, kh::AstStatement::Type::RETURN, expression));
                }
                else
                    goto parse_expr;
            } break;

            case kh::TokenType::SYMBOL:
                switch (token.value.symbol_type) {
                    /* Placeholder semicolon */
                    case kh::Symbol::SEMICOLON: {
                        this->ti++;
                    } break;

                    /* Ends body */
                    case kh::Symbol::CURLY_CLOSE: {
                        this->ti++;
                        goto end;
                    } break;

                    /* VM instruction statement */
                    case kh::Symbol::DOLLAR: {
                        this->ti++;
                        GUARD(0);
                        token = this->to();

                        std::u32string op_name;
                        std::vector<std::shared_ptr<kh::AstExpression>> op_args;

                        /* Expects instruction op-name identifier */
                        if (token.type == kh::TokenType::IDENTIFIER)
                            op_name = token.value.identifier;
                        else
                            this->exceptions.emplace_back(U"Was expecting an identifier of the op-name "
                                                          U"of the instruction statement",
                                                          token.index);

                        this->ti++;
                        GUARD(0);
                        token = this->to();

                        /* Parses through the arguments of the VM instruction */
                        while (true) {
                            /* Ends */
                            if (token.type == kh::TokenType::SYMBOL &&
                                token.value.symbol_type == kh::Symbol::SEMICOLON)
                                break;

                            /* Parses the argument expression */
                            op_args.emplace_back(this->parseExpression());

                            GUARD(0);
                            token = this->to();

                            /* Ends */
                            if (token.type == kh::TokenType::SYMBOL &&
                                token.value.symbol_type == kh::Symbol::SEMICOLON)
                                break;
                            /* Continues to parse arguments if there's a comma (This statement below
                             * negates it) */
                            else if (!(token.type == kh::TokenType::SYMBOL &&
                                       token.value.symbol_type == kh::Symbol::COMMA)) {
                                this->exceptions.emplace_back(U"Was expecting a semicolon or a comma",
                                                              token.index);
                                break;
                            }

                            this->ti++;
                            GUARD(0);
                            token = this->to();
                        }

                        this->ti++;

                        body.emplace_back(new kh::AstInstruction(index, op_name, op_args));
                    } break;

                    default:
                        goto parse_expr;
                }
                break;

            default:
            parse_expr : {
                /* If it isn't any of the statements above, it's probably an expression */
                std::shared_ptr<kh::AstExpression> expr(this->parseExpression());
                GUARD(0);
                token = this->to();

                /* Expects a semicolon */
                if (token.type == kh::TokenType::SYMBOL &&
                    token.value.symbol_type == kh::Symbol::SEMICOLON)
                    this->ti++;
                else
                    this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);

                body.emplace_back(expr);
            }
        }
    }
end:
    return body;
}

kh::AstExpression* kh::Parser::parseExpression() {
    kh::Token token = this->to();
    size_t index = token.index;

    /* Function expression */
    if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"def") {
        bool is_static, is_public;
        this->ti++;
        GUARD(0);
        this->parseAccessAttribs(is_static, is_public);
        GUARD(0);
        return this->parseFunction(is_static, is_public);
    }
    /* Go through "actual" expression, starts recursive descent */
    else
        return this->parseAssignOps();

end:
    return nullptr;
}

kh::AstExpression* kh::Parser::parseAssignOps() {
    kh::AstExpression* expr = this->parseTernary();
    kh::Token token;
    size_t index;

    GUARD(0);
    token = this->to();
    index = token.index;

    while (token.type == kh::TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case kh::Operator::ASSIGN:
            case kh::Operator::IADD:
            case kh::Operator::ISUB:
            case kh::Operator::IMUL:
            case kh::Operator::IDIV:
            case kh::Operator::IMOD:
            case kh::Operator::IPOW: {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> lval(expr);
                std::shared_ptr<kh::AstExpression> rval(this->parseTernary());
                expr = new kh::AstBinaryExpression(token.index, token.value.operator_type, lval, rval);
            } break;

            default:
                goto end;
        }

        GUARD(0);
        token = this->to();
    }
end:
    return expr;
}

kh::AstExpression* kh::Parser::parseTernary() {
    kh::AstExpression* expr = this->parseOr();
    kh::Token token;
    size_t index;

    GUARD(0);
    token = this->to();
    index = token.index;

    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::QUESTION) {
        index = token.index;

        this->ti++;
        GUARD(0);

        std::shared_ptr<kh::AstExpression> value(this->parseOr());

        GUARD(0);
        token = this->to();

        if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COLON)) {
            this->exceptions.emplace_back(U"Was expecting a colon to continue the ternary expression",
                                          token.index);
            goto end;
        }

        this->ti++;
        GUARD(0);

        std::shared_ptr<kh::AstExpression> condition(expr);
        std::shared_ptr<kh::AstExpression> otherwise(this->parseOr());
        expr = new kh::AstTernaryExpression(index, condition, value, otherwise);

        GUARD(0);
        token = this->to();
    }
end:
    return expr;
}

#define RECURSIVE_DESCENT_SINGULAR_OP(lower, op)                                                    \
    do {                                                                                            \
        kh::AstExpression* expr = lower();                                                          \
        kh::Token token;                                                                            \
        size_t index;                                                                               \
        GUARD(0);                                                                                   \
        token = this->to();                                                                         \
        index = token.index;                                                                        \
        while (token.type == kh::TokenType::OPERATOR && token.value.operator_type == op) {          \
            this->ti++;                                                                             \
            GUARD(0);                                                                               \
            std::shared_ptr<kh::AstExpression> lval(expr);                                          \
            std::shared_ptr<kh::AstExpression> rval(lower());                                       \
            expr = new kh::AstBinaryExpression(token.index, token.value.operator_type, lval, rval); \
            GUARD(0);                                                                               \
            token = this->to();                                                                     \
        }                                                                                           \
        GUARD(0);                                                                                   \
        token = this->to();                                                                         \
    end:                                                                                            \
        return expr;                                                                                \
    } while (false)

kh::AstExpression* kh::Parser::parseOr() {
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseAnd, kh::Operator::OR);
}

kh::AstExpression* kh::Parser::parseAnd() {
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseComparison, kh::Operator::AND);
}

kh::AstExpression* kh::Parser::parseComparison() {
    kh::AstExpression* expr = this->parseBitwiseOr();
    kh::Token token;
    size_t index;

    GUARD(0);
    token = this->to();
    index = token.index;

    while (token.type == kh::TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case kh::Operator::EQUAL:
            case kh::Operator::NOT_EQUAL:
            case kh::Operator::LESS:
            case kh::Operator::LESS_EQUAL:
            case kh::Operator::MORE:
            case kh::Operator::MORE_EQUAL: {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> lval(expr);
                std::shared_ptr<kh::AstExpression> rval(this->parseBitwiseOr());
                expr = new kh::AstBinaryExpression(token.index, token.value.operator_type, lval, rval);
            } break;

            default:
                goto end;
        }

        GUARD(0);
        token = this->to();
    }
end:
    return expr;
}

kh::AstExpression* kh::Parser::parseBitwiseOr() {
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseBitwiseAnd, kh::Operator::BIT_OR);
}

kh::AstExpression* kh::Parser::parseBitwiseAnd() {
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseBitwiseShift, kh::Operator::BIT_AND);
}

kh::AstExpression* kh::Parser::parseBitwiseShift() {
    kh::AstExpression* expr = this->parseAddSub();
    kh::Token token;
    size_t index;

    GUARD(0);
    token = this->to();
    index = token.index;

    while (token.type == kh::TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case kh::Operator::BIT_LSHIFT:
            case kh::Operator::BIT_RSHIFT: {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> lval(expr);
                std::shared_ptr<kh::AstExpression> rval(this->parseAddSub());
                expr = new kh::AstBinaryExpression(token.index, token.value.operator_type, lval, rval);
            } break;

            default:
                goto end;
        }

        GUARD(0);
        token = this->to();
    }
end:
    return expr;
}

kh::AstExpression* kh::Parser::parseAddSub() {
    kh::AstExpression* expr = this->parseMulDivMod();
    kh::Token token;
    size_t index;

    GUARD(0);
    token = this->to();
    index = token.index;

    while (token.type == kh::TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case kh::Operator::ADD:
            case kh::Operator::SUB: {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> lval(expr);
                std::shared_ptr<kh::AstExpression> rval(this->parseMulDivMod());
                expr = new kh::AstBinaryExpression(token.index, token.value.operator_type, lval, rval);
            } break;

            default:
                goto end;
        }

        GUARD(0);
        token = this->to();
    }
end:
    return expr;
}

kh::AstExpression* kh::Parser::parseMulDivMod() {
    kh::AstExpression* expr = this->parseUnary();
    kh::Token token;
    size_t index;

    GUARD(0);
    token = this->to();
    index = token.index;

    while (token.type == kh::TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case kh::Operator::MUL:
            case kh::Operator::DIV:
            case kh::Operator::MOD: {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> lval(expr);
                std::shared_ptr<kh::AstExpression> rval(this->parseUnary());

                expr = new kh::AstBinaryExpression(token.index, token.value.operator_type, lval, rval);
            } break;

            default:
                goto end;
        }

        GUARD(0);
        token = this->to();
    }
end:
    return expr;
}

kh::AstExpression* kh::Parser::parseUnary() {
    kh::AstExpression* expr = nullptr;
    kh::Token token = this->to();
    size_t index = token.index;

    if (token.type == kh::TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case kh::Operator::ADD:
            case kh::Operator::SUB:
            case kh::Operator::ADDRESS: {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> rval(this->parseUnary());
                expr = new kh::AstUnaryExpression(token.index, token.value.operator_type, rval);
            } break;

            default:
                this->exceptions.emplace_back(U"Unexpected operator after a unary operator",
                                              token.index);
        }
    }
    else
        expr = this->parseExponentiation();

end:
    return expr;
}

kh::AstExpression* kh::Parser::parseExponentiation() {
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseLiteral, kh::Operator::POW);
}

kh::AstExpression* kh::Parser::parseLiteral() {
    kh::AstExpression* expr = nullptr;
    kh::Token token = this->to();
    size_t index = token.index;

    switch (token.type) {
            /* For all of these literal values be given the AST constant value instance */

        case kh::TokenType::CHARACTER:
            expr = new kh::AstConstValue(token.index, token.value.character);
            this->ti++;
            break;

        case kh::TokenType::UINTEGER:
            expr = new kh::AstConstValue(token.index, token.value.uinteger);
            this->ti++;
            break;

        case kh::TokenType::INTEGER:
            expr = new kh::AstConstValue(token.index, token.value.integer);
            this->ti++;
            break;

        case kh::TokenType::FLOATING:
            expr = new kh::AstConstValue(token.index, token.value.floating);
            this->ti++;
            break;

        case kh::TokenType::STRING:
            expr = new kh::AstConstValue(token.index, token.value.string);
            this->ti++;

            GUARD(0);
            token = this->to();

            while (token.type == kh::TokenType::STRING) {
                ((kh::AstConstValue*)expr)->string += token.value.string;
                this->ti++;
                GUARD(0);
                token = this->to();
            }

            break;

        case kh::TokenType::BUFFER:
            expr = new kh::AstConstValue(token.index, token.value.buffer);
            this->ti++;

            GUARD(0);
            token = this->to();

            while (token.type == kh::TokenType::BUFFER) {
                ((kh::AstConstValue*)expr)->buffer += token.value.buffer;
                this->ti++;
                GUARD(0);
                token = this->to();
            }

            break;

        case kh::TokenType::IDENTIFIER:
            if (token.value.identifier == U"ref" || token.value.identifier == U"static" ||
                token.value.identifier == U"private" || token.value.identifier == U"public") {
                bool is_static, is_public;
                this->parseAccessAttribs(is_static, is_public);
                GUARD(0);
                return this->parseDeclaration(is_static, is_public);
            }
            else {
                expr = this->parseIdentifiers();

                GUARD(0);
                token = this->to();

                if (token.type == kh::TokenType::IDENTIFIER) {
                    std::shared_ptr<kh::AstIdentifierExpression> var_type(
                        (kh::AstIdentifierExpression*)expr);
                    std::u32string var_name = token.value.identifier;
                    std::shared_ptr<kh::AstExpression> expression;

                    this->ti++;
                    GUARD(0);
                    token = this->to();

                    /* The case where: `SomeClass x(1, 2, 3)` */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)
                        expression.reset(this->parseTuple());
                    /* The case where: `int x = 3` */
                    else if (token.type == kh::TokenType::OPERATOR &&
                             token.value.operator_type == kh::Operator::ASSIGN) {
                        this->ti++;
                        GUARD(0);
                        expression.reset(this->parseExpression());
                    }

                    return new kh::AstDeclarationExpression(index, var_type, var_name, expression,
                                                            (size_t)0, false, true);
                }
            }
            break;

        case kh::TokenType::SYMBOL:
            switch (token.value.symbol_type) {
                /* Parentheses/tuple expression */
                case kh::Symbol::PARENTHESES_OPEN: {
                    kh::AstTupleExpression* tuple = (kh::AstTupleExpression*)this->parseTuple();

                    if (tuple->elements.size() == 1) {
                        expr = tuple->elements[0].get();

                        /* Sketchy solution but hope this works */
                        std::shared_ptr<kh::AstExpression>* dummy_ptr =
                            new std::shared_ptr<kh::AstExpression>(tuple->elements[0]);
                        std::free((void*)dummy_ptr);
                    }
                    else
                        expr = tuple;
                } break;

                default:
                    this->exceptions.emplace_back(U"Unexpected token for a literal", token.index);
                    goto end;
            }
            break;

        default:
            this->exceptions.emplace_back(U"Unexpected token for a literal", token.index);
            goto end;
    }

    GUARD(0);
    token = this->to();

    while (token.type == kh::TokenType::SYMBOL &&
           (token.value.symbol_type == kh::Symbol::DOT ||
            token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN ||
            token.value.symbol_type == kh::Symbol::SQUARE_OPEN)) {
        index = token.index;

        switch (token.value.symbol_type) {
            /* Scoping expression */
            case kh::Symbol::DOT: {
                std::vector<std::u32string> identifiers;

                do {
                    this->ti++;
                    GUARD(0);
                    token = this->to();

                    /* Expects an identifier for which to be scoped through from the expression */
                    if (token.type == kh::TokenType::IDENTIFIER)
                        identifiers.push_back(token.value.identifier);
                    else
                        this->exceptions.emplace_back(U"Was expecting an identifier", token.index);

                    this->ti++;
                    GUARD(0);
                    token = this->to();

                    /* Continues again for another scope in */
                } while (token.type == kh::TokenType::SYMBOL &&
                         token.value.symbol_type == kh::Symbol::DOT);

                std::shared_ptr<kh::AstExpression> exprptr(expr);
                expr = new kh::AstScopeExpression(index, exprptr, identifiers);
            } break;

            /* Calling expression */
            case kh::Symbol::PARENTHESES_OPEN: {
                std::shared_ptr<kh::AstExpression> exprptr(expr);
                /* Parses the argument(s) */
                kh::AstTupleExpression* tuple = (kh::AstTupleExpression*)this->parseTuple();
                std::vector<std::shared_ptr<kh::AstExpression>> arguments;

                for (std::shared_ptr<kh::AstExpression>& element : tuple->elements)
                    arguments.push_back(element);

                expr = new kh::AstCallExpression(index, exprptr, arguments);
                delete tuple;
            } break;

            /* Subscription expression */
            case kh::Symbol::SQUARE_OPEN: {
                std::shared_ptr<kh::AstExpression> exprptr(expr);
                /* Parses argument(s) */
                kh::AstTupleExpression* tuple = (kh::AstTupleExpression*)this->parseTuple(
                    kh::Symbol::SQUARE_OPEN, kh::Symbol::SQUARE_CLOSE);
                std::vector<std::shared_ptr<kh::AstExpression>> arguments;

                for (std::shared_ptr<kh::AstExpression>& element : tuple->elements)
                    arguments.push_back(element);

                expr = new kh::AstSubscriptExpression(index, exprptr, arguments);
                delete tuple;
            } break;
        }

        GUARD(0);
        token = this->to();
    }
end:
    return expr;
}

kh::AstExpression* kh::Parser::parseIdentifiers() {
    std::vector<std::u32string> identifiers;
    std::vector<std::shared_ptr<kh::AstIdentifierExpression>> generics;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Expects an identifier */
    if (token.type == kh::TokenType::IDENTIFIER) {
        identifiers.push_back(token.value.identifier);
        this->ti++;
    }
    else {
        this->exceptions.emplace_back(U"Was expecting an identifier", token.index);
        goto end;
    }

    GUARD(0);
    token = this->to();

    /* For each scope in with a dot symbol */
    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        this->ti++;
        GUARD(0);
        token = this->to();

        /* Appends the identifier */
        if (token.type == kh::TokenType::IDENTIFIER)
            identifiers.push_back(token.value.identifier);
        else {
            this->exceptions.emplace_back(U"Was expecting an identifier after the dot", token.index);
            goto end;
        }

        this->ti++;
        GUARD(0);
        token = this->to();
    }

    /* Optional generic-sation */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        /* Instant close */
        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_CLOSE)
            this->ti++;
        else {
            /* Parses the generic-sation type arguments */
            generics.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());
            GUARD(0);
            token = this->to();

            while (token.type == kh::TokenType::SYMBOL &&
                   token.value.symbol_type == kh::Symbol::COMMA) {
                this->ti++;
                GUARD(0);

                generics.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());

                GUARD(0);
                token = this->to();
            }

            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::GENERIC_CLOSE)
                this->ti++;
            else
                this->exceptions.emplace_back(U"Was expecting a generic close", token.index);
        }
    }
end:
    return new kh::AstIdentifierExpression(index, identifiers, generics);
}

kh::AstExpression* kh::Parser::parseTuple(const kh::Symbol opening, const kh::Symbol closing) {
    std::vector<std::shared_ptr<kh::AstExpression>> elements;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Expects the opening symbol */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == opening) {
        this->ti++;
        GUARD(0);
        token = this->to();

        bool skip_parentheses = true;
        while (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing)) {
            /* Parses the element expression */
            elements.emplace_back(this->parseExpression());
            GUARD(0);
            token = this->to();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing) {
                this->ti++;
                skip_parentheses = false;
                break;
            }
            else if (!(token.type == kh::TokenType::SYMBOL &&
                       token.value.symbol_type == kh::Symbol::COMMA)) {
                this->exceptions.emplace_back(closing == kh::Symbol::SQUARE_CLOSE
                                                  ? U"Was expecting a comma or a closing square bracket"
                                                  : U"Was expecting a comma or a closing parentheses",
                                              token.index);
                break;
            }

            this->ti++;
            GUARD(0);
            token = this->to();
        }

        if (skip_parentheses)
            this->ti++;
    }
    else
        this->exceptions.emplace_back(opening == kh::Symbol::SQUARE_OPEN
                                          ? U"Was expecting an opening square bracket"
                                          : U"Was expecting an opening parentheses",
                                      token.index);

end:
    return new kh::AstTupleExpression(index, elements);
}
