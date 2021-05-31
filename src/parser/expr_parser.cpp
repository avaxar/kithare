/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/parser.hpp>


kh::AstExpression* kh::Parser::parseExpression() {
    kh::Token token = this->to();
    size_t index = token.index;

    return this->parseAssignOps();
end:
    return nullptr;
}

#define RECURSIVE_DESCENT_SINGULAR_OP(lower)                                                        \
    do {                                                                                            \
        kh::AstExpression* expr = lower();                                                          \
        kh::Token token;                                                                            \
        size_t index;                                                                               \
        KH_PARSE_GUARD();                                                                           \
        token = this->to();                                                                         \
        index = token.index;                                                                        \
        while (token.type == kh::TokenType::OPERATOR) {                                             \
            bool has_op = false;                                                                    \
            for (const kh::Operator op : operators) {                                               \
                if (token.value.operator_type == op) {                                              \
                    has_op = true;                                                                  \
                    break;                                                                          \
                }                                                                                   \
            }                                                                                       \
            if (!has_op)                                                                            \
                break;                                                                              \
            this->ti++;                                                                             \
            KH_PARSE_GUARD();                                                                       \
            std::shared_ptr<kh::AstExpression> lval(expr);                                          \
            std::shared_ptr<kh::AstExpression> rval(lower());                                       \
            expr = new kh::AstBinaryExpression(token.index, token.value.operator_type, lval, rval); \
            KH_PARSE_GUARD();                                                                       \
            token = this->to();                                                                     \
        }                                                                                           \
        KH_PARSE_GUARD();                                                                           \
        token = this->to();                                                                         \
    end:                                                                                            \
        return expr;                                                                                \
    } while (false)

kh::AstExpression* kh::Parser::parseAssignOps() {
    static std::vector<kh::Operator> operators = {
        kh::Operator::ASSIGN, kh::Operator::IADD, kh::Operator::ISUB, kh::Operator::IMUL,
        kh::Operator::IDIV,   kh::Operator::IMOD, kh::Operator::IPOW};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseTernary);
}

kh::AstExpression* kh::Parser::parseTernary() {
    kh::AstExpression* expr = this->parseOr();
    kh::Token token;
    size_t index;

    KH_PARSE_GUARD();
    token = this->to();
    index = token.index;

    while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"if") {
        index = token.index;

        this->ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<kh::AstExpression> condition(this->parseOr());

        KH_PARSE_GUARD();
        token = this->to();

        if (!(token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"else")) {
            this->parse_exceptions.emplace_back(
                U"Was expecting an `else` identifier to continue the ternary expression", token);
            goto end;
        }

        this->ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<kh::AstExpression> value(expr);
        std::shared_ptr<kh::AstExpression> otherwise(this->parseOr());
        expr = new kh::AstTernaryExpression(index, condition, value, otherwise);

        KH_PARSE_GUARD();
        token = this->to();
    }
end:
    return expr;
}

kh::AstExpression* kh::Parser::parseOr() {
    static std::vector<kh::Operator> operators = {kh::Operator::OR};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseAnd);
}

kh::AstExpression* kh::Parser::parseAnd() {
    static std::vector<kh::Operator> operators = {kh::Operator::AND};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseNot);
}

kh::AstExpression* kh::Parser::parseNot() {
    kh::AstExpression* expr = nullptr;
    kh::Token token = this->to();
    size_t index = token.index;

    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::NOT) {
        this->ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<kh::AstExpression> rval(this->parseNot());
        expr = new kh::AstUnaryExpression(token.index, token.value.operator_type, rval);
    }
    else
        expr = this->parseComparison();

end:
    return expr;
}

kh::AstExpression* kh::Parser::parseComparison() {
    union {
        kh::AstExpression* expr;
        kh::AstComparisonExpression* comparison_expr;
    };

    expr = this->parseBitwiseOr();
    kh::Token token;
    size_t index;

    KH_PARSE_GUARD();
    token = this->to();
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
            this->ti++;
            KH_PARSE_GUARD();
            comparison_expr->operations.push_back(token.value.operator_type);
            comparison_expr->values.emplace_back(this->parseBitwiseOr());
            KH_PARSE_GUARD();
            token = this->to();
        }
    }

end:
    return expr;
}

kh::AstExpression* kh::Parser::parseBitwiseOr() {
    static std::vector<kh::Operator> operators = {kh::Operator::BIT_OR};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseBitwiseAnd);
}

kh::AstExpression* kh::Parser::parseBitwiseAnd() {
    static std::vector<kh::Operator> operators = {kh::Operator::BIT_OR};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseBitwiseShift);
}

kh::AstExpression* kh::Parser::parseBitwiseShift() {
    static std::vector<kh::Operator> operators = {kh::Operator::BIT_LSHIFT, kh::Operator::BIT_RSHIFT};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseAddSub);
}

kh::AstExpression* kh::Parser::parseAddSub() {
    static std::vector<kh::Operator> operators = {kh::Operator::ADD, kh::Operator::SUB};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseMulDivMod);
}

kh::AstExpression* kh::Parser::parseMulDivMod() {
    static std::vector<kh::Operator> operators = {kh::Operator::MUL, kh::Operator::DIV,
                                                  kh::Operator::MOD};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseUnary);
}

kh::AstExpression* kh::Parser::parseUnary() {
    kh::AstExpression* expr = nullptr;
    kh::Token token = this->to();
    size_t index = token.index;

    if (token.type == kh::TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case kh::Operator::ADD:
            case kh::Operator::SUB:
            case kh::Operator::BIT_NOT:
            case kh::Operator::SIZEOF:
            case kh::Operator::ADDRESS: {
                this->ti++;
                KH_PARSE_GUARD();

                std::shared_ptr<kh::AstExpression> rval(this->parseUnary());
                expr = new kh::AstUnaryExpression(token.index, token.value.operator_type, rval);
            } break;

            default:
                this->ti++;
                this->parse_exceptions.emplace_back(U"Unexpected operator as a unary operator", token);
        }
    }
    else
        expr = this->parseExponentiation();

end:
    return expr;
}

kh::AstExpression* kh::Parser::parseExponentiation() {
    static std::vector<kh::Operator> operators = {kh::Operator::POW};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseLiteral);
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

        case kh::TokenType::IMAGINARY:
            expr = new kh::AstConstValue(token.index, token.value.imaginary,
                                         kh::AstConstValue::ValueType::IMAGINARY);
            this->ti++;
            break;

        case kh::TokenType::STRING:
            expr = new kh::AstConstValue(token.index, token.value.string);
            this->ti++;

            KH_PARSE_GUARD();
            token = this->to();

            /* Auto concatenation */
            while (token.type == kh::TokenType::STRING) {
                ((kh::AstConstValue*)expr)->string += token.value.string;
                this->ti++;
                KH_PARSE_GUARD();
                token = this->to();
            }

            break;

        case kh::TokenType::BUFFER:
            expr = new kh::AstConstValue(token.index, token.value.buffer);
            this->ti++;

            KH_PARSE_GUARD();
            token = this->to();

            /* Auto concatenation */
            while (token.type == kh::TokenType::BUFFER) {
                ((kh::AstConstValue*)expr)->buffer += token.value.buffer;
                this->ti++;
                KH_PARSE_GUARD();
                token = this->to();
            }

            break;

        case kh::TokenType::IDENTIFIER:
            /* Function expression */
            if (token.value.identifier == U"def") {
                this->ti++;
                KH_PARSE_GUARD();
                return this->parseFunction(false, true);
            }
            /* Variable declaration */
            else if (token.value.identifier == U"ref" || token.value.identifier == U"static" ||
                     token.value.identifier == U"private" || token.value.identifier == U"public") {
                bool is_static, is_public;
                this->parseAccessAttribs(is_static, is_public);
                KH_PARSE_GUARD();
                if (!is_public)
                    this->parse_exceptions.emplace_back(U"A local variable cannot be `private`", token);
                return this->parseDeclaration(is_static, is_public);
            }
            else {
                size_t _ti = this->ti;
                expr = this->parseIdentifiers();

                KH_PARSE_GUARD();
                token = this->to();

                if (token.type == kh::TokenType::IDENTIFIER &&
                    !kh::isReservedKeyword(token.value.identifier)) {
                    this->ti = _ti;
                    delete expr;
                    expr = this->parseDeclaration(false, true);
                    goto end;
                }
            }
            break;

        case kh::TokenType::SYMBOL:
            switch (token.value.symbol_type) {
                /* Parentheses/tuple expression */
                case kh::Symbol::PARENTHESES_OPEN:
                    expr = this->parseTuple(kh::Symbol::PARENTHESES_OPEN, kh::Symbol::PARENTHESES_CLOSE,
                                            false);
                    break;

                case kh::Symbol::SQUARE_OPEN:
                    expr = this->parseTuple(kh::Symbol::SQUARE_OPEN, kh::Symbol::SQUARE_CLOSE);
                    break;

                default:
                    this->parse_exceptions.emplace_back(U"Unexpected token for a literal", token);
                    this->ti++;
                    goto end;
            }
            break;

        default:
            this->parse_exceptions.emplace_back(U"Unexpected token for a literal", token);
            this->ti++;
            goto end;
    }

    KH_PARSE_GUARD();
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
                    KH_PARSE_GUARD();
                    token = this->to();

                    /* Expects an identifier for which to be scoped through from the expression */
                    if (token.type == kh::TokenType::IDENTIFIER)
                        identifiers.push_back(token.value.identifier);
                    else
                        this->parse_exceptions.emplace_back(U"Was expecting an identifier", token);

                    this->ti++;
                    KH_PARSE_GUARD();
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

        KH_PARSE_GUARD();
        token = this->to();
    }
end:
    return expr;
}

kh::AstExpression* kh::Parser::parseIdentifiers() {
    std::vector<std::u32string> identifiers;
    std::vector<std::shared_ptr<kh::AstIdentifierExpression>> generics;
    std::vector<size_t> generics_refs;
    std::vector<std::vector<uint64_t>> generics_array;

    bool is_function = false;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Expects an identifier */
    if (token.type == kh::TokenType::IDENTIFIER) {
        if (kh::isReservedKeyword(token.value.identifier))
            this->parse_exceptions.emplace_back(U"Could not use a reserved keyword as an identifier",
                                                token);

        identifiers.push_back(token.value.identifier);
        this->ti++;
    }
    else {
        this->parse_exceptions.emplace_back(U"Was expecting an identifier", token);
        goto end;
    }

    KH_PARSE_GUARD();
    token = this->to();

    /* For each scope in with a dot symbol */
    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();

        /* Appends the identifier */
        if (token.type == kh::TokenType::IDENTIFIER) {
            if (kh::isReservedKeyword(token.value.identifier))
                this->parse_exceptions.emplace_back(
                    U"Could not use a reserved keyword as an identifier", token);

            identifiers.push_back(token.value.identifier);
        }
        else {
            this->parse_exceptions.emplace_back(U"Was expecting an identifier after the dot", token);
            goto end;
        }

        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();
    }

    if (identifiers.size() == 1 && identifiers[0] == U"func")
        is_function = true;

    /* Optional genericization */
    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::NOT) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();

        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
            this->ti++;
            KH_PARSE_GUARD();
            token = this->to();

            generics_refs.push_back(0);
            while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
                generics_refs.back() += 1;
                this->ti++;
                KH_PARSE_GUARD();
                token = this->to();
            }

            /* Parses the genericization type arguments */
            generics.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());
            generics_array.push_back(this->parseArrayDimension(generics.back()));
            KH_PARSE_GUARD();
            token = this->to();

            if (is_function) {
                if (token.type == kh::TokenType::SYMBOL &&
                    token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->to();

                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE) {
                        this->ti++;
                        goto funcFinish;
                    }
                    else
                        goto forceIn;
                }
                else {
                    this->parse_exceptions.emplace_back(
                        U"Was expecting an opening parentheses after the "
                        U"return type in the genericization of `func` type",
                        token);
                }
            }

            while (token.type == kh::TokenType::SYMBOL &&
                   token.value.symbol_type == kh::Symbol::COMMA) {
                this->ti++;
                KH_PARSE_GUARD();
                token = this->to();

            forceIn:
                generics_refs.push_back(0);
                while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
                    generics_refs.back() += 1;
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->to();
                }

                generics.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());
                generics_array.push_back(this->parseArrayDimension(generics.back()));

                KH_PARSE_GUARD();
                token = this->to();
            }

            /* Expects closing parentheses */
            if (token.type == kh::TokenType::SYMBOL &&
                token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE) {
                this->ti++;

                if (is_function) {
                funcFinish:
                    KH_PARSE_GUARD();
                    token = this->to();

                    if (token.type == kh::TokenType::SYMBOL &&
                        token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
                        this->ti++;
                    else
                        this->parse_exceptions.emplace_back(U"Was expecting a closing parentheses",
                                                            token);
                }
            }
            else
                this->parse_exceptions.emplace_back(U"Was expecting a closing parentheses", token);
        }
        else if (token.type == kh::TokenType::IDENTIFIER) {
            if (is_function)
                this->parse_exceptions.emplace_back(
                    U"Was expecting an opening parentheses for genericization of `func` type", token);

            generics.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());
            generics_refs.push_back(0);
            generics_array.push_back({0});
        }
        else {
            this->parse_exceptions.emplace_back(U"Was expecting either an identifier or an opening "
                                                U"parentheses for genericization after the "
                                                U"exclamation mark ",
                                                token);
            this->ti++;
        }
    }
    else if (is_function)
        this->parse_exceptions.emplace_back(U"`func` type requires genericization", token);
end:
    return new kh::AstIdentifierExpression(index, identifiers, generics, generics_refs, generics_array);
}

kh::AstExpression* kh::Parser::parseTuple(const kh::Symbol opening, const kh::Symbol closing,
                                          const bool can_contain_one_element) {
    std::vector<kh::AstExpression*> elements;

    kh::Token token = this->to();
    size_t index = token.index;

    /* Expects the opening symbol */
    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == opening) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();

        bool skip_parentheses = true;
        while (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing)) {
            /* Parses the element expression */
            elements.push_back(this->parseExpression());
            KH_PARSE_GUARD();
            token = this->to();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing) {
                this->ti++;
                skip_parentheses = false;
                break;
            }
            else if (!(token.type == kh::TokenType::SYMBOL &&
                       token.value.symbol_type == kh::Symbol::COMMA)) {
                this->parse_exceptions.emplace_back(
                    closing == kh::Symbol::SQUARE_CLOSE
                        ? U"Was expecting a comma or a closing square bracket"
                        : U"Was expecting a comma or a closing parentheses",
                    token);
                break;
            }

            this->ti++;
            KH_PARSE_GUARD();
            token = this->to();
        }

        if (skip_parentheses)
            this->ti++;
    }
    else
        this->parse_exceptions.emplace_back(opening == kh::Symbol::SQUARE_OPEN
                                                ? U"Was expecting an opening square bracket"
                                                : U"Was expecting an opening parentheses",
                                            token);

end:
    if ((!can_contain_one_element) && elements.size() == 1) {
        for (size_t i = 1; i < elements.size(); i++)
            delete elements[i];

        return elements[0];
    }
    else {
        std::vector<std::shared_ptr<kh::AstExpression>> _elements;
        _elements.reserve(elements.size());

        for (kh::AstExpression* element : elements)
            _elements.emplace_back(element);

        return new kh::AstTupleExpression(index, _elements);
    }
}

std::vector<uint64_t>
kh::Parser::parseArrayDimension(std::shared_ptr<kh::AstIdentifierExpression>& type) {
    std::vector<uint64_t> dimension;

    kh::Token token = this->to();
    size_t index = token.index;

    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SQUARE_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();

        if (token.type == kh::TokenType::SYMBOL &&
            token.value.symbol_type == kh::Symbol::SQUARE_CLOSE) {
            type.reset(new kh::AstIdentifierExpression(
                token.index, {U"list"}, {type}, {false},
                dimension.size() ? std::vector<std::vector<uint64_t>>{dimension}
                                 : std::vector<std::vector<uint64_t>>{{0}}));
            dimension.clear();
        }
        else if (token.type == kh::TokenType::INTEGER || token.type == kh::TokenType::UINTEGER) {
            dimension.push_back(token.value.uinteger);

            if (token.type == kh::TokenType::INTEGER && token.value.integer < 0)
                this->parse_exceptions.emplace_back(U"An array could not have a negative size", token);
            else if (token.value.uinteger == 0)
                this->parse_exceptions.emplace_back(U"An array could not be zero sized", token);

            this->ti++;
            KH_PARSE_GUARD();
            token = this->to();

            if (!(token.type == kh::TokenType::SYMBOL &&
                  token.value.symbol_type == kh::Symbol::SQUARE_CLOSE))
                this->parse_exceptions.emplace_back(
                    U"Was expecting a closing square bracket in the array size", token);
        }
        else
            this->parse_exceptions.emplace_back(U"Unexpected token while parsing array size", token);

        this->ti++;
        KH_PARSE_GUARD();
        token = this->to();
    }

end:
    if (dimension.size())
        return dimension;
    else
        return {0};
}
