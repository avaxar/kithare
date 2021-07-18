/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/parser.hpp>


using namespace kh;

#define RECURSIVE_DESCENT_SINGULAR_OP(lower)                                                   \
    do {                                                                                       \
        AstExpression* expr = lower(context);                                                  \
        Token token;                                                                           \
        size_t index;                                                                          \
        KH_PARSE_GUARD();                                                                      \
        token = context.tok();                                                                 \
        index = token.index;                                                                   \
        while (token.type == TokenType::OPERATOR) {                                            \
            bool has_op = false;                                                               \
            for (const Operator op : operators) {                                              \
                if (token.value.operator_type == op) {                                         \
                    has_op = true;                                                             \
                    break;                                                                     \
                }                                                                              \
            }                                                                                  \
            if (!has_op)                                                                       \
                break;                                                                         \
            context.ti++;                                                                      \
            KH_PARSE_GUARD();                                                                  \
            std::shared_ptr<AstExpression> lval(expr);                                         \
            std::shared_ptr<AstExpression> rval(lower(context));                               \
            expr = new AstBinaryOperation(token.index, token.value.operator_type, lval, rval); \
            KH_PARSE_GUARD();                                                                  \
            token = context.tok();                                                             \
        }                                                                                      \
        KH_PARSE_GUARD();                                                                      \
        token = context.tok();                                                                 \
    end:                                                                                       \
        return expr;                                                                           \
    } while (false)

AstExpression* kh::parseExpression(const std::vector<Token>& tokens) {
    std::vector<ParseException> exceptions;
    ParserContext context{tokens, exceptions};
    AstExpression* ast = parseExpression(context);

    if (exceptions.empty()) {
        return ast;
    }
    else {
        throw exceptions;
    }
}

AstExpression* kh::parseExpression(KH_PARSE_CTX) {
    Token token = context.tok();
    size_t index = token.index;

    return parseAssignOps(context);
end:
    return nullptr;
}

AstExpression* kh::parseAssignOps(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::ASSIGN, Operator::IADD, Operator::ISUB,
                                              Operator::IMUL,   Operator::IDIV, Operator::IMOD,
                                              Operator::IPOW};
    RECURSIVE_DESCENT_SINGULAR_OP(parseTernary);
}

AstExpression* kh::parseTernary(KH_PARSE_CTX) {
    AstExpression* expr = parseOr(context);
    Token token;
    size_t index;

    KH_PARSE_GUARD();
    token = context.tok();
    index = token.index;

    while (token.type == TokenType::IDENTIFIER && token.value.identifier == "if") {
        index = token.index;

        context.ti++;
        KH_PARSE_GUARD();
        std::shared_ptr<AstExpression> condition(parseOr(context));

        KH_PARSE_GUARD();
        token = context.tok();

        if (!(token.type == TokenType::IDENTIFIER && token.value.identifier == "else")) {
            context.exceptions.emplace_back(
                "expected an `else` to specify the else case of the ternary expression", token);
            goto end;
        }

        context.ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<AstExpression> value(expr);
        std::shared_ptr<AstExpression> otherwise(parseOr(context));
        expr = new AstTernaryOperation(index, condition, value, otherwise);

        KH_PARSE_GUARD();
        token = context.tok();
    }
end:
    return expr;
}

AstExpression* kh::parseOr(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::OR};
    RECURSIVE_DESCENT_SINGULAR_OP(parseAnd);
}

AstExpression* kh::parseAnd(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::AND};
    RECURSIVE_DESCENT_SINGULAR_OP(parseNot);
}

AstExpression* kh::parseNot(KH_PARSE_CTX) {
    AstExpression* expr = nullptr;
    Token token = context.tok();
    size_t index = token.index;

    if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::NOT) {
        context.ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<AstExpression> rval(parseNot(context));
        expr = new AstUnaryOperation(token.index, token.value.operator_type, rval);
    }
    else {
        expr = parseComparison(context);
    }
end:
    return expr;
}

AstExpression* kh::parseComparison(KH_PARSE_CTX) {
    union {
        AstExpression* expr;
        AstComparisonExpression* comparison_expr;
    };

    expr = parseBitwiseOr(context);
    Token token;
    size_t index;

    KH_PARSE_GUARD();
    token = context.tok();
    index = token.index;

    if (token.type == TokenType::OPERATOR && (token.value.operator_type == Operator::EQUAL ||
                                              token.value.operator_type == Operator::NOT_EQUAL ||
                                              token.value.operator_type == Operator::LESS ||
                                              token.value.operator_type == Operator::LESS_EQUAL ||
                                              token.value.operator_type == Operator::MORE ||
                                              token.value.operator_type == Operator::MORE_EQUAL)) {
        comparison_expr =
            new AstComparisonExpression(index, {}, {std::shared_ptr<AstExpression>(expr)});

        while (token.type == TokenType::OPERATOR &&
               (token.value.operator_type == Operator::EQUAL ||
                token.value.operator_type == Operator::NOT_EQUAL ||
                token.value.operator_type == Operator::LESS ||
                token.value.operator_type == Operator::LESS_EQUAL ||
                token.value.operator_type == Operator::MORE ||
                token.value.operator_type == Operator::MORE_EQUAL)) {
            context.ti++;
            KH_PARSE_GUARD();
            comparison_expr->operations.push_back(token.value.operator_type);
            comparison_expr->values.emplace_back(parseBitwiseOr(context));
            KH_PARSE_GUARD();
            token = context.tok();
        }
    }

end:
    return expr;
}

AstExpression* kh::parseBitwiseOr(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::BIT_OR};
    RECURSIVE_DESCENT_SINGULAR_OP(parseBitwiseAnd);
}

AstExpression* kh::parseBitwiseAnd(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::BIT_OR};
    RECURSIVE_DESCENT_SINGULAR_OP(parseBitwiseShift);
}

AstExpression* kh::parseBitwiseShift(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::BIT_LSHIFT, Operator::BIT_RSHIFT};
    RECURSIVE_DESCENT_SINGULAR_OP(parseAddSub);
}

AstExpression* kh::parseAddSub(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::ADD, Operator::SUB};
    RECURSIVE_DESCENT_SINGULAR_OP(parseMulDivMod);
}

AstExpression* kh::parseMulDivMod(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::MUL, Operator::DIV, Operator::MOD};
    RECURSIVE_DESCENT_SINGULAR_OP(parseUnary);
}

AstExpression* kh::parseUnary(KH_PARSE_CTX) {
    AstExpression* expr = nullptr;
    Token token = context.tok();
    size_t index = token.index;

    if (token.type == TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case Operator::ADD:
            case Operator::SUB:
            case Operator::INCREMENT:
            case Operator::DECREMENT:
            case Operator::BIT_NOT:
            case Operator::SIZEOF:
            case Operator::ADDRESS: {
                context.ti++;
                KH_PARSE_GUARD();

                std::shared_ptr<AstExpression> rval(parseUnary(context));
                expr = new AstUnaryOperation(token.index, token.value.operator_type, rval);
            } break;

            default:
                context.ti++;
                context.exceptions.emplace_back(
                    "unexpected `" + encodeUtf8(strfy(token)) + "` in an expression", token);
        }
    }
    else {
        expr = parseExponentiation(context);
    }
end:
    return expr;
}

AstExpression* kh::parseExponentiation(KH_PARSE_CTX) {
    static std::vector<Operator> operators = {Operator::POW};
    RECURSIVE_DESCENT_SINGULAR_OP(parseRevUnary);
}

AstExpression* kh::parseRevUnary(KH_PARSE_CTX) {
    Token token = context.tok();
    size_t index = token.index;
    AstExpression* expr = parseOthers(context);

    KH_PARSE_GUARD();
    token = context.tok();

    while ((token.type == TokenType::OPERATOR && token.value.operator_type == Operator::INCREMENT ||
            token.value.operator_type == Operator::DECREMENT) ||
           (token.type == TokenType::SYMBOL && (token.value.symbol_type == Symbol::DOT ||
                                                token.value.symbol_type == Symbol::PARENTHESES_OPEN ||
                                                token.value.symbol_type == Symbol::SQUARE_OPEN))) {
        index = token.index;

        /* Post-incrementation and decrementation */
        if (token.type == TokenType::OPERATOR) {
            std::shared_ptr<AstExpression> expr_ptr(expr);
            expr = new AstRevUnaryOperation(index, token.value.operator_type, expr_ptr);
            context.ti++;
        }
        else {
            switch (token.value.symbol_type) {
                /* Scoping expression */
                case Symbol::DOT: {
                    std::vector<std::string> identifiers;

                    do {
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Expects an identifier for which to be scoped through from the expression */
                        if (token.type == TokenType::IDENTIFIER) {
                            identifiers.push_back(token.value.identifier);
                        }
                        else {
                            context.exceptions.emplace_back("expected an identifier", token);
                        }
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Continues again for another scope in */
                    } while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT);

                    std::shared_ptr<AstExpression> exprptr(expr);
                    expr = new AstScoping(index, exprptr, identifiers);
                } break;

                    /* Calling expression */
                case Symbol::PARENTHESES_OPEN: {
                    std::shared_ptr<AstExpression> exprptr(expr);
                    /* Parses the argument(s) */
                    AstTuple* tuple = static_cast<AstTuple*>(parseTuple(context));
                    std::vector<std::shared_ptr<AstExpression>> arguments;

                    for (std::shared_ptr<AstExpression>& element : tuple->elements) {
                        arguments.push_back(element);
                    }

                    expr = new AstCallExpression(index, exprptr, arguments);
                    delete tuple;
                } break;

                    /* Subscription expression */
                case Symbol::SQUARE_OPEN: {
                    std::shared_ptr<AstExpression> exprptr(expr);
                    /* Parses argument(s) */
                    AstTuple* tuple = static_cast<AstTuple*>(
                        parseTuple(context, Symbol::SQUARE_OPEN, Symbol::SQUARE_CLOSE));
                    std::vector<std::shared_ptr<AstExpression>> arguments;

                    for (std::shared_ptr<AstExpression>& element : tuple->elements) {
                        arguments.push_back(element);
                    }

                    expr = new AstSubscriptExpression(index, exprptr, arguments);
                    delete tuple;
                } break;

                default: {
                    /* Unlikely to happen; This is to silence a compiler warning */
                }
            }
        }

        KH_PARSE_GUARD();
        token = context.tok();
    }
end:
    return expr;
}

AstExpression* kh::parseOthers(KH_PARSE_CTX) {
    AstExpression* expr = nullptr;
    Token token = context.tok();
    size_t index = token.index;

    switch (token.type) {
            /* For all of these literal values be given the AST constant value instance */

        case TokenType::CHARACTER:
            expr = new AstValue(token.index, token.value.character);
            context.ti++;
            break;

        case TokenType::UINTEGER:
            expr = new AstValue(token.index, token.value.uinteger);
            context.ti++;
            break;

        case TokenType::INTEGER:
            expr = new AstValue(token.index, token.value.integer);
            context.ti++;
            break;

        case TokenType::FLOATING:
            expr = new AstValue(token.index, token.value.floating);
            context.ti++;
            break;

        case TokenType::IMAGINARY:
            expr = new AstValue(token.index, token.value.imaginary, AstValue::ValueType::IMAGINARY);
            context.ti++;
            break;

        case TokenType::STRING:
            expr = new AstValue(token.index, token.value.string);
            context.ti++;

            KH_PARSE_GUARD();
            token = context.tok();

            /* Auto concatenation */
            while (token.type == TokenType::STRING) {
                ((AstValue*)expr)->string += token.value.string;
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }

            break;

        case TokenType::BUFFER:
            expr = new AstValue(token.index, token.value.buffer);
            context.ti++;

            KH_PARSE_GUARD();
            token = context.tok();

            /* Auto concatenation */
            while (token.type == TokenType::BUFFER) {
                ((AstValue*)expr)->buffer += token.value.buffer;
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }

            break;

        case TokenType::IDENTIFIER:
            /* Lambda expression */
            if (token.value.identifier == "def") {
                context.ti++;
                KH_PARSE_GUARD();
                AstFunction lambda = parseFunction(context, false);

                if (!lambda.identifiers.empty()) {
                    context.exceptions.emplace_back(
                        "a non-lambda function cannot be defined in an expression", token);
                }

                return new AstFunction(lambda);
            }
            /* Variable declaration */
            else if (token.value.identifier == "ref" || token.value.identifier == "static") {
                AstDeclaration* declaration = new AstDeclaration(parseDeclaration(context));
                declaration->is_static = token.value.identifier == "static";
                return declaration;
            }
            else {
                size_t _ti = context.ti;
                expr = new AstIdentifiers(parseIdentifiers(context));

                KH_PARSE_GUARD();
                token = context.tok();

                /* An identifier is next to another identifier `int number` */
                if (token.type == TokenType::IDENTIFIER && token.value.identifier != "if" &&
                    token.value.identifier != "else") {
                    context.ti = _ti;
                    delete expr;
                    expr = new AstDeclaration(parseDeclaration(context));
                }
                /* An opening square parentheses next to an idenifier, possible array variable
                 * declaration */
                else if (token.type == TokenType::SYMBOL &&
                         token.value.symbol_type == Symbol::SQUARE_OPEN) {
                    size_t exception_counts = context.exceptions.size();

                    parseArrayDimension(context, *static_cast<AstIdentifiers*>(expr));
                    delete expr;

                    /* If there was exceptions while parsing the array dimension type, it probably
                     * wasn't an array variable declaration.. rather a subscript or something */
                    if (context.exceptions.size() > exception_counts) {
                        for (size_t i = 0; i < context.exceptions.size() - exception_counts; i++) {
                            context.exceptions.pop_back();
                        }

                        context.ti = _ti;
                        expr = new AstIdentifiers(parseIdentifiers(context));
                    }
                    else {
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Confirmed that it's an array declaration `float[3] position;` */
                        if (token.type == TokenType::IDENTIFIER && token.value.identifier != "if" &&
                            token.value.identifier != "else") {
                            context.ti = _ti;
                            expr = new AstDeclaration(parseDeclaration(context));
                        }
                        /* Probably was just a normal subscript */
                        else {
                            context.ti = _ti;
                            expr = new AstIdentifiers(parseIdentifiers(context));
                        }
                    }
                }
            }
            break;

        case TokenType::SYMBOL:
            switch (token.value.symbol_type) {
                /* Parentheses/tuple expression */
                case Symbol::PARENTHESES_OPEN:
                    expr =
                        parseTuple(context, Symbol::PARENTHESES_OPEN, Symbol::PARENTHESES_CLOSE, false);
                    break;

                    /* List literal expression */
                case Symbol::SQUARE_OPEN:
                    expr = parseList(context);
                    break;

                    /* Dict literal expression */
                case Symbol::CURLY_OPEN:
                    expr = parseDict(context);
                    break;

                default:
                    context.exceptions.emplace_back(
                        "unexpected `" + encodeUtf8(strfy(token)) + "` in an expression", token);
                    context.ti++;
                    goto end;
            }
            break;

        default:
            context.exceptions.emplace_back(
                "unexpected `" + encodeUtf8(strfy(token)) + "` in an expression", token);
            context.ti++;
            goto end;
    }
end:
    return expr;
}

AstIdentifiers kh::parseIdentifiers(KH_PARSE_CTX) {
    std::vector<std::string> identifiers;
    std::vector<AstIdentifiers> generics;
    std::vector<size_t> generics_refs;
    std::vector<std::vector<uint64_t>> generics_array;

    bool is_function = false;

    Token token = context.tok();
    size_t index = token.index;

    /* Expects an identifier */
    if (token.type == TokenType::IDENTIFIER) {
        if (isReservedKeyword(token.value.identifier)) {
            context.exceptions.emplace_back("cannot use a reserved keyword as an identifier", token);
        }

        identifiers.push_back(token.value.identifier);
        context.ti++;
    }
    else {
        context.exceptions.emplace_back("expected an identifier", token);
        goto end;
    }

    KH_PARSE_GUARD();
    token = context.tok();

    /* For each scope in with a dot symbol */
    while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Appends the identifier */
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                context.exceptions.emplace_back("cannot use a reserved keyword as an identifier",
                                                token);
            }
            identifiers.push_back(token.value.identifier);
        }
        else {
            context.exceptions.emplace_back("expected an identifier after the dot", token);
            goto end;
        }

        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

    is_function = identifiers.size() == 1 && identifiers[0] == "func";

    /* Optional genericization */
    if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::NOT) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            generics_refs.push_back(0);
            while (token.type == TokenType::IDENTIFIER && token.value.identifier == "ref") {
                generics_refs.back() += 1;
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }

            /* Parses the genericization type arguments */
            generics.push_back(parseIdentifiers(context));
            generics_array.push_back(parseArrayDimension(context, generics.back()));
            KH_PARSE_GUARD();
            token = context.tok();

            if (is_function) {
                if (token.type == TokenType::SYMBOL &&
                    token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();

                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                        context.ti++;
                        goto funcFinish;
                    }
                    else {
                        goto forceIn;
                    }
                }
                else {
                    context.exceptions.emplace_back("expected an opening parentheses after the "
                                                    "return type in the genericization of `func`",
                                                    token);
                }
            }

            while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA) {
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();

            forceIn:
                generics_refs.push_back(0);
                while (token.type == TokenType::IDENTIFIER && token.value.identifier == "ref") {
                    generics_refs.back() += 1;
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();
                }

                generics.emplace_back(parseIdentifiers(context));
                generics_array.push_back(parseArrayDimension(context, generics.back()));

                KH_PARSE_GUARD();
                token = context.tok();
            }

            /* Expects closing parentheses */
            if (token.type == TokenType::SYMBOL &&
                token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                context.ti++;

                if (is_function) {
                funcFinish:
                    KH_PARSE_GUARD();
                    token = context.tok();

                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                        context.ti++;
                    }
                    else {
                        context.exceptions.emplace_back("expected a closing parentheses", token);
                    }
                }
            }
            else {
                context.exceptions.emplace_back("expected a closing parentheses", token);
            }
        }
        else if (token.type == TokenType::IDENTIFIER) {
            if (is_function) {
                context.exceptions.emplace_back(
                    "expected an opening parentheses for genericization of `func`", token);
            }
            generics.push_back(parseIdentifiers(context));
            generics_refs.push_back(0);
            generics_array.push_back({});
        }
        else {
            context.exceptions.emplace_back("expected either an identifier or an opening "
                                            "parentheses for genericization after the "
                                            "exclamation mark ",
                                            token);
            context.ti++;
        }
    }
    else if (is_function) {
        context.exceptions.emplace_back("`func` requires genericization", token);
    }
end:
    return {index, identifiers, generics, generics_refs, generics_array};
}

AstExpression* kh::parseTuple(KH_PARSE_CTX, Symbol opening, Symbol closing, bool explicit_tuple) {
    std::vector<AstExpression*> elements;

    Token token = context.tok();
    size_t index = token.index;

    /* Expects the opening symbol */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == opening) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Instant close */
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == closing) {
            context.ti++;
            goto end;
        }

        while (true) {
            /* Parses the element expression */
            elements.push_back(parseExpression(context));
            KH_PARSE_GUARD();
            token = context.tok();

            if (token.type == TokenType::SYMBOL && token.value.symbol_type == closing) {
                context.ti++;
                break;
            }
            else if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA)) {
                context.exceptions.emplace_back(closing == Symbol::SQUARE_CLOSE
                                                    ? "expected a comma or a closing square bracket"
                                                    : "expected a comma or a closing parentheses",
                                                token);
                context.ti++;
                break;
            }

            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            /* Cases for explicit one-elemented tuples `(69420,)` */
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == closing) {
                context.ti++;
                explicit_tuple = true;
                break;
            }
        }
    }
    else {
        context.exceptions.emplace_back(opening == Symbol::SQUARE_OPEN
                                            ? "expected an opening square bracket"
                                            : "expected an opening parentheses",
                                        token);
        context.ti++;
    }

end:
    if (!explicit_tuple && elements.size() == 1) {
        for (size_t i = 1; i < elements.size(); i++) {
            delete elements[i];
        }

        return elements[0];
    }
    else {
        std::vector<std::shared_ptr<AstExpression>> _elements;
        _elements.reserve(elements.size());

        for (AstExpression* element : elements) {
            _elements.emplace_back(element);
        }

        return new AstTuple(index, _elements);
    }
}

AstExpression* kh::parseList(KH_PARSE_CTX) {
    Token token = context.tok();
    size_t index = token.index;

    AstTuple* tuple = (AstTuple*)parseTuple(context, Symbol::SQUARE_OPEN, Symbol::SQUARE_CLOSE, false);
    AstList* list = new AstList(tuple->index, tuple->elements);
    delete tuple;

    return list;
}

AstExpression* kh::parseDict(KH_PARSE_CTX) {
    std::vector<std::shared_ptr<AstExpression>> keys;
    std::vector<std::shared_ptr<AstExpression>> items;

    Token token = context.tok();
    size_t index = token.index;

    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Instant close*/
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_CLOSE) {
            context.ti++;
            goto end;
        }
        else {
            goto forceIn;
        }
        do {
            context.ti++;
            KH_PARSE_GUARD();
        forceIn:
            keys.emplace_back(parseExpression(context));

            KH_PARSE_GUARD();
            token = context.tok();
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COLON) {
                context.ti++;
                KH_PARSE_GUARD();
            }
            else {
                context.exceptions.emplace_back("expected a colon after a key of the dict literal",
                                                token);
            }
            items.emplace_back(parseExpression(context));
            KH_PARSE_GUARD();
            token = context.tok();
        } while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA);

        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_CLOSE) {
            context.ti++;
        }
        else {
            context.exceptions.emplace_back("expected a closing curly bracket closing the dict literal",
                                            token);
        }
    }
    else {
        context.exceptions.emplace_back("expected an opening curly bracket for the dict literal",
                                        token);
    }
end:
    return new AstDict(index, keys, items);
}

std::vector<uint64_t> kh::parseArrayDimension(KH_PARSE_CTX, AstIdentifiers& type) {
    std::vector<uint64_t> dimension;

    Token token = context.tok();
    size_t index = token.index;

    while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_CLOSE) {
            type = AstIdentifiers(token.index, {"list"}, {type}, {false},
                                  dimension.size() ? std::vector<std::vector<uint64_t>>{dimension}
                                                   : std::vector<std::vector<uint64_t>>{{}});

            dimension.clear();
        }
        else if (token.type == TokenType::INTEGER || token.type == TokenType::UINTEGER) {
            dimension.push_back(token.value.uinteger);
            if (token.value.uinteger == 0) {
                context.exceptions.emplace_back("an array could not be zero sized", token);
            }

            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_CLOSE)) {
                context.exceptions.emplace_back("expected a closing square bracket in the array size",
                                                token);
            }
        }
        else {
            context.exceptions.emplace_back(
                "unexpected `" + encodeUtf8(strfy(token)) + "` while parsing the array size", token);
        }
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

end:
    return dimension;
}
