/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "array.h"
#include "string.h"


typedef struct _khAst khAst;
typedef struct _khAstExpression khAstExpression;


typedef enum {
    khAstType_EXPRESSION,

    khAstType_IMPORT,
    khAstType_IMPORT_AS,
    khAstType_INCLUDE,
    khAstType_FUNCTION,
    khAstType_CLASS,
    khAstType_STRUCT,
    khAstType_ENUM,
    khAstType_ALIAS,

    khAstType_IF_BRANCH,
    khAstType_WHILE_LOOP,
    khAstType_DO_WHILE_LOOP,
    khAstType_FOR_LOOP,
    khAstType_FOR_EACH_LOOP,
    khAstType_BREAK,
    khAstType_CONTINUE,
    khAstType_RETURN
} khAstType;


typedef enum {
    khAstExpressionType_IDENTIFIER,
    khAstExpressionType_CHAR,
    khAstExpressionType_STRING,
    khAstExpressionType_BUFFER,
    khAstExpressionType_BYTE,
    khAstExpressionType_INTEGER,
    khAstExpressionType_UINTEGER,
    khAstExpressionType_FLOAT,
    khAstExpressionType_DOUBLE,
    khAstExpressionType_IFLOAT,
    khAstExpressionType_IDOUBLE,

    khAstExpressionType_TUPLE,
    khAstExpressionType_ARRAY,
    khAstExpressionType_DICT,

    khAstExpressionType_UNARY,
    khAstExpressionType_BINARY,
    khAstExpressionType_TERNARY,
    khAstExpressionType_COMPARISON,
    khAstExpressionType_CALL,
    khAstExpressionType_INDEX,

    khAstExpressionType_VARIABLE_DECLARATION,
    khAstExpressionType_LAMBDA,
    khAstExpressionType_SCOPE,
    khAstExpressionType_TEMPLATIZE
} khAstExpressionType;


typedef struct {
    khArray(khAstExpression) values;
} khAstTuple;


typedef struct {
    khArray(khAstExpression) values;
} khAstArray;


typedef struct {
    khArray(khAstExpression) keys;
    khArray(khAstExpression) values;
} khAstDict;


typedef enum {
    khAstUnaryExpressionType_SUB,

    khAstUnaryExpressionType_PRE_INCREMENT,
    khAstUnaryExpressionType_PRE_DECREMENT,
    khAstUnaryExpressionType_POST_INCREMENT,
    khAstUnaryExpressionType_POST_DECREMENT,

    khAstUnaryExpressionType_NOT,
    khAstUnaryExpressionType_BIT_NOT
} khAstUnaryExpressionType;

typedef struct {
    khAstUnaryExpressionType type;
    khAstExpression* operand;
} khAstUnaryExpression;


typedef enum {
    khAstBinaryExpressionType_ADD,
    khAstBinaryExpressionType_SUB,
    khAstBinaryExpressionType_MUL,
    khAstBinaryExpressionType_DIV,
    khAstBinaryExpressionType_MODULO,
    khAstBinaryExpressionType_POWER,
    khAstBinaryExpressionType_DOT,

    khAstBinaryExpressionType_IADD,
    khAstBinaryExpressionType_ISUB,
    khAstBinaryExpressionType_IMUL,
    khAstBinaryExpressionType_IDIV,
    khAstBinaryExpressionType_IMODULO,
    khAstBinaryExpressionType_IPOWER,
    khAstBinaryExpressionType_IDOT,

    khAstBinaryExpressionType_ASSIGN,

    khAstBinaryExpressionType_AND,
    khAstBinaryExpressionType_OR,
    khAstBinaryExpressionType_XOR,

    khAstBinaryExpressionType_BIT_AND,
    khAstBinaryExpressionType_BIT_OR,
    khAstBinaryExpressionType_BIT_XOR,
    khAstBinaryExpressionType_BIT_LSHIFT,
    khAstBinaryExpressionType_BIT_RSHIFT,

    khAstBinaryExpressionType_IBIT_AND,
    khAstBinaryExpressionType_IBIT_OR,
    khAstBinaryExpressionType_IBIT_XOR,
    khAstBinaryExpressionType_IBIT_LSHIFT,
    khAstBinaryExpressionType_IBIT_RSHIFT
} khAstBinaryExpressionType;

typedef struct {
    khAstBinaryExpressionType type;
    khAstExpression* left;
    khAstExpression* right;
} khAstBinaryExpression;


typedef struct {
    khAstExpression* condition;
    khAstExpression* value;
    khAstExpression* otherwise;
} khAstTernaryExpression;


typedef enum {
    khAstComparisonExpressionType_EQUAL,
    khAstComparisonExpressionType_NOT_EQUAL,
    khAstComparisonExpressionType_LESS,
    khAstComparisonExpressionType_MORE,
    khAstComparisonExpressionType_ELESS,
    khAstComparisonExpressionType_EMORE
} khAstComparisonExpressionType;

typedef struct {
    khArray(khAstComparisonExpressionType) operations;
    khArray(khAstExpression) operands;
} khAstComparisonExpression;


typedef struct {
    khAstExpression* value;
    khArray(khAstExpression) arguments;
} khAstCallExpression;


typedef struct {
    khAstExpression* value;
    khArray(khAstExpression) arguments;
} khAstIndexExpression;


typedef struct {
    khAstExpression* type;
    khArray(char32_t) name;
    khAstExpression* optional_initializer;
} khAstVariableDeclaration;


typedef struct {
    khAstExpression* return_type;
    khArray(khAstVariableDeclaration) arguments;
    khAstVariableDeclaration* optional_variadic_argument;
    khArray(khAst) content;
} khAstLambdaExpression;


typedef struct {
    khAstExpression* value;
    khArray(khArray(char32_t)) scope_names;
} khAstScopeExpression;


typedef struct {
    khAstExpression* value;
    khArray(khAstExpression) template_arguments;
} khAstTemplatizeExpression;


struct _khAstExpression {
    khAstExpressionType type;
    union {
        khArray(char32_t) identifier;
        char32_t char_v;
        khArray(char32_t) string;
        khArray(uint8_t) buffer;
        uint8_t byte;
        int64_t integer;
        uint64_t uinteger;
        float float_v;
        double double_v;
        float ifloat;
        double idouble;

        khAstUnaryExpression unary;
        khAstBinaryExpression binary;
        khAstTernaryExpression ternary;
        khAstComparisonExpression comparison;
        khAstCallExpression call;
        khAstIndexExpression index;

        khAstVariableDeclaration variable_declaration;
        khAstLambdaExpression lambda;
        khAstScopeExpression scope;
        khAstTemplatizeExpression templatize;
    };
};


typedef struct {
    khArray(khArray(char32_t)) path;
    bool relative;
} khAstImport;


typedef struct {
    khArray(khArray(char32_t)) path;
    bool relative;
    khArray(char32_t) alias;
} khAstImportAs;


typedef struct {
    khArray(khArray(char32_t)) path;
    bool relative;
} khAstInclude;


typedef struct {
    khAstExpression name_point;
    khAstExpression return_type;
    khArray(khAstVariableDeclaration) arguments;
    khAstVariableDeclaration* optional_variadic_argument;
    khArray(khAst) content;
} khAstFunction;


typedef struct {
    khArray(char32_t) name;
    khArray(khArray(char32_t)) template_arguments;
    khArray(khAst) content;
} khAstClass;


typedef struct {
    khArray(char32_t) name;
    khArray(khArray(char32_t)) template_arguments;
    khArray(khAst) content;
} khAstStruct;


typedef struct {
    khArray(char32_t) name;
    khArray(khArray(char32_t)) members;
    khArray(uint64_t) values;
} khAstEnum;


typedef struct {
    khArray(char32_t) name;
    khAstExpression expression;
} khAstAlias;


typedef struct {
    khArray(khAstExpression) branch_conditions;
    khArray(khArray(khAst)) branch_contents;
    khArray(khAst) else_content;
} khAstIfBranch;


typedef struct {
    khAstExpression condition;
    khArray(khAst) content;
} khAstWhileLoop;


typedef struct {
    khAstExpression condition;
    khArray(khAst) content;
} khAstDoWhileLoop;


typedef struct {
    khAstExpression initial_expression;
    khAstExpression loop_condition;
    khAstExpression update_expression;
    khArray(khAst) content;
} khAstForLoop;


typedef struct {
    khArray(khAstExpression) iterators;
    khAstExpression iteratee;
    khArray(khAst) content;
} khAstForEachLoop;


typedef struct {
    uint64_t breakings;
} khAstBreak;


typedef struct {
    uint64_t continuations;
} khAstContinue;


typedef struct {
    khArray(khAstExpression) expressions;
} khAstReturn;


struct _khAst {
    khAstType type;
    union {
        khAstExpression expression;

        khAstImport import_v;
        khAstImportAs import_as;
        khAstInclude include;
        khAstFunction function;
        khAstClass class_v;
        khAstStruct struct_v;
        khAstEnum enum_v;
        khAstAlias alias_v;

        khAstIfBranch if_branch;
        khAstWhileLoop while_loop;
        khAstDoWhileLoop do_while_loop;
        khAstForLoop for_loop;
        khAstForEachLoop for_each_loop;
        khAstBreak break_v;
        khAstContinue continue_v;
        khAstReturn return_v;
    };
};


#ifdef __cplusplus
}
#endif
