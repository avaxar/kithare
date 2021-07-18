/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/parser.hpp>


using namespace kh;

std::string kh::ParseException::format() const {
    return this->what + " at line " + std::to_string(this->token.line) + " column " +
           std::to_string(this->token.column);
}

AstModule kh::parse(const std::vector<Token>& tokens) {
    std::vector<ParseException> exceptions;
    ParserContext context{tokens, exceptions};
    AstModule ast = parseWhole(context);

    if (exceptions.empty()) {
        return ast;
    }
    else {
        throw exceptions;
    }
}

AstModule kh::parseWhole(KH_PARSE_CTX) {
    context.exceptions.clear();

    std::vector<AstImport> imports;
    std::vector<AstFunction> functions;
    std::vector<AstUserType> user_types;
    std::vector<AstEnumType> enums;
    std::vector<AstDeclaration> variables;

    for (context.ti = 0; context.ti < context.tokens.size(); /* Nothing */) {
        Token token = context.tok();

        bool is_public, is_static;
        parseAccessAttribs(context, is_public, is_static);
        KH_PARSE_GUARD();
        token = context.tok();

        switch (token.type) {
            case TokenType::IDENTIFIER: {
                const std::string& identifier = token.value.identifier;

                /* Function declaration identifier keyword */
                if (identifier == "def" || identifier == "try") {
                    /* Skips initial keyword */
                    context.ti++;
                    KH_PARSE_GUARD();

                    /* Case for conditional functions */
                    bool conditional = false;
                    if (identifier == "try") {
                        conditional = true;
                        token = context.tok();
                        if (token.type == TokenType::IDENTIFIER && token.value.identifier == "def") {
                            context.ti++;
                            KH_PARSE_GUARD();
                        }
                        else {
                            context.exceptions.emplace_back(
                                "expected `def` after `try` at the top scope", token);
                        }
                    }

                    KH_PARSE_GUARD();
                    /* Parses return type, name, arguments, and body */
                    functions.push_back(parseFunction(context, conditional));

                    functions.back().is_public = is_public;
                    functions.back().is_static = is_static;

                    if (functions.back().identifiers.empty()) {
                        context.exceptions.emplace_back(
                            "a lambda function cannot be declared at the top scope", token);
                    }

                    if (is_static && functions.back().identifiers.size() == 1) {
                        context.exceptions.emplace_back("a top scope function cannot be static", token);
                    }
                }
                /* Parses class declaration */
                else if (identifier == "class") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    user_types.push_back(parseUserType(context, true));

                    user_types.back().is_public = is_public;
                    if (is_static) {
                        context.exceptions.emplace_back("a class cannot be static", token);
                    }
                }
                /* Parses struct declaration */
                else if (identifier == "struct") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    user_types.push_back(parseUserType(context, false));

                    user_types.back().is_public = is_public;
                    if (is_static) {
                        context.exceptions.emplace_back("a struct cannot be static", token);
                    }
                }
                /* Parses enum declaration */
                else if (identifier == "enum") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    enums.push_back(parseEnum(context));

                    enums.back().is_public = is_public;
                    if (is_static) {
                        context.exceptions.emplace_back("an enum cannot be static", token);
                    }
                }
                /* Parses import statement */
                else if (identifier == "import") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    imports.push_back(parseImport(context, false)); /* is_include = false */

                    imports.back().is_public = is_public;
                    if (is_static) {
                        context.exceptions.emplace_back("an import cannot be static", token);
                    }
                }
                /* Parses include statement */
                else if (identifier == "include") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    imports.push_back(parseImport(context, true)); /* is_include = true */

                    imports.back().is_public = is_public;
                    if (is_static) {
                        context.exceptions.emplace_back("an include cannot be static", token);
                    }
                }
                /* If it was none of those above, it's probably a variable declaration */
                else {
                    /* Parses the variable's return type, name, and assignment value */
                    variables.push_back(parseDeclaration(context));

                    /* Makes sure it ends with a semicolon */
                    KH_PARSE_GUARD();
                    token = context.tok();
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        context.ti++;
                    }
                    else {
                        context.exceptions.emplace_back(
                            "expected a semicolon after a variable declaration", token);
                    }

                    if (is_static) {
                        context.exceptions.emplace_back("a top scope variable cannot be static", token);
                    }
                }
            } break;

            case TokenType::SYMBOL:
                if (token.value.symbol_type == Symbol::SEMICOLON) {
                    context.ti++;
                }
                else {
                    context.ti++;
                    context.exceptions.emplace_back("unexpected `" + utf8Encode(strfy(token)) +
                                                        "` while parsing the top scope",
                                                    token);
                }
                break;

                /* Unknown token */
            default:
                context.ti++;
                context.exceptions.emplace_back(
                    "unexpected `" + utf8Encode(strfy(token)) + "` while parsing the top scope", token);
        }
    }

end:
    /* Removes exceptions that's got duplicate errors at the same index */
    if (context.exceptions.size() > 1) {
        size_t last_index = -1;
        ParseException* last_element = nullptr;

        std::vector<ParseException> cleaned_exceptions;
        cleaned_exceptions.reserve(context.exceptions.size());

        for (ParseException& exc : context.exceptions) {
            if (last_element == nullptr || last_index != exc.token.index ||
                last_element->what != exc.what) {
                cleaned_exceptions.push_back(exc);
            }
            last_index = exc.token.index;
            last_element = &exc;
        }

        context.exceptions = cleaned_exceptions;
    }

    return {imports, functions, user_types, enums, variables};
}

void kh::parseAccessAttribs(KH_PARSE_CTX, bool& is_public, bool& is_static) {
    is_public = true;
    is_static = false;

    bool specified_public = false;
    bool specified_private = false;
    bool specified_static = false;

    /* It parses these kinds of access types: `[static | private/public] int x = 3` */
    Token token = context.tok();
    while (token.type == TokenType::IDENTIFIER) {
        if (token.value.identifier == "public") {
            is_public = true;

            if (specified_public) {
                context.exceptions.emplace_back("`public` was already specified", token);
            }
            if (specified_private) {
                context.exceptions.emplace_back("`private` was already specified", token);
            }

            specified_public = true;
        }
        else if (token.value.identifier == "private") {
            is_public = false;

            if (specified_public) {
                context.exceptions.emplace_back("`public` was already specified", token);
            }
            if (specified_private) {
                context.exceptions.emplace_back("`private` was already specified", token);
            }

            specified_private = true;
        }
        else if (token.value.identifier == "static") {
            is_static = true;

            if (specified_static) {
                context.exceptions.emplace_back("`static` was already specified", token);
            }

            specified_static = true;
        }
        else {
            break;
        }

        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

end:
    return;
}

AstImport kh::parseImport(KH_PARSE_CTX, bool is_include) {
    std::vector<std::string> path;
    bool is_relative = false;
    std::string identifier;
    Token token = context.tok();
    size_t index = token.index;

    std::string type = is_include ? "include" : "import";

    /* Check if an import/include is relative */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT) {
        is_relative = true;
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

    /* Making sure that it starts with an identifier (an import/include statement must has at least
     * one identifier to be imported) */
    if (token.type == TokenType::IDENTIFIER) {
        if (isReservedKeyword(token.value.identifier)) {
            context.exceptions.emplace_back("was trying to " + type + " a reserved keyword", token);
        }

        path.push_back(token.value.identifier);
        context.ti++;
    }
    else {
        context.exceptions.emplace_back("expected an identifier after the `" + type + "` keyword",
                                        token);
        context.ti++;
    }

    KH_PARSE_GUARD();
    token = context.tok();

    /* Parses each identifier after a dot `import a.b.c.d ...` */
    while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Appends the identifier */
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                context.exceptions.emplace_back("was trying to " + type + " a reserved keyword", token);
            }
            path.push_back(token.value.identifier);
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();
        }
        else {
            context.exceptions.emplace_back(
                "expected an identifier after the dot in the " + type + " statement", token);
            break;
        }
    }

    /* An optional `as` for changing the namespace name in import statements */
    if (!is_include && token.type == TokenType::IDENTIFIER && token.value.identifier == "as") {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Gets the set namespace identifier */
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                context.exceptions.emplace_back(
                    "could not use a reserved keyword as the alias of the import", token);
            }
            identifier = token.value.identifier;
        }
        else {
            context.exceptions.emplace_back(
                "expected an identifier after the `as` keyword in the import statement", token);
        }

        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

    /* Ensure that it ends with a semicolon */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SEMICOLON) {
        context.ti++;
    }
    else {
        context.exceptions.emplace_back("expected a semicolon after the " + type + " statement", token);
    }
end:
    return {index, path, is_include, is_relative,
            path.empty() ? "" : (identifier.empty() ? path.back() : identifier)};
}

AstFunction kh::parseFunction(KH_PARSE_CTX, bool is_conditional) {
    std::vector<std::string> identifiers;
    std::vector<std::string> generic_args;
    std::vector<uint64_t> id_array;
    AstIdentifiers return_type{0, {}, {}, {}, {}};
    std::vector<uint64_t> return_array = {};
    size_t return_refs = 0;
    std::vector<AstDeclaration> arguments;
    std::vector<std::shared_ptr<AstBody>> body;

    Token token = context.tok();
    size_t index = token.index;

    if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN)) {
        /* Parses the function's identifiers and generic args */
        parseTopScopeIdentifiersAndGenericArgs(context, identifiers, generic_args);
        KH_PARSE_GUARD();
        token = context.tok();

        /* Array dimension method extension/overloading/overriding specifier `def float[3].add(float[3]
         * other) {}` */
        while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_OPEN) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            if (token.type == TokenType::INTEGER || token.type == TokenType::UINTEGER) {
                if (token.value.uinteger == 0) {
                    context.exceptions.emplace_back("an array could not be zero sized", token);
                }

                id_array.push_back(token.value.uinteger);
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }
            else {
                context.exceptions.emplace_back("expected an integer for the array size", token);
            }
            if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_CLOSE)) {
                context.exceptions.emplace_back("expected a closing square bracket", token);
            }
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();
        }

        /* Extra identifier `def something!int.extraIdentifier() {}` */
        KH_PARSE_GUARD();
        token = context.tok();
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            if (token.type == TokenType::IDENTIFIER) {
                identifiers.push_back(token.value.identifier);
                context.ti++;
            }
            else {
                context.exceptions.emplace_back(
                    "expected an identifier after the dot in the function declaration name", token);
            }
        }

        /* Ensures it has an opening parentheses */
        KH_PARSE_GUARD();
        token = context.tok();
        if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN)) {
            context.exceptions.emplace_back(
                "expected an opening parentheses of the argument(s) in the function declaration",
                token);
            goto end;
        }
    }

    context.ti++;
    KH_PARSE_GUARD();
    token = context.tok();

    /* Loops until it reaches a closing parentheses */
    while (true) {
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
            break;
        }
        /* Parses the argument */
        arguments.emplace_back(parseDeclaration(context));

        KH_PARSE_GUARD();
        token = context.tok();

        if (token.type == TokenType::SYMBOL) {
            /* Continues on parsing an argument if there's a comma */
            if (token.value.symbol_type == Symbol::COMMA) {
                context.ti++;
                KH_PARSE_GUARD();
                continue;
            }
            /* Stops parsing arguments */
            else if (token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                break;
            }
            else {
                context.exceptions.emplace_back("expected a closing parentheses or a comma in "
                                                "the function declaration's argument(s)",
                                                token);
                goto end;
            }
        }
        else {
            context.exceptions.emplace_back("expected a closing parentheses or a comma in the "
                                            "function declaration's argument(s)",
                                            token);
            goto end;
        }
    }

    context.ti++;
    KH_PARSE_GUARD();
    token = context.tok();

    /* Specifying return type `def function() -> int {}` */
    if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::SUB) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::MORE) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            /* Checks if the return type is a `ref`erence type */
            while (token.type == TokenType::IDENTIFIER && token.value.identifier == "ref") {
                return_refs += 1;
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }

            return_type = parseIdentifiers(context);
            KH_PARSE_GUARD();
            token = context.tok();

            /* Array return type */
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_OPEN) {
                return_array = parseArrayDimension(context, return_type);
            }
        }
        else {
            return_type = AstIdentifiers(token.index, {"void"}, {}, {}, {});

            context.exceptions.emplace_back("expected a `->` specifying a return type", token);
        }
    }
    else {
        return_type = AstIdentifiers(token.index, {"void"}, {}, {}, {});
    }

    /* Parses the function's body */
    body = parseBody(context);
end:
    return {index,       identifiers, generic_args, id_array, return_array,
            return_type, return_refs, arguments,    body,     is_conditional};
}

AstDeclaration kh::parseDeclaration(KH_PARSE_CTX) {
    AstIdentifiers var_type{0, {}, {}, {}, {}};
    std::vector<uint64_t> var_array = {};
    std::string var_name;
    std::shared_ptr<AstExpression> expression = nullptr;
    size_t refs = 0;

    Token token = context.tok();
    size_t index = token.index;

    /* Checks if the variable type is a `ref`erence type */
    while (token.type == TokenType::IDENTIFIER && token.value.identifier == "ref") {
        refs += 1;
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

    /* Parses the variable's type */
    var_type = parseIdentifiers(context);

    /* Possible array type `float[3] var;` */
    KH_PARSE_GUARD();
    token = context.tok();
    var_array = parseArrayDimension(context, var_type);

    /* Gets the variable's name */
    KH_PARSE_GUARD();
    token = context.tok();
    if (token.type != TokenType::IDENTIFIER) {
        context.exceptions.emplace_back(
            "expected an identifier of the name of the variable declaration", token);
        goto end;
    }

    if (isReservedKeyword(token.value.identifier)) {
        context.exceptions.emplace_back("cannot use a reserved keyword as a variable name", token);
    }

    var_name = token.value.identifier;
    context.ti++;
    KH_PARSE_GUARD();
    token = context.tok();

    /* The case where: `SomeClass x(1, 2, 3)` */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
        expression.reset(parseTuple(context));
    }
    /* The case where: `int x = 3` */
    else if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::ASSIGN) {
        context.ti++;
        KH_PARSE_GUARD();
        expression.reset(parseExpression(context));
    }
    else {
        goto end;
    }
end:
    return {index, var_type, var_array, var_name, expression, refs};
}

AstUserType kh::parseUserType(KH_PARSE_CTX, bool is_class) {
    std::vector<std::string> identifiers;
    std::shared_ptr<AstIdentifiers> base;
    std::vector<std::string> generic_args;
    std::vector<AstDeclaration> members;
    std::vector<AstFunction> methods;

    Token token = context.tok();
    size_t index = token.index;

    std::string type_name = is_class ? "class" : "struct";

    /* Parses the class'/struct's identifiers and generic arguments */
    parseTopScopeIdentifiersAndGenericArgs(context, identifiers, generic_args);
    KH_PARSE_GUARD();
    token = context.tok();

    /* Optional inheriting */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();

        /* Parses base class' identifier */
        base.reset(new AstIdentifiers(parseIdentifiers(context)));
        KH_PARSE_GUARD();
        token = context.tok();

        /* Expects a closing parentheses */
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
            context.ti++;
        }
        else {
            context.exceptions.emplace_back("expected a closing parentheses after the base class "
                                            "argument in the " +
                                                type_name + " declaration",
                                            token);
        }
    }

    KH_PARSE_GUARD();
    token = context.tok();

    /* Parses the body */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        while (true) {
            KH_PARSE_GUARD();
            Token token = context.tok();

            bool is_public, is_static;
            parseAccessAttribs(context, is_public, is_static);
            KH_PARSE_GUARD();
            token = context.tok();

            switch (token.type) {
                case TokenType::IDENTIFIER: {
                    /* Methods */
                    if (token.value.identifier == "def" || token.value.identifier == "try") {
                        bool conditional = token.value.identifier == "try";

                        context.ti++;
                        KH_PARSE_GUARD();

                        /* Case for conditional methods */
                        if (conditional) {
                            token = context.tok();
                            if (token.type == TokenType::IDENTIFIER &&
                                token.value.identifier == "def") {
                                context.ti++;
                                KH_PARSE_GUARD();
                            }
                            else {
                                context.exceptions.emplace_back(
                                    "expected `def` after `try` at the top scope", token);
                            }
                        }

                        /* Parse function declaration */
                        methods.push_back(parseFunction(context, conditional));

                        /* Ensures that methods don't have generic argument(s) */
                        if (!methods.back().generic_args.empty()) {
                            context.exceptions.emplace_back("a method cannot have generic arguments",
                                                            token);
                        }

                        /* Nor a lambda.. */
                        if (methods.back().identifiers.empty()) {
                            context.exceptions.emplace_back("a method cannot be a lambda", token);
                        }

                        methods.back().is_public = is_public;
                        methods.back().is_static = is_static;
                    }
                    /* Member/class variables */
                    else {
                        /* Parse variable declaration */
                        members.push_back(parseDeclaration(context));

                        KH_PARSE_GUARD();
                        token = context.tok();
                        /* Expects semicolon */
                        if (token.type == TokenType::SYMBOL &&
                            token.value.symbol_type == Symbol::SEMICOLON) {
                            context.ti++;
                        }
                        else {
                            context.ti++;
                            context.exceptions.emplace_back("expected a semicolon after a "
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
                            context.ti++;
                        } break;

                            /* Closing curly bracket where it ends the class body */
                        case Symbol::CURLY_CLOSE: {
                            context.ti++;
                            goto end;
                        } break;

                        default:
                            context.ti++;
                            context.exceptions.emplace_back("unexpected `" + utf8Encode(strfy(token)) +
                                                                "` while parsing the " + type_name +
                                                                " body",
                                                            token);
                    }
                } break;

                default:
                    context.ti++;
                    context.exceptions.emplace_back("unexpected `" + utf8Encode(strfy(token)) +
                                                        "` while parsing the " + type_name + " body",
                                                    token);
            }
        }
    }
    else {
        context.exceptions.emplace_back(
            "expected an opening curly bracket for the " + type_name + " body", token);
    }
end:
    return {index, identifiers, base, generic_args, members, methods, is_class};
}

AstEnumType kh::parseEnum(KH_PARSE_CTX) {
    std::vector<std::string> identifiers;
    std::vector<std::string> members;
    std::vector<uint64_t> values;

    /* Internal enum counter */
    uint64_t counter = 0;

    Token token = context.tok();
    size_t index = token.index;

    /* Gets the enum identifiers */
    std::vector<std::string> _generic_args;
    parseTopScopeIdentifiersAndGenericArgs(context, identifiers, _generic_args);
    if (!_generic_args.empty()) {
        context.exceptions.emplace_back("an enum could not have generic arguments", token);
    }

    KH_PARSE_GUARD();
    token = context.tok();

    /* Opens with a curly bracket */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Parses the enum content */
        while (true) {
            /* Stops parsing enum body */
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_CLOSE) {
                context.ti++;
                break;
            }
            /* Appends member */
            else if (token.type == TokenType::IDENTIFIER) {
                members.push_back(token.value.identifier);
            }
            else {
                context.exceptions.emplace_back(
                    "unexpected `" + utf8Encode(strfy(token)) + "` while parsing the enum body", token);

                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
                continue;
            }

            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            /* Checks if there's an assignment operation on an enum member */
            if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::ASSIGN) {
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();

                /* Ensures there's an integer constant */
                if (token.type == TokenType::INTEGER || token.type == TokenType::UINTEGER) {
                    /* Don't worry about this line as it's a union */
                    values.push_back(token.value.uinteger);
                    counter = token.value.uinteger + 1;

                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();
                }
                else {
                    context.exceptions.emplace_back("expected an integer constant after the "
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
                    context.exceptions.emplace_back("this enum member has the same name as the #" +
                                                        std::to_string(member + 1) + " member",
                                                    token);
                    break;
                }

                if (values[member] == values.back()) {
                    context.exceptions.emplace_back(
                        "this enum member has a same index value as `" + members[member] + "`", token);
                    break;
                }
            }

            /* Stops parsing enum body */
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_CLOSE) {
                context.ti++;
                break;
            }
            /* Ensures a comma after an enum member */
            else if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA)) {
                context.exceptions.emplace_back("expected a closing curly bracket or a comma "
                                                "after an enum member in the enum body",
                                                token);
            }
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();
        }
    }
    else {
        context.exceptions.emplace_back("expected an opening curly bracket after the enum declaration",
                                        token);
    }
end:
    return {index, identifiers, members, values};
}

std::vector<std::shared_ptr<AstBody>> kh::parseBody(KH_PARSE_CTX, size_t loop_count) {
    std::vector<std::shared_ptr<AstBody>> body;
    Token token = context.tok();

    /* Expects an opening curly bracket */
    if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_OPEN)) {
        context.exceptions.emplace_back("expected an opening curly bracket", token);
        goto end;
    }

    context.ti++;

    /* Parses the body */
    while (true) {
        KH_PARSE_GUARD();
        token = context.tok();
        size_t index = token.index;

        switch (token.type) {
            case TokenType::IDENTIFIER: {
                if (token.value.identifier == "if") {
                    std::vector<std::shared_ptr<AstExpression>> conditions;
                    std::vector<std::vector<std::shared_ptr<AstBody>>> bodies;
                    std::vector<std::shared_ptr<AstBody>> else_body;

                    do {
                        /* Parses the expression and if body */
                        context.ti++;
                        token = context.tok();
                        KH_PARSE_GUARD();
                        conditions.emplace_back(parseExpression(context));
                        KH_PARSE_GUARD();
                        bodies.emplace_back(parseBody(context, loop_count + 1));
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Recontinues if there's an else if (`elif`) clause */
                    } while (token.type == TokenType::IDENTIFIER && token.value.identifier == "elif");

                    /* Parses the body if there's an `else` clause */
                    if (token.type == TokenType::IDENTIFIER && token.value.identifier == "else") {
                        context.ti++;
                        KH_PARSE_GUARD();
                        else_body = parseBody(context, loop_count + 1);
                    }

                    body.emplace_back(new AstIf(index, conditions, bodies, else_body));
                }
                /* While statement */
                else if (token.value.identifier == "while") {
                    context.ti++;
                    KH_PARSE_GUARD();

                    /* Parses the expression and body */
                    std::shared_ptr<AstExpression> condition(parseExpression(context));
                    std::vector<std::shared_ptr<AstBody>> while_body =
                        parseBody(context, loop_count + 1);

                    body.emplace_back(new AstWhile(index, condition, while_body));
                }
                /* Do while statement */
                else if (token.value.identifier == "do") {
                    context.ti++;
                    KH_PARSE_GUARD();

                    /* Parses the body */
                    std::vector<std::shared_ptr<AstBody>> do_while_body =
                        parseBody(context, loop_count + 1);
                    std::shared_ptr<AstExpression> condition;

                    KH_PARSE_GUARD();
                    token = context.tok();

                    /* Expects `while` and then parses the condition expression */
                    if (token.type == TokenType::IDENTIFIER && token.value.identifier == "while") {
                        context.ti++;
                        condition.reset(parseExpression(context));
                    }
                    else
                        context.exceptions.emplace_back("expected `while` after the `do {...}`", token);

                    KH_PARSE_GUARD();
                    token = context.tok();

                    /* Expects a semicolon */
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        context.ti++;
                    }
                    else
                        context.exceptions.emplace_back(
                            "expected a semicolon after `do {...} while ...`", token);

                    body.emplace_back(new AstDoWhile(index, condition, do_while_body));
                }
                /* For statement */
                else if (token.value.identifier == "for") {
                    context.ti++;
                    KH_PARSE_GUARD();

                    std::shared_ptr<AstExpression> target_or_initializer(parseExpression(context));

                    KH_PARSE_GUARD();
                    token = context.tok();
                    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COLON) {
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();

                        std::shared_ptr<AstExpression> iterator(parseExpression(context));
                        KH_PARSE_GUARD();
                        std::vector<std::shared_ptr<AstBody>> foreach_body =
                            parseBody(context, loop_count + 1);

                        body.emplace_back(
                            new AstForEach(index, target_or_initializer, iterator, foreach_body));
                    }
                    else if (token.type == TokenType::SYMBOL &&
                             token.value.symbol_type == Symbol::COMMA) {
                        context.ti++;
                        KH_PARSE_GUARD();
                        std::shared_ptr<AstExpression> condition(parseExpression(context));
                        KH_PARSE_GUARD();
                        token = context.tok();

                        if (token.type == TokenType::SYMBOL &&
                            token.value.symbol_type == Symbol::COMMA) {
                            context.ti++;
                            KH_PARSE_GUARD();
                        }
                        else {
                            context.exceptions.emplace_back("expected a comma after `for ..., ...`",
                                                            token);
                        }
                        std::shared_ptr<AstExpression> step(parseExpression(context));
                        KH_PARSE_GUARD();
                        std::vector<std::shared_ptr<AstBody>> for_body =
                            parseBody(context, loop_count + 1);

                        body.emplace_back(
                            new AstFor(index, target_or_initializer, condition, step, for_body));
                    }
                    else {
                        context.exceptions.emplace_back(
                            "expected a colon or a comma after the `for` target/initializer", token);
                    }
                }
                /* `continue` statement */
                else if (token.value.identifier == "continue") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();

                    if (!loop_count) {
                        context.exceptions.emplace_back(
                            "`continue` cannot be used outside of while or for loops", token);
                    }
                    size_t loop_breaks = 0;
                    /* Continuing multiple loops `continue 4;` */
                    if (token.type == TokenType::UINTEGER || token.type == TokenType::INTEGER) {
                        if (token.value.uinteger >= loop_count) {
                            context.exceptions.emplace_back(
                                "trying to `continue` an invalid amount of loops", token);
                        }
                        loop_breaks = token.value.uinteger;
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();
                    }

                    /* Expects semicolon */
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        context.ti++;
                    }
                    else {
                        context.exceptions.emplace_back(
                            "expected a semicolon or an integer after `continue`", token);
                    }
                    body.emplace_back(
                        new AstStatement(index, AstStatement::Type::CONTINUE, loop_breaks));
                }
                /* `break` statement */
                else if (token.value.identifier == "break") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();

                    if (!loop_count) {
                        context.exceptions.emplace_back(
                            "`break` cannot be used outside of while or for loops", token);
                    }
                    size_t loop_breaks = 0;
                    /* Breaking multiple loops `break 2;` */
                    if (token.type == TokenType::UINTEGER || token.type == TokenType::INTEGER) {
                        if (token.value.uinteger >= loop_count) {
                            context.exceptions.emplace_back(
                                "trying to `break` an invalid amount of loops", token);
                        }
                        loop_breaks = token.value.uinteger;
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();
                    }

                    /* Expects semicolon */
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        context.ti++;
                    }
                    else {
                        context.exceptions.emplace_back(
                            "expected a semicolon or an integer after `break`", token);
                    }
                    body.emplace_back(new AstStatement(index, AstStatement::Type::BREAK, loop_breaks));
                }
                /* `return` statement */
                else if (token.value.identifier == "return") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();

                    std::shared_ptr<AstExpression> expression((AstExpression*)nullptr);

                    /* No expression given */
                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::SEMICOLON) {
                        context.ti++;
                    } /* If there's a provided return value expression */
                    else {
                        expression.reset(parseExpression(context));
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Expects semicolon */
                        if (token.type == TokenType::SYMBOL &&
                            token.value.symbol_type == Symbol::SEMICOLON) {
                            context.ti++;
                        }
                        else {
                            context.exceptions.emplace_back("expected a semicolon after `return ...`",
                                                            token);
                        }
                    }

                    body.emplace_back(new AstStatement(index, AstStatement::Type::RETURN, expression));
                }
                else {
                    goto parse_expr;
                }
            } break;

            case TokenType::SYMBOL:
                switch (token.value.symbol_type) {
                    /* Placeholder semicolon */
                    case Symbol::SEMICOLON: {
                        context.ti++;
                    } break;

                        /* Ends body */
                    case Symbol::CURLY_CLOSE: {
                        context.ti++;
                        goto end;
                    } break;

                    default:
                        goto parse_expr;
                }
                break;

            default:
            parse_expr : {
                /* If it isn't any of the statements above, it's probably an expression */
                std::shared_ptr<AstExpression> expr(parseExpression(context));
                KH_PARSE_GUARD();
                token = context.tok();

                /* Expects a semicolon */
                if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SEMICOLON) {
                    context.ti++;
                }
                else {
                    context.exceptions.emplace_back(
                        "expected a semicolon after the expression in the body", token);
                }
                body.emplace_back(expr);
            }
        }
    }
end:
    return body;
}

void kh::parseTopScopeIdentifiersAndGenericArgs(KH_PARSE_CTX, std::vector<std::string>& identifiers,
                                                std::vector<std::string>& generic_args) {
    Token token = context.tok();
    goto forceIn;

    /* Parses the identifiers */
    do {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

    forceIn:
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                context.exceptions.emplace_back("cannot use a reserved keyword as an identifier",
                                                token);
            }
            identifiers.push_back(token.value.identifier);
            context.ti++;
        }
        else {
            context.exceptions.emplace_back("expected an identifier", token);
        }
        KH_PARSE_GUARD();
        token = context.tok();
    } while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT);

    /* Generic arguments */
    if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::NOT) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* a.b.c!T */
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                context.exceptions.emplace_back(
                    "cannot use a reserved keyword as an identifier of a generic argument", token);
            }
            generic_args.push_back(token.value.identifier);
            context.ti++;
        }
        /* a.b.c!(A, B) */
        else if (token.type == TokenType::SYMBOL &&
                 token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();
            goto forceInGenericArgs;

            do {
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();

            forceInGenericArgs:
                if (token.type == TokenType::IDENTIFIER) {
                    if (isReservedKeyword(token.value.identifier)) {
                        context.exceptions.emplace_back(
                            "cannot use a reserved keyword as an identifier of a generic argument",
                            token);
                    }
                    generic_args.push_back(token.value.identifier);
                    context.ti++;
                }
                else {
                    context.exceptions.emplace_back("expected an identifier for a generic argument",
                                                    token);
                }
                KH_PARSE_GUARD();
                token = context.tok();
            } while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA);

            if (token.type == TokenType::SYMBOL &&
                token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                context.ti++;
            }
            else {
                context.exceptions.emplace_back("expected a closing parentheses", token);
            }
        }
    }

end:
    return;
}
