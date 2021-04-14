/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/parser/ast.hpp
 * Declares AST node types and their kh::repr overloads.
 */

#pragma once

#include <complex>
#include <memory>
#include <vector>

#include "parser/token.hpp"
#include "utility/string.hpp"


namespace kh {
    class Ast;

    class AstImport;
    class AstClass;
    class AstStruct;
    class AstEnum;

    class AstBody;
    class AstIf;
    class AstWhile;
    class AstDoWhile;
    class AstFor;
    class AstStatement;

    class AstExpression;
    class AstIdentifierExpression;
    class AstUnaryExpression;
    class AstBinaryExpression;
    class AstTrinaryExpression;
    class AstSubscriptExpression;
    class AstCallExpression;
    class AstDeclarationExpression;
    class AstFunctionExpression;
    class AstScopeExpression;
    class AstConstValue;
    class AstTupleExpression;

    std::u32string repr(const kh::Ast& module_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstImport& import_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstClass& class_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstStruct& struct_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstEnum& enum_ast, const size_t indent = 0);
    std::u32string repr(const kh::AstBody& ast, const size_t indent = 0);
    std::u32string repr(const kh::AstExpression& expr, const size_t indent = 0);

    class Ast {
    public:
        std::vector<std::shared_ptr<kh::AstImport>> imports;
        std::vector<std::shared_ptr<kh::AstFunctionExpression>> functions;
        std::vector<std::shared_ptr<kh::AstClass>> classes;
        std::vector<std::shared_ptr<kh::AstStruct>> structs;
        std::vector<std::shared_ptr<kh::AstEnum>> enums;
        std::vector<std::shared_ptr<kh::AstDeclarationExpression>> variables;

        Ast(const std::vector<std::shared_ptr<kh::AstImport>>& _imports,
            const std::vector<std::shared_ptr<kh::AstFunctionExpression>>& _functions,
            const std::vector<std::shared_ptr<kh::AstClass>>& _classes,
            const std::vector<std::shared_ptr<kh::AstStruct>>& _structs,
            const std::vector<std::shared_ptr<kh::AstEnum>>& _enums,
            const std::vector<std::shared_ptr<kh::AstDeclarationExpression>>& _variables)
            : variables(_variables), imports(_imports), functions(_functions), classes(_classes),
              structs(_structs), enums(_enums) {}
        virtual ~Ast() {}
    };

    class AstImport {
    public:
        size_t index;
        std::vector<std::u32string> path;
        bool is_include;
        std::u32string identifier;

        AstImport(const size_t _index, const std::vector<std::u32string>& _path, const bool _is_include,
                  const std::u32string& _identifier)
            : index(_index), path(_path), is_include(_is_include), identifier(_identifier) {}
        virtual ~AstImport() {}
    };

    class AstClass {
    public:
        size_t index;
        std::u32string name;
        std::shared_ptr<kh::AstIdentifierExpression> base;
        std::vector<std::u32string> generic_args;
        std::vector<std::shared_ptr<kh::AstDeclarationExpression>> members;
        std::vector<std::shared_ptr<kh::AstFunctionExpression>> methods;

        AstClass(const size_t _index, const std::u32string& _name,
                 std::shared_ptr<kh::AstIdentifierExpression>& _base,
                 const std::vector<std::u32string>& _generic_args,
                 const std::vector<std::shared_ptr<kh::AstDeclarationExpression>>& _members,
                 const std::vector<std::shared_ptr<kh::AstFunctionExpression>>& _methods)
            : index(_index), name(_name), base(_base), generic_args(_generic_args), members(_members),
              methods(_methods) {}
        virtual ~AstClass() {}
    };

    class AstStruct {
    public:
        size_t index;
        std::u32string name;
        std::shared_ptr<kh::AstIdentifierExpression> base;
        std::vector<std::shared_ptr<kh::AstDeclarationExpression>> members;

        AstStruct(const size_t _index, const std::u32string& _name,
                  std::shared_ptr<kh::AstIdentifierExpression>& _base,
                  const std::vector<std::shared_ptr<kh::AstDeclarationExpression>>& _members)
            : index(_index), name(_name), base(_base), members(_members) {}
        virtual ~AstStruct() {}
    };

    class AstEnum {
    public:
        size_t index;
        std::u32string name;
        std::shared_ptr<kh::AstIdentifierExpression> base;
        std::vector<std::u32string> members;

        AstEnum(const size_t _index, const std::u32string& _name,
                std::shared_ptr<kh::AstIdentifierExpression>& _base,
                const std::vector<std::u32string>& _members)
            : index(_index), name(_name), base(_base), members(_members) {}
        virtual ~AstEnum() {}
    };

    class AstBody {
    public:
        size_t index;
        enum class Type {
            NONE,
            EXPRESSION,
            IF,
            WHILE,
            DO_WHILE,
            FOR,
            STATEMENT,
            INSTRUCTION
        } type = Type::NONE;

        virtual ~AstBody() {}
    };

    class AstExpression : public kh::AstBody {
    public:
        enum class ExType {
            NONE,
            IDENTIFIER,
            UNARY,
            BINARY,
            TERNARY,
            SUBSCRIPT,
            CALL,
            DECLARE,
            FUNCTION,
            SCOPE,
            CONSTANT,
            TUPLE
        } expression_type = ExType::NONE;

        virtual ~AstExpression() {}
    };

    class AstIdentifierExpression : public kh::AstExpression {
    public:
        std::vector<std::u32string> identifiers;
        std::vector<std::shared_ptr<kh::AstIdentifierExpression>> generics;

        AstIdentifierExpression(
            const size_t _index, const std::vector<std::u32string>& _identifiers,
            const std::vector<std::shared_ptr<kh::AstIdentifierExpression>>& _generics)
            : identifiers(_identifiers), generics(_generics) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::IDENTIFIER;
        }
        virtual ~AstIdentifierExpression() {}
    };

    class AstUnaryExpression : public kh::AstExpression {
    public:
        kh::Operator operation;
        std::shared_ptr<kh::AstExpression> rvalue;

        AstUnaryExpression(const size_t _index, const kh::Operator _operation,
                           std::shared_ptr<kh::AstExpression>& _rvalue)
            : operation(_operation), rvalue(_rvalue) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::UNARY;
        }
        virtual ~AstUnaryExpression() {}
    };

    class AstBinaryExpression : public kh::AstExpression {
    public:
        kh::Operator operation;
        std::shared_ptr<kh::AstExpression> lvalue;
        std::shared_ptr<kh::AstExpression> rvalue;

        AstBinaryExpression(const size_t _index, const kh::Operator _operation,
                            std::shared_ptr<kh::AstExpression>& _lvalue,
                            std::shared_ptr<kh::AstExpression>& _rvalue)
            : operation(_operation), lvalue(_lvalue), rvalue(_rvalue) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::BINARY;
        }
        virtual ~AstBinaryExpression() {}
    };

    class AstTernaryExpression : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstExpression> condition;
        std::shared_ptr<kh::AstExpression> value;
        std::shared_ptr<kh::AstExpression> otherwise;

        AstTernaryExpression(const size_t _index, std::shared_ptr<kh::AstExpression>& _condition,
                             std::shared_ptr<kh::AstExpression>& _value,
                             std::shared_ptr<kh::AstExpression>& _otherwise)
            : condition(_condition), value(_value), otherwise(_otherwise) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::TERNARY;
        }
        virtual ~AstTernaryExpression() {}
    };

    class AstSubscriptExpression : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstExpression> expression;
        std::vector<std::shared_ptr<kh::AstExpression>> arguments;

        AstSubscriptExpression(const size_t _index, std::shared_ptr<kh::AstExpression>& _expression,
                               const std::vector<std::shared_ptr<kh::AstExpression>>& _arguments)
            : expression(_expression), arguments(_arguments) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::SUBSCRIPT;
        }
        virtual ~AstSubscriptExpression() {}
    };

    class AstCallExpression : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstExpression> expression;
        std::vector<std::shared_ptr<kh::AstExpression>> arguments;

        AstCallExpression(const size_t _index, std::shared_ptr<kh::AstExpression>& _expression,
                          const std::vector<std::shared_ptr<kh::AstExpression>>& _arguments)
            : expression(_expression), arguments(_arguments) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CALL;
        }
        virtual ~AstCallExpression() {}
    };

    class AstDeclarationExpression : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstIdentifierExpression> var_type;
        std::u32string var_name;
        std::shared_ptr<kh::AstExpression> expression;
        size_t ref_depth;
        bool is_static;
        bool is_public;

        AstDeclarationExpression(const size_t _index,
                                 std::shared_ptr<kh::AstIdentifierExpression>& _var_type,
                                 const std::u32string& _var_name,
                                 std::shared_ptr<kh::AstExpression>& _expression,
                                 const size_t _ref_depth, const bool _is_static, const bool _is_public)
            : var_type(_var_type), var_name(_var_name), expression(_expression), ref_depth(_ref_depth),
              is_static(_is_static), is_public(_is_public) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::DECLARE;
        }
        virtual ~AstDeclarationExpression() {}
    };

    class AstFunctionExpression : public kh::AstExpression {
    public:
        size_t index;
        std::vector<std::u32string> identifiers;
        std::vector<std::u32string> generic_args;
        std::shared_ptr<kh::AstIdentifierExpression> return_type;
        size_t return_ref_depth;
        std::vector<std::shared_ptr<kh::AstDeclarationExpression>> arguments;
        std::vector<std::shared_ptr<kh::AstBody>> body;
        bool is_static;
        bool is_public;

        AstFunctionExpression(
            const size_t _index, const std::vector<std::u32string>& _identifiers,
            const std::vector<std::u32string>& _generic_args,
            std::shared_ptr<kh::AstIdentifierExpression>& _return_type, const size_t _return_ref_depth,
            const std::vector<std::shared_ptr<kh::AstDeclarationExpression>>& _arguments,
            const std::vector<std::shared_ptr<kh::AstBody>>& _body, const bool _is_static,
            const bool _is_public)
            : index(_index), identifiers(_identifiers), generic_args(_generic_args),
              return_type(_return_type), return_ref_depth(_return_ref_depth), arguments(_arguments),
              body(_body), is_static(_is_static), is_public(_is_public) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::FUNCTION;
        }
        virtual ~AstFunctionExpression() {}
    };

    class AstScopeExpression : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstExpression> expression;
        std::vector<std::u32string> identifiers;

        AstScopeExpression(const size_t _index, std::shared_ptr<kh::AstExpression>& _expression,
                           const std::vector<std::u32string>& _identifiers)
            : expression(_expression), identifiers(_identifiers) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::SCOPE;
        }
        virtual ~AstScopeExpression() {}
    };

    class AstConstValue : public kh::AstExpression {
    public:
        enum class ValueType {
            CHARACTER,
            UINTEGER,
            INTEGER,
            FLOATING,
            COMPLEX,
            BUFFER,
            STRING
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

        AstConstValue(
            const size_t _index, const char32_t _character,
            const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::CHARACTER)
            : value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->character = _character;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(
            const size_t _index, const uint64_t _uinteger,
            const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::UINTEGER)
            : value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->uinteger = _uinteger;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(
            const size_t _index, const int64_t _integer,
            const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::INTEGER)
            : value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->integer = _integer;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(
            const size_t _index, const double _floating,
            const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::FLOATING)
            : value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->floating = _floating;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(
            const size_t _index, const std::complex<double> _complex,
            const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::COMPLEX)
            : value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->complex = _complex;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(
            const size_t _index, const std::string& _buffer,
            const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::BUFFER)
            : value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->buffer = _buffer;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        AstConstValue(
            const size_t _index, const std::u32string& _string,
            const kh::AstConstValue::ValueType _value_type = kh::AstConstValue::ValueType::STRING)
            : value_type((ValueType)((size_t)_value_type)) {
            this->index = _index;
            this->string = _string;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::CONSTANT;
        }
        virtual ~AstConstValue() {}
    };

    class AstTupleExpression : public kh::AstExpression {
    public:
        std::vector<std::shared_ptr<kh::AstExpression>> elements;

        AstTupleExpression(const size_t _index,
                           const std::vector<std::shared_ptr<kh::AstExpression>>& _elements)
            : elements(_elements) {
            this->index = _index;
            this->type = kh::AstBody::Type::EXPRESSION;
            this->expression_type = kh::AstExpression::ExType::TUPLE;
        }
        virtual ~AstTupleExpression() {}
    };

    class AstIf : public kh::AstBody {
    public:
        std::vector<std::shared_ptr<kh::AstExpression>>
            conditions; /* Including the else if conditions */
        std::vector<std::vector<std::shared_ptr<kh::AstBody>>> bodies;
        std::vector<std::shared_ptr<kh::AstBody>> else_body;

        AstIf(const size_t _index, const std::vector<std::shared_ptr<kh::AstExpression>>& _conditions,
              const std::vector<std::vector<std::shared_ptr<kh::AstBody>>>& _bodies,
              const std::vector<std::shared_ptr<kh::AstBody>>& _else_body)
            : conditions(_conditions), bodies(_bodies), else_body(_else_body) {
            this->index = _index;
            this->type = kh::AstBody::Type::IF;
        }
        virtual ~AstIf() {}
    };

    class AstWhile : public kh::AstBody {
    public:
        std::shared_ptr<kh::AstExpression> condition;
        std::vector<std::shared_ptr<kh::AstBody>> body;

        AstWhile(const size_t _index, std::shared_ptr<kh::AstExpression>& _condition,
                 const std::vector<std::shared_ptr<kh::AstBody>>& _body)
            : condition(_condition), body(_body) {
            this->index = _index;
            this->type = kh::AstBody::Type::WHILE;
        }
        virtual ~AstWhile() {}
    };

    class AstDoWhile : public kh::AstBody {
    public:
        std::shared_ptr<kh::AstExpression> condition;
        std::vector<std::shared_ptr<kh::AstBody>> body;

        AstDoWhile(const size_t _index, std::shared_ptr<kh::AstExpression>& _condition,
                   const std::vector<std::shared_ptr<kh::AstBody>>& _body)
            : condition(_condition), body(_body) {
            this->index = _index;
            this->type = kh::AstBody::Type::DO_WHILE;
        }
        virtual ~AstDoWhile() {}
    };

    class AstFor : public kh::AstBody {
    public:
        std::vector<std::shared_ptr<kh::AstExpression>> targets;
        std::shared_ptr<kh::AstExpression> iterator;
        std::vector<std::shared_ptr<kh::AstBody>> body;

        AstFor(const size_t _index, const std::vector<std::shared_ptr<kh::AstExpression>>& _targets,
               std::shared_ptr<kh::AstExpression>& _iterator,
               const std::vector<std::shared_ptr<kh::AstBody>>& _body)
            : targets(_targets), iterator(_iterator), body(_body) {
            this->index = _index;
            this->type = kh::AstBody::Type::FOR;
        }
        virtual ~AstFor() {}
    };

    class AstStatement : public kh::AstBody {
    public:
        enum class Type { CONTINUE, BREAK, RETURN } statement_type;
        std::shared_ptr<kh::AstExpression> expression;

        AstStatement(const size_t _index, const kh::AstStatement::Type _statement_type,
                     std::shared_ptr<kh::AstExpression>& _expression)
            : statement_type((Type)((size_t)_statement_type)), expression(_expression) {
            this->index = _index;
            this->type = kh::AstBody::Type::STATEMENT;
        }
        virtual ~AstStatement() {}
    };

    class AstInstruction : public kh::AstBody {
    public:
        std::u32string op_name;
        std::vector<std::shared_ptr<kh::AstExpression>> op_arguments;

        AstInstruction(const size_t _index, const std::u32string& _op_name,
                       const std::vector<std::shared_ptr<kh::AstExpression>>& _op_arguments)
            : op_name(_op_name), op_arguments(_op_arguments) {
            this->index = _index;
            this->type = kh::AstBody::Type::INSTRUCTION;
        }
        virtual ~AstInstruction() {}
    };
} // namespace kh
