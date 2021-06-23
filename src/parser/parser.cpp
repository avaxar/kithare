/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/parser.hpp>


std::u32string kh::ParseException::format() const {
    return this->what + U" at line " + kh::str((uint64_t)this->token.line) + U" column " +
           kh::str((uint64_t)this->token.column);
}

kh::Ast kh::parse(const std::vector<kh::Token>& tokens) {
    std::vector<kh::ParseException> exceptions;
    kh::ParserContext context{tokens, exceptions};
    kh::Ast ast = kh::parseWhole(context);

    if (exceptions.empty())
        return ast;
    else
        throw exceptions;
}

kh::Ast kh::parseWhole(KH_PARSE_CTX) {
    context.exceptions.clear();

    std::vector<kh::AstImport> imports;
    std::vector<kh::AstFunction> functions;
    std::vector<kh::AstUserType> user_types;
    std::vector<kh::AstEnumType> enums;
    std::vector<kh::AstDeclaration> variables;

    for (context.ti = 0; context.ti < context.tokens.size();) {
        kh::Token& token = context.tok();

        switch (token.type) {
            case kh::TokenType::IDENTIFIER: {
                const std::u32string& identifier = token.value.identifier;

                /* Function declaration identifier keyword */
                if (identifier == U"def" || identifier == U"try") {
                    /* Skips initial keyword */
                    context.ti++;
                    KH_PARSE_GUARD();

                    /* Case for conditional functions */
                    bool conditional = false;
                    if (identifier == U"try") {
                        conditional = true;
                        token = context.tok();
                        if (token.type == kh::TokenType::IDENTIFIER &&
                            token.value.identifier == U"def") {
                            context.ti++;
                            KH_PARSE_GUARD();
                        }
                        else
                            context.exceptions.emplace_back(
                                U"Was expecting `def` after `try` at the top scope", token);
                    }

                    /* Parses access type */
                    bool is_static, is_public;
                    kh::parseAccessAttribs(context, is_static, is_public);

                    KH_PARSE_GUARD();
                    /* Parses return type, name, arguments, and body */
                    functions.push_back(kh::parseFunction(context, is_static, is_public, conditional));

                    if (functions.back().identifiers.empty())
                        context.exceptions.emplace_back(
                            U"Lambda functions cannot be declared at the top scope", token);
                    if (is_static && functions.back().identifiers.size() == 1)
                        context.exceptions.emplace_back(U"A top scope function cannot be `static`",
                                                        token);
                }
                /* Parses class declaration */
                else if (identifier == U"class") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    user_types.push_back(kh::parseUserType(context, true));
                }
                /* Parses struct declaration */
                else if (identifier == U"struct") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    user_types.push_back(kh::parseUserType(context, false));
                }
                /* Parses enum declaration */
                else if (identifier == U"enum") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    enums.push_back(kh::parseEnum(context));
                }
                /* Parses import statement */
                else if (identifier == U"import") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    imports.push_back(kh::parseImport(context, false)); /* is_include = false */
                }
                /* Parses include statement */
                else if (identifier == U"include") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    imports.push_back(kh::parseImport(context, true)); /* is_include = true */
                }
                /* If it was none of those above, it's probably a variable declaration */
                else {
                    /* Gets variable's declaration access type */
                    bool is_static, is_public;
                    kh::parseAccessAttribs(context, is_static, is_public);
                    KH_PARSE_GUARD();
                    if (is_static)
                        context.exceptions.emplace_back(U"A top scope variable cannot be `static`",
                                                        token);

                    /* Parses the variable's return type, name, and assignment value */
                    variables.push_back(kh::parseDeclaration(context, is_static, is_public));

                    /* Makes sure it ends with a semicolon */
                    KH_PARSE_GUARD();
                    token = context.tok();
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        context.ti++;
                    else
                        context.exceptions.emplace_back(
                            U"Was expecting a semicolon after a variable declaration", token);
                }
            } break;

            case kh::TokenType::SYMBOL:
                switch (token.value.symbol_type) {
                    /* Placeholder semicolon, ignore */
                    case kh::Symbol::SEMICOLON: {
                        context.ti++;
                    } break;

                    /* Unknown symbol */
                    default:
                        context.ti++;
                        context.exceptions.emplace_back(U"Unexpected token while parsing the top scope",
                                                        token);
                }
                break;

            /* Unknown token */
            default:
                context.ti++;
                context.exceptions.emplace_back(U"Unexpected token while parsing the top scope", token);
        }
    }

end:
    /* Removes exceptions that's got duplicate errors at the same index */
    if (context.exceptions.size() > 1) {
        size_t last_index = -1;
        kh::ParseException* last_element = nullptr;

        std::vector<kh::ParseException> cleaned_exceptions;
        cleaned_exceptions.reserve(context.exceptions.size());

        for (kh::ParseException& exc : context.exceptions) {
            if (last_element == nullptr || last_index != exc.token.index ||
                last_element->what != exc.what)
                cleaned_exceptions.push_back(exc);

            last_index = exc.token.index;
            last_element = &exc;
        }

        context.exceptions = cleaned_exceptions;
    }

    return {imports, functions, user_types, enums, variables};
}

kh::AstImport kh::parseImport(KH_PARSE_CTX, bool is_include) {
    std::vector<std::u32string> path;
    bool is_relative = false;
    std::u32string identifier;
    kh::Token token = context.tok();
    size_t index = token.index;

    std::u32string type = is_include ? U"include" : U"import";

    /* Check if an import/include is relative */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        is_relative = true;
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

    /* Making sure that it starts with an identifier (an import/include statement must has at least
     * one identifier to be imported) */
    if (token.type == kh::TokenType::IDENTIFIER) {
        if (kh::isReservedKeyword(token.value.identifier))
            context.exceptions.emplace_back(U"Was trying to `" + type + U"` with a reserved keyword",
                                            token);

        path.push_back(token.value.identifier);
        context.ti++;
    }
    else {
        context.exceptions.emplace_back(
            U"Was expecting an identifier after the `" + type + U"` keyword", token);
        context.ti++;
    }

    KH_PARSE_GUARD();
    token = context.tok();

    /* Parses each identifier after a dot `import a.b.c.d ...` */
    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Appends the identifier */
        if (token.type == kh::TokenType::IDENTIFIER) {
            if (kh::isReservedKeyword(token.value.identifier))
                context.exceptions.emplace_back(U"Was trying to " + type + U" a reserved keyword",
                                                token);

            path.push_back(token.value.identifier);
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();
        }
        else {
            context.exceptions.emplace_back(
                U"Was expecting an identifier after the dot in the " + type + U" statement", token);
            break;
        }
    }

    /* An optional `as` for changing the namespace name in import statements */
    if (!is_include && token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"as") {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Gets the set namespace identifier */
        if (token.type == kh::TokenType::IDENTIFIER) {
            if (kh::isReservedKeyword(token.value.identifier))
                context.exceptions.emplace_back(
                    U"Could not use a reserved keyword as the alias of the import", token);

            identifier = token.value.identifier;
        }
        else {
            context.exceptions.emplace_back(
                U"Was expecting an identifier after the `as` keyword in the import statement", token);
        }

        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

    /* Ensure that it ends with a semicolon */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
        context.ti++;
    else
        context.exceptions.emplace_back(U"Was expecting a semicolon after the " + type + U" statement",
                                        token);
end:
    return {index, path, is_include, is_relative,
            path.empty() ? U"" : (identifier.empty() ? path.back() : identifier)};
}

void kh::parseAccessAttribs(KH_PARSE_CTX, bool& is_static, bool& is_public) {
    is_static = false;
    is_public = true;

    /* It parses these kinds of access types: `[static | private/public] int x = 3` */

    kh::Token token = context.tok();
    while (token.type == kh::TokenType::IDENTIFIER) {
        if (token.value.identifier == U"static")
            is_static = true;
        else if (token.value.identifier == U"public")
            is_public = true;
        else if (token.value.identifier == U"private")
            is_public = false;
        else
            break;

        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

end:
    return;
}

kh::AstFunction kh::parseFunction(KH_PARSE_CTX, bool is_static, bool is_public, bool is_conditional) {
    std::vector<std::u32string> identifiers;
    std::vector<std::u32string> generic_args;
    std::vector<uint64_t> id_array;
    kh::AstIdentifiers return_type{0, {}, {}, {}, {}};
    std::vector<uint64_t> return_array = {};
    size_t return_refs = 0;
    std::vector<kh::AstDeclaration> arguments;
    std::vector<std::shared_ptr<kh::AstBody>> body;

    kh::Token token = context.tok();
    size_t index = token.index;

    if (!(token.type == kh::TokenType::SYMBOL &&
          token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)) {
        /* Parses the function's identifiers and generic args */
        kh::parseTopScopeIdentifiersAndGenericArgs(context, identifiers, generic_args);
        KH_PARSE_GUARD();
        token = context.tok();

        /* Array dimension method extension/overloading/overriding specifier `def float[3].add(float[3]
         * other) {}` */
        while (token.type == kh::TokenType::SYMBOL &&
               token.value.symbol_type == kh::Symbol::SQUARE_OPEN) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            if (token.type == kh::TokenType::INTEGER || token.type == kh::TokenType::UINTEGER) {
                if (token.value.uinteger == 0)
                    context.exceptions.emplace_back(U"An array could not be zero sized", token);

                id_array.push_back(token.value.uinteger);
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }
            else
                context.exceptions.emplace_back(U"Was expecting an integer for the array size", token);

            if (!(token.type == kh::TokenType::SYMBOL &&
                  token.value.symbol_type == kh::Symbol::SQUARE_CLOSE))
                context.exceptions.emplace_back(U"Was expecting a closing square bracket", token);

            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();
        }

        /* Extra identifier `def something!int.extraIdentifier() {}` */
        KH_PARSE_GUARD();
        token = context.tok();
        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            if (token.type == kh::TokenType::IDENTIFIER) {
                identifiers.push_back(token.value.identifier);
                context.ti++;
            }
            else
                context.exceptions.emplace_back(
                    U"Was expecting an identifier after the dot in the function declaration name",
                    token);
        }

        /* Ensures it has an opening parentheses */
        KH_PARSE_GUARD();
        token = context.tok();
        if (!(token.type == kh::TokenType::SYMBOL &&
              token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)) {
            context.exceptions.emplace_back(
                U"Was expecting an opening parentheses of the argument(s) in the function declaration",
                token);
            goto end;
        }
    }

    context.ti++;
    KH_PARSE_GUARD();
    token = context.tok();

    /* Loops until it reaches a closing parentheses */
    while (true) {
        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            break;

        /* Parses the argument */
        arguments.emplace_back(kh::parseDeclaration(context, false, true));

        KH_PARSE_GUARD();
        token = context.tok();

        if (token.type == kh::TokenType::SYMBOL) {
            /* Continues on parsing an argument if there's a comma */
            if (token.value.symbol_type == kh::Symbol::COMMA) {
                context.ti++;
                KH_PARSE_GUARD();
                continue;
            }
            /* Stops parsing arguments */
            else if (token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
                break;
            else {
                context.exceptions.emplace_back(U"Was expecting a closing parentheses or a comma in "
                                                U"the function declaration's argument(s)",
                                                token);
                goto end;
            }
        }
        else {
            context.exceptions.emplace_back(U"Was expecting a closing parentheses or a comma in the "
                                            U"function declaration's argument(s)",
                                            token);
            goto end;
        }
    }

    context.ti++;
    KH_PARSE_GUARD();
    token = context.tok();

    /* Specifying return type `def function() -> int {}` */
    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::SUB) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::MORE) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            /* Checks if the return type is a `ref`erence type */
            while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
                return_refs += 1;
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }

            return_type = kh::parseIdentifiers(context);
            KH_PARSE_GUARD();
            token = context.tok();

            /* Array return type */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::SQUARE_OPEN) {
                return_array = kh::parseArrayDimension(context, return_type);
            }
        }
        else {
            return_type = kh::AstIdentifiers(token.index, {U"void"}, {}, {}, {});

            context.exceptions.emplace_back(U"Was expecting an arrow specifying a return type", token);
        }
    }
    else {
        return_type = kh::AstIdentifiers(token.index, {U"void"}, {}, {}, {});
    }

    /* Parses the function's body */
    body = kh::parseBody(context);
end:
    return {index,       identifiers, generic_args, id_array,       return_array, return_type,
            return_refs, arguments,   body,         is_conditional, is_static,    is_public};
}

kh::AstDeclaration kh::parseDeclaration(KH_PARSE_CTX, bool is_static, bool is_public) {
    kh::AstIdentifiers var_type{0, {}, {}, {}, {}};
    std::vector<uint64_t> var_array = {};
    std::u32string var_name;
    std::shared_ptr<kh::AstExpression> expression = nullptr;
    size_t refs = 0;

    kh::Token token = context.tok();
    size_t index = token.index;

    /* Checks if the variable type is a `ref`erence type */
    while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
        refs += 1;
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

    /* Parses the variable's type */
    var_type = kh::parseIdentifiers(context);

    /* Possible array type `float[3] var;` */
    KH_PARSE_GUARD();
    token = context.tok();
    var_array = kh::parseArrayDimension(context, var_type);

    /* Gets the variable's name */
    KH_PARSE_GUARD();
    token = context.tok();
    if (token.type != kh::TokenType::IDENTIFIER) {
        context.exceptions.emplace_back(
            U"Was expecting an identifier of the variable declaration's name", token);
        goto end;
    }

    if (kh::isReservedKeyword(token.value.identifier))
        context.exceptions.emplace_back(U"Cannot use a reserved keyword as a variable name", token);

    var_name = token.value.identifier;
    context.ti++;
    KH_PARSE_GUARD();
    token = context.tok();

    /* The case where: `SomeClass x(1, 2, 3)` */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)
        expression.reset(kh::parseTuple(context));
    /* The case where: `int x = 3` */
    else if (token.type == kh::TokenType::OPERATOR &&
             token.value.operator_type == kh::Operator::ASSIGN) {
        context.ti++;
        KH_PARSE_GUARD();
        expression.reset(kh::parseExpression(context));
    }
    else
        goto end;
end:
    return {index, var_type, var_array, var_name, expression, refs, is_static, is_public};
}

kh::AstUserType kh::parseUserType(KH_PARSE_CTX, bool is_class) {
    std::vector<std::u32string> identifiers;
    std::shared_ptr<kh::AstIdentifiers> base;
    std::vector<std::u32string> generic_args;
    std::vector<kh::AstDeclaration> members;
    std::vector<kh::AstFunction> methods;

    kh::Token token = context.tok();
    size_t index = token.index;

    const std::u32string type_name = is_class ? U"class" : U"struct";

    /* Parses the class'/struct's identifiers and generic arguments */
    kh::parseTopScopeIdentifiersAndGenericArgs(context, identifiers, generic_args);
    KH_PARSE_GUARD();
    token = context.tok();

    /* Optional inheriting */
    if (token.type == kh::TokenType::SYMBOL &&
        token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();

        /* Parses base class' identifier */
        base.reset(new kh::AstIdentifiers(kh::parseIdentifiers(context)));
        KH_PARSE_GUARD();
        token = context.tok();

        /* Expects a closing parentheses */
        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            context.ti++;
        else
            context.exceptions.emplace_back(U"Was expecting a closing parentheses after inheritment "
                                            U"argument in the " +
                                                type_name + U" declaration",
                                            token);
    }

    KH_PARSE_GUARD();
    token = context.tok();

    /* Parses the body */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        while (true) {
            KH_PARSE_GUARD();
            kh::Token& token = context.tok();

            switch (token.type) {
                case kh::TokenType::IDENTIFIER: {
                    /* Methods */
                    if (token.value.identifier == U"def" || token.value.identifier == U"try") {
                        bool conditional = token.value.identifier == U"try";

                        context.ti++;
                        KH_PARSE_GUARD();

                        /* Case for conditional methods */
                        if (conditional) {
                            token = context.tok();
                            if (token.type == kh::TokenType::IDENTIFIER &&
                                token.value.identifier == U"def") {
                                context.ti++;
                                KH_PARSE_GUARD();
                            }
                            else
                                context.exceptions.emplace_back(
                                    U"Was expecting `def` after `try` at the top scope", token);
                        }

                        bool is_static, is_public;
                        /* Parse access types */
                        kh::parseAccessAttribs(context, is_static, is_public);
                        KH_PARSE_GUARD();

                        /* Parse function declaration */
                        methods.push_back(
                            kh::parseFunction(context, is_static, is_public, conditional));

                        /* Ensures that methods don't have generic argument(s) */
                        if (methods.back().generic_args.empty()) {
                            context.exceptions.emplace_back(
                                U"A method cannot have (a) generic argument(s)", token);
                        }

                        /* Nor a lambda.. */
                        if (methods.back().identifiers.empty())
                            context.exceptions.emplace_back(U"A method cannot be a lambda", token);
                    }
                    /* Member/class variables */
                    else {
                        bool is_static, is_public;
                        /* Parse access types */
                        kh::parseAccessAttribs(context, is_static, is_public);
                        KH_PARSE_GUARD();

                        /* Parse variable declaration */
                        members.push_back(kh::parseDeclaration(context, is_static, is_public));

                        KH_PARSE_GUARD();
                        token = context.tok();
                        /* Expects semicolon */
                        if (token.type == kh::TokenType::SYMBOL &&
                            token.value.symbol_type == kh::Symbol::SEMICOLON)
                            context.ti++;
                        else {
                            context.ti++;
                            context.exceptions.emplace_back(U"Was expecting a semicolon after a "
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
                            context.ti++;
                        } break;

                        /* Closing curly bracket where it ends the class body */
                        case kh::Symbol::CURLY_CLOSE: {
                            context.ti++;
                            goto end;
                        } break;

                        default:
                            context.ti++;
                            context.exceptions.emplace_back(
                                U"Unexpected symbol while parsing the " + type_name + U" body", token);
                    }
                } break;

                default:
                    context.ti++;
                    context.exceptions.emplace_back(
                        U"Unexpected token while parsing the " + type_name + U" body", token);
            }
        }
    }
    else
        context.exceptions.emplace_back(
            U"Was expecting an opening curly bracket after the " + type_name + U" declaration", token);
end:
    return {index, identifiers, base, generic_args, members, methods, is_class};
}

kh::AstEnumType kh::parseEnum(KH_PARSE_CTX) {
    std::vector<std::u32string> identifiers;
    std::vector<std::u32string> members;
    std::vector<uint64_t> values;

    /* Internal enum counter */
    uint64_t counter = 0;

    kh::Token token = context.tok();
    size_t index = token.index;

    /* Gets the enum identifiers */
    std::vector<std::u32string> _generic_args;
    kh::parseTopScopeIdentifiersAndGenericArgs(context, identifiers, _generic_args);
    if (!_generic_args.empty())
        context.exceptions.emplace_back(U"An enum could not have generic arguments", token);

    KH_PARSE_GUARD();
    token = context.tok();

    /* Opens with a curly bracket */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Parses the enum content */
        while (true) {
            /* Stops parsing enum body */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                context.ti++;
                break;
            }
            /* Appends member */
            else if (token.type == kh::TokenType::IDENTIFIER)
                members.push_back(token.value.identifier);
            else {
                context.exceptions.emplace_back(U"Unexpected token while parsing the enum body", token);

                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
                continue;
            }

            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            /* Checks if there's an assignment operation on an enum member */
            if (token.type == kh::TokenType::OPERATOR &&
                token.value.operator_type == kh::Operator::ASSIGN) {
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();

                /* Ensures there's an integer constant */
                if (token.type == kh::TokenType::INTEGER || token.type == kh::TokenType::UINTEGER) {
                    /* Don't worry about this line as it's a union */
                    values.push_back(token.value.uinteger);
                    counter = token.value.uinteger + 1;

                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();
                }
                else {
                    context.exceptions.emplace_back(U"Was expecting an integer constant after the "
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
                    context.exceptions.emplace_back(U"This enum member has the same name as #" +
                                                        kh::str((uint64_t)member + 1),
                                                    token);
                    break;
                }

                if (values[member] == values.back()) {
                    context.exceptions.emplace_back(
                        U"This enum member has a same index value as " + members[member], token);
                    break;
                }
            }

            /* Stops parsing enum body */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                context.ti++;
                break;
            }
            /* Ensures a comma after an enum member */
            else if (!(token.type == kh::TokenType::SYMBOL &&
                       token.value.symbol_type == kh::Symbol::COMMA))
                context.exceptions.emplace_back(U"Was expecting a closing curly bracket or a comma "
                                                U"after an enum member in the enum body",
                                                token);

            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();
        }
    }
    else
        context.exceptions.emplace_back(
            U"Was expecting an opening curly bracket after the enum declaration", token);
end:
    return {index, identifiers, members, values};
}

std::vector<std::shared_ptr<kh::AstBody>> kh::parseBody(KH_PARSE_CTX, size_t loop_count) {
    std::vector<std::shared_ptr<kh::AstBody>> body;
    kh::Token token = context.tok();
    bool is_single_liner = false;

    /* Checks if it's a single liner statement `if a < b : something();` */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COLON)
        is_single_liner = true;
    /* Expects an opening curly bracket */
    else if (!(token.type == kh::TokenType::SYMBOL &&
               token.value.symbol_type == kh::Symbol::CURLY_OPEN)) {
        context.exceptions.emplace_back(U"Was expecting an opening curly bracket", token);
        goto end;
    }

    context.ti++;

    /* Parses the body */
    while (true) {
        KH_PARSE_GUARD();
        token = context.tok();
        size_t index = token.index;

        switch (token.type) {
            case kh::TokenType::IDENTIFIER: {
                if (token.value.identifier == U"if") {
                    std::vector<std::shared_ptr<kh::AstExpression>> conditions;
                    std::vector<std::vector<std::shared_ptr<kh::AstBody>>> bodies;
                    std::vector<std::shared_ptr<kh::AstBody>> else_body;

                    do {
                        /* Parses the expression and if body */
                        context.ti++;
                        token = context.tok();
                        KH_PARSE_GUARD();
                        conditions.emplace_back(kh::parseExpression(context));
                        KH_PARSE_GUARD();
                        bodies.emplace_back(kh::parseBody(context, loop_count + 1));
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Recontinues if there's an else if (`elif`) clause */
                    } while (token.type == kh::TokenType::IDENTIFIER &&
                             token.value.identifier == U"elif");

                    /* Parses the body if there's an `else` clause */
                    if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"else") {
                        context.ti++;
                        KH_PARSE_GUARD();
                        else_body = kh::parseBody(context, loop_count + 1);
                    }

                    body.emplace_back(new kh::AstIf(index, conditions, bodies, else_body));
                }
                /* While statement */
                else if (token.value.identifier == U"while") {
                    context.ti++;
                    KH_PARSE_GUARD();

                    /* Parses the expression and body */
                    std::shared_ptr<kh::AstExpression> condition(kh::parseExpression(context));
                    std::vector<std::shared_ptr<kh::AstBody>> while_body =
                        kh::parseBody(context, loop_count + 1);

                    body.emplace_back(new kh::AstWhile(index, condition, while_body));
                }
                /* Do while statement */
                else if (token.value.identifier == U"do") {
                    context.ti++;
                    KH_PARSE_GUARD();

                    /* Parses the body */
                    std::vector<std::shared_ptr<kh::AstBody>> do_while_body =
                        kh::parseBody(context, loop_count + 1);
                    std::shared_ptr<kh::AstExpression> condition;

                    KH_PARSE_GUARD();
                    token = context.tok();

                    /* Expects `while` and then parses the condition expression */
                    if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"while") {
                        context.ti++;
                        condition.reset(kh::parseExpression(context));
                    }
                    else
                        context.exceptions.emplace_back(
                            U"Was expecting `while` after the `do {...}` body", token);

                    KH_PARSE_GUARD();
                    token = context.tok();

                    /* Expects a semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        context.ti++;
                    else
                        context.exceptions.emplace_back(
                            U"Was expecting a semicolon after the `do {...} while (...)` statement",
                            token);

                    body.emplace_back(new kh::AstDoWhile(index, condition, do_while_body));
                }
                /* For statement */
                else if (token.value.identifier == U"for") {
                    context.ti++;
                    KH_PARSE_GUARD();

                    std::shared_ptr<kh::AstExpression> target_or_initializer(
                        kh::parseExpression(context));

                    KH_PARSE_GUARD();
                    token = context.tok();
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::COLON) {
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();

                        std::shared_ptr<kh::AstExpression> iterator(kh::parseExpression(context));
                        KH_PARSE_GUARD();
                        std::vector<std::shared_ptr<kh::AstBody>> foreach_body =
                            kh::parseBody(context, loop_count + 1);

                        body.emplace_back(
                            new kh::AstForEach(index, target_or_initializer, iterator, foreach_body));
                    }
                    else if (token.type == kh::TokenType::SYMBOL &&
                             token.value.symbol_type == kh::Symbol::COMMA) {
                        context.ti++;
                        KH_PARSE_GUARD();
                        std::shared_ptr<kh::AstExpression> condition(kh::parseExpression(context));
                        KH_PARSE_GUARD();
                        token = context.tok();

                        if (token.type == kh::TokenType::SYMBOL &&
                            token.value.symbol_type == kh::Symbol::COMMA) {
                            context.ti++;
                            KH_PARSE_GUARD();
                        }
                        else
                            context.exceptions.emplace_back(
                                U"Was expecting a comma after the `for` condition expression", token);

                        std::shared_ptr<kh::AstExpression> step(kh::parseExpression(context));
                        KH_PARSE_GUARD();
                        std::vector<std::shared_ptr<kh::AstBody>> for_body =
                            kh::parseBody(context, loop_count + 1);

                        body.emplace_back(
                            new kh::AstFor(index, target_or_initializer, condition, step, for_body));
                    }
                    else
                        context.exceptions.emplace_back(
                            U"Was expecting a colon or a comma after the `for` target/initializer",
                            token);
                }
                /* `continue` statement */
                else if (token.value.identifier == U"continue") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();

                    if (!loop_count)
                        context.exceptions.emplace_back(
                            U"`continue` cannot be used outside of `while` or `for` loops", token);

                    size_t loop_breaks = 0;
                    /* Continuing multiple loops `continue 4;` */
                    if (token.type == kh::TokenType::UINTEGER || token.type == kh::TokenType::INTEGER) {
                        if (token.value.uinteger >= loop_count)
                            context.exceptions.emplace_back(
                                U"Trying to `continue` an invalid amount of loops", token);

                        loop_breaks = token.value.uinteger;
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();
                    }

                    /* Expects semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        context.ti++;
                    else
                        context.exceptions.emplace_back(
                            U"Was expecting a semicolon or an integer after `continue`", token);

                    body.emplace_back(
                        new kh::AstStatement(index, kh::AstStatement::Type::CONTINUE, loop_breaks));
                }
                /* `break` statement */
                else if (token.value.identifier == U"break") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();

                    if (!loop_count)
                        context.exceptions.emplace_back(
                            U"`break` cannot be used outside of `while` or `for` loops", token);

                    size_t loop_breaks = 0;
                    /* Breaking multiple loops `break 2;` */
                    if (token.type == kh::TokenType::UINTEGER || token.type == kh::TokenType::INTEGER) {
                        if (token.value.uinteger >= loop_count)
                            context.exceptions.emplace_back(
                                U"Trying to `break` an invalid amount of loops", token);

                        loop_breaks = token.value.uinteger;
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();
                    }

                    /* Expects semicolon */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        context.ti++;
                    else
                        context.exceptions.emplace_back(
                            U"Was expecting a semicolon or an integer after `break`", token);

                    body.emplace_back(
                        new kh::AstStatement(index, kh::AstStatement::Type::BREAK, loop_breaks));
                }
                /* `return` statement */
                else if (token.value.identifier == U"return") {
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();

                    std::shared_ptr<kh::AstExpression> expression((kh::AstExpression*)nullptr);

                    /* No expression given */
                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::SEMICOLON)
                        context.ti++;
                    /* If there's a provided return value expression */
                    else {
                        expression.reset(kh::parseExpression(context));
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Expects semicolon */
                        if (token.type == kh::TokenType::SYMBOL &&
                            token.value.symbol_type == kh::Symbol::SEMICOLON)
                            context.ti++;
                        else
                            context.exceptions.emplace_back(
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
                        context.ti++;
                    } break;

                    /* Ends body */
                    case kh::Symbol::CURLY_CLOSE: {
                        if (is_single_liner)
                            context.exceptions.emplace_back(
                                U"Unexpected closing curly bracket in a single liner statement", token);

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
                std::shared_ptr<kh::AstExpression> expr(kh::parseExpression(context));
                KH_PARSE_GUARD();
                token = context.tok();

                /* Expects a semicolon */
                if (token.type == kh::TokenType::SYMBOL &&
                    token.value.symbol_type == kh::Symbol::SEMICOLON)
                    context.ti++;
                else
                    context.exceptions.emplace_back(U"Was expecting a semicolon", token);

                body.emplace_back(expr);
            }
        }

        if (is_single_liner)
            break;
    }
end:
    return body;
}

void kh::parseTopScopeIdentifiersAndGenericArgs(KH_PARSE_CTX, std::vector<std::u32string>& identifiers,
                                                std::vector<std::u32string>& generic_args) {
    kh::Token token = context.tok();
    goto forceIn;

    /* Parses the identifiers */
    do {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

    forceIn:
        if (token.type == kh::TokenType::IDENTIFIER) {
            if (kh::isReservedKeyword(token.value.identifier))
                context.exceptions.emplace_back(U"Cannot use a reserved keyword as an identifier",
                                                token);

            identifiers.push_back(token.value.identifier);
            context.ti++;
        }
        else
            context.exceptions.emplace_back(U"Was expecting an identifier", token);

        KH_PARSE_GUARD();
        token = context.tok();
    } while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT);

    /* Generic arguments */
    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::NOT) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* a.b.c!T */
        if (token.type == kh::TokenType::IDENTIFIER) {
            if (kh::isReservedKeyword(token.value.identifier))
                context.exceptions.emplace_back(
                    U"Cannot use a reserved keyword as an identifier of a generic argument", token);

            generic_args.push_back(token.value.identifier);
            context.ti++;
        }
        /* a.b.c!(A, B) */
        else if (token.type == kh::TokenType::SYMBOL &&
                 token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();
            goto forceInGenericArgs;

            do {
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();

            forceInGenericArgs:
                if (token.type == kh::TokenType::IDENTIFIER) {
                    if (kh::isReservedKeyword(token.value.identifier))
                        context.exceptions.emplace_back(
                            U"Cannot use a reserved keyword as an identifier of a generic argument",
                            token);

                    generic_args.push_back(token.value.identifier);
                    context.ti++;
                }
                else
                    context.exceptions.emplace_back(U"Was expecting a generic argument identifier",
                                                    token);

                KH_PARSE_GUARD();
                token = context.tok();
            } while (token.type == kh::TokenType::SYMBOL &&
                     token.value.symbol_type == kh::Symbol::COMMA);

            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
                context.ti++;
            else
                context.exceptions.emplace_back(U"Was expecting a closing parentheses", token);
        }
    }

end:
    return;
}
