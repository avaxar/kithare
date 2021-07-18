/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <memory>
#include <vector>

#include <kithare/token.hpp>


namespace kh {
    class AstModule;
    class AstImport;
    class AstUserType;
    class AstEnumType;

    class AstBody;
    class AstIf;
    class AstWhile;
    class AstDoWhile;
    class AstFor;
    class AstForEach;
    class AstStatement;

    class AstExpression;
    class AstIdentifiers;
    class AstDeclaration;
    class AstFunction;
    class AstUnaryOperation;
    class AstRevUnaryOperation;
    class AstBinaryOperation;
    class AstTernaryOperation;
    class AstComparisonOperation;
    class AstSubscriptExpression;
    class AstCallExpression;
    class AstScoping;
    class AstValue;
    class AstTuple;
    class AstList;
    class AstDict;

    std::u32string strfy(const AstModule& module_ast, size_t indent = 0);
    std::u32string strfy(const AstImport& import_ast, size_t indent = 0);
    std::u32string strfy(const AstUserType& type_ast, size_t indent = 0);
    std::u32string strfy(const AstEnumType& enum_ast, size_t indent = 0);
    std::u32string strfy(const AstBody& body_ast, size_t indent = 0);

    class AstModule {
    public:
        std::vector<AstImport> imports;
        std::vector<AstFunction> functions;
        std::vector<AstUserType> user_types;
        std::vector<AstEnumType> enums;
        std::vector<AstDeclaration> variables;

        AstModule(const std::vector<AstImport>& _imports, const std::vector<AstFunction>& _functions,
                  const std::vector<AstUserType>& _user_types, const std::vector<AstEnumType>& _enums,
                  const std::vector<AstDeclaration>& _variables);
    };

    class AstImport {
    public:
        size_t index;
        std::vector<std::string> path;
        bool is_include;
        bool is_relative;
        std::string identifier;

        bool is_public = true;

        AstImport(size_t _index, const std::vector<std::string>& _path, bool _is_include,
                  bool _is_relative, const std::string& _identifier);
    };

    class AstUserType {
    public:
        size_t index;
        std::vector<std::string> identifiers;
        std::shared_ptr<AstIdentifiers> base;
        std::vector<std::string> generic_args;
        std::vector<AstDeclaration> members;
        std::vector<AstFunction> methods;
        bool is_class;

        bool is_public = true;

        AstUserType(size_t _index, const std::vector<std::string>& _identifiers,
                    const std::shared_ptr<AstIdentifiers>& _base,
                    const std::vector<std::string>& _generic_args,
                    const std::vector<AstDeclaration>& _members,
                    const std::vector<AstFunction>& _methods, bool _is_class);
    };

    class AstEnumType {
    public:
        size_t index;
        std::vector<std::string> identifiers;
        std::vector<std::string> members;
        std::vector<uint64_t> values;

        bool is_public = true;

        AstEnumType(size_t _index, const std::vector<std::string>& _identifiers,
                    const std::vector<std::string>& _members, const std::vector<uint64_t>& _values);
    };

    class AstBody {
    public:
        size_t index;
        enum Type {
            NONE,
            EXPRESSION,
            IF,
            WHILE,
            DO_WHILE,
            FOR,
            FOREACH,
            STATEMENT
        } type = AstBody::NONE;

        virtual ~AstBody() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstExpression : public AstBody {
    public:
        enum ExType {
            NONE,
            IDENTIFIER,
            DECLARE,
            FUNCTION,
            UNARY,
            REV_UNARY,
            BINARY,
            TERNARY,
            COMPARISON,
            SUBSCRIPT,
            CALL,
            SCOPE,
            CONSTANT,
            TUPLE,
            LIST,
            DICT
        } expression_type = AstExpression::NONE;

        virtual ~AstExpression() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstIdentifiers : public AstExpression {
    public:
        std::vector<std::string> identifiers;
        std::vector<AstIdentifiers> generics;
        std::vector<size_t> generics_refs;
        std::vector<std::vector<uint64_t>> generics_array;

        AstIdentifiers(size_t _index, const std::vector<std::string>& _identifiers,
                       const std::vector<AstIdentifiers>& _generics,
                       const std::vector<size_t>& _generics_refs,
                       const std::vector<std::vector<uint64_t>>& _generics_array);
        virtual ~AstIdentifiers() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstDeclaration : public AstExpression {
    public:
        AstIdentifiers var_type;
        std::vector<uint64_t> var_array;
        std::string var_name;
        std::shared_ptr<AstExpression> expression;
        size_t refs;

        bool is_public = true;
        bool is_static = false;

        AstDeclaration(size_t _index, const AstIdentifiers& _var_type,
                       const std::vector<uint64_t>& _var_array, const std::string& _var_name,
                       std::shared_ptr<AstExpression>& _expression, size_t _refs);
        virtual ~AstDeclaration() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstFunction : public AstExpression {
    public:
        std::vector<std::string> identifiers;
        std::vector<std::string> generic_args;
        std::vector<uint64_t> id_array;

        AstIdentifiers return_type;
        std::vector<uint64_t> return_array;
        size_t return_refs;

        std::vector<AstDeclaration> arguments;
        std::vector<std::shared_ptr<AstBody>> body;
        bool is_conditional;

        bool is_public = true;
        bool is_static = false;

        AstFunction(size_t _index, const std::vector<std::string>& _identifiers,
                    const std::vector<std::string>& _generic_args,
                    const std::vector<uint64_t>& _id_array, const std::vector<uint64_t>& _return_array,
                    const AstIdentifiers& _return_type, size_t _return_refs,
                    const std::vector<AstDeclaration>& _arguments,
                    const std::vector<std::shared_ptr<AstBody>>& _body, bool _is_conditional);
        virtual ~AstFunction() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstUnaryOperation : public AstExpression {
    public:
        Operator operation;
        std::shared_ptr<AstExpression> rvalue;

        AstUnaryOperation(size_t _index, Operator _operation, std::shared_ptr<AstExpression>& _rvalue);
        virtual ~AstUnaryOperation() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstRevUnaryOperation : public AstExpression {
    public:
        Operator operation;
        std::shared_ptr<AstExpression> rvalue;

        AstRevUnaryOperation(size_t _index, Operator _operation,
                             std::shared_ptr<AstExpression>& _rvalue);
        virtual ~AstRevUnaryOperation() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstBinaryOperation : public AstExpression {
    public:
        Operator operation;
        std::shared_ptr<AstExpression> lvalue;
        std::shared_ptr<AstExpression> rvalue;

        AstBinaryOperation(size_t _index, Operator _operation, std::shared_ptr<AstExpression>& _lvalue,
                           std::shared_ptr<AstExpression>& _rvalue);
        virtual ~AstBinaryOperation() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstTernaryOperation : public AstExpression {
    public:
        std::shared_ptr<AstExpression> condition;
        std::shared_ptr<AstExpression> value;
        std::shared_ptr<AstExpression> otherwise;

        AstTernaryOperation(size_t _index, std::shared_ptr<AstExpression>& _condition,
                            std::shared_ptr<AstExpression>& _value,
                            std::shared_ptr<AstExpression>& _otherwise);
        virtual ~AstTernaryOperation() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstComparisonExpression : public AstExpression {
    public:
        std::vector<Operator> operations;
        std::vector<std::shared_ptr<AstExpression>> values;

        AstComparisonExpression(size_t _index, const std::vector<Operator>& _operations,
                                const std::vector<std::shared_ptr<AstExpression>>& _values);
        virtual ~AstComparisonExpression() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstSubscriptExpression : public AstExpression {
    public:
        std::shared_ptr<AstExpression> expression;
        std::vector<std::shared_ptr<AstExpression>> arguments;

        AstSubscriptExpression(size_t _index, std::shared_ptr<AstExpression>& _expression,
                               const std::vector<std::shared_ptr<AstExpression>>& _arguments);
        virtual ~AstSubscriptExpression() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstCallExpression : public AstExpression {
    public:
        std::shared_ptr<AstExpression> expression;
        std::vector<std::shared_ptr<AstExpression>> arguments;

        AstCallExpression(size_t _index, std::shared_ptr<AstExpression>& _expression,
                          const std::vector<std::shared_ptr<AstExpression>>& _arguments);
        virtual ~AstCallExpression() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstScoping : public AstExpression {
    public:
        std::shared_ptr<AstExpression> expression;
        std::vector<std::string> identifiers;

        AstScoping(size_t _index, std::shared_ptr<AstExpression>& _expression,
                   const std::vector<std::string>& _identifiers);
        virtual ~AstScoping() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstValue : public AstExpression {
    public:
        enum ValueType { CHARACTER, UINTEGER, INTEGER, FLOATING, IMAGINARY, BUFFER, STRING } value_type;

        union {
            char32_t character;
            uint64_t uinteger;
            int64_t integer;
            double floating;
            double imaginary;
        };

        std::string buffer = "";
        std::u32string string = U"";

        AstValue(size_t _index, char32_t _character,
                 AstValue::ValueType _value_type = AstValue::ValueType::CHARACTER);
        AstValue(size_t _index, uint64_t _uinteger,
                 AstValue::ValueType _value_type = AstValue::ValueType::UINTEGER);
        AstValue(size_t _index, int64_t _integer,
                 AstValue::ValueType _value_type = AstValue::ValueType::INTEGER);
        AstValue(size_t _index, double _floating,
                 AstValue::ValueType _value_type = AstValue::ValueType::FLOATING);
        AstValue(size_t _index, const std::string& _buffer,
                 AstValue::ValueType _value_type = AstValue::ValueType::BUFFER);
        AstValue(size_t _index, const std::u32string& _string,
                 AstValue::ValueType _value_type = AstValue::ValueType::STRING);
        virtual ~AstValue() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstTuple : public AstExpression {
    public:
        std::vector<std::shared_ptr<AstExpression>> elements;

        AstTuple(size_t _index, const std::vector<std::shared_ptr<AstExpression>>& _elements);
        virtual ~AstTuple() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstList : public AstExpression {
    public:
        std::vector<std::shared_ptr<AstExpression>> elements;

        AstList(size_t _index, const std::vector<std::shared_ptr<AstExpression>>& _elements);
        virtual ~AstList() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstDict : public AstExpression {
    public:
        std::vector<std::shared_ptr<AstExpression>> keys;
        std::vector<std::shared_ptr<AstExpression>> items;

        AstDict(size_t _index, const std::vector<std::shared_ptr<AstExpression>>& _keys,
                const std::vector<std::shared_ptr<AstExpression>>& _items);
        virtual ~AstDict() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstIf : public AstBody {
    public:
        std::vector<std::shared_ptr<AstExpression>> conditions; /* Including the else if conditions */
        std::vector<std::vector<std::shared_ptr<AstBody>>> bodies;
        std::vector<std::shared_ptr<AstBody>> else_body;

        AstIf(size_t _index, const std::vector<std::shared_ptr<AstExpression>>& _conditions,
              const std::vector<std::vector<std::shared_ptr<AstBody>>>& _bodies,
              const std::vector<std::shared_ptr<AstBody>>& _else_body);
        virtual ~AstIf() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstWhile : public AstBody {
    public:
        std::shared_ptr<AstExpression> condition;
        std::vector<std::shared_ptr<AstBody>> body;

        AstWhile(size_t _index, std::shared_ptr<AstExpression>& _condition,
                 const std::vector<std::shared_ptr<AstBody>>& _body);
        virtual ~AstWhile() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstDoWhile : public AstBody {
    public:
        std::shared_ptr<AstExpression> condition;
        std::vector<std::shared_ptr<AstBody>> body;

        AstDoWhile(size_t _index, std::shared_ptr<AstExpression>& _condition,
                   const std::vector<std::shared_ptr<AstBody>>& _body);
        virtual ~AstDoWhile() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstFor : public AstBody {
    public:
        std::shared_ptr<AstExpression> initialize;
        std::shared_ptr<AstExpression> condition;
        std::shared_ptr<AstExpression> step;
        std::vector<std::shared_ptr<AstBody>> body;

        AstFor(size_t _index, std::shared_ptr<AstExpression>& initialize,
               std::shared_ptr<AstExpression>& condition, std::shared_ptr<AstExpression>& step,
               const std::vector<std::shared_ptr<AstBody>>& _body);
        virtual ~AstFor() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstForEach : public AstBody {
    public:
        std::shared_ptr<AstExpression> target;
        std::shared_ptr<AstExpression> iterator;
        std::vector<std::shared_ptr<AstBody>> body;

        AstForEach(size_t _index, std::shared_ptr<AstExpression>& _target,
                   std::shared_ptr<AstExpression>& _iterator,
                   const std::vector<std::shared_ptr<AstBody>>& _body);
        virtual ~AstForEach() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };

    class AstStatement : public AstBody {
    public:
        enum class Type { CONTINUE, BREAK, RETURN } statement_type;

        std::shared_ptr<AstExpression> expression;
        size_t loop_count;

        AstStatement(size_t _index, AstStatement::Type _statement_type,
                     std::shared_ptr<AstExpression>& _expression);
        AstStatement(size_t _index, AstStatement::Type _statement_type, size_t _loop_count);
        virtual ~AstStatement() {}

        virtual std::u32string strfy(size_t indent = 0) const;
    };
}
