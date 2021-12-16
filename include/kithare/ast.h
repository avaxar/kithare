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

khArray(char32_t) khAstType_string(khAstType type);


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

khArray(char32_t) khAstExpressionType_string(khAstExpressionType type);


typedef struct {
    khArray(khAstExpression) values;
} khAstTuple;

khAstTuple khAstTuple_copy(khAstTuple* tuple);
void khAstTuple_delete(khAstTuple* tuple);
khArray(char32_t) khAstTuple_string(khAstTuple* tuple);


typedef struct {
    khArray(khAstExpression) values;
} khAstArray;

khAstArray khAstArray_copy(khAstArray* array);
void khAstArray_delete(khAstArray* array);
khArray(char32_t) khAstArray_string(khAstArray* array);


typedef struct {
    khArray(khAstExpression) keys;
    khArray(khAstExpression) values;
} khAstDict;

khAstDict khAstDict_copy(khAstDict* dict);
void khAstDict_delete(khAstDict* dict);
khArray(char32_t) khAstDict_string(khAstDict* dict);


typedef enum {
    khAstUnaryExpressionType_POSITIVE,
    khAstUnaryExpressionType_NEGATIVE,

    khAstUnaryExpressionType_PRE_INCREMENT,
    khAstUnaryExpressionType_PRE_DECREMENT,
    khAstUnaryExpressionType_POST_INCREMENT,
    khAstUnaryExpressionType_POST_DECREMENT,

    khAstUnaryExpressionType_NOT,
    khAstUnaryExpressionType_BIT_NOT
} khAstUnaryExpressionType;

khArray(char32_t) khAstUnaryExpressionType_string(khAstUnaryExpressionType type);


typedef struct {
    khAstUnaryExpressionType type;
    khAstExpression* operand;
} khAstUnaryExpression;

khAstUnaryExpression khAstUnaryExpression_copy(khAstUnaryExpression* unary_exp);
void khAstUnaryExpression_delete(khAstUnaryExpression* unary_exp);
khArray(char32_t) khAstUnaryExpression_string(khAstUnaryExpression* unary_exp);


typedef enum {
    khAstBinaryExpressionType_ADD,
    khAstBinaryExpressionType_SUB,
    khAstBinaryExpressionType_MUL,
    khAstBinaryExpressionType_DIV,
    khAstBinaryExpressionType_MOD,
    khAstBinaryExpressionType_POW,
    khAstBinaryExpressionType_DOT,

    khAstBinaryExpressionType_IADD,
    khAstBinaryExpressionType_ISUB,
    khAstBinaryExpressionType_IMUL,
    khAstBinaryExpressionType_IDIV,
    khAstBinaryExpressionType_IMOD,
    khAstBinaryExpressionType_IPOW,
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

khArray(char32_t) khAstBinaryExpressionType_string(khAstBinaryExpressionType type);


typedef struct {
    khAstBinaryExpressionType type;
    khAstExpression* left;
    khAstExpression* right;
} khAstBinaryExpression;

khAstBinaryExpression khAstBinaryExpression_copy(khAstBinaryExpression* binary_exp);
void khAstBinaryExpression_delete(khAstBinaryExpression* binary_exp);
khArray(char32_t) khAstBinaryExpression_string(khAstBinaryExpression* binary_exp);


typedef struct {
    khAstExpression* condition;
    khAstExpression* value;
    khAstExpression* otherwise;
} khAstTernaryExpression;

khAstTernaryExpression khAstTernaryExpression_copy(khAstTernaryExpression* ternary_exp);
void khAstTernaryExpression_delete(khAstTernaryExpression* ternary_exp);
khArray(char32_t) khAstTernaryExpression_string(khAstTernaryExpression* ternary_exp);


typedef enum {
    khAstComparisonExpressionType_EQUAL,
    khAstComparisonExpressionType_NOT_EQUAL,
    khAstComparisonExpressionType_LESS,
    khAstComparisonExpressionType_MORE,
    khAstComparisonExpressionType_ELESS,
    khAstComparisonExpressionType_EMORE
} khAstComparisonExpressionType;

khArray(char32_t) khAstComparisonExpressionType_string(khAstComparisonExpressionType type);


typedef struct {
    khArray(khAstComparisonExpressionType) operations;
    khArray(khAstExpression) operands;
} khAstComparisonExpression;

khAstComparisonExpression khAstComparisonExpression_copy(khAstComparisonExpression* comparison_exp);
void khAstComparisonExpression_delete(khAstComparisonExpression* comparison_exp);
khArray(char32_t) khAstComparisonExpression_string(khAstComparisonExpression* comparison_exp);


typedef struct {
    khAstExpression* callee;
    khArray(khAstExpression) arguments;
} khAstCallExpression;

khAstCallExpression khAstCallExpression_copy(khAstCallExpression* call_exp);
void khAstCallExpression_delete(khAstCallExpression* call_exp);
khArray(char32_t) khAstCallExpression_string(khAstCallExpression* call_exp);


typedef struct {
    khAstExpression* indexee;
    khArray(khAstExpression) arguments;
} khAstIndexExpression;

khAstIndexExpression khAstIndexExpression_copy(khAstIndexExpression* index_exp);
void khAstIndexExpression_delete(khAstIndexExpression* index_exp);
khArray(char32_t) khAstIndexExpression_string(khAstIndexExpression* index_exp);


typedef struct {
    khAstExpression* optional_type;
    khArray(char32_t) name;
    khAstExpression* optional_initializer;
} khAstVariableDeclaration;

khAstVariableDeclaration khAstVariableDeclaration_copy(khAstVariableDeclaration* declaration);
void khAstVariableDeclaration_delete(khAstVariableDeclaration* declaration);
khArray(char32_t) khAstVariableDeclaration_string(khAstVariableDeclaration* declaration);


typedef struct {
    khArray(khAstVariableDeclaration) arguments;
    khAstVariableDeclaration* optional_variadic_argument;
    khAstExpression* optional_return_type;
    khArray(khAst) content;
} khAstLambdaExpression;

khAstLambdaExpression khAstLambdaExpression_copy(khAstLambdaExpression* lambda);
void khAstLambdaExpression_delete(khAstLambdaExpression* lambda);
khArray(char32_t) khAstLambdaExpression_string(khAstLambdaExpression* lambda);


typedef struct {
    khAstExpression* value;
    khArray(khArray(char32_t)) scope_names;
} khAstScopeExpression;

khAstScopeExpression khAstScopeExpression_copy(khAstScopeExpression* scope_exp);
void khAstScopeExpression_delete(khAstScopeExpression* scope_exp);
khArray(char32_t) khAstScopeExpression_string(khAstScopeExpression* scope_exp);


typedef struct {
    khAstExpression* value;
    khArray(khAstExpression) template_arguments;
} khAstTemplatizeExpression;

khAstTemplatizeExpression khAstTemplatizeExpression_copy(khAstTemplatizeExpression* templatize_exp);
void khAstTemplatizeExpression_delete(khAstTemplatizeExpression* templatize_exp);
khArray(char32_t) khAstTemplatizeExpression_string(khAstTemplatizeExpression* templatize_exp);


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

        khAstTuple tuple;
        khAstArray array;
        khAstDict dict;

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

khAstExpression khAstExpression_copy(khAstExpression* expression);
void khAstExpression_delete(khAstExpression* expression);
khArray(char32_t) khAstExpression_string(khAstExpression* expression);


typedef struct {
    khArray(khArray(char32_t)) path;
    bool relative;
    khArray(char32_t) * optional_alias;
} khAstImport;

khAstImport khAstImport_copy(khAstImport* import_v);
void khAstImport_delete(khAstImport* import_v);
khArray(char32_t) khAstImport_string(khAstImport* import_v);


typedef struct {
    khArray(khArray(char32_t)) path;
    bool relative;
} khAstInclude;

khAstInclude khAstInclude_copy(khAstInclude* include);
void khAstInclude_delete(khAstInclude* include);
khArray(char32_t) khAstInclude_string(khAstInclude* include);


typedef struct {
    khAstExpression name_point;
    khArray(khAstVariableDeclaration) arguments;
    khAstVariableDeclaration* optional_variadic_argument;
    khAstExpression* optional_return_type;
    khArray(khAst) content;
} khAstFunction;

khAstFunction khAstFunction_copy(khAstFunction* function);
void khAstFunction_delete(khAstFunction* function);
khArray(char32_t) khAstFunction_string(khAstFunction* function);


typedef struct {
    khArray(char32_t) name;
    khArray(khArray(char32_t)) template_arguments;
    khArray(khAst) content;
} khAstClass;

khAstClass khAstClass_copy(khAstClass* class_v);
void khAstClass_delete(khAstClass* class_v);
khArray(char32_t) khAstClass_string(khAstClass* class_v);


typedef struct {
    khArray(char32_t) name;
    khArray(khArray(char32_t)) template_arguments;
    khArray(khAst) content;
} khAstStruct;

khAstStruct khAstStruct_copy(khAstStruct* struct_v);
void khAstStruct_delete(khAstStruct* struct_v);
khArray(char32_t) khAstStruct_string(khAstStruct* struct_v);


typedef struct {
    khArray(char32_t) name;
    khArray(khArray(char32_t)) members;
    khArray(uint64_t) values;
} khAstEnum;

khAstEnum khAstEnum_copy(khAstEnum* enum_v);
void khAstEnum_delete(khAstEnum* enum_v);
khArray(char32_t) khAstEnum_string(khAstEnum* enum_v);


typedef struct {
    khArray(char32_t) name;
    khAstExpression expression;
} khAstAlias;

khAstAlias khAstAlias_copy(khAstAlias* alias);
void khAstAlias_delete(khAstAlias* alias);
khArray(char32_t) khAstAlias_string(khAstAlias* alias);


typedef struct {
    khArray(khAstExpression) branch_conditions;
    khArray(khArray(khAst)) branch_contents;
    khArray(khAst) else_content;
} khAstIfBranch;

khAstIfBranch khAstIfBranch_copy(khAstIfBranch* if_branch);
void khAstIfBranch_delete(khAstIfBranch* if_branch);
khArray(char32_t) khAstIfBranch_string(khAstIfBranch* if_branch);


typedef struct {
    khAstExpression condition;
    khArray(khAst) content;
} khAstWhileLoop;

khAstWhileLoop khAstWhileLoop_copy(khAstWhileLoop* while_loop);
void khAstWhileLoop_delete(khAstWhileLoop* while_loop);
khArray(char32_t) khAstWhileLoop_string(khAstWhileLoop* while_loop);


typedef struct {
    khAstExpression condition;
    khArray(khAst) content;
} khAstDoWhileLoop;

khAstDoWhileLoop khAstDoWhileLoop_copy(khAstDoWhileLoop* do_while_loop);
void khAstDoWhileLoop_delete(khAstDoWhileLoop* do_while_loop);
khArray(char32_t) khAstDoWhileLoop_string(khAstDoWhileLoop* do_while_loop);


typedef struct {
    khAstExpression initial_expression;
    khAstExpression loop_condition;
    khAstExpression update_expression;
    khArray(khAst) content;
} khAstForLoop;

khAstForLoop khAstForLoop_copy(khAstForLoop* for_loop);
void khAstForLoop_delete(khAstForLoop* for_loop);
khArray(char32_t) khAstForLoop_string(khAstForLoop* for_loop);


typedef struct {
    khArray(khAstExpression) iterators;
    khAstExpression iteratee;
    khArray(khAst) content;
} khAstForEachLoop;

khAstForEachLoop khAstForEachLoop_copy(khAstForEachLoop* for_each_loop);
void khAstForEachLoop_delete(khAstForEachLoop* for_each_loop);
khArray(char32_t) khAstForEachLoop_string(khAstForEachLoop* for_each_loop);


typedef struct {
    uint64_t breakings;
} khAstBreak;

khAstBreak khAstBreak_copy(khAstBreak* break_v);
void khAstBreak_delete(khAstBreak* break_v);
khArray(char32_t) khAstBreak_string(khAstBreak* break_v);


typedef struct {
    uint64_t continuations;
} khAstContinue;

khAstContinue khAstContinue_copy(khAstContinue* continue_v);
void khAstContinue_delete(khAstContinue* continue_v);
khArray(char32_t) khAstContinue_string(khAstContinue* continue_v);


typedef struct {
    khArray(khAstExpression) values;
} khAstReturn;

khAstReturn khAstReturn_copy(khAstReturn* return_v);
void khAstReturn_delete(khAstReturn* return_v);
khArray(char32_t) khAstReturn_string(khAstReturn* return_v);


struct _khAst {
    khAstType type;
    union {
        khAstExpression expression;

        khAstImport import_v;
        khAstInclude include;
        khAstFunction function;
        khAstClass class_v;
        khAstStruct struct_v;
        khAstEnum enum_v;
        khAstAlias alias;

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

khAst khAst_copy(khAst* ast);
void khAst_delete(khAst* ast);
khArray(char32_t) khAst_string(khAst* ast);


#ifdef __cplusplus
}
#endif
