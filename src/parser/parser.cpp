/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/parser.hpp>


using namespace kh;

std::string kh::ParseException::format() const {
    return this->what + " at line " + strfy((uint64_t)this->token.line) + " column " +
           strfy((uint64_t)this->token.column);
}

AstModule kh::parse(const std::vector<Token>& tokens) {
    std::vector<ParseException> exceptions;
    Parser parser{tokens, exceptions};
    AstModule ast = parser.parseWhole();

    if (exceptions.empty()) {
        return ast;
    }
    else {
        throw exceptions;
    }
}

AstModule kh::Parser::parseWhole() {
    this->exceptions.clear();

    std::vector<AstImport> imports;
    std::vector<AstFunction> functions;
    std::vector<AstUserType> user_types;
    std::vector<AstEnumType> enums;
    std::vector<AstDeclaration> variables;

    for (this->ti = 0; this->ti < this->tokens.size(); /* Nothing */) {
        Token token = this->tok();

        bool is_public, is_static;
        this->parseAccessAttribs(is_public, is_static);
        KH_PARSE_GUARD();
        token = this->tok();

        switch (token.type) {
            case TokenType::IDENTIFIER: {
                const std::string& identifier = token.value.identifier;

                /* Function declaration identifier keyword */
                if (identifier == "def" || identifier == "try") {
                    /* Skips initial keyword */
                    this->ti++;
                    KH_PARSE_GUARD();

                    /* Case for conditional functions */
                    bool conditional = false;
                    if (identifier == "try") {
                        conditional = true;
                        token = this->tok();
                        if (token.type == TokenType::IDENTIFIER && token.value.identifier == "def") {
                            this->ti++;
                            KH_PARSE_GUARD();
                        }
                        else {
                            this->exceptions.emplace_back("expected `def` after `try` at the top scope",
                                                          token);
                        }
                    }

                    KH_PARSE_GUARD();
                    /* Parses return type, name, arguments, and body */
                    functions.push_back(this->parseFunction(conditional));

                    functions.back().is_public = is_public;
                    functions.back().is_static = is_static;

                    if (functions.back().identifiers.empty()) {
                        this->exceptions.emplace_back(
                            "a lambda function cannot be declared at the top scope", token);
                    }

                    if (is_static && functions.back().identifiers.size() == 1) {
                        this->exceptions.emplace_back("a top scope function cannot be static", token);
                    }
                }
                /* Parses class declaration */
                else if (identifier == "class") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    user_types.push_back(this->parseUserType(true));

                    user_types.back().is_public = is_public;
                    if (is_static) {
                        this->exceptions.emplace_back("a class cannot be static", token);
                    }
                }
                /* Parses struct declaration */
                else if (identifier == "struct") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    user_types.push_back(this->parseUserType(false));

                    user_types.back().is_public = is_public;
                    if (is_static) {
                        this->exceptions.emplace_back("a struct cannot be static", token);
                    }
                }
                /* Parses enum declaration */
                else if (identifier == "enum") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    enums.push_back(this->parseEnum());

                    enums.back().is_public = is_public;
                    if (is_static) {
                        this->exceptions.emplace_back("an enum cannot be static", token);
                    }
                }
                /* Parses import statement */
                else if (identifier == "import") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    imports.push_back(this->parseImport(false)); /* is_include = false */

                    imports.back().is_public = is_public;
                    if (is_static) {
                        this->exceptions.emplace_back("an import cannot be static", token);
                    }
                }
                /* Parses include statement */
                else if (identifier == "include") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    imports.push_back(this->parseImport(true)); /* is_include = true */

                    imports.back().is_public = is_public;
                    if (is_static) {
                        this->exceptions.emplace_back("an include cannot be static", token);
                    }
                }
                /* If it was none of those above, it's probably a variable declaration */
                else {
                    /* Parses the variable's return type, name, and assignment value */
                    variables.push_back(this->parseDeclaration());

                    /* Makes sure it ends with a semicolon */
                    KH_PARSE_GUARD();
                    token = this->tok();
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        this->ti++;
                    }
                    else {
                        this->exceptions.emplace_back(
                            "expected a semicolon after a variable declaration", token);
                    }

                    if (is_static) {
                        this->exceptions.emplace_back("a top scope variable cannot be static", token);
                    }
                }
            } break;

            case TokenType::SYMBOL:
                if (token.value.symbol_type == Symbol::SEMICOLON) {
                    this->ti++;
                }
                else {
                    this->ti++;
                    this->exceptions.emplace_back(
                        "unexpected `" + strfy(token) + "` while parsing the top scope", token);
                }
                break;

                /* Unknown token */
            default:
                this->ti++;
                this->exceptions.emplace_back(
                    "unexpected `" + strfy(token) + "` while parsing the top scope", token);
        }
    }

end:
    /* Removes exceptions that's got duplicate errors at the same index */
    if (this->exceptions.size() > 1) {
        size_t last_index = -1;
        ParseException* last_element = nullptr;

        std::vector<ParseException> cleaned_exceptions;
        cleaned_exceptions.reserve(this->exceptions.size());

        for (ParseException& exc : this->exceptions) {
            if (last_element == nullptr || last_index != exc.token.begin ||
                last_element->what != exc.what) {
                cleaned_exceptions.push_back(exc);
            }
            last_index = exc.token.begin;
            last_element = &exc;
        }

        this->exceptions = cleaned_exceptions;
    }

    return {imports, functions, user_types, enums, variables};
}

void kh::Parser::parseAccessAttribs(bool& is_public, bool& is_static) {
    is_public = true;
    is_static = false;

    bool specified_public = false;
    bool specified_private = false;
    bool specified_static = false;

    /* It this->parses these kinds of access types: `[static | private/public] int x = 3` */
    Token token = this->tok();
    while (token.type == TokenType::IDENTIFIER) {
        if (token.value.identifier == "public") {
            is_public = true;

            if (specified_public) {
                this->exceptions.emplace_back("`public` was already specified", token);
            }
            if (specified_private) {
                this->exceptions.emplace_back("`private` was already specified", token);
            }

            specified_public = true;
        }
        else if (token.value.identifier == "private") {
            is_public = false;

            if (specified_public) {
                this->exceptions.emplace_back("`public` was already specified", token);
            }
            if (specified_private) {
                this->exceptions.emplace_back("`private` was already specified", token);
            }

            specified_private = true;
        }
        else if (token.value.identifier == "static") {
            is_static = true;

            if (specified_static) {
                this->exceptions.emplace_back("`static` was already specified", token);
            }

            specified_static = true;
        }
        else {
            break;
        }

        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();
    }

end:
    return;
}

AstImport kh::Parser::parseImport(bool is_include) {
    std::vector<std::string> path;
    bool is_relative = false;
    std::string identifier;

    Token token = this->tok();
    size_t index = token.begin;
    std::string type = is_include ? "include" : "import";

    /* Check if an import/include is relative */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT) {
        is_relative = true;
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();
    }

    /* Making sure that it starts with an identifier (an import/include statement must has at least
     * one identifier to be imported) */
    if (token.type == TokenType::IDENTIFIER) {
        if (isReservedKeyword(token.value.identifier)) {
            this->exceptions.emplace_back("was trying to " + type + " a reserved keyword", token);
        }

        path.push_back(token.value.identifier);
        this->ti++;
    }
    else {
        this->exceptions.emplace_back("expected an identifier after the `" + type + "` keyword", token);
        this->ti++;
    }

    KH_PARSE_GUARD();
    token = this->tok();

    /* Parses each identifier after a dot `import a.b.c.d ...` */
    while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        /* Appends the identifier */
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                this->exceptions.emplace_back("was trying to " + type + " a reserved keyword", token);
            }
            path.push_back(token.value.identifier);
            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();
        }
        else {
            this->exceptions.emplace_back(
                "expected an identifier after the dot in the " + type + " statement", token);
            break;
        }
    }

    /* An optional `as` for changing the namespace name in import statements */
    if (!is_include && token.type == TokenType::IDENTIFIER && token.value.identifier == "as") {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        /* Gets the set namespace identifier */
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                this->exceptions.emplace_back(
                    "could not use a reserved keyword as the alias of the import", token);
            }
            identifier = token.value.identifier;
        }
        else {
            this->exceptions.emplace_back(
                "expected an identifier after the `as` keyword in the import statement", token);
        }

        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();
    }

    /* Ensure that it ends with a semicolon */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SEMICOLON) {
        this->ti++;
    }
    else {
        this->exceptions.emplace_back("expected a semicolon after the " + type + " statement", token);
    }
end:
    return {index, path, is_include, is_relative,
            path.empty() ? "" : (identifier.empty() ? path.back() : identifier)};
}

AstFunction kh::Parser::parseFunction(bool is_conditional) {
    std::vector<std::string> identifiers;
    std::vector<std::string> generic_args;
    std::vector<uint64_t> id_array;

    AstIdentifiers return_type{0, {}, {}, {}, {}};
    std::vector<uint64_t> return_array = {};
    size_t return_refs = 0;

    std::vector<AstDeclaration> arguments;
    std::vector<std::shared_ptr<AstBody>> body;

    Token token = this->tok();
    size_t index = token.begin;

    if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN)) {
        /* Parses the function's identifiers and generic args */
        this->parseTopScopeIdentifiersAndGenericArgs(identifiers, generic_args);
        KH_PARSE_GUARD();
        token = this->tok();

        /* Array dimension method extension/overloading/overriding specifier `def float[3].add(float[3]
         * other) {}` */
        while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_OPEN) {
            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();

            if (token.type == TokenType::INTEGER || token.type == TokenType::UINTEGER) {
                if (token.value.uinteger == 0) {
                    this->exceptions.emplace_back("an array could not be zero sized", token);
                }

                id_array.push_back(token.value.uinteger);
                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();
            }
            else {
                this->exceptions.emplace_back("expected an integer for the array size", token);
            }
            if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_CLOSE)) {
                this->exceptions.emplace_back("expected a closing square bracket", token);
            }
            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();
        }

        /* Extra identifier `def something!int.extraIdentifier() {}` */
        KH_PARSE_GUARD();
        token = this->tok();
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT) {
            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();

            if (token.type == TokenType::IDENTIFIER) {
                identifiers.push_back(token.value.identifier);
                this->ti++;
            }
            else {
                this->exceptions.emplace_back(
                    "expected an identifier after the dot in the function declaration name", token);
            }
        }

        /* Ensures it has an opening parentheses */
        KH_PARSE_GUARD();
        token = this->tok();
        if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN)) {
            this->exceptions.emplace_back(
                "expected an opening parentheses of the argument(s) in the function declaration",
                token);
            goto end;
        }
    }

    this->ti++;
    KH_PARSE_GUARD();
    token = this->tok();

    /* Loops until it reaches a closing parentheses */
    while (true) {
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
            break;
        }
        /* Parses the argument */
        arguments.emplace_back(this->parseDeclaration());

        KH_PARSE_GUARD();
        token = this->tok();

        if (token.type == TokenType::SYMBOL) {
            /* Continues on parsing an argument if there's a comma */
            if (token.value.symbol_type == Symbol::COMMA) {
                this->ti++;
                KH_PARSE_GUARD();
                continue;
            }
            /* Stops parsing arguments */
            else if (token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                break;
            }
            else {
                this->exceptions.emplace_back("expected a closing parentheses or a comma in "
                                              "the function declaration's argument(s)",
                                              token);
                goto end;
            }
        }
        else {
            this->exceptions.emplace_back("expected a closing parentheses or a comma in the "
                                          "function declaration's argument(s)",
                                          token);
            goto end;
        }
    }

    this->ti++;
    KH_PARSE_GUARD();
    token = this->tok();

    /* Specifying return type `def function() -> int {}` */
    if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::SUB) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::MORE) {
            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();

            /* Checks if the return type is a `ref`erence type */
            while (token.type == TokenType::IDENTIFIER && token.value.identifier == "ref") {
                return_refs += 1;
                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();
            }

            return_type = this->parseIdentifiers();
            KH_PARSE_GUARD();
            token = this->tok();

            /* Array return type */
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_OPEN) {
                return_array = this->parseArrayDimension(return_type);
            }
        }
        else {
            return_type = AstIdentifiers(token.begin, {"void"}, {}, {}, {});

            this->exceptions.emplace_back("expected a `->` specifying a return type", token);
        }
    }
    else {
        return_type = AstIdentifiers(token.begin, {"void"}, {}, {}, {});
    }

    /* Parses the function's body */
    body = this->parseBody();
end:
    return {index,        identifiers, generic_args, id_array, return_type,
            return_array, return_refs, arguments,    body,     is_conditional};
}

AstDeclaration kh::Parser::parseDeclaration() {
    AstIdentifiers var_type{0, {}, {}, {}, {}};
    std::vector<uint64_t> var_array = {};
    std::string var_name;
    std::shared_ptr<AstExpression> expression = nullptr;
    size_t refs = 0;

    Token token = this->tok();
    size_t index = token.begin;

    /* Checks if the variable type is a `ref`erence type */
    while (token.type == TokenType::IDENTIFIER && token.value.identifier == "ref") {
        refs += 1;
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();
    }

    /* Parses the variable's type */
    var_type = this->parseIdentifiers();

    /* Possible array type `float[3] var;` */
    KH_PARSE_GUARD();
    token = this->tok();
    var_array = this->parseArrayDimension(var_type);

    /* Gets the variable's name */
    KH_PARSE_GUARD();
    token = this->tok();
    if (token.type != TokenType::IDENTIFIER) {
        this->exceptions.emplace_back("expected an identifier of the name of the variable declaration",
                                      token);
        goto end;
    }

    if (isReservedKeyword(token.value.identifier)) {
        this->exceptions.emplace_back("cannot use a reserved keyword as a variable name", token);
    }

    var_name = token.value.identifier;
    this->ti++;
    KH_PARSE_GUARD();
    token = this->tok();

    /* The case where: `SomeClass x(1, 2, 3)` */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
        expression.reset(this->parseTuple());
    }
    /* The case where: `int x = 3` */
    else if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::ASSIGN) {
        this->ti++;
        KH_PARSE_GUARD();
        expression.reset(this->parseExpression());
    }
    else {
        goto end;
    }
end:
    return {index, var_type, var_array, var_name, expression, refs};
}

AstUserType kh::Parser::parseUserType(bool is_class) {
    std::vector<std::string> identifiers;
    std::shared_ptr<AstIdentifiers> base;
    std::vector<std::string> generic_args;
    std::vector<AstDeclaration> members;
    std::vector<AstFunction> methods;

    Token token = this->tok();
    size_t index = token.begin;

    std::string type_name = is_class ? "class" : "struct";

    /* Parses the class'/struct's identifiers and generic arguments */
    this->parseTopScopeIdentifiersAndGenericArgs(identifiers, generic_args);
    KH_PARSE_GUARD();
    token = this->tok();

    /* Optional inheriting */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();

        /* Parses base class' identifier */
        base.reset(new AstIdentifiers(this->parseIdentifiers()));
        KH_PARSE_GUARD();
        token = this->tok();

        /* Expects a closing parentheses */
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
            this->ti++;
        }
        else {
            this->exceptions.emplace_back("expected a closing parentheses after the base class "
                                          "argument in the " +
                                              type_name + " declaration",
                                          token);
        }
    }

    KH_PARSE_GUARD();
    token = this->tok();

    /* Parses the body */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        while (true) {
            KH_PARSE_GUARD();
            Token token = this->tok();

            bool is_public, is_static;
            this->parseAccessAttribs(is_public, is_static);
            KH_PARSE_GUARD();
            token = this->tok();

            switch (token.type) {
                case TokenType::IDENTIFIER: {
                    /* Methods */
                    if (token.value.identifier == "def" || token.value.identifier == "try") {
                        bool conditional = token.value.identifier == "try";

                        this->ti++;
                        KH_PARSE_GUARD();

                        /* Case for conditional methods */
                        if (conditional) {
                            token = this->tok();
                            if (token.type == TokenType::IDENTIFIER &&
                                token.value.identifier == "def") {
                                this->ti++;
                                KH_PARSE_GUARD();
                            }
                            else {
                                this->exceptions.emplace_back(
                                    "expected `def` after `try` at the top scope", token);
                            }
                        }

                        /* Parse function declaration */
                        methods.push_back(this->parseFunction(conditional));

                        /* Ensures that methods don't have generic argument(s) */
                        if (!methods.back().generic_args.empty()) {
                            this->exceptions.emplace_back("a method cannot have generic arguments",
                                                          token);
                        }

                        /* Nor a lambda.. */
                        if (methods.back().identifiers.empty()) {
                            this->exceptions.emplace_back("a method cannot be a lambda", token);
                        }

                        methods.back().is_public = is_public;
                        methods.back().is_static = is_static;
                    }
                    /* Member/class variables */
                    else {
                        /* Parse variable declaration */
                        members.push_back(this->parseDeclaration());

                        KH_PARSE_GUARD();
                        token = this->tok();
                        /* Expects semicolon */
                        if (token.type == TokenType::SYMBOL &&
                            token.value.symbol_type == Symbol::SEMICOLON) {
                            this->ti++;
                        }
                        else {
                            this->ti++;
                            this->exceptions.emplace_back("expected a semicolon after a "
                                                          "variable declaration in the " +
                                                              type_name + " body",
                                                          token);
                        }

                        members.back().is_public = is_public;
                        members.back().is_static = is_static;
                    }
                } break;

                case TokenType::SYMBOL: {
                    switch (token.value.symbol_type) {
                        /* Placeholder "does nothing" semicolon */
                        case Symbol::SEMICOLON: {
                            this->ti++;
                        } break;

                            /* Closing curly bracket where it ends the class body */
                        case Symbol::CURLY_CLOSE: {
                            this->ti++;
                            goto end;
                        } break;

                        default:
                            this->ti++;
                            this->exceptions.emplace_back("unexpected `" + strfy(token) +
                                                              "` while parsing the " + type_name +
                                                              " body",
                                                          token);
                    }
                } break;

                default:
                    this->ti++;
                    this->exceptions.emplace_back("unexpected `" + strfy(token) +
                                                      "` while parsing the " + type_name + " body",
                                                  token);
            }
        }
    }
    else {
        this->exceptions.emplace_back(
            "expected an opening curly bracket for the " + type_name + " body", token);
    }
end:
    return {index, identifiers, base, generic_args, members, methods, is_class};
}

AstEnumType kh::Parser::parseEnum() {
    std::vector<std::string> identifiers;
    std::vector<std::string> members;
    std::vector<uint64_t> values;

    /* Internal enum counter */
    uint64_t counter = 0;

    Token token = this->tok();
    size_t index = token.begin;

    /* Gets the enum identifiers */
    std::vector<std::string> _generic_args;
    this->parseTopScopeIdentifiersAndGenericArgs(identifiers, _generic_args);
    if (!_generic_args.empty()) {
        this->exceptions.emplace_back("an enum could not have generic arguments", token);
    }

    KH_PARSE_GUARD();
    token = this->tok();

    /* Opens with a curly bracket */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        /* Parses the enum content */
        while (true) {
            /* Stops parsing enum body */
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            /* Appends member */
            else if (token.type == TokenType::IDENTIFIER) {
                members.push_back(token.value.identifier);
            }
            else {
                this->exceptions.emplace_back(
                    "unexpected `" + strfy(token) + "` while parsing the enum body", token);

                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();
                continue;
            }

            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();

            /* Checks if there's an assignment operation on an enum member */
            if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::ASSIGN) {
                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();

                /* Ensures there's an integer constant */
                if (token.type == TokenType::INTEGER || token.type == TokenType::UINTEGER) {
                    /* Don't worry about this line as it's a union */
                    values.push_back(token.value.uinteger);
                    counter = token.value.uinteger + 1;

                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->tok();
                }
                else {
                    this->exceptions.emplace_back("expected an integer constant after the "
                                                  "assignment operator on the enum member",
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
                    this->exceptions.emplace_back("this enum member has the same name as the #" +
                                                      strfy((uint64_t)member + 1) + " member",
                                                  token);
                    break;
                }

                if (values[member] == values.back()) {
                    this->exceptions.emplace_back(
                        "this enum member has a same index value as `" + members[member] + "`", token);
                    break;
                }
            }

            /* Stops parsing enum body */
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            /* Ensures a comma after an enum member */
            else if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA)) {
                this->exceptions.emplace_back("expected a closing curly bracket or a comma "
                                              "after an enum member in the enum body",
                                              token);
            }
            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();
        }
    }
    else {
        this->exceptions.emplace_back("expected an opening curly bracket after the enum declaration",
                                      token);
    }
end:
    return {index, identifiers, members, values};
}

std::vector<std::shared_ptr<AstBody>> kh::Parser::parseBody(size_t loop_count) {
    std::vector<std::shared_ptr<AstBody>> body;
    Token token = this->tok();

    /* Expects an opening curly bracket */
    if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_OPEN)) {
        this->exceptions.emplace_back("expected an opening curly bracket", token);
        goto end;
    }

    this->ti++;

    /* Parses the body */
    while (true) {
        KH_PARSE_GUARD();
        token = this->tok();
        size_t index = token.begin;

        switch (token.type) {
            case TokenType::IDENTIFIER: {
                if (token.value.identifier == "if") {
                    std::vector<std::shared_ptr<AstExpression>> conditions;
                    std::vector<std::vector<std::shared_ptr<AstBody>>> bodies;
                    std::vector<std::shared_ptr<AstBody>> else_body;

                    do {
                        /* Parses the expression and if body */
                        this->ti++;
                        token = this->tok();
                        KH_PARSE_GUARD();
                        conditions.emplace_back(this->parseExpression());
                        KH_PARSE_GUARD();
                        bodies.emplace_back(this->parseBody(loop_count + 1));
                        KH_PARSE_GUARD();
                        token = this->tok();

                        /* Recontinues if there's an else if (`elif`) clause */
                    } while (token.type == TokenType::IDENTIFIER && token.value.identifier == "elif");

                    /* Parses the body if there's an `else` clause */
                    if (token.type == TokenType::IDENTIFIER && token.value.identifier == "else") {
                        this->ti++;
                        KH_PARSE_GUARD();
                        else_body = this->parseBody(loop_count + 1);
                    }

                    body.emplace_back(new AstIf(index, conditions, bodies, else_body));
                }
                /* While statement */
                else if (token.value.identifier == "while") {
                    this->ti++;
                    KH_PARSE_GUARD();

                    /* Parses the expression and body */
                    std::shared_ptr<AstExpression> condition(this->parseExpression());
                    std::vector<std::shared_ptr<AstBody>> while_body = this->parseBody(loop_count + 1);

                    body.emplace_back(new AstWhile(index, condition, while_body));
                }
                /* Do while statement */
                else if (token.value.identifier == "do") {
                    this->ti++;
                    KH_PARSE_GUARD();

                    /* Parses the body */
                    std::vector<std::shared_ptr<AstBody>> do_while_body =
                        this->parseBody(loop_count + 1);
                    std::shared_ptr<AstExpression> condition;

                    KH_PARSE_GUARD();
                    token = this->tok();

                    /* Expects `while` and then this->parses the condition expression */
                    if (token.type == TokenType::IDENTIFIER && token.value.identifier == "while") {
                        this->ti++;
                        condition.reset(this->parseExpression());
                    }
                    else
                        this->exceptions.emplace_back("expected `while` after the `do {...}`", token);

                    KH_PARSE_GUARD();
                    token = this->tok();

                    /* Expects a semicolon */
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        this->ti++;
                    }
                    else
                        this->exceptions.emplace_back("expected a semicolon after `do {...} while ...`",
                                                      token);

                    body.emplace_back(new AstDoWhile(index, condition, do_while_body));
                }
                /* For statement */
                else if (token.value.identifier == "for") {
                    this->ti++;
                    KH_PARSE_GUARD();

                    std::shared_ptr<AstExpression> target_or_initializer(this->parseExpression());

                    KH_PARSE_GUARD();
                    token = this->tok();
                    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COLON) {
                        this->ti++;
                        KH_PARSE_GUARD();
                        token = this->tok();

                        std::shared_ptr<AstExpression> iterator(this->parseExpression());
                        KH_PARSE_GUARD();
                        std::vector<std::shared_ptr<AstBody>> foreach_body =
                            this->parseBody(loop_count + 1);

                        body.emplace_back(
                            new AstForEach(index, target_or_initializer, iterator, foreach_body));
                    }
                    else if (token.type == TokenType::SYMBOL &&
                             token.value.symbol_type == Symbol::COMMA) {
                        this->ti++;
                        KH_PARSE_GUARD();
                        std::shared_ptr<AstExpression> condition(this->parseExpression());
                        KH_PARSE_GUARD();
                        token = this->tok();

                        if (token.type == TokenType::SYMBOL &&
                            token.value.symbol_type == Symbol::COMMA) {
                            this->ti++;
                            KH_PARSE_GUARD();
                        }
                        else {
                            this->exceptions.emplace_back("expected a comma after `for ..., ...`",
                                                          token);
                        }
                        std::shared_ptr<AstExpression> step(this->parseExpression());
                        KH_PARSE_GUARD();
                        std::vector<std::shared_ptr<AstBody>> for_body =
                            this->parseBody(loop_count + 1);

                        body.emplace_back(
                            new AstFor(index, target_or_initializer, condition, step, for_body));
                    }
                    else {
                        this->exceptions.emplace_back(
                            "expected a colon or a comma after the `for` target/initializer", token);
                    }
                }
                /* `continue` statement */
                else if (token.value.identifier == "continue") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->tok();

                    if (!loop_count) {
                        this->exceptions.emplace_back(
                            "`continue` cannot be used outside of while or for loops", token);
                    }
                    size_t loop_breaks = 0;
                    /* Continuing multiple loops `continue 4;` */
                    if (token.type == TokenType::UINTEGER || token.type == TokenType::INTEGER) {
                        if (token.value.uinteger >= loop_count) {
                            this->exceptions.emplace_back(
                                "trying to `continue` an invalid amount of loops", token);
                        }
                        loop_breaks = token.value.uinteger;
                        this->ti++;
                        KH_PARSE_GUARD();
                        token = this->tok();
                    }

                    /* Expects semicolon */
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        this->ti++;
                    }
                    else {
                        this->exceptions.emplace_back(
                            "expected a semicolon or an integer after `continue`", token);
                    }
                    body.emplace_back(
                        new AstStatement(index, AstStatement::Type::CONTINUE, loop_breaks));
                }
                /* `break` statement */
                else if (token.value.identifier == "break") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->tok();

                    if (!loop_count) {
                        this->exceptions.emplace_back(
                            "`break` cannot be used outside of while or for loops", token);
                    }
                    size_t loop_breaks = 0;
                    /* Breaking multiple loops `break 2;` */
                    if (token.type == TokenType::UINTEGER || token.type == TokenType::INTEGER) {
                        if (token.value.uinteger >= loop_count) {
                            this->exceptions.emplace_back(
                                "trying to `break` an invalid amount of loops", token);
                        }
                        loop_breaks = token.value.uinteger;
                        this->ti++;
                        KH_PARSE_GUARD();
                        token = this->tok();
                    }

                    /* Expects semicolon */
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        this->ti++;
                    }
                    else {
                        this->exceptions.emplace_back(
                            "expected a semicolon or an integer after `break`", token);
                    }
                    body.emplace_back(new AstStatement(index, AstStatement::Type::BREAK, loop_breaks));
                }
                /* `return` statement */
                else if (token.value.identifier == "return") {
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->tok();

                    std::shared_ptr<AstExpression> expression((AstExpression*)nullptr);

                    /* No expression given */
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        this->ti++;
                    } /* If there's a provided return value expression */
                    else {
                        expression.reset(this->parseExpression());
                        KH_PARSE_GUARD();
                        token = this->tok();

                        /* Expects semicolon */
                        if (token.type == TokenType::SYMBOL &&
                            token.value.symbol_type == Symbol::SEMICOLON) {
                            this->ti++;
                        }
                        else {
                            this->exceptions.emplace_back("expected a semicolon after `return ...`",
                                                          token);
                        }
                    }

                    body.emplace_back(new AstStatement(index, AstStatement::Type::RETURN, expression));
                }
                else {
                    goto parseExpr;
                }
            } break;

            case TokenType::SYMBOL:
                switch (token.value.symbol_type) {
                    /* Placeholder semicolon */
                    case Symbol::SEMICOLON: {
                        this->ti++;
                    } break;

                        /* Ends body */
                    case Symbol::CURLY_CLOSE: {
                        this->ti++;
                        goto end;
                    } break;

                    default:
                        goto parseExpr;
                }
                break;

            default:
            parseExpr : {
                /* If it isn't any of the statements above, it's probably an expression */
                std::shared_ptr<AstExpression> expr(this->parseExpression());
                KH_PARSE_GUARD();
                token = this->tok();

                /* Expects a semicolon */
                if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SEMICOLON) {
                    this->ti++;
                }
                else {
                    this->exceptions.emplace_back(
                        "expected a semicolon after the expression in the body", token);
                }
                body.emplace_back(expr);
            }
        }
    }
end:
    return body;
}

void kh::Parser::parseTopScopeIdentifiersAndGenericArgs(std::vector<std::string>& identifiers,
                                                        std::vector<std::string>& generic_args) {
    Token token = this->tok();
    goto forceIn;

    /* Parses the identifiers */
    do {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

    forceIn:
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                this->exceptions.emplace_back("cannot use a reserved keyword as an identifier", token);
            }
            identifiers.push_back(token.value.identifier);
            this->ti++;
        }
        else {
            this->exceptions.emplace_back("expected an identifier", token);
        }
        KH_PARSE_GUARD();
        token = this->tok();
    } while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT);

    /* Generic arguments */
    if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::NOT) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        /* a.b.c!T */
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                this->exceptions.emplace_back(
                    "cannot use a reserved keyword as an identifier of a generic argument", token);
            }
            generic_args.push_back(token.value.identifier);
            this->ti++;
        }
        /* a.b.c!(A, B) */
        else if (token.type == TokenType::SYMBOL &&
                 token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();
            goto forceInGenericArgs;

            do {
                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();

            forceInGenericArgs:
                if (token.type == TokenType::IDENTIFIER) {
                    if (isReservedKeyword(token.value.identifier)) {
                        this->exceptions.emplace_back(
                            "cannot use a reserved keyword as an identifier of a generic argument",
                            token);
                    }
                    generic_args.push_back(token.value.identifier);
                    this->ti++;
                }
                else {
                    this->exceptions.emplace_back("expected an identifier for a generic argument",
                                                  token);
                }
                KH_PARSE_GUARD();
                token = this->tok();
            } while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA);

            if (token.type == TokenType::SYMBOL &&
                token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                this->ti++;
            }
            else {
                this->exceptions.emplace_back("expected a closing parentheses", token);
            }
        }
    }

end:
    return;
}
