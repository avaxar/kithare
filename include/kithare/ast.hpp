/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <memory>
#include <vector>

#include <kithare/string.hpp>
#include <kithare/token.hpp>


namespace kh {
    using namespace std;

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

    u32string str(const kh::AstModule& module_ast, size_t indent = 0);
    u32string str(const kh::AstImport& import_ast, size_t indent = 0);
    u32string str(const kh::AstUserType& type_ast, size_t indent = 0);
    u32string str(const kh::AstEnumType& enum_ast, size_t indent = 0);
    u32string str(const kh::AstBody& body_ast, size_t indent = 0);

    class AstModule {
    public:
        vector<kh::AstImport> imports;
        vector<kh::AstFunction> functions;
        vector<kh::AstUserType> user_types;
        vector<kh::AstEnumType> enums;
        vector<kh::AstDeclaration> variables;

        AstModule(const vector<kh::AstImport>& _imports, const vector<kh::AstFunction>& _functions,
                  const vector<kh::AstUserType>& _user_types, const vector<kh::AstEnumType>& _enums,
                  const vector<kh::AstDeclaration>& _variables);
    };

    class AstImport {
    public:
        size_t index;
        vector<string> path;
        bool is_include;
        bool is_relative;
        string identifier;

        bool is_public = true;

        AstImport(size_t _index, const vector<string>& _path, bool _is_include, bool _is_relative,
                  const string& _identifier);
    };

    class AstUserType {
    public:
        size_t index;
        vector<string> identifiers;
        shared_ptr<kh::AstIdentifiers> base;
        vector<string> generic_args;
        vector<kh::AstDeclaration> members;
        vector<kh::AstFunction> methods;
        bool is_class;

        bool is_public = true;

        AstUserType(size_t _index, const vector<string>& _identifiers,
                    const shared_ptr<kh::AstIdentifiers>& _base, const vector<string>& _generic_args,
                    const vector<kh::AstDeclaration>& _members, const vector<kh::AstFunction>& _methods,
                    bool _is_class);
    };

    class AstEnumType {
    public:
        size_t index;
        vector<string> identifiers;
        vector<string> members;
        vector<uint64_t> values;

        bool is_public = true;

        AstEnumType(size_t _index, const vector<string>& _identifiers, const vector<string>& _members,
                    const vector<uint64_t>& _values);
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
        } type = kh::AstBody::NONE;

        virtual ~AstBody() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstExpression : public kh::AstBody {
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
        } expression_type = kh::AstExpression::NONE;

        virtual ~AstExpression() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstIdentifiers : public kh::AstExpression {
    public:
        vector<string> identifiers;
        vector<kh::AstIdentifiers> generics;
        vector<size_t> generics_refs;
        vector<vector<uint64_t>> generics_array;

        AstIdentifiers(size_t _index, const vector<string>& _identifiers,
                       const vector<kh::AstIdentifiers>& _generics,
                       const vector<size_t>& _generics_refs,
                       const vector<vector<uint64_t>>& _generics_array);
        virtual ~AstIdentifiers() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstDeclaration : public kh::AstExpression {
    public:
        kh::AstIdentifiers var_type;
        vector<uint64_t> var_array;
        string var_name;
        shared_ptr<kh::AstExpression> expression;
        size_t refs;

        bool is_public = true;
        bool is_static = false;

        AstDeclaration(size_t _index, const kh::AstIdentifiers& _var_type,
                       const vector<uint64_t>& _var_array, const string& _var_name,
                       shared_ptr<kh::AstExpression>& _expression, size_t _refs);
        virtual ~AstDeclaration() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstFunction : public kh::AstExpression {
    public:
        vector<string> identifiers;
        vector<string> generic_args;
        vector<uint64_t> id_array;

        kh::AstIdentifiers return_type;
        vector<uint64_t> return_array;
        size_t return_refs;

        vector<kh::AstDeclaration> arguments;
        vector<shared_ptr<kh::AstBody>> body;
        bool is_conditional;

        bool is_public = true;
        bool is_static = false;

        AstFunction(size_t _index, const vector<string>& _identifiers,
                    const vector<string>& _generic_args, const vector<uint64_t>& _id_array,
                    const vector<uint64_t>& _return_array, const kh::AstIdentifiers& _return_type,
                    size_t _return_refs, const vector<kh::AstDeclaration>& _arguments,
                    const vector<shared_ptr<kh::AstBody>>& _body, bool _is_conditional);
        virtual ~AstFunction() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstUnaryOperation : public kh::AstExpression {
    public:
        kh::Operator operation;
        shared_ptr<kh::AstExpression> rvalue;

        AstUnaryOperation(size_t _index, kh::Operator _operation,
                          shared_ptr<kh::AstExpression>& _rvalue);
        virtual ~AstUnaryOperation() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstRevUnaryOperation : public kh::AstExpression {
    public:
        kh::Operator operation;
        shared_ptr<kh::AstExpression> rvalue;

        AstRevUnaryOperation(size_t _index, kh::Operator _operation,
                             shared_ptr<kh::AstExpression>& _rvalue);
        virtual ~AstRevUnaryOperation() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstBinaryOperation : public kh::AstExpression {
    public:
        kh::Operator operation;
        shared_ptr<kh::AstExpression> lvalue;
        shared_ptr<kh::AstExpression> rvalue;

        AstBinaryOperation(size_t _index, kh::Operator _operation,
                           shared_ptr<kh::AstExpression>& _lvalue,
                           shared_ptr<kh::AstExpression>& _rvalue);
        virtual ~AstBinaryOperation() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstTernaryOperation : public kh::AstExpression {
    public:
        shared_ptr<kh::AstExpression> condition;
        shared_ptr<kh::AstExpression> value;
        shared_ptr<kh::AstExpression> otherwise;

        AstTernaryOperation(size_t _index, shared_ptr<kh::AstExpression>& _condition,
                            shared_ptr<kh::AstExpression>& _value,
                            shared_ptr<kh::AstExpression>& _otherwise);
        virtual ~AstTernaryOperation() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstComparisonExpression : public kh::AstExpression {
    public:
        vector<kh::Operator> operations;
        vector<shared_ptr<kh::AstExpression>> values;

        AstComparisonExpression(size_t _index, const vector<kh::Operator>& _operations,
                                const vector<shared_ptr<kh::AstExpression>>& _values);
        virtual ~AstComparisonExpression() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstSubscriptExpression : public kh::AstExpression {
    public:
        shared_ptr<kh::AstExpression> expression;
        vector<shared_ptr<kh::AstExpression>> arguments;

        AstSubscriptExpression(size_t _index, shared_ptr<kh::AstExpression>& _expression,
                               const vector<shared_ptr<kh::AstExpression>>& _arguments);
        virtual ~AstSubscriptExpression() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstCallExpression : public kh::AstExpression {
    public:
        shared_ptr<kh::AstExpression> expression;
        vector<shared_ptr<kh::AstExpression>> arguments;

        AstCallExpression(size_t _index, shared_ptr<kh::AstExpression>& _expression,
                          const vector<shared_ptr<kh::AstExpression>>& _arguments);
        virtual ~AstCallExpression() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstScoping : public kh::AstExpression {
    public:
        shared_ptr<kh::AstExpression> expression;
        vector<string> identifiers;

        AstScoping(size_t _index, shared_ptr<kh::AstExpression>& _expression,
                   const vector<string>& _identifiers);
        virtual ~AstScoping() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstValue : public kh::AstExpression {
    public:
        enum ValueType { CHARACTER, UINTEGER, INTEGER, FLOATING, IMAGINARY, BUFFER, STRING } value_type;

        union {
            char32_t character;
            uint64_t uinteger;
            int64_t integer;
            double floating;
            double imaginary;
        };

        string buffer = "";
        u32string ustring = U"";

        AstValue(size_t _index, char32_t _character,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::CHARACTER);
        AstValue(size_t _index, uint64_t _uinteger,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::UINTEGER);
        AstValue(size_t _index, int64_t _integer,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::INTEGER);
        AstValue(size_t _index, double _floating,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::FLOATING);
        AstValue(size_t _index, const string& _buffer,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::BUFFER);
        AstValue(size_t _index, const u32string& _ustring,
                 kh::AstValue::ValueType _value_type = kh::AstValue::ValueType::STRING);
        virtual ~AstValue() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstTuple : public kh::AstExpression {
    public:
        vector<shared_ptr<kh::AstExpression>> elements;

        AstTuple(size_t _index, const vector<shared_ptr<kh::AstExpression>>& _elements);
        virtual ~AstTuple() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstList : public kh::AstExpression {
    public:
        vector<shared_ptr<kh::AstExpression>> elements;

        AstList(size_t _index, const vector<shared_ptr<kh::AstExpression>>& _elements);
        virtual ~AstList() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstDict : public kh::AstExpression {
    public:
        vector<shared_ptr<kh::AstExpression>> keys;
        vector<shared_ptr<kh::AstExpression>> items;

        AstDict(size_t _index, const vector<shared_ptr<kh::AstExpression>>& _keys,
                const vector<shared_ptr<kh::AstExpression>>& _items);
        virtual ~AstDict() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstIf : public kh::AstBody {
    public:
        vector<shared_ptr<kh::AstExpression>> conditions; /* Including the else if conditions */
        vector<vector<shared_ptr<kh::AstBody>>> bodies;
        vector<shared_ptr<kh::AstBody>> else_body;

        AstIf(size_t _index, const vector<shared_ptr<kh::AstExpression>>& _conditions,
              const vector<vector<shared_ptr<kh::AstBody>>>& _bodies,
              const vector<shared_ptr<kh::AstBody>>& _else_body);
        virtual ~AstIf() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstWhile : public kh::AstBody {
    public:
        shared_ptr<kh::AstExpression> condition;
        vector<shared_ptr<kh::AstBody>> body;

        AstWhile(size_t _index, shared_ptr<kh::AstExpression>& _condition,
                 const vector<shared_ptr<kh::AstBody>>& _body);
        virtual ~AstWhile() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstDoWhile : public kh::AstBody {
    public:
        shared_ptr<kh::AstExpression> condition;
        vector<shared_ptr<kh::AstBody>> body;

        AstDoWhile(size_t _index, shared_ptr<kh::AstExpression>& _condition,
                   const vector<shared_ptr<kh::AstBody>>& _body);
        virtual ~AstDoWhile() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstFor : public kh::AstBody {
    public:
        shared_ptr<kh::AstExpression> initialize;
        shared_ptr<kh::AstExpression> condition;
        shared_ptr<kh::AstExpression> step;
        vector<shared_ptr<kh::AstBody>> body;

        AstFor(size_t _index, shared_ptr<kh::AstExpression>& initialize,
               shared_ptr<kh::AstExpression>& condition, shared_ptr<kh::AstExpression>& step,
               const vector<shared_ptr<kh::AstBody>>& _body);
        virtual ~AstFor() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstForEach : public kh::AstBody {
    public:
        shared_ptr<kh::AstExpression> target;
        shared_ptr<kh::AstExpression> iterator;
        vector<shared_ptr<kh::AstBody>> body;

        AstForEach(size_t _index, shared_ptr<kh::AstExpression>& _target,
                   shared_ptr<kh::AstExpression>& _iterator,
                   const vector<shared_ptr<kh::AstBody>>& _body);
        virtual ~AstForEach() {}

        virtual u32string str(size_t indent = 0) const;
    };

    class AstStatement : public kh::AstBody {
    public:
        enum class Type { CONTINUE, BREAK, RETURN } statement_type;

        shared_ptr<kh::AstExpression> expression;
        size_t loop_count;

        AstStatement(size_t _index, kh::AstStatement::Type _statement_type,
                     shared_ptr<kh::AstExpression>& _expression);
        AstStatement(size_t _index, kh::AstStatement::Type _statement_type, size_t _loop_count);
        virtual ~AstStatement() {}

        virtual u32string str(size_t indent = 0) const;
    };
}
