/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/parser/ast.hpp
* Declares AST node types and their kh::repr overloads.
*/

#pragma once

#include <complex>
#include <string>
#include <vector>

#include "parser/token.hpp"


namespace kh {
    class AstModule;
    class AstImport;
    class AstFunction;
    class AstClass;
    class AstStruct;
    class AstEnum;
    class AstBody;
    class AstExpression;
    class AstIdentifierExpression;
    class AstUnaryExpression;
    class AstBinaryExpression;
    class AstTrinaryExpression;
    class AstSubscriptExpression;
    class AstCallExpression;
    class AstDeclarationExpression;
    class AstScopeExpression;
    class AstConstValue;
    class AstTupleExpression;
    class AstIf;
    class AstWhile;
    class AstDoWhile;
    class AstFor;
    class AstStatement;

    std::u32string repr(const kh::AstModule& module_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstImport& import_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstFunction& function_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstClass& class_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstStruct& struct_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstEnum& enum_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstBody& ast, const size_t indent = 0);
    std::u32string repr(const kh::AstExpression& expr, const size_t indent = 0);

    class AstModule {
    public:
        std::vector<kh::AstImport*> imports;
        std::vector<kh::AstFunction*> functions;
        std::vector<kh::AstClass*> classes;
        std::vector<kh::AstStruct*> structs;
        std::vector<kh::AstEnum*> enums;
        std::vector<kh::AstDeclarationExpression*> variables;

        AstModule(const std::vector<kh::AstImport*>& _imports, const std::vector<kh::AstFunction*>& _functions,
            const std::vector<kh::AstClass*>& _classes, const std::vector<kh::AstStruct*>& _structs, const std::vector<kh::AstEnum*>& _enums,
            const std::vector<kh::AstDeclarationExpression*>& _variables) :
            variables(_variables), imports(_imports), functions(_functions), classes(_classes), structs(_structs), enums(_enums) {}
        virtual ~AstModule();
    };

    class AstImport {
    public:
        size_t index;
        std::vector<std::u32string> path;
        std::u32string identifier;

        AstImport(const size_t _index, const std::vector<std::u32string>& _path, const std::u32string& _identifier) :
            index(_index), path(_path), identifier(_identifier) {}
        virtual ~AstImport() {}
    };

    class AstFunction {
    public:
        size_t index;
        std::vector<std::u32string> identifiers;
        std::vector<std::u32string> generic_args;
        kh::AstIdentifierExpression* return_type;
        size_t return_ref_depth;
        std::vector<kh::AstDeclarationExpression*> arguments;
        std::vector<kh::AstFunction*> nested_functions;
        std::vector<kh::AstBody*> body;
        bool is_static;
        bool is_public;

        AstFunction(const size_t _index, const std::vector<std::u32string>& _identifiers, const std::vector<std::u32string>& _generic_args,
            kh::AstIdentifierExpression* _return_type, const size_t _return_ref_depth,
            const std::vector<kh::AstDeclarationExpression*>& _arguments, const std::vector<kh::AstFunction*>& _nested_functions,
            const std::vector<kh::AstBody*>& _body, const bool _is_static, const bool _is_public) :
            index(_index), identifiers(_identifiers), generic_args(_generic_args), return_type(_return_type), return_ref_depth(_return_ref_depth),
            arguments(_arguments), nested_functions(_nested_functions), body(_body), is_static(_is_static), is_public(_is_public) {}
        virtual ~AstFunction();
    };

    class AstClass {
    public:
        size_t index;
        std::u32string name;
        kh::AstIdentifierExpression* base;
        std::vector<std::u32string> generic_args;
        std::vector<kh::AstDeclarationExpression*> members;
        std::vector<kh::AstFunction*> methods;

        AstClass(const size_t _index, const std::u32string& _name, kh::AstIdentifierExpression* _base, const std::vector<std::u32string>& _generic_args,
            const std::vector<kh::AstDeclarationExpression*>& _members, const std::vector<kh::AstFunction*>& _methods) :
            index(_index), name(_name), base(_base), generic_args(_generic_args), members(_members), methods(_methods) {}
        virtual ~AstClass();
    };

    class AstStruct {
    public:
        size_t index;
        std::u32string name;
        kh::AstIdentifierExpression* base;
        std::vector<kh::AstDeclarationExpression*> members;

        AstStruct(const size_t _index, const std::u32string& _name, kh::AstIdentifierExpression* _base, const std::vector<kh::AstDeclarationExpression*> _members) :
            index(_index), name(_name), base(_base), members(_members) {}
        virtual ~AstStruct();
    };

    class AstEnum {
    public:
        size_t index;
        std::u32string name;
        kh::AstIdentifierExpression* base;
        std::vector<std::u32string> members;

        AstEnum(const size_t _index, const std::u32string& _name, kh::AstIdentifierExpression* _base, const std::vector<std::u32string>& _members) :
            index(_index), name(_name), base(_base), members(_members) {}
        virtual ~AstEnum();
    };

    class AstBody {
    public:
        size_t index;
        enum class Type {
            NONE, EXPRESSION, IF, WHILE, DO_WHILE, FOR, STATEMENT, INSTRUCTION
        } type = Type::NONE;

        virtual ~AstBody() {}
    };

    class AstExpression : public kh::AstBody {
    public:
        enum class ExType {
            NONE,
            IDENTIFIER,
            UNARY, BINARY, TERNARY,
            SUBSCRIPT, CALL,
            DECLARE,
            SCOPE, CONSTANT, TUPLE
        } expression_type = ExType::NONE;

        virtual ~AstExpression() {}
    };

    class AstIdentifierExpression : public kh::AstExpression {
    public:
        std::vector<std::u32string> identifiers;
        std::vector<kh::AstIdentifierExpression*> generics;

        AstIdentifierExpression(const size_t _index, const std::vector<std::u32string>& _identifiers, const std::vector<kh::AstIdentifierExpression*>& _generics) :
            identifiers(_identifiers), generics(_generics) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::IDENTIFIER;
        }
        virtual ~AstIdentifierExpression() {
            for (kh::AstIdentifierExpression* template_ : this->generics)
                if (template_) delete template_;
        }
    };

    class AstUnaryExpression : public kh::AstExpression {
    public:
        kh::Operator operation;
        kh::AstExpression* rvalue;

        AstUnaryExpression(const size_t _index, const kh::Operator _operation, kh::AstExpression* _rvalue) :
            operation(_operation), rvalue(_rvalue) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::UNARY;
        }
        virtual ~AstUnaryExpression() {
            if (this->rvalue) delete this->rvalue;
        }
    };

    class AstBinaryExpression : public kh::AstExpression {
    public:
        kh::Operator operation;
        kh::AstExpression* lvalue;
        kh::AstExpression* rvalue;

        AstBinaryExpression(const size_t _index, const kh::Operator _operation, kh::AstExpression* _lvalue, kh::AstExpression* _rvalue) :
            operation(_operation), lvalue(_lvalue), rvalue(_rvalue) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::BINARY;
        }
        virtual ~AstBinaryExpression() {
            if (this->lvalue) delete this->lvalue;
            if (this->rvalue) delete this->rvalue;
        }
    };

    class AstTernaryExpression : public kh::AstExpression {
    public:
        kh::AstExpression* condition;
        kh::AstExpression* value;
        kh::AstExpression* otherwise;

        AstTernaryExpression(const size_t _index, kh::AstExpression* _condition, kh::AstExpression* _value, kh::AstExpression* _otherwise) :
            condition(_condition), value(_value), otherwise(_otherwise) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::TERNARY;
        }
        virtual ~AstTernaryExpression() {
            if (this->condition) delete this->condition;
            if (this->value) delete this->value;
            if (this->otherwise) delete this->otherwise;
        }
    };

    class AstSubscriptExpression : public kh::AstExpression {
    public:
        kh::AstExpression* expression;
        std::vector<kh::AstExpression*> arguments;

        AstSubscriptExpression(const size_t _index, kh::AstExpression* _expression, const std::vector<kh::AstExpression*>& _arguments) :
            expression(_expression), arguments(_arguments) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::SUBSCRIPT;
        }
        virtual ~AstSubscriptExpression() {
            if (this->expression) delete this->expression;

            for (kh::AstExpression* argument : this->arguments)
                if (argument) delete argument;
        }
    };

    class AstCallExpression : public kh::AstExpression {
    public:
        kh::AstExpression* expression;
        std::vector<kh::AstExpression*> arguments;

        AstCallExpression(const size_t _index, kh::AstExpression* _expression, const std::vector<kh::AstExpression*>& _arguments) :
            expression(_expression), arguments(_arguments) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CALL;
        }
        virtual ~AstCallExpression() {
            if (this->expression) delete this->expression;

            for (kh::AstExpression* argument : this->arguments)
                if (argument) delete argument;
        }
    };

    class AstDeclarationExpression : public kh::AstExpression {
    public:
        kh::AstIdentifierExpression* var_type;
        std::u32string var_name;
        kh::AstExpression* expression;
        size_t ref_depth;
        bool is_static;
        bool is_public;

        AstDeclarationExpression(const size_t _index, kh::AstIdentifierExpression* _var_type, const std::u32string& _var_name, kh::AstExpression* _expression,
            const size_t _ref_depth, const bool _is_static, const bool _is_public) :
            var_type(_var_type), var_name(_var_name), expression(_expression),
            ref_depth(_ref_depth), is_static(_is_static), is_public(_is_public) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::DECLARE;
        }
        virtual ~AstDeclarationExpression() {
            if (this->var_type)
                delete this->var_type;

            if (this->expression)
                delete this->expression;
        }
    };

    class AstScopeExpression : public kh::AstExpression {
    public:
        kh::AstExpression* expression;
        std::vector<std::u32string> identifiers;

        AstScopeExpression(const size_t _index, kh::AstExpression* _expression, const std::vector<std::u32string>& _identifiers) :
            expression(_expression), identifiers(_identifiers) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::SCOPE;
        }
        virtual ~AstScopeExpression() {
            if (this->expression)
                delete this->expression;
        }
    };

    class AstConstValue : public kh::AstExpression {
    public:
        enum class ValueType {
            CHARACTER, UINTEGER, INTEGER, FLOATING, COMPLEX,
            BUFFER, STRING
        } value_type;

        union {
            char32_t character;
            uint64_t uinteger;
            int64_t integer;
            double floating;
        };

        std::complex<double> complex = 0;
        std::string buffer = "";
        std::u32string string = U"";

        AstConstValue(const size_t _index, const char32_t _character, const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::CHARACTER) :
            value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->character = _character;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(const size_t _index, const uint64_t _uinteger, const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::UINTEGER) :
            value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->uinteger = _uinteger;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(const size_t _index, const int64_t _integer, const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::INTEGER) :
            value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->integer = _integer;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(const size_t _index, const double _floating, const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::FLOATING) :
            value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->floating = _floating;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(const size_t _index, const std::complex<double> _complex, const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::COMPLEX) :
            value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->complex = _complex;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(const size_t _index, const std::string& _buffer, const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::BUFFER) :
            value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->buffer = _buffer;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(const size_t _index, const std::u32string& _string, const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::STRING) :
            value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->string = _string;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        virtual ~AstConstValue() {}
    };

    class AstTupleExpression : public kh::AstExpression {
    public:
        std::vector<kh::AstExpression*> elements;

        AstTupleExpression(const size_t _index, const std::vector<kh::AstExpression*>& _elements) :
            elements(_elements) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::TUPLE;
        }
        virtual ~AstTupleExpression() {
            for (kh::AstExpression* expression : this->elements)
                if (expression) delete expression;
        }
    };

    class AstIf : public kh::AstBody {
    public:
        std::vector<kh::AstExpression*> conditions; /* Including the else if conditions */
        std::vector<std::vector<kh::AstBody*>> bodies;
        std::vector<kh::AstBody*> else_body;

        AstIf(const size_t _index, const std::vector<kh::AstExpression*>& _conditions, const std::vector<std::vector<kh::AstBody*>>& _bodies, const std::vector<kh::AstBody*>& _else_body) :
            conditions(_conditions), bodies(_bodies), else_body(_else_body) {
            this->index = _index;
            this->type = kh::AstBody::Type::IF;
        }
        virtual ~AstIf() {
            for (kh::AstExpression* condition : this->conditions)
                if (condition) delete condition;

            for (std::vector<kh::AstBody*>& body : this->bodies)
                for (kh::AstBody* part : body)
                    if (part) delete part;

            for (kh::AstBody* part : this->else_body)
                if (part) delete part;
        }
    };

    class AstWhile : public kh::AstBody {
    public:
        kh::AstExpression* condition;
        std::vector<kh::AstBody*> body;

        AstWhile(const size_t _index, kh::AstExpression* _condition, const std::vector<kh::AstBody*>& _body) :
            condition(_condition), body(_body) {
            this->index = _index;
            this->type = kh::AstBody::Type::WHILE;
        }
        virtual ~AstWhile() {
            if (this->condition)
                delete this->condition;

            for (kh::AstBody* part : this->body)
                if (part) delete part;
        }
    };

    class AstDoWhile : public kh::AstBody {
    public:
        kh::AstExpression* condition;
        std::vector<kh::AstBody*> body;

        AstDoWhile(const size_t _index, kh::AstExpression* _condition, const std::vector<kh::AstBody*>& _body) :
            condition(_condition), body(_body) {
            this->index = _index;
            this->type = kh::AstBody::Type::DO_WHILE;
        }
        virtual ~AstDoWhile() {
            if (this->condition)
                delete this->condition;

            for (kh::AstBody* part : this->body)
                if (part) delete part;
        }
    };

    class AstFor : public kh::AstBody {
    public:
        std::vector<kh::AstExpression*> targets;
        kh::AstExpression* iterator;
        std::vector<kh::AstBody*> body;

        AstFor(const size_t _index, const std::vector<kh::AstExpression*>& _targets, kh::AstExpression* _iterator, const std::vector<kh::AstBody*>& _body) :
            targets(_targets), iterator(_iterator), body(_body) {
            this->index = _index;
            this->type = kh::AstBody::Type::FOR;
        }
        virtual ~AstFor() {
            for (kh::AstExpression* target : this->targets)
                if (target) delete target;

            if (this->iterator) delete this->iterator;

            for (kh::AstBody* part : this->body)
                if (part) delete part;
        }
    };

    class AstStatement : public kh::AstBody {
    public:
        enum class Type {
            CONTINUE, BREAK, RETURN
        } statement_type;
        kh::AstExpression* expression;

        AstStatement(const size_t _index, const kh::AstStatement::Type _statement_type, kh::AstExpression* _expression) :
            statement_type((Type)((size_t)_statement_type)), expression(expression) {
            this->index = _index;
            this->type = kh::AstBody::Type::STATEMENT;
        }
        virtual ~AstStatement() {
            if (this->expression)
                delete this->expression;
        }
    };

    class AstInstruction : public kh::AstBody {
    public:
        std::u32string op_name;
        std::vector<kh::AstExpression*> op_arguments;

        AstInstruction(const size_t _index, const std::u32string& _op_name, const std::vector<kh::AstExpression*>& _op_arguments) :
            op_name(_op_name), op_arguments(_op_arguments) {
            this->index = _index;
            this->type = kh::AstBody::Type::INSTRUCTION;
        }
        virtual ~AstInstruction() {
            for (kh::AstExpression* argument : this->op_arguments)
                if (argument) delete argument;
        }
    };
}
