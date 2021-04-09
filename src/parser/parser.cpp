/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/parser/parser.cpp
* Defines include/parser/parser.hpp.
*/

#include <iostream>
#include "parser/parser.hpp"

#define GUARD(offset) do {                                                              \
        if (this->ti + offset >= this->tokens.size()) {                                 \
            this->exceptions.emplace_back(U"Was expecting a token but hit EOF",         \
                this->tokens.size() ? this->tokens[this->tokens.size() - 1].index : 0); \
        goto end;                                                                       \
    }} while (false)


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

kh::Parser::~Parser() {

}

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

            if (identifier == U"var" || identifier == U"ref" || identifier == U"static" || identifier == U"private" || identifier == U"public") {
                bool is_static, is_public;
                this->parseAccessAttribs(is_static, is_public);
                GUARD(0);
                variables.emplace_back(this->parseDeclaration(is_static, is_public));

                GUARD(0);
                token = this->to();
                if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                    this->ti++;
                else
                    this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);
            }
            else if (identifier == U"def") {
                this->ti++;
                GUARD(0);

                bool is_static, is_public;
                this->parseAccessAttribs(is_static, is_public);
                GUARD(0);
                functions.emplace_back(this->parseFunction(is_static, is_public));
            }
            else if (identifier == U"class") {
                this->ti++;
                GUARD(0);
                classes.emplace_back(this->parseClass());
            }
            else if (identifier == U"struct") {
                this->ti++;
                GUARD(0);
                structs.emplace_back(this->parseStruct());
            }
            else if (identifier == U"enum") {
                this->ti++;
                GUARD(0);
                enums.emplace_back(this->parseEnum());
            }
            else if (identifier == U"import") {
                this->ti++;
                GUARD(0);
                imports.emplace_back(this->parseImport(false));
            }
            else if (identifier == U"include") {
                this->ti++;
                GUARD(0);
                imports.emplace_back(this->parseImport(true));
            }
            else
                this->exceptions.emplace_back(U"Unexpected identifier", token.index);
        } break;

        case kh::TokenType::SYMBOL:
            switch (token.value.symbol_type) {
            case kh::Symbol::SEMICOLON: {
                this->ti++;
            } break;

            default:
                this->ti++;
                this->exceptions.emplace_back(U"Unexpected token", token.index);
            } break;

        default:
            this->ti++;
            this->exceptions.emplace_back(U"Unexpected token", token.index);
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

    if (token.type == kh::TokenType::IDENTIFIER) {
        path.push_back(token.value.identifier);
        this->ti++;
    }
    else
        this->exceptions.emplace_back(U"Was expecting an identifier", token.index);

    GUARD(0);
    token = this->to();

    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        this->ti++;
        GUARD(0);
        token = this->to();

        if (token.type == kh::TokenType::IDENTIFIER) {
            path.push_back(token.value.identifier);
            this->ti++;
            GUARD(0);
            token = this->to();
        }
        else
            this->exceptions.emplace_back(U"Was expecting an identifier", token.index);
    }

    if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"as") {
        this->ti++;
        GUARD(0);
        token = this->to();

        if (token.type == kh::TokenType::IDENTIFIER) {
            identifier = token.value.identifier;

            this->ti++;
            GUARD(0);
            token = this->to();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                this->ti++;
            else
                this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);
        }
        else if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
            this->ti++;
    }
end:
    return new kh::AstImport(index, path, is_include, path.empty() ? U"" : (identifier.empty() ? path.back() : identifier));
}

void kh::Parser::parseAccessAttribs(bool& is_static, bool& is_public) {
    is_static = false;
    is_public = true;

    kh::Token token = this->to();
    while (token.type == kh::TokenType::IDENTIFIER) {
        if (token.value.identifier == U"var") {} /* dummy */
        else if (token.value.identifier == U"static") is_static = true;
        else if (token.value.identifier == U"public")  is_public = true;
        else if (token.value.identifier == U"private") is_public = false;
        else break;

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

    while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
        return_ref_depth++;
        this->ti++;
        GUARD(0);
        token = this->to();
    }

    if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::BIT_AND) {
        this->ti++;
        return_type.reset(new kh::AstIdentifierExpression(token.index, { U"void" }, {}));
        goto parseArgs;
    }

    return_type_or_identifiers.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());

    GUARD(0);
    token = this->to();

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        return_type.reset(new kh::AstIdentifierExpression(return_type_or_identifiers->index, { U"void" }, {}));
        identifiers = return_type_or_identifiers->identifiers;
    }
    else if (token.type == kh::TokenType::IDENTIFIER) {
        return_type = return_type_or_identifiers;

        std::shared_ptr<kh::AstIdentifierExpression> id_generic_args((kh::AstIdentifierExpression*)this->parseIdentifiers());
        identifiers = id_generic_args->identifiers;

        for (auto generic_ : id_generic_args->generics) {
            if (generic_->identifiers.size() != 1)
                this->exceptions.emplace_back(U"Could not have multiple identifiers as a generic argument name", generic_->index);
            if (!generic_->generics.empty())
                this->exceptions.emplace_back(U"Could not have generic arguments in a generic argument", generic_->generics[0]->index);

            generic_args.push_back(generic_->identifiers.empty() ? U"" : generic_->identifiers[0]);
        }
    }
    else if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::BIT_AND) {
        return_type = return_type_or_identifiers;
        this->ti++;
    }
    else{
        this->exceptions.emplace_back(U"Unexpected token", token.index);
        goto end;
    }

parseArgs:
    GUARD(0);
    token = this->to();

    if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)) {
        this->exceptions.emplace_back(U"Was expecting an opening parentheses", token.index);
        goto end;
    }

    this->ti++;
    GUARD(0);
    token = this->to();

    while (true) {
        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            break;

        arguments.emplace_back(this->parseDeclaration(false, false));

        GUARD(0);
        token = this->to();
        
        if (token.type == kh::TokenType::SYMBOL) {
            if (token.value.symbol_type == kh::Symbol::COMMA) {
                this->ti++;
                GUARD(0);
                continue;
            }
            else if (token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
                break;
            else {
                this->exceptions.emplace_back(U"Was expecting a closing parentheses or a comma", token.index);
                goto end;
            }
        }
        else {
            this->exceptions.emplace_back(U"Was expecting a closing parentheses or a comma", token.index);
            goto end;
        }
    }

    this->ti++;
    GUARD(0);

    body = this->parseBody();
end:
    return new kh::AstFunctionExpression(
        index, identifiers, generic_args,
        return_type, return_ref_depth,
        arguments, body, is_static, is_public
    );
}

kh::AstDeclarationExpression* kh::Parser::parseDeclaration(const bool is_static, const bool is_public) {
    std::shared_ptr<kh::AstIdentifierExpression> var_type;
    std::u32string var_name;
    std::shared_ptr<kh::AstExpression> expression = nullptr;
    size_t ref_depth = 0;

    kh::Token token = this->to();
    size_t index = token.index;

    while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"ref") {
        ref_depth++;
        this->ti++;
        GUARD(0);
        token = this->to();
    }

    var_type.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());

    GUARD(0);
    token = this->to();
    if (token.type != kh::TokenType::IDENTIFIER) {
        this->exceptions.emplace_back(U"Was expecting an identifier", token.index);
        goto end;
    }

    var_name = token.value.identifier;
    this->ti++;
    GUARD(0);
    token = this->to();

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN)
        expression.reset(this->parseTuple());
    else if (token.type == kh::TokenType::OPERATOR && token.value.operator_type == kh::Operator::ASSIGN) {
        this->ti++;
        GUARD(0);
        expression.reset(this->parseExpression());
    }
    else
        goto end;
end:
    return new kh::AstDeclarationExpression(
        index, var_type, var_name, expression, ref_depth,
        is_static, is_public
    );
}

kh::AstClass* kh::Parser::parseClass() {
    std::u32string name;
    std::shared_ptr<kh::AstIdentifierExpression> base;
    std::vector<std::u32string> generic_args;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> members;
    std::vector<std::shared_ptr<kh::AstFunctionExpression>> methods;

    kh::Token token = this->to();
    size_t index = token.index;

    if (token.type == kh::TokenType::IDENTIFIER) {
        name = token.value.identifier;
        this->ti++;
    }
    else
        this->exceptions.emplace_back(U"Was expecting an identifier", token.index);

    GUARD(0);
    token = this->to();

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        while (true) {
            if (token.type == kh::TokenType::IDENTIFIER)
                generic_args.push_back(token.value.identifier);
            else if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_CLOSE) {
                this->ti++;
                break;
            }

            this->ti++;
            GUARD(0);
            token = this->to();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_CLOSE) {
                this->ti++;
                break;
            }
            else if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COMMA)) {
                this->exceptions.emplace_back(U"Was expecting a comma or a generic close", token.index);
                break;
            }

            this->ti++;
            GUARD(0);
            token = this->to();
        }
    }

    GUARD(0);
    token = this->to();

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        this->ti++;
        GUARD(0);
        base.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());
        GUARD(0);
        token = this->to();
        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            this->ti++;
        else
            this->exceptions.emplace_back(U"Was expecting a closing parentheses", token.index);
    }

    GUARD(0);
    token = this->to();

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        while (true) {
            GUARD(0);
            kh::Token& token = this->to();

            switch (token.type) {
            case kh::TokenType::IDENTIFIER: {
                if (token.value.identifier == U"var" || token.value.identifier == U"ref" || token.value.identifier == U"static" || token.value.identifier == U"private" || token.value.identifier == U"public") {
                    bool is_static, is_public;
                    this->parseAccessAttribs(is_static, is_public);
                    GUARD(0);
                    members.emplace_back(this->parseDeclaration(is_static, is_public));

                    GUARD(0);
                    token = this->to();
                    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else {
                        this->ti++;
                        this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);
                    }
                }
                else if (token.value.identifier == U"def") {
                    this->ti++;
                    GUARD(0);

                    bool is_static, is_public;
                    this->parseAccessAttribs(is_static, is_public);
                    GUARD(0);
                    methods.emplace_back(this->parseFunction(is_static, is_public));
                }
                else {
                    this->ti++;
                    this->exceptions.emplace_back(U"Unexpected identifier", token.index);
                }
            } break;

            case kh::TokenType::SYMBOL: {
                switch (token.value.symbol_type) {
                case kh::Symbol::SEMICOLON: {
                    this->ti++;
                } break;

                case kh::Symbol::CURLY_CLOSE: {
                    this->ti++;
                    goto end;
                } break;

                default:
                    this->ti++;
                    this->exceptions.emplace_back(U"Unexpected token", token.index);
                }
            } break;

            default:
                this->ti++;
                this->exceptions.emplace_back(U"Unexpected token", token.index);
            }
        }
    }
    else
        this->exceptions.emplace_back(U"Was expecting an opening curly bracket", token.index);
end:
    return new kh::AstClass(index, name, base, generic_args, members, methods);
}

kh::AstStruct* kh::Parser::parseStruct() {
    std::u32string name;
    std::shared_ptr<kh::AstIdentifierExpression> base;
    std::vector<std::shared_ptr<kh::AstDeclarationExpression>> members;

    kh::Token token = this->to();
    size_t index = token.index;

    if (token.type == kh::TokenType::IDENTIFIER) {
        name = token.value.identifier;
        this->ti++;
    }
    else
        this->exceptions.emplace_back(U"Was expecting an identifier", token.index);

    GUARD(0);
    token = this->to();

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        while (true) {
            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            else if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON) {
                this->ti++;
                GUARD(0);
                token = this->to();
                continue;
            }
            else {
                bool is_static, is_public;
                this->parseAccessAttribs(is_static, is_public);
                members.emplace_back(this->parseDeclaration(is_static, is_public));
            }

            GUARD(0);
            token = this->to();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            else if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON))
                this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);

            this->ti++;
            GUARD(0);
            token = this->to();
        }
    }
    else
        this->exceptions.emplace_back(U"Was expecting an opening curly bracket", token.index);
end:
    return new kh::AstStruct(index, name, base, members);
}

kh::AstEnum* kh::Parser::parseEnum() {
    std::u32string name;
    std::shared_ptr<kh::AstIdentifierExpression> base;
    std::vector<std::u32string> members;

    kh::Token token = this->to();
    size_t index = token.index;

    if (token.type == kh::TokenType::IDENTIFIER) {
        name = token.value.identifier;
        this->ti++;
    }
    else
        this->exceptions.emplace_back(U"Was expecting an identifier", token.index);

    GUARD(0);
    token = this->to();

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN) {
        base.reset((kh::AstIdentifierExpression*)this->parseIdentifiers());
        GUARD(0);
        token = this->to();
        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::PARENTHESES_CLOSE)
            this->ti++;
        else
            this->exceptions.emplace_back(U"Was expecting a closing parentheses", token.index);
    }

    GUARD(0);
    token = this->to();

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        while (true) {
            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            else if (token.type == kh::TokenType::IDENTIFIER)
                members.push_back(token.value.identifier);

            this->ti++;
            GUARD(0);
            token = this->to();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_CLOSE) {
                this->ti++;
                break;
            }
            else if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COMMA))
                this->exceptions.emplace_back(U"Was expecting a closing curly bracket or a comma", token.index);

            this->ti++;
            GUARD(0);
            token = this->to();
        }
    }
    else
        this->exceptions.emplace_back(U"Was expecting an opening curly bracket", token.index);
end:
    return new kh::AstEnum(index, name, base, members);
}

std::vector<std::shared_ptr<kh::AstBody>> kh::Parser::parseBody() {
    std::vector<std::shared_ptr<kh::AstBody>> body;
    kh::Token token = this->to();

    if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::CURLY_OPEN)) {
        this->exceptions.emplace_back(U"Was expecting an opening curly bracket", token.index);
        goto end;
    }

    this->ti++;

    while (true) {
        GUARD(0);
        token = this->to();
        size_t index = token.index;

        switch (token.type) {
        case kh::TokenType::IDENTIFIER: {
            if (token.value.identifier == U"def") {
                this->ti++;
                GUARD(0);

                bool is_static, is_public;
                this->parseAccessAttribs(is_static, is_public);
                GUARD(0);
                body.emplace_back(this->parseFunction(is_static, is_public));
            }
            else if (token.value.identifier == U"if") {
                std::vector<std::shared_ptr<kh::AstExpression>> conditions;
                std::vector<std::vector<std::shared_ptr<kh::AstBody>>> bodies;
                std::vector<std::shared_ptr<kh::AstBody>> else_body;

                do {
                    this->ti++;
                    token = this->to();
                    GUARD(0);
                    conditions.emplace_back(this->parseExpression());
                    GUARD(0);
                    bodies.emplace_back(this->parseBody());
                    GUARD(0);
                    token = this->to();
                } while (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"elif");

                if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"else") {
                    this->ti++;
                    GUARD(0);
                    else_body = this->parseBody();
                }

                body.emplace_back(new kh::AstIf(index, conditions, bodies, else_body));
            }
            else if (token.value.identifier == U"while") {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> condition(this->parseExpression());
                std::vector<std::shared_ptr<kh::AstBody>> while_body = this->parseBody();

                body.emplace_back(new kh::AstWhile(index, condition, while_body));
            }
            else if (token.value.identifier == U"do") {
                this->ti++;
                GUARD(0);

                std::vector<std::shared_ptr<kh::AstBody>> do_while_body = this->parseBody();
                std::shared_ptr<kh::AstExpression> condition;

                GUARD(0);
                token = this->to();

                if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"while") {
                    this->ti++;
                    condition.reset(this->parseExpression());
                }
                else
                    this->exceptions.emplace_back(U"Was expecting `while`", token.index);

                GUARD(0);
                token = this->to();

                if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                    this->ti++;
                else
                    this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);

                body.emplace_back(new kh::AstDoWhile(index, condition, do_while_body));
            }
            else if (token.value.identifier == U"for") {
                this->ti++;
                GUARD(0);

                std::vector<std::shared_ptr<kh::AstExpression>> targets;
                std::shared_ptr <kh::AstExpression> iterator;
                std::vector<std::shared_ptr<kh::AstBody>> for_body;

                while (true) {
                    targets.emplace_back(this->parseExpression());

                    GUARD(0);
                    kh::Token token = this->to();
                    if (token.type == kh::TokenType::SYMBOL) {
                        switch (token.value.symbol_type) {
                        case kh::Symbol::COMMA:
                            this->ti++;
                            GUARD(0);
                            break;

                        case kh::Symbol::COLON:
                            this->ti++;
                            GUARD(0);
                            goto _break_while;

                        default:
                            this->exceptions.emplace_back(U"Was expecting a colon", token.index);
                        }
                    }
                    else
                        this->exceptions.emplace_back(U"Was expecting a colon", token.index);
                } _break_while:

                iterator.reset(this->parseExpression());
                for_body = this->parseBody();

                body.emplace_back(new kh::AstFor(index, targets, iterator, for_body));
            }
            else if (token.value.identifier == U"continue") {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> expression((kh::AstExpression*)nullptr);
                token = this->to();

                if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                    this->ti++;
                else
                    this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);

                body.emplace_back(new kh::AstStatement(index, kh::AstStatement::Type::CONTINUE, expression));
            }
            else if (token.value.identifier == U"break") {
                this->ti++;
                GUARD(0);

                std::shared_ptr<kh::AstExpression> expression((kh::AstExpression*)nullptr);
                token = this->to();

                if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                    this->ti++;
                else
                    this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);

                body.emplace_back(new kh::AstStatement(index, kh::AstStatement::Type::BREAK, expression));
            }
            else if (token.value.identifier == U"return") {
                this->ti++;
                GUARD(0);
                token = this->to();
                
                std::shared_ptr<kh::AstExpression> expression((kh::AstExpression*)nullptr);
                if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                    this->ti++;
                else {
                    expression.reset(this->parseExpression());
                    GUARD(0);
                    token = this->to();

                    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                        this->ti++;
                    else
                        this->exceptions.emplace_back(U"Was expecting a semicolon", token.index);
                }

                body.emplace_back(new kh::AstStatement(index, kh::AstStatement::Type::RETURN, expression));
            }
            else
                goto parse_expr;
        } break;

        case kh::TokenType::SYMBOL:
            switch (token.value.symbol_type) {
            case kh::Symbol::SEMICOLON: {
                this->ti++;
            } break;

            case kh::Symbol::CURLY_CLOSE: {
                this->ti++;
                goto end;
            } break;

            case kh::Symbol::DOLLAR: {
                this->ti++;
                GUARD(0);
                token = this->to();

                std::u32string op_name;
                std::vector<std::shared_ptr<kh::AstExpression>> op_args;

                if (token.type == kh::TokenType::IDENTIFIER)
                    op_name = token.value.identifier;
                else
                    this->exceptions.emplace_back(U"Was expecting an identifier", token.index);

                this->ti++;
                GUARD(0);
                token = this->to();

                while (true) {
                    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                        break;

                    op_args.emplace_back(this->parseExpression());

                    GUARD(0);
                    token = this->to();

                    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
                        break;
                    else if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COMMA)) {
                        this->exceptions.emplace_back(U"Was expecting a semicolon or a comma", token.index);
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
            } break;

        default:
        parse_expr: {
            std::shared_ptr<kh::AstExpression> expr(this->parseExpression());
            GUARD(0);
            token = this->to();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::SEMICOLON)
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

    if (token.type == kh::TokenType::IDENTIFIER && (
        token.value.identifier == U"var" || token.value.identifier == U"ref" || token.value.identifier == U"static" || token.value.identifier == U"private" || token.value.identifier == U"public"
        )) {
        bool is_static, is_public;
        this->parseAccessAttribs(is_static, is_public);
        GUARD(0);
        return this->parseDeclaration(is_static, is_public);
    }
    else if (token.type == kh::TokenType::IDENTIFIER && token.value.identifier == U"def") {
        bool is_static, is_public;
        this->ti++;
        GUARD(0);
        this->parseAccessAttribs(is_static, is_public);
        GUARD(0);
        return this->parseFunction(is_static, is_public);
    }
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
            this->exceptions.emplace_back(U"Was expecting a colon", token.index);
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

#define RECURSIVE_DESCENT_SINGULAR_OP(lower, op) do {                                   \
        kh::AstExpression* expr = lower();                                              \
        kh::Token token;                                                                \
        size_t index;                                                                   \
        GUARD(0);                                                                       \
        token = this->to();                                                             \
        index = token.index;                                                            \
        while (token.type == kh::TokenType::OPERATOR && token.value.operator_type == op) { \
            this->ti++;                                                                 \
            GUARD(0);                                                                   \
            std::shared_ptr<kh::AstExpression> lval(expr);                              \
            std::shared_ptr<kh::AstExpression> rval(lower());                           \
            expr = new kh::AstBinaryExpression(token.index, token.value.operator_type, lval, rval); \
            GUARD(0);                                                                   \
            token = this->to();                                                         \
        }                                                                               \
        GUARD(0);                                                                       \
        token = this->to();                                                             \
    end:                                                                                \
        return expr;                                                                    \
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
            this->exceptions.emplace_back(U"Unexpected operator", token.index);
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
        break;

    case kh::TokenType::BUFFER:
        expr = new kh::AstConstValue(token.index, token.value.buffer);
        this->ti++;
        break;

    case kh::TokenType::IDENTIFIER:
        expr = this->parseIdentifiers();
        break;

    case kh::TokenType::SYMBOL:
        switch (token.value.symbol_type) {
        case kh::Symbol::PARENTHESES_OPEN: {
            kh::AstTupleExpression* tuple = (kh::AstTupleExpression*)this->parseTuple();

            if (tuple->elements.size() == 1) {
                expr = tuple->elements[0].get();

                /* Sketchy solution but hope this works */
                std::shared_ptr<kh::AstExpression>* dummy_ptr = new std::shared_ptr<kh::AstExpression>(tuple->elements[0]);
                std::free((void*)dummy_ptr);
            }
            else
                expr = tuple;
        } break;

        default:
            this->exceptions.emplace_back(U"Unexpected token", token.index);
            goto end;
        } break;

    default:
        this->exceptions.emplace_back(U"Unexpected token", token.index);
        goto end;
    }

    GUARD(0);
    token = this->to();

    while (token.type == kh::TokenType::SYMBOL && (
        token.value.symbol_type == kh::Symbol::DOT ||
        token.value.symbol_type == kh::Symbol::PARENTHESES_OPEN ||
        token.value.symbol_type == kh::Symbol::SQUARE_OPEN
    )) {
        index = token.index;

        switch (token.value.symbol_type) {
        case kh::Symbol::DOT: {
            std::vector<std::u32string> identifiers;

            do {
                this->ti++;
                GUARD(0);
                token = this->to();

                if (token.type == kh::TokenType::IDENTIFIER)
                    identifiers.push_back(token.value.identifier);
                else
                    this->exceptions.emplace_back(U"Was expecting an identifier", token.index);

                this->ti++;
                GUARD(0);
                token = this->to();
            } while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT);

            std::shared_ptr<kh::AstExpression> exprptr(expr);
            expr = new kh::AstScopeExpression(index, exprptr, identifiers);
        } break;

        case kh::Symbol::PARENTHESES_OPEN: {
            std::shared_ptr<kh::AstExpression> exprptr(expr);
            kh::AstTupleExpression* tuple = (kh::AstTupleExpression*)this->parseTuple();
            std::vector<std::shared_ptr<kh::AstExpression>> arguments;

            for (std::shared_ptr<kh::AstExpression>& element : tuple->elements)
                arguments.push_back(element);

            expr = new kh::AstCallExpression(index, exprptr, arguments);
            delete tuple;
        } break;

        case kh::Symbol::SQUARE_OPEN: {
            std::shared_ptr<kh::AstExpression> exprptr(expr);
            kh::AstTupleExpression* tuple = (kh::AstTupleExpression*)this->parseTuple(kh::Symbol::SQUARE_OPEN, kh::Symbol::SQUARE_CLOSE);
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

    while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::DOT) {
        this->ti++;
        GUARD(0);
        token = this->to();

        if (token.type == kh::TokenType::IDENTIFIER)
            identifiers.push_back(token.value.identifier);
        else {
            this->exceptions.emplace_back(U"Was expecting an identifier", token.index);
            goto end;
        }

        this->ti++;
        GUARD(0);
        token = this->to();
    }

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_OPEN) {
        this->ti++;
        GUARD(0);
        token = this->to();

        if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_CLOSE)
            this->ti++;
        else {
            generics.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());
            GUARD(0);
            token = this->to();

            while (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COMMA) {
                this->ti++;
                GUARD(0);

                generics.emplace_back((kh::AstIdentifierExpression*)this->parseIdentifiers());

                GUARD(0);
                token = this->to();
            }

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::GENERIC_CLOSE)
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

    if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == opening) {
        this->ti++;
        GUARD(0);
        token = this->to();

        bool skip_parentheses = true;
        while (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing)) {
            elements.emplace_back(this->parseExpression());
            GUARD(0);
            token = this->to();

            if (token.type == kh::TokenType::SYMBOL && token.value.symbol_type == closing) {
                this->ti++;
                skip_parentheses = false;
                break;
            }
            else if (!(token.type == kh::TokenType::SYMBOL && token.value.symbol_type == kh::Symbol::COMMA)) {
                this->exceptions.emplace_back(closing == kh::Symbol::SQUARE_CLOSE ? U"Was expecting a comma or a closing square bracket" : U"Was expecting a comma or a closing parentheses", token.index);
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
        this->exceptions.emplace_back(opening == kh::Symbol::SQUARE_OPEN ? U"Was expecting an opening square bracket" : U"Was expecting an opening parentheses", token.index);

end:
    return new kh::AstTupleExpression(index, elements);
}
