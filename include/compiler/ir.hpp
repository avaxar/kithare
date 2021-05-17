/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/compiler/ir.hpp
 * Declares the IR tree types.
 */

#pragma once

#include <complex>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <vector>

#include "parser/ast.hpp"


namespace kh {
    class Ir;

    class IrModule;
    class IrScope;
    class IrVariable;

    class IrBody;
    class IrIf;
    class IrWhile;
    class IrStatement;
    class IrExpressionBody;

    class IrExpression;
    class IrConst;
    class IrAssignExpression;
    class IrVariableHandle;
    class IrFunctionHandle;
    class IrBuiltinFunctionHandle;
    class IrTypeHandle;
    class IrCallExpression;
    class IrReinterpret;
    class IrScopeExpression;
    class IrPointerScopeExpression;
    class IrReferenceExpression;
    class IrDereferenceExpression;
    class IrReferenceAssignExpression;

    class IrFunction;
    class IrFunctionTemplate;

    class IrType;
    class IrEnum;
    class IrStruct;
    class IrClass;
    class IrClassTemplate;
    class IrPrimitive;
    class IrRefType;
    class IrArrayType;

    class Ir {
    public:
        std::vector<std::shared_ptr<kh::IrModule>> modules;
        std::vector<std::shared_ptr<kh::IrVariable>> globals;
        std::vector<std::shared_ptr<kh::IrFunctionTemplate>> functions;
        std::vector<std::shared_ptr<kh::IrEnum>> enums;
        std::vector<std::shared_ptr<kh::IrStruct>> structs;
        std::vector<std::shared_ptr<kh::IrClassTemplate>> classes;
    };

    class IrModule {
    public:
        std::vector<kh::IrModule*> modules;
        std::vector<kh::IrVariable*> globals;
        std::vector<kh::IrFunctionTemplate*> functions;
        std::vector<kh::IrEnum*> enums;
        std::vector<kh::IrStruct*> structs;
        std::vector<kh::IrClassTemplate*> classes;
        bool is_semanticized = false;
    };

    class IrScope {
    public:
        kh::IrModule* origin_module;
        kh::IrScope* origin_scope;
        std::vector<std::shared_ptr<kh::IrVariable>> locals;
        std::vector<std::shared_ptr<kh::IrFunctionTemplate>> functions;
        uint64_t bound;
    };

    class IrVariable {
    public:
        kh::IrType* type;
        std::u32string identifier;
        uint64_t offset;
    };

    class IrBody {
    public:
        enum class Type { IF, WHILE, STATEMENT, EXPRESSION } type;

        virtual ~IrBody() {}
    };

    class IrIf : public kh::IrBody {
    public:
    };

    class IrWhile : public kh::IrBody {
    public:
    };

    class IrStatement : public kh::IrBody {
    public:
    };

    class IrExpressionBody : public kh::IrBody {
    public:
    };

    class IrExpression {
    public:
        enum class Type {
            CONST,
            VARIABLE,
            ASSIGN,
            FUNCTION,
            BUILTIN_FUNCTION,
            TYPE,
            CALL,
            REINTERPRET,
            SCOPE,
            PTR_SCOPE,
            REF,
            DEREF,
            REF_ASSIGN
        } type;

        virtual ~IrExpression() {}
    };

    class IrConst : public kh::IrExpression {
    public:
        enum class ConstType {
            UINT8, INT8, UINT16, INT16,
            UINT32, INT32, UINT64, INT64,
            CHAR, FLOAT, DOUBLE,
            COMPLEXF, COMPLEX,
            BUFFER, STRING
        } const_type;
        union {
            uint8_t u8;
            int8_t i8;
            uint16_t u16;
            int16_t i16;
            uint32_t u32;
            int32_t i32;
            uint64_t u64;
            int64_t i64;
            char32_t chr;
            float f32;
            double f64;
        };
        std::complex<float> cmplxf;
        std::complex<double> cmplx;
        std::string buf;
        std::u32string str;
    };

    class IrVariableHandle : public kh::IrExpression {
    public:
        kh::IrVariable* variable;
    };

    class IrAssignExpression : public kh::IrExpression {
    public:
        std::shared_ptr<kh::IrExpression> expression;
        std::shared_ptr<kh::IrExpression> assign_to;
    };

    class IrFunctionHandler : public kh::IrExpression {
    public:
        kh::IrFunction* function;
        std::vector<std::shared_ptr<kh::IrExpression>> prefilled;
    };

    class IrBuiltinFunctionHandler : public kh::IrExpression {
    public:
        enum class Function {

        } function;
        std::vector<std::shared_ptr<kh::IrExpression>> prefilled;
    };

    class IrTypeHandler : public kh::IrExpression {
    public:
        kh::IrType* handled_type;
    };

    class IrCallExpression : public kh::IrExpression {
    public:
        std::shared_ptr<kh::IrExpression> expression;
        std::vector<std::shared_ptr<kh::IrExpression>> arguments;
    };

    class IrReinterpret : public kh::IrExpression {
    public:
        std::shared_ptr<kh::IrExpression> expression;
        kh::IrType* reinterpret_type;
    };

    class IrScopeExpression : public kh::IrExpression {
    public:
        std::shared_ptr<kh::IrExpression> expression;
        std::u32string scope_identifier;
        uint64_t offset;
    };

    class IrPointerScopeExpression : public kh::IrExpression {
    public:
        std::shared_ptr<kh::IrExpression> expression;
        std::u32string scope_identifier;
        uint64_t offset;
    };

    class IrReferenceExpression : public kh::IrExpression {
    public:
        std::shared_ptr<kh::IrExpression> expression;
    };

    class IrDereferenceExpression : public kh::IrExpression {
    public:
        std::shared_ptr<kh::IrExpression> expression;
    };

    class IrReferenceAssignExpression : public kh::IrExpression {
    public:
        std::shared_ptr<kh::IrExpression> expression;
        std::shared_ptr<kh::IrExpression> assign_to;
    };

    class IrFunction {
    public:
        kh::IrModule* origin_module;
        std::shared_ptr<kh::IrScope> scope;
        kh::IrType* return_type;
        std::vector<std::shared_ptr<kh::IrVariable>> arguments;
    };

    class IrFunctionTemplate {
    public:
        std::u32string identifier;
        std::vector<std::vector<kh::IrType*>> templated_types;
        std::vector<std::shared_ptr<kh::IrFunction>> templated_functions;
        kh::IrModule* reference_module;
        std::shared_ptr<kh::AstFunctionExpression> reference_ast;
    };

    class IrType {
    public:
        enum class Type { ENUM, STRUCT, CLASS, PRIMITIVE, ARRAY, REF } type;

        uint64_t size;
        uint64_t alignment;
        std::shared_ptr<kh::IrRefType> ref_variant;
        std::unordered_map<uint64_t, std::shared_ptr<kh::IrArrayType>> array_variants;

        virtual ~IrType() {}
    };

    class IrEnum : public kh::IrType {
    public:
        std::u32string identifier;
        std::vector<std::u32string> members;
        std::vector<uint64_t> values;
    };

    class IrStruct : public kh::IrType {
    public:
        std::u32string identifier;
        std::vector<kh::IrVariable> member_variables;
    };

    class IrClass : public kh::IrType {
    public:
        std::u32string identifier;
        std::vector<std::shared_ptr<kh::IrVariable>> member_variables;
        std::vector<std::shared_ptr<kh::IrFunction>> member_methods;
        std::vector<kh::IrVariable*> static_variables;
        std::vector<std::shared_ptr<kh::IrFunction>> static_methods;
    };

    class IrClassTemplate {
    public:
        std::u32string identifier;
        std::vector<std::vector<kh::IrType*>> templated_types;
        std::vector<std::shared_ptr<kh::IrClass>> templated_classes;
        kh::IrModule* reference_module;
        std::shared_ptr<kh::AstClass> reference_ast;
    };

    class IrPrimitive : public kh::IrType {
    public:
    };

    class IrRefType : public kh::IrType {
    public:
    };

    class IrArrayType : public kh::IrType {
    public:
    };
}
