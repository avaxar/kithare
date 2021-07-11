/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <complex>
#include <memory>
#include <vector>

#include <kithare/string.hpp>
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

    std::u32string str(const kh::AstModule& module_ast, size_t indent = 0);
    std::u32string str(const kh::AstImport& import_ast, size_t indent = 0);
    std::u32string str(const kh::AstUserType& type_ast, size_t indent = 0);
    std::u32string str(const kh::AstEnumType& enum_ast, size_t indent = 0);
    std::u32string str(const kh::AstBody& body_ast, size_t indent = 0);

    class AstModule {
    public:
        std::vector<kh::AstImport> imports;
        std::vector<kh::AstFunction> functions;
        std::vector<kh::AstUserType> user_types;
        std::vector<kh::AstEnumType> enums;
        std::vector<kh::AstDeclaration> variables;

        AstModule(const std::vector<kh::AstImport>& _imports,
                  const std::vector<kh::AstFunction>& _functions,
                  const std::vector<kh::AstUserType>& _user_types,
                  const std::vector<kh::AstEnumType>& _enums,
                  const std::vector<kh::AstDeclaration>& _variables);
    };

    class AstImport {
    public:
        size_t index;
        std::vector<std::string> path;
        bool is_include;
        bool is_relative;
        std::string identifier;

        AstImport(size_t _index, const std::vector<std::string>& _path, bool _is_include,
                  bool _is_relative, const std::string& _identifier);
    };

    class AstUserType {
    public:
        size_t index;
        std::vector<std::string> identifiers;
        std::shared_ptr<kh::AstIdentifiers> base;
        std::vector<std::string> generic_args;
        std::vector<kh::AstDeclaration> members;
        std::vector<kh::AstFunction> methods;
        bool is_class;

        AstUserType(size_t _index, const std::vector<std::string>& _identifiers,
                    const std::shared_ptr<kh::AstIdentifiers>& _base,
                    const std::vector<std::string>& _generic_args,
                    const std::vector<kh::AstDeclaration>& _members,
                    const std::vector<kh::AstFunction>& _methods, bool _is_class);
    };

    class AstEnumType {
    public:
        size_t index;
        std::vector<std::string> identifiers;
        std::vector<std::string> members;
        std::vector<uint64_t> values;

        AstEnumType(size_t _index, const std::vector<std::string>& _identifiers,
                    const std::vector<std::string>& _members, const std::vector<uint64_t>& _values);
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
            FOREACH,
            STATEMENT
        } type = Type::NONE;

        virtual ~AstBody() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstExpression : public kh::AstBody {
    public:
        enum class ExType {
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
        } expression_type = ExType::NONE;

        virtual ~AstExpression() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstIdentifiers : public kh::AstExpression {
    public:
        std::vector<std::string> identifiers;
        std::vector<kh::AstIdentifiers> generics;
        std::vector<size_t> generics_refs;
        std::vector<std::vector<uint64_t>> generics_array;

        AstIdentifiers(size_t _index, const std::vector<std::string>& _identifiers,
                       const std::vector<kh::AstIdentifiers>& _generics,
                       const std::vector<size_t>& _generics_refs,
                       const std::vector<std::vector<uint64_t>>& _generics_array);
        virtual ~AstIdentifiers() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstDeclaration : public kh::AstExpression {
    public:
        kh::AstIdentifiers var_type;
        std::vector<uint64_t> var_array;
        std::string var_name;
        std::shared_ptr<kh::AstExpression> expression;
        size_t refs;
        bool is_static;
        bool is_public;

        AstDeclaration(size_t _index, const kh::AstIdentifiers& _var_type,
                       const std::vector<uint64_t>& _var_array, const std::string& _var_name,
                       std::shared_ptr<kh::AstExpression>& _expression, size_t _refs, bool _is_static,
                       bool _is_public);
        virtual ~AstDeclaration() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstFunction : public kh::AstExpression {
    public:
        std::vector<std::string> identifiers;
        std::vector<std::string> generic_args;
        std::vector<uint64_t> id_array;

        kh::AstIdentifiers return_type;
        std::vector<uint64_t> return_array;
        size_t return_refs;

        std::vector<kh::AstDeclaration> arguments;
        std::vector<std::shared_ptr<kh::AstBody>> body;
        bool is_conditional;
        bool is_static;
        bool is_public;

        AstFunction(size_t _index, const std::vector<std::string>& _identifiers,
                    const std::vector<std::string>& _generic_args,
                    const std::vector<uint64_t>& _id_array, const std::vector<uint64_t>& _return_array,
                    const kh::AstIdentifiers& _return_type, size_t _return_refs,
                    const std::vector<kh::AstDeclaration>& _arguments,
                    const std::vector<std::shared_ptr<kh::AstBody>>& _body, bool _is_conditional,
                    bool _is_static, bool _is_public);
        virtual ~AstFunction() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstUnaryOperation : public kh::AstExpression {
    public:
        kh::Operator operation;
        std::shared_ptr<kh::AstExpression> rvalue;

        AstUnaryOperation(size_t _index, kh::Operator _operation,
                          std::shared_ptr<kh::AstExpression>& _rvalue);
        virtual ~AstUnaryOperation() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstRevUnaryOperation : public kh::AstExpression {
    public:
        kh::Operator operation;
        std::shared_ptr<kh::AstExpression> rvalue;

        AstRevUnaryOperation(size_t _index, kh::Operator _operation,
                             std::shared_ptr<kh::AstExpression>& _rvalue);
        virtual ~AstRevUnaryOperation() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstBinaryOperation : public kh::AstExpression {
    public:
        kh::Operator operation;
        std::shared_ptr<kh::AstExpression> lvalue;
        std::shared_ptr<kh::AstExpression> rvalue;

        AstBinaryOperation(size_t _index, kh::Operator _operation,
                           std::shared_ptr<kh::AstExpression>& _lvalue,
                           std::shared_ptr<kh::AstExpression>& _rvalue);
        virtual ~AstBinaryOperation() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstTernaryOperation : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstExpression> condition;
        std::shared_ptr<kh::AstExpression> value;
        std::shared_ptr<kh::AstExpression> otherwise;

        AstTernaryOperation(size_t _index, std::shared_ptr<kh::AstExpression>& _condition,
                            std::shared_ptr<kh::AstExpression>& _value,
                            std::shared_ptr<kh::AstExpression>& _otherwise);
        virtual ~AstTernaryOperation() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstComparisonExpression : public kh::AstExpression {
    public:
        std::vector<kh::Operator> operations;
        std::vector<std::shared_ptr<kh::AstExpression>> values;

        AstComparisonExpression(size_t _index, const std::vector<kh::Operator>& _operations,
                                const std::vector<std::shared_ptr<kh::AstExpression>>& _values);
        virtual ~AstComparisonExpression() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstSubscriptExpression : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstExpression> expression;
        std::vector<std::shared_ptr<kh::AstExpression>> arguments;

        AstSubscriptExpression(size_t _index, std::shared_ptr<kh::AstExpression>& _expression,
                               const std::vector<std::shared_ptr<kh::AstExpression>>& _arguments);
        virtual ~AstSubscriptExpression() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstCallExpression : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstExpression> expression;
        std::vector<std::shared_ptr<kh::AstExpression>> arguments;

        AstCallExpression(size_t _index, std::shared_ptr<kh::AstExpression>& _expression,
                          const std::vector<std::shared_ptr<kh::AstExpression>>& _arguments);
        virtual ~AstCallExpression() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstScoping : public kh::AstExpression {
    public:
        std::shared_ptr<kh::AstExpression> expression;
        std::vector<std::string> identifiers;

        AstScoping(size_t _index, std::shared_ptr<kh::AstExpression>& _expression,
                   const std::vector<std::string>& _identifiers);
        virtual ~AstScoping() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstValue : public kh::AstExpression {
    public:
        enum class ValueType {
            CHARACTER,
            UINTEGER,
            INTEGER,
            FLOATING,
            IMAGINARY,
            BUFFER,
            STRING
        } value_type;

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
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::CHARACTER);
        AstValue(size_t _index, uint64_t _uinteger,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::UINTEGER);
        AstValue(size_t _index, int64_t _integer,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::INTEGER);
        AstValue(size_t _index, double _floating,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::FLOATING);
        AstValue(size_t _index, const std::string& _buffer,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::BUFFER);
        AstValue(size_t _index, const std::u32string& _string,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::STRING);
        virtual ~AstValue() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstTuple : public kh::AstExpression {
    public:
        std::vector<std::shared_ptr<kh::AstExpression>> elements;

        AstTuple(size_t _index, const std::vector<std::shared_ptr<kh::AstExpression>>& _elements);
        virtual ~AstTuple() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstList : public kh::AstExpression {
    public:
        std::vector<std::shared_ptr<kh::AstExpression>> elements;

        AstList(size_t _index, const std::vector<std::shared_ptr<kh::AstExpression>>& _elements);
        virtual ~AstList() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstDict : public kh::AstExpression {
    public:
        std::vector<std::shared_ptr<kh::AstExpression>> keys;
        std::vector<std::shared_ptr<kh::AstExpression>> items;

        AstDict(size_t _index, const std::vector<std::shared_ptr<kh::AstExpression>>& _keys,
                const std::vector<std::shared_ptr<kh::AstExpression>>& _items);
        virtual ~AstDict() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstIf : public kh::AstBody {
    public:
        std::vector<std::shared_ptr<kh::AstExpression>>
            conditions; /* Including the else if conditions */
        std::vector<std::vector<std::shared_ptr<kh::AstBody>>> bodies;
        std::vector<std::shared_ptr<kh::AstBody>> else_body;

        AstIf(size_t _index, const std::vector<std::shared_ptr<kh::AstExpression>>& _conditions,
              const std::vector<std::vector<std::shared_ptr<kh::AstBody>>>& _bodies,
              const std::vector<std::shared_ptr<kh::AstBody>>& _else_body);
        virtual ~AstIf() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstWhile : public kh::AstBody {
    public:
        std::shared_ptr<kh::AstExpression> condition;
        std::vector<std::shared_ptr<kh::AstBody>> body;

        AstWhile(size_t _index, std::shared_ptr<kh::AstExpression>& _condition,
                 const std::vector<std::shared_ptr<kh::AstBody>>& _body);
        virtual ~AstWhile() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstDoWhile : public kh::AstBody {
    public:
        std::shared_ptr<kh::AstExpression> condition;
        std::vector<std::shared_ptr<kh::AstBody>> body;

        AstDoWhile(size_t _index, std::shared_ptr<kh::AstExpression>& _condition,
                   const std::vector<std::shared_ptr<kh::AstBody>>& _body);
        virtual ~AstDoWhile() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstFor : public kh::AstBody {
    public:
        std::shared_ptr<kh::AstExpression> initialize;
        std::shared_ptr<kh::AstExpression> condition;
        std::shared_ptr<kh::AstExpression> step;
        std::vector<std::shared_ptr<kh::AstBody>> body;

        AstFor(size_t _index, std::shared_ptr<kh::AstExpression>& initialize,
               std::shared_ptr<kh::AstExpression>& condition, std::shared_ptr<kh::AstExpression>& step,
               const std::vector<std::shared_ptr<kh::AstBody>>& _body);
        virtual ~AstFor() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstForEach : public kh::AstBody {
    public:
        std::shared_ptr<kh::AstExpression> target;
        std::shared_ptr<kh::AstExpression> iterator;
        std::vector<std::shared_ptr<kh::AstBody>> body;

        AstForEach(size_t _index, std::shared_ptr<kh::AstExpression>& _target,
                   std::shared_ptr<kh::AstExpression>& _iterator,
                   const std::vector<std::shared_ptr<kh::AstBody>>& _body);
        virtual ~AstForEach() {}

        virtual std::u32string str(size_t indent = 0) const;
    };

    class AstStatement : public kh::AstBody {
    public:
        enum class Type { CONTINUE, BREAK, RETURN } statement_type;

        std::shared_ptr<kh::AstExpression> expression;
        size_t loop_count;

        AstStatement(size_t _index, kh::AstStatement::Type _statement_type,
                     std::shared_ptr<kh::AstExpression>& _expression);
        AstStatement(size_t _index, kh::AstStatement::Type _statement_type, size_t _loop_count);
        virtual ~AstStatement() {}

        virtual std::u32string str(size_t indent = 0) const;
    };
}
