/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/parser.hpp>


using namespace kh;

#define RECURSIVE_DESCENT_SINGULAR_OP(lower)                                                   \
    do {                                                                                       \
        AstExpression* expr = lower();                                                         \
        Token token;                                                                           \
        KH_PARSE_GUARD();                                                                      \
        token = this->tok();                                                                   \
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
            this->ti++;                                                                        \
            KH_PARSE_GUARD();                                                                  \
            std::shared_ptr<AstExpression> lval(expr);                                         \
            std::shared_ptr<AstExpression> rval(lower());                                      \
            expr = new AstBinaryOperation(token.begin, token.value.operator_type, lval, rval); \
            KH_PARSE_GUARD();                                                                  \
            token = this->tok();                                                               \
        }                                                                                      \
        KH_PARSE_GUARD();                                                                      \
        token = this->tok();                                                                   \
    end:                                                                                       \
        return expr;                                                                           \
    } while (false)

AstExpression* kh::parseExpression(const std::vector<Token>& tokens) {
    std::vector<ParseException> exceptions;
    Parser parser{tokens, exceptions};
    AstExpression* ast = parser.parseExpression();

    if (exceptions.empty()) {
        return ast;
    }
    else {
        throw exceptions;
    }
}

AstExpression* kh::Parser::parseExpression() {
    return this->parseAssignOps();
}

AstExpression* kh::Parser::parseAssignOps() {
    static std::vector<Operator> operators = {Operator::ASSIGN, Operator::IADD, Operator::ISUB,
                                              Operator::IMUL,   Operator::IDIV, Operator::IMOD,
                                              Operator::IPOW};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseTernary);
}

AstExpression* kh::Parser::parseTernary() {
    AstExpression* expr = this->parseOr();
    Token token;
    size_t index;

    KH_PARSE_GUARD();
    token = this->tok();
    index = token.begin;

    while (token.type == TokenType::IDENTIFIER && token.value.identifier == "if") {
        index = token.begin;

        this->ti++;
        KH_PARSE_GUARD();
        std::shared_ptr<AstExpression> condition(this->parseOr());

        KH_PARSE_GUARD();
        token = this->tok();

        if (!(token.type == TokenType::IDENTIFIER && token.value.identifier == "else")) {
            this->exceptions.emplace_back(
                "expected an `else` to specify the else case of the ternary expression", token);
            goto end;
        }

        this->ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<AstExpression> value(expr);
        std::shared_ptr<AstExpression> otherwise(this->parseOr());
        expr = new AstTernaryOperation(index, condition, value, otherwise);

        KH_PARSE_GUARD();
        token = this->tok();
    }
end:
    return expr;
}

AstExpression* kh::Parser::parseOr() {
    static std::vector<Operator> operators = {Operator::OR};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseAnd);
}

AstExpression* kh::Parser::parseAnd() {
    static std::vector<Operator> operators = {Operator::AND};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseNot);
}

AstExpression* kh::Parser::parseNot() {
    AstExpression* expr = nullptr;
    Token token = this->tok();

    if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::NOT) {
        this->ti++;
        KH_PARSE_GUARD();

        std::shared_ptr<AstExpression> rval(this->parseNot());
        expr = new AstUnaryOperation(token.begin, token.value.operator_type, rval);
    }
    else {
        expr = this->parseComparison();
    }
end:
    return expr;
}

AstExpression* kh::Parser::parseComparison() {
    union {
        AstExpression* expr;
        AstComparisonExpression* comparison_expr;
    };

    expr = this->parseBitwiseOr();
    Token token;
    size_t index;

    KH_PARSE_GUARD();
    token = this->tok();
    index = token.begin;

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
            this->ti++;
            KH_PARSE_GUARD();
            comparison_expr->operations.push_back(token.value.operator_type);
            comparison_expr->values.emplace_back(this->parseBitwiseOr());
            KH_PARSE_GUARD();
            token = this->tok();
        }
    }

end:
    return expr;
}

AstExpression* kh::Parser::parseBitwiseOr() {
    static std::vector<Operator> operators = {Operator::BIT_OR};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseBitwiseAnd);
}

AstExpression* kh::Parser::parseBitwiseAnd() {
    static std::vector<Operator> operators = {Operator::BIT_OR};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseBitwiseShift);
}

AstExpression* kh::Parser::parseBitwiseShift() {
    static std::vector<Operator> operators = {Operator::BIT_LSHIFT, Operator::BIT_RSHIFT};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseAddSub);
}

AstExpression* kh::Parser::parseAddSub() {
    static std::vector<Operator> operators = {Operator::ADD, Operator::SUB};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseMulDivMod);
}

AstExpression* kh::Parser::parseMulDivMod() {
    static std::vector<Operator> operators = {Operator::MUL, Operator::DIV, Operator::MOD};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseUnary);
}

AstExpression* kh::Parser::parseUnary() {
    AstExpression* expr = nullptr;
    Token token = this->tok();

    if (token.type == TokenType::OPERATOR) {
        switch (token.value.operator_type) {
            case Operator::ADD:
            case Operator::SUB:
            case Operator::INCREMENT:
            case Operator::DECREMENT:
            case Operator::BIT_NOT:
            case Operator::SIZEOF:
            case Operator::ADDRESS: {
                this->ti++;
                KH_PARSE_GUARD();

                std::shared_ptr<AstExpression> rval(this->parseUnary());
                expr = new AstUnaryOperation(token.begin, token.value.operator_type, rval);
            } break;

            default:
                this->ti++;
                this->exceptions.emplace_back("unexpected `" + strfy(token) + "` in an expression",
                                              token);
        }
    }
    else {
        expr = this->parseExponentiation();
    }
end:
    return expr;
}

AstExpression* kh::Parser::parseExponentiation() {
    static std::vector<Operator> operators = {Operator::POW};
    RECURSIVE_DESCENT_SINGULAR_OP(this->parseRevUnary);
}

AstExpression* kh::Parser::parseRevUnary() {
    AstExpression* expr = this->parseOthers();
    Token token = this->tok();
    size_t index = token.begin;

    KH_PARSE_GUARD();
    token = this->tok();

    while ((token.type == TokenType::OPERATOR && (token.value.operator_type == Operator::INCREMENT ||
                                                  token.value.operator_type == Operator::DECREMENT)) ||
           (token.type == TokenType::SYMBOL && (token.value.symbol_type == Symbol::DOT ||
                                                token.value.symbol_type == Symbol::PARENTHESES_OPEN ||
                                                token.value.symbol_type == Symbol::SQUARE_OPEN))) {
        index = token.begin;

        /* Post-incrementation and decrementation */
        if (token.type == TokenType::OPERATOR) {
            std::shared_ptr<AstExpression> expr_ptr(expr);
            expr = new AstRevUnaryOperation(index, token.value.operator_type, expr_ptr);
            this->ti++;
        }
        else {
            switch (token.value.symbol_type) {
                /* Scoping expression */
                case Symbol::DOT: {
                    std::vector<std::string> identifiers;

                    do {
                        this->ti++;
                        KH_PARSE_GUARD();
                        token = this->tok();

                        /* Expects an identifier for which to be scoped through from the expression */
                        if (token.type == TokenType::IDENTIFIER) {
                            identifiers.push_back(token.value.identifier);
                        }
                        else {
                            this->exceptions.emplace_back("expected an identifier", token);
                        }
                        this->ti++;
                        KH_PARSE_GUARD();
                        token = this->tok();

                        /* Continues again for another scope in */
                    } while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT);

                    std::shared_ptr<AstExpression> exprptr(expr);
                    expr = new AstScoping(index, exprptr, identifiers);
                } break;

                    /* Calling expression */
                case Symbol::PARENTHESES_OPEN: {
                    std::shared_ptr<AstExpression> exprptr(expr);
                    /* Parses the argument(s) */
                    AstTuple* tuple = static_cast<AstTuple*>(this->parseTuple());
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
                        this->parseTuple(Symbol::SQUARE_OPEN, Symbol::SQUARE_CLOSE));
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
        token = this->tok();
    }
end:
    return expr;
}

AstExpression* kh::Parser::parseOthers() {
    AstExpression* expr = nullptr;
    Token token = this->tok();

    switch (token.type) {
            /* For all of these literal values be given the AST constant value instance */

        case TokenType::CHARACTER:
            expr = new AstValue(token.begin, token.value.character);
            this->ti++;
            break;

        case TokenType::UINTEGER:
            expr = new AstValue(token.begin, token.value.uinteger);
            this->ti++;
            break;

        case TokenType::INTEGER:
            expr = new AstValue(token.begin, token.value.integer);
            this->ti++;
            break;

        case TokenType::FLOATING:
            expr = new AstValue(token.begin, token.value.floating);
            this->ti++;
            break;

        case TokenType::IMAGINARY:
            expr = new AstValue(token.begin, token.value.imaginary, AstValue::ValueType::IMAGINARY);
            this->ti++;
            break;

        case TokenType::STRING:
            expr = new AstValue(token.begin, token.value.string);
            this->ti++;

            KH_PARSE_GUARD();
            token = this->tok();

            /* Auto concatenation */
            while (token.type == TokenType::STRING) {
                ((AstValue*)expr)->string += token.value.string;
                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();
            }

            break;

        case TokenType::BUFFER:
            expr = new AstValue(token.begin, token.value.buffer);
            this->ti++;

            KH_PARSE_GUARD();
            token = this->tok();

            /* Auto concatenation */
            while (token.type == TokenType::BUFFER) {
                ((AstValue*)expr)->buffer += token.value.buffer;
                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();
            }

            break;

        case TokenType::IDENTIFIER:
            /* Lambda expression */
            if (token.value.identifier == "def") {
                this->ti++;
                KH_PARSE_GUARD();
                AstFunction lambda = this->parseFunction(false);

                if (!lambda.identifiers.empty()) {
                    this->exceptions.emplace_back(
                        "a non-lambda function cannot be defined in an expression", token);
                }

                return new AstFunction(lambda);
            }
            /* Variable declaration */
            else if (token.value.identifier == "ref" || token.value.identifier == "static") {
                AstDeclaration* declaration = new AstDeclaration(this->parseDeclaration());
                declaration->is_static = token.value.identifier == "static";
                return declaration;
            }
            else {
                size_t _ti = this->ti;
                expr = new AstIdentifiers(this->parseIdentifiers());

                KH_PARSE_GUARD();
                token = this->tok();

                /* An identifier is next to another identifier `int number` */
                if (token.type == TokenType::IDENTIFIER && token.value.identifier != "if" &&
                    token.value.identifier != "else") {
                    this->ti = _ti;
                    delete expr;
                    expr = new AstDeclaration(this->parseDeclaration());
                }
                /* An opening square parentheses next to an idenifier, possible array variable
                 * declaration */
                else if (token.type == TokenType::SYMBOL &&
                         token.value.symbol_type == Symbol::SQUARE_OPEN) {
                    size_t exception_counts = this->exceptions.size();

                    this->parseArrayDimension(*static_cast<AstIdentifiers*>(expr));
                    delete expr;

                    /* If there was exceptions while parsing the array dimension type, it probably
                     * wasn't an array variable declaration.. rather a subscript or something */
                    if (this->exceptions.size() > exception_counts) {
                        for (size_t i = 0; i < this->exceptions.size() - exception_counts; i++) {
                            this->exceptions.pop_back();
                        }

                        this->ti = _ti;
                        expr = new AstIdentifiers(this->parseIdentifiers());
                    }
                    else {
                        KH_PARSE_GUARD();
                        token = this->tok();

                        /* Confirmed that it's an array declaration `float[3] position;` */
                        if (token.type == TokenType::IDENTIFIER && token.value.identifier != "if" &&
                            token.value.identifier != "else") {
                            this->ti = _ti;
                            expr = new AstDeclaration(this->parseDeclaration());
                        }
                        /* Probably was just a normal subscript */
                        else {
                            this->ti = _ti;
                            expr = new AstIdentifiers(this->parseIdentifiers());
                        }
                    }
                }
            }
            break;

        case TokenType::SYMBOL:
            switch (token.value.symbol_type) {
                /* Parentheses/tuple expression */
                case Symbol::PARENTHESES_OPEN:
                    expr = this->parseTuple(Symbol::PARENTHESES_OPEN, Symbol::PARENTHESES_CLOSE, false);
                    break;

                    /* List literal expression */
                case Symbol::SQUARE_OPEN:
                    expr = this->parseList();
                    break;

                    /* Dict literal expression */
                case Symbol::CURLY_OPEN:
                    expr = this->parseDict();
                    break;

                default:
                    this->exceptions.emplace_back("unexpected `" + strfy(token) + "` in an expression",
                                                  token);
                    this->ti++;
                    goto end;
            }
            break;

        default:
            this->exceptions.emplace_back("unexpected `" + strfy(token) + "` in an expression", token);
            this->ti++;
            goto end;
    }
end:
    return expr;
}

AstIdentifiers kh::Parser::parseIdentifiers() {
    std::vector<std::string> identifiers;
    std::vector<AstIdentifiers> generics;
    std::vector<size_t> generics_refs;
    std::vector<std::vector<uint64_t>> generics_array;

    bool is_function = false;

    Token token = this->tok();
    size_t index = token.begin;

    /* Expects an identifier */
    if (token.type == TokenType::IDENTIFIER) {
        if (isReservedKeyword(token.value.identifier)) {
            this->exceptions.emplace_back("cannot use a reserved keyword as an identifier", token);
        }

        identifiers.push_back(token.value.identifier);
        this->ti++;
    }
    else {
        this->exceptions.emplace_back("expected an identifier", token);
        goto end;
    }

    KH_PARSE_GUARD();
    token = this->tok();

    /* For each scope in with a dot symbol */
    while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::DOT) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        /* Appends the identifier */
        if (token.type == TokenType::IDENTIFIER) {
            if (isReservedKeyword(token.value.identifier)) {
                this->exceptions.emplace_back("cannot use a reserved keyword as an identifier", token);
            }
            identifiers.push_back(token.value.identifier);
        }
        else {
            this->exceptions.emplace_back("expected an identifier after the dot", token);
            goto end;
        }

        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();
    }

    is_function = identifiers.size() == 1 && identifiers[0] == "func";

    /* Optional genericization */
    if (token.type == TokenType::OPERATOR && token.value.operator_type == Operator::NOT) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();

            generics_refs.push_back(0);
            while (token.type == TokenType::IDENTIFIER && token.value.identifier == "ref") {
                generics_refs.back() += 1;
                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();
            }

            /* Parses the genericization type arguments */
            generics.push_back(this->parseIdentifiers());
            generics_array.push_back(this->parseArrayDimension(generics.back()));
            KH_PARSE_GUARD();
            token = this->tok();

            if (is_function) {
                if (token.type == TokenType::SYMBOL &&
                    token.value.symbol_type == Symbol::PARENTHESES_OPEN) {
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->tok();

                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                        this->ti++;
                        goto funcFinish;
                    }
                    else {
                        goto forceIn;
                    }
                }
                else {
                    this->exceptions.emplace_back("expected an opening parentheses after the "
                                                  "return type in the genericization of `func`",
                                                  token);
                }
            }

            while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA) {
                this->ti++;
                KH_PARSE_GUARD();
                token = this->tok();

            forceIn:
                generics_refs.push_back(0);
                while (token.type == TokenType::IDENTIFIER && token.value.identifier == "ref") {
                    generics_refs.back() += 1;
                    this->ti++;
                    KH_PARSE_GUARD();
                    token = this->tok();
                }

                generics.emplace_back(this->parseIdentifiers());
                generics_array.push_back(this->parseArrayDimension(generics.back()));

                KH_PARSE_GUARD();
                token = this->tok();
            }

            /* Expects closing parentheses */
            if (token.type == TokenType::SYMBOL &&
                token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                this->ti++;

                if (is_function) {
                funcFinish:
                    KH_PARSE_GUARD();
                    token = this->tok();

                    if (token.type == TokenType::SYMBOL &&
                        token.value.symbol_type == Symbol::PARENTHESES_CLOSE) {
                        this->ti++;
                    }
                    else {
                        this->exceptions.emplace_back("expected a closing parentheses", token);
                    }
                }
            }
            else {
                this->exceptions.emplace_back("expected a closing parentheses", token);
            }
        }
        else if (token.type == TokenType::IDENTIFIER) {
            if (is_function) {
                this->exceptions.emplace_back(
                    "expected an opening parentheses for genericization of `func`", token);
            }
            generics.push_back(this->parseIdentifiers());
            generics_refs.push_back(0);
            generics_array.push_back({});
        }
        else {
            this->exceptions.emplace_back("expected either an identifier or an opening "
                                          "parentheses for genericization after the "
                                          "exclamation mark ",
                                          token);
            this->ti++;
        }
    }
    else if (is_function) {
        this->exceptions.emplace_back("`func` requires genericization", token);
    }
end:
    return {index, identifiers, generics, generics_refs, generics_array};
}

AstExpression* kh::Parser::parseTuple(Symbol opening, Symbol closing, bool explicit_tuple) {
    std::vector<AstExpression*> elements;

    Token token = this->tok();
    size_t index = token.begin;

    /* Expects the opening symbol */
    if (token.type == TokenType::SYMBOL && token.value.symbol_type == opening) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        /* Instant close */
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == closing) {
            this->ti++;
            goto end;
        }

        while (true) {
            /* Parses the element expression */
            elements.push_back(this->parseExpression());
            KH_PARSE_GUARD();
            token = this->tok();

            if (token.type == TokenType::SYMBOL && token.value.symbol_type == closing) {
                this->ti++;
                break;
            }
            else if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA)) {
                this->exceptions.emplace_back(closing == Symbol::SQUARE_CLOSE
                                                  ? "expected a comma or a closing square bracket"
                                                  : "expected a comma or a closing parentheses",
                                              token);
                this->ti++;
                break;
            }

            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();

            /* Cases for explicit one-elemented tuples `(69420,)` */
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == closing) {
                this->ti++;
                explicit_tuple = true;
                break;
            }
        }
    }
    else {
        this->exceptions.emplace_back(opening == Symbol::SQUARE_OPEN
                                          ? "expected an opening square bracket"
                                          : "expected an opening parentheses",
                                      token);
        this->ti++;
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

AstExpression* kh::Parser::parseList() {
    Token token = this->tok();
    AstTuple* tuple = (AstTuple*)parseTuple(Symbol::SQUARE_OPEN, Symbol::SQUARE_CLOSE, false);
    AstList* list = new AstList(tuple->index, tuple->elements);
    delete tuple;

    return list;
}

AstExpression* kh::Parser::parseDict() {
    std::vector<std::shared_ptr<AstExpression>> keys;
    std::vector<std::shared_ptr<AstExpression>> items;

    Token token = this->tok();
    size_t index = token.begin;

    if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        /* Instant close*/
        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_CLOSE) {
            this->ti++;
            goto end;
        }
        else {
            goto forceIn;
        }
        do {
            this->ti++;
            KH_PARSE_GUARD();
        forceIn:
            keys.emplace_back(this->parseExpression());

            KH_PARSE_GUARD();
            token = this->tok();
            if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COLON) {
                this->ti++;
                KH_PARSE_GUARD();
            }
            else {
                this->exceptions.emplace_back("expected a colon after a key of the dict literal",
                                              token);
            }
            items.emplace_back(this->parseExpression());
            KH_PARSE_GUARD();
            token = this->tok();
        } while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::COMMA);

        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::CURLY_CLOSE) {
            this->ti++;
        }
        else {
            this->exceptions.emplace_back("expected a closing curly bracket closing the dict literal",
                                          token);
        }
    }
    else {
        this->exceptions.emplace_back("expected an opening curly bracket for the dict literal", token);
    }
end:
    return new AstDict(index, keys, items);
}

std::vector<uint64_t> kh::Parser::parseArrayDimension(AstIdentifiers& type) {
    std::vector<uint64_t> dimension;
    Token token = this->tok();

    while (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_OPEN) {
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();

        if (token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_CLOSE) {
            type = AstIdentifiers(token.begin, {"list"}, {type}, {false},
                                  dimension.size() ? std::vector<std::vector<uint64_t>>{dimension}
                                                   : std::vector<std::vector<uint64_t>>{{}});

            dimension.clear();
        }
        else if (token.type == TokenType::INTEGER || token.type == TokenType::UINTEGER) {
            dimension.push_back(token.value.uinteger);
            if (token.value.uinteger == 0) {
                this->exceptions.emplace_back("an array could not be zero sized", token);
            }

            this->ti++;
            KH_PARSE_GUARD();
            token = this->tok();

            if (!(token.type == TokenType::SYMBOL && token.value.symbol_type == Symbol::SQUARE_CLOSE)) {
                this->exceptions.emplace_back("expected a closing square bracket in the array size",
                                              token);
            }
        }
        else {
            this->exceptions.emplace_back(
                "unexpected `" + strfy(token) + "` while parsing the array size", token);
        }
        this->ti++;
        KH_PARSE_GUARD();
        token = this->tok();
    }

end:
    return dimension;
}
