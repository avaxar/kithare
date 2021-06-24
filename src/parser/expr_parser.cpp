/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/parser.hpp>

#define RECURSIVE_DESCENT_SINGULAR_OP(lower)                                                       \
    do {                                                                                           \
        kh::AstExpression* expr = lower(context);                                                  \
        kh::Token token;                                                                           \
        size_t index;                                                                              \
        KH_PARSE_GUARD();                                                                          \
        token = context.tok();                                                                     \
        index = token.index;                                                                       \
        while (token.type == kh::TokenType::OPERATOR) {                                            \
            bool has_op = false;                                                                   \
            for (const kh::Operator op : operators) {                                              \
                if (token.value.operator_type == op) {                                             \
                    has_op = true;                                                                 \
                    break;                                                                         \
                }                                                                                  \
            }                                                                                      \
            if (!has_op)                                                                           \
                break;                                                                             \
            context.ti++;                                                                          \
            KH_PARSE_GUARD();                                                                      \
            std::shared_ptr<kh::AstExpression> lval(expr);                                         \
            std::shared_ptr<kh::AstExpression> rval(lower(context));                               \
            expr = new kh::AstBinaryOperation(token.index, token.value.operator_type, lval, rval); \
            KH_PARSE_GUARD();                                                                      \
            token = context.tok();                                                                 \
        }                                                                                          \
        KH_PARSE_GUARD();                                                                          \
        token = context.tok();                                                                     \
    end:                                                                                           \
        return expr;                                                                               \
    } while (false)


kh::AstExpression* kh::parseExpression(const std::vector<kh::Token>& tokens) {
    std::vector<kh::ParseException> exceptions;
    kh::ParserContext context{tokens, exceptions};
    kh::AstExpression* ast = kh::parseExpression(context);

    if (exceptions.empty())
        return ast;
    else
        throw exceptions;
}

kh::AstExpression* kh::parseExpression(KH_PARSE_CTX) {
    kh::Token token = context.tok();
    size_t index = token.index;

    return kh::parseAssignOps(context);
end:
    return nullptr;
}

kh::AstExpression* kh::parseAssignOps(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {
        kh::Operator::ASSIGN, kh::Operator::IADD, kh::Operator::ISUB, kh::Operator::IMUL,
        kh::Operator::IDIV,   kh::Operator::IMOD, kh::Operator::IPOW};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseTernary);
}

kh::AstExpression* kh::parseTernary(KH_PARSE_CTX) {
    kh::AstExpression* expr = kh::parseOr(context);
    kh::Token token;
    size_t index;

    KH_PARSE_GUARD();
    token = context.tok();
    index = token.index;

    while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"if") {
        index = token.index;

        context.ti++;
        KH_PARSE_GUARD();
        std::shared_ptr<kh::AstExpression> condition(kh::parseOr(context));

        KH_PARSE_GUARD();
        token = context.tok();

        if (!(token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"else")) {
            context.exceptions.emplace_back(
                U"expected an `else` to specify the else case of the ternary expression", token);
            goto end;
        }

        context.ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<kh::AstExpression> value(expr);
        std::shared_ptr<kh::AstExpression> otherwise(kh::parseOr(context));
        expr = new kh::AstComparisonOperation(index, condition, value, otherwise);

        KH_PARSE_GUARD();
        token = context.tok();
    }
end:
    return expr;
}

kh::AstExpression* kh::parseOr(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {kh::Operator::OR};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseAnd);
}

kh::AstExpression* kh::parseAnd(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {kh::Operator::AND};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseNot);
}

kh::AstExpression* kh::parseNot(KH_PARSE_CTX) {
    kh::AstExpression* expr = nullptr;
    kh::Token token = context.tok();
    size_t index = token.index;

    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::NOT) {
        context.ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<kh::AstExpression> rval(kh::parseNot(context));
        expr = new kh::AstUnaryOperation(token.index, token.value.operator_type, rval);
    }
    else
        expr = kh::parseComparison(context);

end:
    return expr;
}

kh::AstExpression* kh::parseComparison(KH_PARSE_CTX) {
    union {
        kh::AstExpression* expr;
        kh::AstComparisonExpression* comparison_expr;
    };

    expr = kh::parseBitwiseOr(context);
    kh::Token token;
    size_t index;

    KH_PARSE_GUARD();
    token = context.tok();
    index = token.index;

    if (token.type == kh::TokenType::OPERATOR &&
        (token.value.operator_type == kh::Operator::EQUAL ||
         token.value.operator_type == kh::Operator::NOT_EQUAL ||
         token.value.operator_type == kh::Operator::LESS ||
         token.value.operator_type == kh::Operator::LESS_EQUAL ||
         token.value.operator_type == kh::Operator::MORE ||
         token.value.operator_type == kh::Operator::MORE_EQUAL)) {
        comparison_expr =
            new kh::AstComparisonExpression(index, {}, {std::shared_ptr<kh::AstExpression>(expr)});

        while (token.type == kh::TokenType::OPERATOR &&
               (token.value.operator_type == kh::Operator::EQUAL ||
                token.value.operator_type == kh::Operator::NOT_EQUAL ||
                token.value.operator_type == kh::Operator::LESS ||
                token.value.operator_type == kh::Operator::LESS_EQUAL ||
                token.value.operator_type == kh::Operator::MORE ||
                token.value.operator_type == kh::Operator::MORE_EQUAL)) {
            context.ti++;
            KH_PARSE_GUARD();
            comparison_expr->operations.push_back(token.value.operator_type);
            comparison_expr->values.emplace_back(kh::parseBitwiseOr(context));
            KH_PARSE_GUARD();
            token = context.tok();
        }
    }

end:
    return expr;
}

kh::AstExpression* kh::parseBitwiseOr(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {kh::Operator::BIT_OR};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseBitwiseAnd);
}

kh::AstExpression* kh::parseBitwiseAnd(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {kh::Operator::BIT_OR};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseBitwiseShift);
}

kh::AstExpression* kh::parseBitwiseShift(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {kh::Operator::BIT_LSHIFT, kh::Operator::BIT_RSHIFT};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseAddSub);
}

kh::AstExpression* kh::parseAddSub(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {kh::Operator::ADD, kh::Operator::SUB};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseMulDivMod);
}

kh::AstExpression* kh::parseMulDivMod(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {kh::Operator::MUL, kh::Operator::DIV,
                                                  kh::Operator::MOD};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseUnary);
}

kh::AstExpression* kh::parseUnary(KH_PARSE_CTX) {
    kh::AstExpression* expr = nullptr;
    kh::Token token = context.tok();
    size_t index = token.index;

    if (token.type == kh::TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case kh::Operator::ADD:
            case kh::Operator::SUB:
            case kh::Operator::INCREMENT:
            case kh::Operator::DECREMENT:
            case kh::Operator::BIT_NOT:
            case kh::Operator::SIZEOF:
            case kh::Operator::ADDRESS: {
                context.ti++;
                KH_PARSE_GUARD();

                std::shared_ptr<kh::AstExpression> rval(kh::parseUnary(context));
                expr = new kh::AstUnaryOperation(token.index, token.value.operator_type, rval);
            } break;

            default:
                context.ti++;
                context.exceptions.emplace_back(
                    U"unexpected `" + kh::str(token) + U"` in an expression", token);
        }
    }
    else
        expr = kh::parseExponentiation(context);

end:
    return expr;
}

kh::AstExpression* kh::parseExponentiation(KH_PARSE_CTX) {
    static std::vector<kh::Operator> operators = {kh::Operator::POW};
    RECURSIVE_DESCENT_SINGULAR_OP(kh::parseRevUnary);
}

kh::AstExpression* kh::parseRevUnary(KH_PARSE_CTX) {
    kh::Token token = context.tok();
    size_t index = token.index;
    kh::AstExpression* expr = kh::parseOthers(context);

    KH_PARSE_GUARD();
    token = context.tok();

    while ((token.type == kh::TokenType::OPERATOR &&
                token.value.operator_type == kh::Operator::INCREMENT ||
            token.value.operator_type == kh::Operator::DECREMENT) ||
           (token.type == kh::TokenType::SYMBOL &&
            (token.value.symbol_type == kh::Symbol::DOT ||
             token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN ||
             token.value.symbol_type == kh::Symbol::SQUARE_OPEN))) {
        index = token.index;

        /* Post-incrementation and decrementation */
        if (token.type == kh::TokenType::OPERATOR) {
            std::shared_ptr<kh::AstExpression> expr_ptr(expr);
            expr = new kh::AstRevUnaryOperation(index, token.value.operator_type, expr_ptr);
            context.ti++;
        }
        else {
            switch (token.value.symbol_type) {
                /* Scoping expression */
                case kh::Symbol::DOT: {
                    std::vector<std::u32string> identifiers;

                    do {
                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Expects an identifier for which to be scoped through from the expression */
                        if (token.type == kh::TokenType::IDENTIFIER)
                            identifiers.push_back(token.value.identifier);
                        else
                            context.exceptions.emplace_back(U"expected an identifier", token);

                        context.ti++;
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Continues again for another scope in */
                    } while (token.type == kh::TokenType::SYMBOL &&
                             token.value.symbol_type == kh::Symbol::DOT);

                    std::shared_ptr<kh::AstExpression> exprptr(expr);
                    expr = new kh::AstScoping(index, exprptr, identifiers);
                } break;

                /* Calling expression */
                case kh::Symbol::PARENTHESES_OPEN: {
                    std::shared_ptr<kh::AstExpression> exprptr(expr);
                    /* Parses the argument(s) */
                    kh::AstTuple* tuple = static_cast<kh::AstTuple*>(kh::parseTuple(context));
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
                    kh::AstTuple* tuple = static_cast<kh::AstTuple*>(
                        kh::parseTuple(context, kh::Symbol::SQUARE_OPEN, kh::Symbol::SQUARE_CLOSE));
                    std::vector<std::shared_ptr<kh::AstExpression>> arguments;

                    for (std::shared_ptr<kh::AstExpression>& element : tuple->elements)
                        arguments.push_back(element);

                    expr = new kh::AstSubscriptExpression(index, exprptr, arguments);
                    delete tuple;
                } break;
            }
        }

        KH_PARSE_GUARD();
        token = context.tok();
    }
end:
    return expr;
}

kh::AstExpression* kh::parseOthers(KH_PARSE_CTX) {
    kh::AstExpression* expr = nullptr;
    kh::Token token = context.tok();
    size_t index = token.index;

    switch (token.type) {
            /* For all of these literal values be given the AST constant value instance */

        case kh::TokenType::CHARACTER:
            expr = new kh::AstValue(token.index, token.value.character);
            context.ti++;
            break;

        case kh::TokenType::UINTEGER:
            expr = new kh::AstValue(token.index, token.value.uinteger);
            context.ti++;
            break;

        case kh::TokenType::INTEGER:
            expr = new kh::AstValue(token.index, token.value.integer);
            context.ti++;
            break;

        case kh::TokenType::FLOATING:
            expr = new kh::AstValue(token.index, token.value.floating);
            context.ti++;
            break;

        case kh::TokenType::IMAGINARY:
            expr = new kh::AstValue(token.index, token.value.imaginary,
                                    kh::AstValue::ValueType::IMAGINARY);
            context.ti++;
            break;

        case kh::TokenType::STRING:
            expr = new kh::AstValue(token.index, token.value.string);
            context.ti++;

            KH_PARSE_GUARD();
            token = context.tok();

            /* Auto concatenation */
            while (token.type == kh::TokenType::STRING) {
                ((kh::AstValue*)expr)->string += token.value.string;
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }

            break;

        case kh::TokenType::BUFFER:
            expr = new kh::AstValue(token.index, token.value.buffer);
            context.ti++;

            KH_PARSE_GUARD();
            token = context.tok();

            /* Auto concatenation */
            while (token.type == kh::TokenType::BUFFER) {
                ((kh::AstValue*)expr)->buffer += token.value.buffer;
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }

            break;

        case kh::TokenType::IDENTIFIER:
            /* Lambda expression */
            if (token.value.identifier == U"def") {
                context.ti++;
                KH_PARSE_GUARD();
                return new kh::AstFunction(kh::parseFunction(context, false, true, false));
            }
            /* Variable declaration */
            else if (token.value.identifier == U"ref" || token.value.identifier == U"static" ||
                     token.value.identifier == U"private" || token.value.identifier == U"public") {
                bool is_static, is_public;
                kh::parseAccessAttribs(context, is_static, is_public);
                KH_PARSE_GUARD();
                if (!is_public)
                    context.exceptions.emplace_back(U"a local variable cannot be private", token);
                return new kh::AstDeclaration(kh::parseDeclaration(context, is_static, is_public));
            }
            else {
                size_t _ti = context.ti;
                expr = new kh::AstIdentifiers(kh::parseIdentifiers(context));

                KH_PARSE_GUARD();
                token = context.tok();

                /* An identifier is next to another identifier `int number` */
                if (token.type == kh::TokenType::IDENTIFIER &&
                    !kh::isReservedKeyword(token.value.identifier)) {
                    context.ti = _ti;
                    delete expr;
                    expr = new kh::AstDeclaration(kh::parseDeclaration(context, false, true));
                }
                /* An opening square parentheses next to an idenifier, possible array variable
                 * declaration */
                else if (token.type == kh::TokenType::SYMBOL &&
                         token.value.symbol_type == kh::Symbol::SQUARE_OPEN) {
                    size_t exception_counts = context.exceptions.size();

                    kh::parseArrayDimension(context, *static_cast<kh::AstIdentifiers*>(expr));
                    delete expr;

                    /* If there was exceptions while parsing the array dimension type, it probably
                     * wasn't an array variable declaration.. rather a subscript or something */
                    if (context.exceptions.size() > exception_counts) {
                        for (size_t i = 0; i < context.exceptions.size() - exception_counts; i++)
                            context.exceptions.pop_back();

                        context.ti = _ti;
                        expr = new kh::AstIdentifiers(kh::parseIdentifiers(context));
                    }
                    else {
                        KH_PARSE_GUARD();
                        token = context.tok();

                        /* Confirmed that it's an array declaration `float[3] position;` */
                        if (token.type == kh::TokenType::IDENTIFIER &&
                            !kh::isReservedKeyword(token.value.identifier)) {
                            context.ti = _ti;
                            expr = new kh::AstDeclaration(kh::parseDeclaration(context, false, true));
                        }
                        /* Probably was just a normal subscript */
                        else {
                            context.ti = _ti;
                            expr = new kh::AstIdentifiers(kh::parseIdentifiers(context));
                        }
                    }
                }
            }
            break;

        case kh::TokenType::SYMBOL:
            switch (token.value.symbol_type) {
                /* Parentheses/tuple expression */
                case kh::Symbol::PARENTHESES_OPEN:
                    expr = kh::parseTuple(context, kh::Symbol::PARENTHESES_OPEN,
                                          kh::Symbol::PARENTHESES_CLOSE, false);
                    break;

                /* List literal expression */
                case kh::Symbol::SQUARE_OPEN:
                    expr = kh::parseList(context);
                    break;

                /* Dict literal expression */
                case kh::Symbol::CURLY_OPEN:
                    expr = kh::parseDict(context);
                    break;

                default:
                    context.exceptions.emplace_back(
                        U"unexpected `" + kh::str(token) + U"` in an expression", token);
                    context.ti++;
                    goto end;
            }
            break;

        default:
            context.exceptions.emplace_back(U"unexpected `" + kh::str(token) + U"` in an expression",
                                            token);
            context.ti++;
            goto end;
    }
end:
    return expr;
}

kh::AstIdentifiers kh::parseIdentifiers(KH_PARSE_CTX) {
    std::vector<std::u32string> identifiers;
    std::vector<kh::AstIdentifiers> generics;
    std::vector<size_t> generics_refs;
    std::vector<std::vector<uint64_t>> generics_array;

    bool is_function = false;

    kh::Token token = context.tok();
    size_t index = token.index;

    /* Expects an identifier */
    if (token.type == kh::TokenType::IDENTIFIER) {
        if (kh::isReservedKeyword(token.value.identifier))
            context.exceptions.emplace_back(U"cannot use a reserved keyword as an identifier", token);

        identifiers.push_back(token.value.identifier);
        context.ti++;
    }
    else {
        context.exceptions.emplace_back(U"expected an identifier", token);
        goto end;
    }

    KH_PARSE_GUARD();
    token = context.tok();

    /* For each scope in with a dot symbol */
    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Appends the identifier */
        if (token.type == kh::TokenType::IDENTIFIER) {
            if (kh::isReservedKeyword(token.value.identifier))
                context.exceptions.emplace_back(U"cannot use a reserved keyword as an identifier",
                                                token);

            identifiers.push_back(token.value.identifier);
        }
        else {
            context.exceptions.emplace_back(U"expected an identifier after the dot", token);
            goto end;
        }

        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

    is_function = identifiers.size() == 1 && identifiers[0] == U"func";

    /* Optional genericization */
    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::NOT) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            generics_refs.push_back(0);
            while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
                generics_refs.back() += 1;
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();
            }

            /* Parses the genericization type arguments */
            generics.push_back(kh::parseIdentifiers(context));
            generics_array.push_back(kh::parseArrayDimension(context, generics.back()));
            KH_PARSE_GUARD();
            token = context.tok();

            if (is_function) {
                if (token.type == kh::TokenType::SYMBOL &&
                    token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();

                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE) {
                        context.ti++;
                        goto funcFinish;
                    }
                    else
                        goto forceIn;
                }
                else {
                    context.exceptions.emplace_back(U"expected an opening parentheses after the "
                                                    U"return type in the genericization of `func`",
                                                    token);
                }
            }

            while (token.type == kh::TokenType::SYMBOL &&
                   token.value.symbol_type == kh::Symbol::COMMA) {
                context.ti++;
                KH_PARSE_GUARD();
                token = context.tok();

            forceIn:
                generics_refs.push_back(0);
                while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
                    generics_refs.back() += 1;
                    context.ti++;
                    KH_PARSE_GUARD();
                    token = context.tok();
                }

                generics.emplace_back(kh::parseIdentifiers(context));
                generics_array.push_back(kh::parseArrayDimension(context, generics.back()));

                KH_PARSE_GUARD();
                token = context.tok();
            }

            /* Expects closing parentheses */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE) {
                context.ti++;

                if (is_function) {
                funcFinish:
                    KH_PARSE_GUARD();
                    token = context.tok();

                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
                        context.ti++;
                    else
                        context.exceptions.emplace_back(U"expected a closing parentheses", token);
                }
            }
            else
                context.exceptions.emplace_back(U"expected a closing parentheses", token);
        }
        else if (token.type == kh::TokenType::IDENTIFIER) {
            if (is_function)
                context.exceptions.emplace_back(
                    U"expected an opening parentheses for genericization of `func`", token);

            generics.push_back(kh::parseIdentifiers(context));
            generics_refs.push_back(0);
            generics_array.push_back({});
        }
        else {
            context.exceptions.emplace_back(U"expected either an identifier or an opening "
                                            U"parentheses for genericization after the "
                                            U"exclamation mark ",
                                            token);
            context.ti++;
        }
    }
    else if (is_function)
        context.exceptions.emplace_back(U"`func` requires genericization", token);
end:
    return {index, identifiers, generics, generics_refs, generics_array};
}

kh::AstExpression* kh::parseTuple(KH_PARSE_CTX, kh::Symbol opening, kh::Symbol closing,
                                  bool explicit_tuple) {
    std::vector<kh::AstExpression*> elements;

    kh::Token token = context.tok();
    size_t index = token.index;

    /* Expects the opening symbol */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == opening) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Instant close */
        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing) {
            context.ti++;
            goto end;
        }

        while (true) {
            /* Parses the element expression */
            elements.push_back(kh::parseExpression(context));
            KH_PARSE_GUARD();
            token = context.tok();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing) {
                context.ti++;
                break;
            }
            else if (!(token.type == kh::TokenType::SYMBOL &&
                       token.value.symbol_type == kh::Symbol::COMMA)) {
                context.exceptions.emplace_back(closing == kh::Symbol::SQUARE_CLOSE
                                                    ? U"expected a comma or a closing square bracket"
                                                    : U"expected a comma or a closing parentheses",
                                                token);
                context.ti++;
                break;
            }

            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            /* Cases for explicit one-elemented tuples `(69420,)` */
            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing) {
                context.ti++;
                explicit_tuple = true;
                break;
            }
        }
    }
    else {
        context.exceptions.emplace_back(opening == kh::Symbol::SQUARE_OPEN
                                            ? U"expected an opening square bracket"
                                            : U"expected an opening parentheses",
                                        token);
        context.ti++;
    }

end:
    if (!explicit_tuple && elements.size() == 1) {
        for (size_t i = 1; i < elements.size(); i++)
            delete elements[i];

        return elements[0];
    }
    else {
        std::vector<std::shared_ptr<kh::AstExpression>> _elements;
        _elements.reserve(elements.size());

        for (kh::AstExpression* element : elements)
            _elements.emplace_back(element);

        return new kh::AstTuple(index, _elements);
    }
}

kh::AstExpression* kh::parseList(KH_PARSE_CTX) {
    kh::Token token = context.tok();
    size_t index = token.index;

    kh::AstTuple* tuple = (kh::AstTuple*)kh::parseTuple(context, kh::Symbol::SQUARE_OPEN,
                                                        kh::Symbol::SQUARE_CLOSE, false);
    kh::AstList* list = new kh::AstList(tuple->index, tuple->elements);
    delete tuple;

    return list;
}

kh::AstExpression* kh::parseDict(KH_PARSE_CTX) {
    std::vector<std::shared_ptr<kh::AstExpression>> keys;
    std::vector<std::shared_ptr<kh::AstExpression>> items;

    kh::Token token = context.tok();
    size_t index = token.index;

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        /* Instant close*/
        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
            context.ti++;
            goto end;
        }
        else
            goto forceIn;

        do {
            context.ti++;
            KH_PARSE_GUARD();
        forceIn:
            keys.emplace_back(kh::parseExpression(context));

            KH_PARSE_GUARD();
            token = context.tok();
            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COLON) {
                context.ti++;
                KH_PARSE_GUARD();
            }
            else
                context.exceptions.emplace_back(U"expected a colon after a key of the dict literal",
                                                token);

            items.emplace_back(kh::parseExpression(context));
            KH_PARSE_GUARD();
            token = context.tok();
        } while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COMMA);

        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_CLOSE)
            context.ti++;
        else
            context.exceptions.emplace_back(
                U"expected a closing curly bracket closing the dict literal", token);
    }
    else
        context.exceptions.emplace_back(U"expected an opening curly bracket for the dict literal",
                                        token);

end:
    return new kh::AstDict(index, keys, items);
}

std::vector<uint64_t> kh::parseArrayDimension(KH_PARSE_CTX, kh::AstIdentifiers& type) {
    std::vector<uint64_t> dimension;

    kh::Token token = context.tok();
    size_t index = token.index;

    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SQUARE_OPEN) {
        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();

        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::SQUARE_CLOSE) {
            type = kh::AstIdentifiers(token.index, {U"list"}, {type}, {false},
                                      dimension.size() ? std::vector<std::vector<uint64_t>>{dimension}
                                                       : std::vector<std::vector<uint64_t>>{{}});

            dimension.clear();
        }
        else if (token.type == kh::TokenType::INTEGER || token.type == kh::TokenType::UINTEGER) {
            dimension.push_back(token.value.uinteger);
            if (token.value.uinteger == 0)
                context.exceptions.emplace_back(U"an array could not be zero sized", token);

            context.ti++;
            KH_PARSE_GUARD();
            token = context.tok();

            if (!(token.type == kh::TokenType::SYMBOL &&
                  token.value.symbol_type == kh::Symbol::SQUARE_CLOSE))
                context.exceptions.emplace_back(U"expected a closing square bracket in the array size",
                                                token);
        }
        else
            context.exceptions.emplace_back(
                U"unexpected `" + kh::str(token) + U"` while parsing the array size", token);

        context.ti++;
        KH_PARSE_GUARD();
        token = context.tok();
    }

end:
    return dimension;
}
