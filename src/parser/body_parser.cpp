/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <chrono>
#include <functional>

#include <kithare/parser.hpp>


void kh::Parser::parse() {
    this->parse_exceptions.clear();
    auto parse_start = std::chrono::high_resolution_clock::now();

    std::vector<std::shared_ptr<kh::AstImport>> imports;
    std::vector<std::shared_ptr<kh::AstFunctionExpression>> functions;
    std::vector<std::shared_ptr<kh::AstUserType>> user_types;
    std::vector<std::shared_ptr<kh::AstEnumType>> enums;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> variables;

    for (this->ti = 0; this->ti < this->tokens.size();) {
        kh::Token& token = this->to();

        switch (token.type) {
            case kh::TokenType::IDENTIFIER: {
                const std::u32string& identifier = token.value.identifier;

                /* Function declaration identifier keyword */
                if (identifier == U"def" || identifier == U"try") {
                    /* Skips initial keyword */
                    this->ti++;
                    KH_PARSE_GUARD();

                    /* Case for conditional functions */
                    bool conditional = false;
                    if (identifier == U"try") {
                        conditional = true;
                        token = this->to();
                        if (token.type == kh::TokenType::IDENTIFIER &&
                            token.value.identifier == U"def") {
                            this->ti++;
                            KH_PARSE_GUARD();
                        }
                        else
                            this->parse_exceptions.emplace_back(
                                U"Was expecting `def` after `try` at the top scope", token);
                    }

                    /* Parses access type */
                    bool is_static, is_public;
                    this->parseAccessAttribs(is_static, is_public);

                    KH_PARSE_GUARD();
                    /* Parses return type, name, arguments, and body */
                    functions.emplace_back(this->parseFunction(is_static, is_public, conditional));

                    if (!functions.back()->identifiers.size())
                        this->parse_exceptions.emplace_back(
                            U"Lambda functions cannot be declared at the top scope", token);
                    if (is_static && functions.back()->identifiers.size() == 1)
                        this->parse_exceptions.emplace_back(U"A top scope function cannot be `static`",
                                                            token);
                }
                /* Parses class declaration */
                else if (identifier == U"class") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    user_types.emplace_back(this->parseUserType(true));
                }
                /* Parses struct declaration */
                else if (identifier == U"struct") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    user_types.emplace_back(this->parseUserType(false));
                }
                /* Parses enum declaration */
                else if (identifier == U"enum") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    enums.emplace_back(this->parseEnum());
                }
                /* Parses import statement */
                else if (identifier == U"import") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    imports.emplace_back(this->parseImport(false)); /* is_include = false */
                }
                /* Parses include statement */
                else if (identifier == U"include") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    imports.emplace_back(this->parseImport(true)); /* is_include = true */
                }
                /* If it was none of those above, it's probably a variable declaration */
                else {
                    /* Gets variable's declaration access type */
                    bool is_static, is_public;
                    this->parseAccessAttribs(is_static, is_public);
                    KH_PARSE_GUARD();
                    if (is_static)
                        this->parse_exceptions.emplace_back(U"A top scope variable cannot be `static`",
                                                            token);

                    /* Parses the variable's return type, name, and assignment value */
                    variables.emplace_back(this->parseDeclaration(is_static, is_public));

                    /* Makes sure it ends with a semicolon */
                    KH_PARSE_GUARD();
                    token = this->to();
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->parse_exceptions.emplace_back(
                            U"Was expecting a semicolon after a variable declaration", token);
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
                        this->parse_exceptions.emplace_back(
                            U"Unexpected token while parsing the top scope", token);
                }
                break;

            /* Unknown token */
            default:
                this->ti++;
                this->parse_exceptions.emplace_back(U"Unexpected token while parsing the top scope",
                                                    token);
        }
    }

end:
    /* Removes exceptions that's got a duplicate index */
    if (this->parse_exceptions.size() > 1) {
        size_t last_index = -1;
        std::vector<kh::ParseException> cleaned_exceptions;
        cleaned_exceptions.reserve(this->parse_exceptions.size());

        for (kh::ParseException& exc : this->parse_exceptions) {
            if (last_index != exc.token.index)
                cleaned_exceptions.push_back(exc);

            last_index = exc.token.index;
        }

        this->parse_exceptions = cleaned_exceptions;
    }

    this->ast.reset(new kh::Ast(imports, functions, user_types, enums, variables));

    auto parse_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = parse_end - parse_start;
    this->parse_time = elapsed.count();
}

kh::AstImport* kh::Parser::parseImport(const bool is_include) {
    std::vector<std::u32string> path;
    bool is_relative = false;
    std::u32string identifier;
    kh::Token token = this->to();
    size_t index = token.index;

    std::u32string type = is_include ? U"include" : U"import";

    /* Check if an import/include is relative */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        is_relative = true;
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();
    }

    /* Making sure that it starts with an identifier (an import/include statement must has at least
     * one identifier to be imported) */
    if (token.type == kh::TokenType::IDENTIFIER) {
        if (kh::isReservedKeyword(token.value.identifier))
            this->parse_exceptions.emplace_back(
                U"Was trying to `" + type + U"` with a reserved keyword", token);

        path.push_back(token.value.identifier);
        this->ti++;
    }
    else {
        this->parse_exceptions.emplace_back(
            U"Was expecting an identifier after the `" + type + U"` keyword", token);
        this->ti++;
    }

    KH_PARSE_GUARD();
    token = this->to();

    /* Parses each identifier after a dot `import a.b.c.d ...` */
    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();

        /* Appends the identifier */
        if (token.type == kh::TokenType::IDENTIFIER) {
            if (kh::isReservedKeyword(token.value.identifier))
                this->parse_exceptions.emplace_back(U"Was trying to " + type + U" a reserved keyword",
                                                    token);

            path.push_back(token.value.identifier);
            this->ti++;
            KH_PARSE_GUARD();
            token = this->to();
        }
        else {
            this->parse_exceptions.emplace_back(
                U"Was expecting an identifier after the dot in the " + type + U" statement", token);
            break;
        }
    }

    /* An optional `as` for changing the namespace name in import statements */
    if (!is_include && token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"as") {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();

        /* Gets the set namespace identifier */
        if (token.type == kh::TokenType::IDENTIFIER) {
            if (kh::isReservedKeyword(token.value.identifier))
                this->parse_exceptions.emplace_back(
                    U"Could not use a reserved keyword as the alias of the import", token);

            identifier = token.value.identifier;
        }
        else {
            this->parse_exceptions.emplace_back(
                U"Was expecting an identifier after the `as` keyword in the import statement", token);
        }

        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();
    }

    /* Ensure that it ends with a semicolon */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
        this->ti++;
    else
        this->parse_exceptions.emplace_back(
            U"Was expecting a semicolon after the " + type + U" statement", token);
end:
    return new kh::AstImport(index, path, is_include, is_relative,
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
        KH_PARSE_GUARD();
        token = this->to();
    }

end:
    return;
}

kh::AstFunctionExpression* kh::Parser::parseFunction(const bool is_static, const bool is_public,
                                                     const bool is_conditional) {
    std::vector<std::u32string> identifiers;
    std::vector<std::u32string> generic_args;
    std::shared_ptr<kh::AstIdentifierExpression> return_type;
    std::vector<uint64_t> return_array = {};
    size_t return_refs = 0;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> arguments;
    std::vector<std::shared_ptr<kh::AstBody>> body;

    kh::Token token = this->to();
    size_t index = token.index;

    if (!(token.type == kh::TokenType::SYMBOL &&
          token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)) {
        std::shared_ptr<kh::AstIdentifierExpression> identifiers_with_generics(
            (kh::AstIdentifierExpression*)this->parseIdentifiers());
        identifiers = identifiers_with_generics->identifiers;

        for (auto& generic_ : identifiers_with_generics->generics) {
            if (generic_->identifiers.size() != 1)
                this->parse_exceptions.emplace_back(U"Could not have multiple identifiers as a generic "
                                                    U"argument name in the function declaration",
                                                    this->tokFromIndex(generic_->index));
            if (!generic_->generics.empty())
                this->parse_exceptions.emplace_back(U"Could not have generic arguments in a generic "
                                                    U"argument in the function declaration",
                                                    this->tokFromIndex(generic_->generics[0]->index));

            generic_args.push_back(generic_->identifiers.empty() ? U"" : generic_->identifiers[0]);
        }

        /* Ensures it has an opening parentheses */
        KH_PARSE_GUARD();
        token = this->to();
        if (!(token.type == kh::TokenType::SYMBOL &&
              token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)) {
            this->parse_exceptions.emplace_back(
                U"Was expecting an opening parentheses of the argument(s) in the function declaration",
                token);
            goto end;
        }
    }

    this->ti++;
    KH_PARSE_GUARD();
    token = this->to();

    /* Loops until it reaches a closing parentheses */
    while (true) {
        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            break;

        /* Parses the argument */
        arguments.emplace_back(this->parseDeclaration(false, true));

        KH_PARSE_GUARD();
        token = this->to();

        if (token.type == kh::TokenType::SYMBOL) {
            /* Continues on parsing an argument if there's a comma */
            if (token.value.symbol_type == kh::Symbol::COMMA) {
                this->ti++;
                KH_PARSE_GUARD();
                continue;
            }
            /* Stops parsing arguments */
            else if (token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
                break;
            else {
                this->parse_exceptions.emplace_back(
                    U"Was expecting a closing parentheses or a comma in "
                    U"the function declaration's argument(s)",
                    token);
                goto end;
            }
        }
        else {
            this->parse_exceptions.emplace_back(
                U"Was expecting a closing parentheses or a comma in the "
                U"function declaration's argument(s)",
                token);
            goto end;
        }
    }

    this->ti++;
    KH_PARSE_GUARD();
    token = this->to();

    /* Specifying return type `def function() -> int {}` */
    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::SUB) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();

        if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::MORE) {
            this->ti++;
            KH_PARSE_GUARD();
            token = this->to();

            /* Checks if the return type is a `ref`erence type */
            while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
                return_refs += 1;
                this->ti++;
                KH_PARSE_GUARD();
                token = this->to();
            }

            return_type.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());
            KH_PARSE_GUARD();
            token = this->to();

            /* Array return type */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::SQUARE_OPEN) {
                return_array = this->parseArrayDimension(return_type);
            }
        }
        else {
            return_type.reset(new kh::AstIdentifierExpression(token.index, {U"void"}, {}, {}, {}));

            this->parse_exceptions.emplace_back(U"Was expecting an arrow specifying a return type",
                                                token);
        }
    }
    else {
        return_type.reset(new kh::AstIdentifierExpression(token.index, {U"void"}, {}, {}, {}));
    }

    /* Parses the function's body */
    body = this->parseBody();
end:
    return new kh::AstFunctionExpression(index, identifiers, generic_args, return_array, return_type,
                                         return_refs, arguments, body, is_conditional, is_static,
                                         is_public);
}

kh::AstDeclarationExpression* kh::Parser::parseDeclaration(const bool is_static, const bool is_public) {
    std::shared_ptr<kh::AstIdentifierExpression> var_type;
    std::vector<uint64_t> var_array = {};
    std::u32string var_name;
    std::shared_ptr<kh::AstExpression> expression = nullptr;
    size_t refs = 0;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Checks if the variable type is a `ref`erence type */
    while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
        refs += 1;
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();
    }

    /* Parses the variable's type */
    var_type.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());

    /* Possible array type `float[3] var;` */
    KH_PARSE_GUARD();
    token = this->to();
    var_array = this->parseArrayDimension(var_type);

    /* Gets the variable's name */
    KH_PARSE_GUARD();
    token = this->to();
    if (token.type != kh::TokenType::IDENTIFIER) {
        this->parse_exceptions.emplace_back(
            U"Was expecting an identifier of the variable declaration's name", token);
        goto end;
    }

    if (kh::isReservedKeyword(token.value.identifier))
        this->parse_exceptions.emplace_back(U"Cannot use a reserved keyword as a variable name", token);

    var_name = token.value.identifier;
    this->ti++;
    KH_PARSE_GUARD();
    token = this->to();

    /* The case where: `SomeClass x(1, 2, 3)` */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)
        expression.reset(this->parseTuple());
    /* The case where: `int x = 3` */
    else if (token.type == kh::TokenType::OPERATOR &&
             token.value.operator_type == kh::Operator::ASSIGN) {
        this->ti++;
        KH_PARSE_GUARD();
        expression.reset(this->parseExpression());
    }
    else
        goto end;
end:
    return new kh::AstDeclarationExpression(index, var_type, var_array, var_name, expression, refs,
                                            is_static, is_public);
}

kh::AstUserType* kh::Parser::parseUserType(const bool is_class) {
    std::vector<std::u32string> identifiers;
    std::vector<std::shared_ptr<kh::AstIdentifierExpression>> bases;
    std::vector<std::u32string> generic_args;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> members;
    std::vector<std::shared_ptr<kh::AstFunctionExpression>> methods;

    kh::Token token = this->to();
    size_t index = token.index;

    const std::u32string type_name = is_class ? U"class" : U"struct";

    /* Gets the class identifiers */
    std::shared_ptr<kh::AstIdentifierExpression> identifiers_with_generics(
        (kh::AstIdentifierExpression*)this->parseIdentifiers());
    identifiers = identifiers_with_generics->identifiers;

    for (auto& generic_ : identifiers_with_generics->generics) {
        if (generic_->identifiers.size() != 1)
            this->parse_exceptions.emplace_back(U"Could not have multiple identifiers as a generic "
                                                U"argument name in the " +
                                                    type_name + U" declaration",
                                                this->tokFromIndex(generic_->index));
        if (!generic_->generics.empty())
            this->parse_exceptions.emplace_back(U"Could not have generic arguments in a generic "
                                                U"argument in the " +
                                                    type_name + U" declaration",
                                                this->tokFromIndex(generic_->generics[0]->index));

        generic_args.push_back(generic_->identifiers.empty() ? U"" : generic_->identifiers[0]);
    }

    KH_PARSE_GUARD();
    token = this->to();

    /* Optional inheriting */
    if (token.type == kh::TokenType::SYMBOL &&
        token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();

        /* Parses base class' identifier */
        bases.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());
        KH_PARSE_GUARD();
        token = this->to();

        /* If there is any more bases to inherit */
        while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COMMA) {
            this->ti++;
            KH_PARSE_GUARD();
            bases.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());
            KH_PARSE_GUARD();
            token = this->to();
        }

        /* Expects a closing parentheses */
        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            this->ti++;
        else
            this->parse_exceptions.emplace_back(
                U"Was expecting a closing parentheses after inheritment "
                U"argument in the " +
                    type_name + U" declaration",
                token);
    }

    KH_PARSE_GUARD();
    token = this->to();

    /* Parses the body */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();

        while (true) {
            KH_PARSE_GUARD();
            kh::Token& token = this->to();

            switch (token.type) {
                case kh::TokenType::IDENTIFIER: {
                    /* Methods */
                    if (token.value.identifier == U"def" || token.value.identifier == U"try") {
                        bool conditional = token.value.identifier == U"try";

                        this->ti++;
                        KH_PARSE_GUARD();

                        /* Case for conditional methods */
                        if (conditional) {
                            token = this->to();
                            if (token.type == kh::TokenType::IDENTIFIER &&
                                token.value.identifier == U"def") {
                                this->ti++;
                                KH_PARSE_GUARD();
                            }
                            else
                                this->parse_exceptions.emplace_back(
                                    U"Was expecting `def` after `try` at the top scope", token);
                        }

                        bool is_static, is_public;
                        /* Parse access types */
                        this->parseAccessAttribs(is_static, is_public);
                        KH_PARSE_GUARD();
                        /* Parse function declaration */
                        methods.emplace_back(this->parseFunction(is_static, is_public, conditional));

                        /* Ensures that methods don't have generic argument(s) */
                        if (methods.back() && !methods.back()->generic_args.empty()) {
                            this->parse_exceptions.emplace_back(
                                U"A method cannot have (a) generic argument(s)", token);
                        }
                    }
                    /* Member/class variables */
                    else {
                        bool is_static, is_public;
                        /* Parse access types */
                        this->parseAccessAttribs(is_static, is_public);
                        KH_PARSE_GUARD();
                        /* Parse variable declaration */
                        members.emplace_back(this->parseDeclaration(is_static, is_public));

                        KH_PARSE_GUARD();
                        token = this->to();
                        /* Expects semicolon */
                        if (token.type == kh::TokenType::SYMBOL &&
                            token.value.symbol_type == kh::Symbol::SEMICOLON)
                            this->ti++;
                        else {
                            this->ti++;
                            this->parse_exceptions.emplace_back(U"Was expecting a semicolon after a "
                                                                U"variable declaration in the " +
                                                                    type_name + U" body",
                                                                token);
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
                            this->parse_exceptions.emplace_back(
                                U"Unexpected symbol while parsing the " + type_name + U" body", token);
                    }
                } break;

                default:
                    this->ti++;
                    this->parse_exceptions.emplace_back(
                        U"Unexpected token while parsing the " + type_name + U" body", token);
            }
        }
    }
    else
        this->parse_exceptions.emplace_back(
            U"Was expecting an opening curly bracket after the " + type_name + U" declaration", token);
end:
    return new kh::AstUserType(index, identifiers, bases, generic_args, members, methods, is_class);
}

kh::AstEnumType* kh::Parser::parseEnum() {
    std::vector<std::u32string> identifiers;
    std::vector<std::u32string> members;
    std::vector<uint64_t> values;

    /* Internal enum counter */
    uint64_t counter = 0;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Gets the enum identifiers */
    std::shared_ptr<kh::AstIdentifierExpression> identifiers_with_generics(
        (kh::AstIdentifierExpression*)this->parseIdentifiers());
    identifiers = identifiers_with_generics->identifiers;
    if (!identifiers_with_generics->generics.empty())
        this->parse_exceptions.emplace_back(
            U"An enum could not have generic arguments",
            this->tokFromIndex(identifiers_with_generics->generics[0]->index));

    KH_PARSE_GUARD();
    token = this->to();

    /* Opens with a curly bracket */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();
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
            else {
                this->parse_exceptions.emplace_back(U"Unexpected token while parsing the enum body",
                                                    token);

                this->ti++;
                KH_PARSE_GUARD();
                token = this->to();
                continue;
            }

            this->ti++;
            KH_PARSE_GUARD();
            token = this->to();

            /* Checks if there's an assignment operation on an enum member */
            if (token.type == kh::TokenType::OPERATOR &&
                token.value.operator_type == kh::Operator::ASSIGN) {
                this->ti++;
                KH_PARSE_GUARD();
                token = this->to();

                /* Ensures there's an integer constant */
                if (token.type == kh::TokenType::INTEGER || token.type == kh::TokenType::UINTEGER) {
                    /* Don't worry about this line as it's a union */
                    values.push_back(token.value.uinteger);
                    counter = token.value.uinteger + 1;

                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->to();
                }
                else {
                    this->parse_exceptions.emplace_back(U"Was expecting an integer constant after the "
                                                        U"assignment operation on the enum member",
                                                        token);

                    values.push_back(counter);
                    counter++;
                }
            }
            else {
                values.push_back(counter);
                counter++;
            }

            /* Ensures there's no enum member with the same name or index value */
            for (size_t member = 0; member < members.size() - 1; member++) {
                if (members[member] == members.back()) {
                    this->parse_exceptions.emplace_back(U"This enum member has the same name as #" +
                                                            kh::repr((uint64_t)member + 1),
                                                        token);
                    break;
                }

                if (values[member] == values.back()) {
                    this->parse_exceptions.emplace_back(
                        U"This enum member has a same index value as " + members[member], token);
                    break;
                }
            }

            /* Stops parsing enum body */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            /* Ensures a comma after an enum member */
            else if (!(token.type == kh::TokenType::SYMBOL &&
                       token.value.symbol_type == kh::Symbol::COMMA))
                this->parse_exceptions.emplace_back(U"Was expecting a closing curly bracket or a comma "
                                                    U"after an enum member in the enum body",
                                                    token);

            this->ti++;
            KH_PARSE_GUARD();
            token = this->to();
        }
    }
    else
        this->parse_exceptions.emplace_back(
            U"Was expecting an opening curly bracket after the enum declaration", token);
end:
    return new kh::AstEnumType(index, identifiers, members, values);
}

std::vector<std::shared_ptr<kh::AstBody>> kh::Parser::parseBody(const size_t loop_count) {
    std::vector<std::shared_ptr<kh::AstBody>> body;
    kh::Token token = this->to();
    bool is_single_liner = false;

    /* Checks if it's a single liner statement `if a < b : something();` */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COLON)
        is_single_liner = true;
    /* Expects an opening curly bracket */
    else if (!(token.type == kh::TokenType::SYMBOL &&
               token.value.symbol_type == kh::Symbol::CURLY_OPEN)) {
        this->parse_exceptions.emplace_back(U"Was expecting an opening curly bracket", token);
        goto end;
    }

    this->ti++;

    /* Parses the body */
    while (true) {
        KH_PARSE_GUARD();
        token = this->to();
        size_t index = token.index;

        switch (token.type) {
            case kh::TokenType::IDENTIFIER: {
                if (token.value.identifier == U"if") {
                    std::vector<std::shared_ptr<kh::AstExpression>> conditions;
                    std::vector<std::vector<std::shared_ptr<kh::AstBody>>> bodies;
                    std::vector<std::shared_ptr<kh::AstBody>> else_body;

                    do {
                        /* Parses the expression and if body */
                        this->ti++;
                        token = this->to();
                        KH_PARSE_GUARD();
                        conditions.emplace_back(this->parseExpression());
                        KH_PARSE_GUARD();
                        bodies.emplace_back(this->parseBody(loop_count + 1));
                        KH_PARSE_GUARD();
                        token = this->to();

                        /* Recontinues if there's an else if (`elif`) clause */
                    } while (token.type == kh::TokenType::IDENTIFIER &&
                             token.value.identifier == U"elif");

                    /* Parses the body if there's an `else` clause */
                    if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"else") {
                        this->ti++;
                        KH_PARSE_GUARD();
                        else_body = this->parseBody(loop_count + 1);
                    }

                    body.emplace_back(new kh::AstIf(index, conditions, bodies, else_body));
                }
                /* While statement */
                else if (token.value.identifier == U"while") {
                    this->ti++;
                    KH_PARSE_GUARD();

                    /* Parses the expression and body */
                    std::shared_ptr<kh::AstExpression> condition(this->parseExpression());
                    std::vector<std::shared_ptr<kh::AstBody>> while_body =
                        this->parseBody(loop_count + 1);

                    body.emplace_back(new kh::AstWhile(index, condition, while_body));
                }
                /* Do while statement */
                else if (token.value.identifier == U"do") {
                    this->ti++;
                    KH_PARSE_GUARD();

                    /* Parses the body */
                    std::vector<std::shared_ptr<kh::AstBody>> do_while_body =
                        this->parseBody(loop_count + 1);
                    std::shared_ptr<kh::AstExpression> condition;

                    KH_PARSE_GUARD();
                    token = this->to();

                    /* Expects `while` and then parses the condition expression */
                    if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"while") {
                        this->ti++;
                        condition.reset(this->parseExpression());
                    }
                    else
                        this->parse_exceptions.emplace_back(
                            U"Was expecting `while` after the `do {...}` body", token);

                    KH_PARSE_GUARD();
                    token = this->to();

                    /* Expects a semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->parse_exceptions.emplace_back(
                            U"Was expecting a semicolon after the `do {...} while (...)` statement",
                            token);

                    body.emplace_back(new kh::AstDoWhile(index, condition, do_while_body));
                }
                /* For statement */
                else if (token.value.identifier == U"for") {
                    this->ti++;
                    KH_PARSE_GUARD();

                    std::shared_ptr<kh::AstExpression> target_or_initializer(this->parseExpression());

                    KH_PARSE_GUARD();
                    token = this->to();
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::COLON) {
                        this->ti++;
                        KH_PARSE_GUARD();
                        token = this->to();

                        std::shared_ptr<kh::AstExpression> iterator(this->parseExpression());
                        KH_PARSE_GUARD();
                        std::vector<std::shared_ptr<kh::AstBody>> foreach_body =
                            this->parseBody(loop_count + 1);

                        body.emplace_back(
                            new kh::AstForEach(index, target_or_initializer, iterator, foreach_body));
                    }
                    else if (token.type == kh::TokenType::SYMBOL &&
                             token.value.symbol_type == kh::Symbol::COMMA) {
                        this->ti++;
                        KH_PARSE_GUARD();
                        std::shared_ptr<kh::AstExpression> condition(this->parseExpression());
                        KH_PARSE_GUARD();
                        token = this->to();

                        if (token.type == kh::TokenType::SYMBOL &&
                            token.value.symbol_type == kh::Symbol::COMMA) {
                            this->ti++;
                            KH_PARSE_GUARD();
                        }
                        else
                            this->parse_exceptions.emplace_back(
                                U"Was expecting a comma after the `for` condition expression", token);

                        std::shared_ptr<kh::AstExpression> step(this->parseExpression());
                        KH_PARSE_GUARD();
                        std::vector<std::shared_ptr<kh::AstBody>> for_body =
                            this->parseBody(loop_count + 1);

                        body.emplace_back(
                            new kh::AstFor(index, target_or_initializer, condition, step, for_body));
                    }
                    else
                        this->parse_exceptions.emplace_back(
                            U"Was expecting a colon or a comma after the `for` target/initializer",
                            token);
                }
                /* `continue` statement */
                else if (token.value.identifier == U"continue") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->to();

                    if (!loop_count)
                        this->parse_exceptions.emplace_back(
                            U"`continue` cannot be used outside of `while` or `for` loops", token);

                    size_t loop_breaks = 0;
                    /* Continuing multiple loops `continue 4;` */
                    if (token.type == kh::TokenType::UINTEGER || token.type == kh::TokenType::INTEGER) {
                        if (token.value.uinteger >= loop_count)
                            this->parse_exceptions.emplace_back(
                                U"Trying to `continue` an invalid amount of loops", token);

                        loop_breaks = token.value.uinteger;
                        this->ti++;
                        KH_PARSE_GUARD();
                        token = this->to();
                    }

                    /* Expects semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->parse_exceptions.emplace_back(
                            U"Was expecting a semicolon or an integer after `continue`", token);

                    body.emplace_back(
                        new kh::AstStatement(index, kh::AstStatement::Type::CONTINUE, loop_breaks));
                }
                /* `break` statement */
                else if (token.value.identifier == U"break") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->to();

                    if (!loop_count)
                        this->parse_exceptions.emplace_back(
                            U"`break` cannot be used outside of `while` or `for` loops", token);

                    size_t loop_breaks = 0;
                    /* Breaking multiple loops `break 2;` */
                    if (token.type == kh::TokenType::UINTEGER || token.type == kh::TokenType::INTEGER) {
                        if (token.value.uinteger >= loop_count)
                            this->parse_exceptions.emplace_back(
                                U"Trying to `break` an invalid amount of loops", token);

                        loop_breaks = token.value.uinteger;
                        this->ti++;
                        KH_PARSE_GUARD();
                        token = this->to();
                    }

                    /* Expects semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->parse_exceptions.emplace_back(
                            U"Was expecting a semicolon or an integer after `break`", token);

                    body.emplace_back(
                        new kh::AstStatement(index, kh::AstStatement::Type::BREAK, loop_breaks));
                }
                /* `return` statement */
                else if (token.value.identifier == U"return") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->to();

                    std::shared_ptr<kh::AstExpression> expression((kh::AstExpression*)nullptr);

                    /* No expression given */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    /* If there's a provided return value expression */
                    else {
                        expression.reset(this->parseExpression());
                        KH_PARSE_GUARD();
                        token = this->to();

                        /* Expects semicolon */
                        if (token.type == kh::TokenType::SYMBOL &&
                            token.value.symbol_type == kh::Symbol::SEMICOLON)
                            this->ti++;
                        else
                            this->parse_exceptions.emplace_back(
                                U"Was expecting a semicolon after the `return` statement", token);
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
                        if (is_single_liner)
                            this->parse_exceptions.emplace_back(
                                U"Unexpected closing curly bracket in a single liner statement", token);

                        this->ti++;
                        goto end;
                    } break;

                    default:
                        goto parse_expr;
                }
                break;

            default:
            parse_expr : {
                /* If it isn't any of the statements above, it's probably an expression */
                std::shared_ptr<kh::AstExpression> expr(this->parseExpression());
                KH_PARSE_GUARD();
                token = this->to();

                /* Expects a semicolon */
                if (token.type == kh::TokenType::SYMBOL &&
                    token.value.symbol_type == kh::Symbol::SEMICOLON)
                    this->ti++;
                else
                    this->parse_exceptions.emplace_back(U"Was expecting a semicolon", token);

                body.emplace_back(expr);
            }
        }

        if (is_single_liner)
            break;
    }
end:
    return body;
}
