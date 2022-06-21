/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <kithare/lib/array.h>
#include <kithare/lib/buffer.h>
#include <kithare/lib/string.h>


typedef struct _khAst khAst;
typedef struct _khAstExpression khAstExpression;


typedef enum {
    khAstType_INVALID,

    khAstType_VARIABLE,
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
    khAstType_BREAK,
    khAstType_CONTINUE,
    khAstType_RETURN
} khAstType;

khstring khAstType_string(khAstType type);


// This variable declaration AST struct is declared up here, as it is used by khAstLambdaExpression.
typedef struct {
    bool is_static;
    bool is_wild;
    bool is_ref;
    khstring name;
    khAstExpression* optional_type;
    khAstExpression* optional_initializer;
} khAstVariable;

khAstVariable khAstVariable_copy(khAstVariable* variable);
void khAstVariable_delete(khAstVariable* variable);
khstring khAstVariable_string(khAstVariable* variable, char32_t* origin);


typedef enum {
    khAstExpressionType_INVALID,

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

    khAstExpressionType_LAMBDA,
    khAstExpressionType_SCOPE,
    khAstExpressionType_FUNCTION_TYPE,
    khAstExpressionType_TEMPLATIZE
} khAstExpressionType;

khstring khAstExpressionType_string(khAstExpressionType type);


typedef struct {
    kharray(khAstExpression) values;
} khAstTuple;

khAstTuple khAstTuple_copy(khAstTuple* tuple);
void khAstTuple_delete(khAstTuple* tuple);
khstring khAstTuple_string(khAstTuple* tuple, char32_t* origin);


typedef struct {
    kharray(khAstExpression) values;
} khAstArray;

khAstArray khAstArray_copy(khAstArray* array);
void khAstArray_delete(khAstArray* array);
khstring khAstArray_string(khAstArray* array, char32_t* origin);


typedef struct {
    kharray(khAstExpression) keys;
    kharray(khAstExpression) values;
} khAstDict;

khAstDict khAstDict_copy(khAstDict* dict);
void khAstDict_delete(khAstDict* dict);
khstring khAstDict_string(khAstDict* dict, char32_t* origin);


typedef enum {
    khAstUnaryExpressionType_POSITIVE,
    khAstUnaryExpressionType_NEGATIVE,

    khAstUnaryExpressionType_NOT,
    khAstUnaryExpressionType_BIT_NOT
} khAstUnaryExpressionType;

khstring khAstUnaryExpressionType_string(khAstUnaryExpressionType type);


typedef struct {
    khAstUnaryExpressionType type;
    khAstExpression* operand;
} khAstUnaryExpression;

khAstUnaryExpression khAstUnaryExpression_copy(khAstUnaryExpression* unary_exp);
void khAstUnaryExpression_delete(khAstUnaryExpression* unary_exp);
khstring khAstUnaryExpression_string(khAstUnaryExpression* unary_exp, char32_t* origin);


typedef enum {
    khAstBinaryExpressionType_ASSIGN,
    khAstBinaryExpressionType_RANGE,

    khAstBinaryExpressionType_ADD,
    khAstBinaryExpressionType_SUB,
    khAstBinaryExpressionType_MUL,
    khAstBinaryExpressionType_DIV,
    khAstBinaryExpressionType_MOD,
    khAstBinaryExpressionType_DOT,
    khAstBinaryExpressionType_POW,

    khAstBinaryExpressionType_IADD,
    khAstBinaryExpressionType_ISUB,
    khAstBinaryExpressionType_IMUL,
    khAstBinaryExpressionType_IDIV,
    khAstBinaryExpressionType_IMOD,
    khAstBinaryExpressionType_IDOT,
    khAstBinaryExpressionType_IPOW,

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

khstring khAstBinaryExpressionType_string(khAstBinaryExpressionType type);


typedef struct {
    khAstBinaryExpressionType type;
    khAstExpression* left;
    khAstExpression* right;
} khAstBinaryExpression;

khAstBinaryExpression khAstBinaryExpression_copy(khAstBinaryExpression* binary_exp);
void khAstBinaryExpression_delete(khAstBinaryExpression* binary_exp);
khstring khAstBinaryExpression_string(khAstBinaryExpression* binary_exp, char32_t* origin);


typedef struct {
    khAstExpression* condition;
    khAstExpression* value;
    khAstExpression* otherwise;
} khAstTernaryExpression;

khAstTernaryExpression khAstTernaryExpression_copy(khAstTernaryExpression* ternary_exp);
void khAstTernaryExpression_delete(khAstTernaryExpression* ternary_exp);
khstring khAstTernaryExpression_string(khAstTernaryExpression* ternary_exp, char32_t* origin);


typedef enum {
    khAstComparisonExpressionType_EQUAL,
    khAstComparisonExpressionType_NOT_EQUAL,
    khAstComparisonExpressionType_LESS,
    khAstComparisonExpressionType_MORE,
    khAstComparisonExpressionType_ELESS,
    khAstComparisonExpressionType_EMORE
} khAstComparisonExpressionType;

khstring khAstComparisonExpressionType_string(khAstComparisonExpressionType type);


typedef struct {
    kharray(khAstComparisonExpressionType) operations;
    kharray(khAstExpression) operands;
} khAstComparisonExpression;

khAstComparisonExpression khAstComparisonExpression_copy(khAstComparisonExpression* comparison_exp);
void khAstComparisonExpression_delete(khAstComparisonExpression* comparison_exp);
khstring khAstComparisonExpression_string(khAstComparisonExpression* comparison_exp, char32_t* origin);


typedef struct {
    khAstExpression* callee;
    kharray(khAstExpression) arguments;
} khAstCallExpression;

khAstCallExpression khAstCallExpression_copy(khAstCallExpression* call_exp);
void khAstCallExpression_delete(khAstCallExpression* call_exp);
khstring khAstCallExpression_string(khAstCallExpression* call_exp, char32_t* origin);


typedef struct {
    khAstExpression* indexee;
    kharray(khAstExpression) arguments;
} khAstIndexExpression;

khAstIndexExpression khAstIndexExpression_copy(khAstIndexExpression* index_exp);
void khAstIndexExpression_delete(khAstIndexExpression* index_exp);
khstring khAstIndexExpression_string(khAstIndexExpression* index_exp, char32_t* origin);


typedef struct {
    kharray(khAstVariable) arguments;
    khAstVariable* optional_variadic_argument;
    bool is_return_type_ref;
    khAstExpression* optional_return_type;
    kharray(khAst) content;
} khAstLambdaExpression;

khAstLambdaExpression khAstLambdaExpression_copy(khAstLambdaExpression* lambda);
void khAstLambdaExpression_delete(khAstLambdaExpression* lambda);
khstring khAstLambdaExpression_string(khAstLambdaExpression* lambda, char32_t* origin);


typedef struct {
    khAstExpression* value;
    kharray(khstring) scope_names;
} khAstScopeExpression;

khAstScopeExpression khAstScopeExpression_copy(khAstScopeExpression* scope_exp);
void khAstScopeExpression_delete(khAstScopeExpression* scope_exp);
khstring khAstScopeExpression_string(khAstScopeExpression* scope_exp, char32_t* origin);


typedef struct {
    kharray(bool) are_arguments_refs;
    kharray(khAstExpression) argument_types;
    bool is_return_type_ref;
    khAstExpression* optional_return_type;
} khAstFunctionTypeExpression;

// I swear, I hate clang-format for this
khAstFunctionTypeExpression
khAstFunctionTypeExpression_copy(khAstFunctionTypeExpression* function_type);

void khAstFunctionTypeExpression_delete(khAstFunctionTypeExpression* function_type);
khstring khAstFunctionTypeExpression_string(khAstFunctionTypeExpression* function_type,
                                            char32_t* origin);


typedef struct {
    khAstExpression* value;
    kharray(khAstExpression) template_arguments;
} khAstTemplatizeExpression;

khAstTemplatizeExpression khAstTemplatizeExpression_copy(khAstTemplatizeExpression* templatize_exp);
void khAstTemplatizeExpression_delete(khAstTemplatizeExpression* templatize_exp);
khstring khAstTemplatizeExpression_string(khAstTemplatizeExpression* templatize_exp, char32_t* origin);


struct _khAstExpression {
    char32_t* begin;
    char32_t* end;

    khAstExpressionType type;
    union {
        khstring identifier;
        char32_t char_v;
        khstring string;
        khbuffer buffer;
        uint8_t byte;
        __int128_t integer;
        __uint128_t uinteger;
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

        khAstLambdaExpression lambda;
        khAstScopeExpression scope;
        khAstFunctionTypeExpression function_type;
        khAstTemplatizeExpression templatize;
    };
};

khAstExpression khAstExpression_copy(khAstExpression* expression);
void khAstExpression_delete(khAstExpression* expression);
khstring khAstExpression_string(khAstExpression* expression, char32_t* origin);


typedef struct {
    kharray(khstring) path;
    bool relative;
    khstring* optional_alias;
} khAstImport;

khAstImport khAstImport_copy(khAstImport* import_v);
void khAstImport_delete(khAstImport* import_v);
khstring khAstImport_string(khAstImport* import_v, char32_t* origin);


typedef struct {
    kharray(khstring) path;
    bool relative;
} khAstInclude;

khAstInclude khAstInclude_copy(khAstInclude* include);
void khAstInclude_delete(khAstInclude* include);
khstring khAstInclude_string(khAstInclude* include, char32_t* origin);


typedef struct {
    bool is_incase;
    bool is_static;
    kharray(khstring) identifiers;
    kharray(khstring) template_arguments;
    kharray(khAstVariable) arguments;
    khAstVariable* optional_variadic_argument;
    bool is_return_type_ref;
    khAstExpression* optional_return_type;
    kharray(khAst) content;
} khAstFunction;

khAstFunction khAstFunction_copy(khAstFunction* function);
void khAstFunction_delete(khAstFunction* function);
khstring khAstFunction_string(khAstFunction* function, char32_t* origin);


typedef struct {
    bool is_incase;
    khstring name;
    kharray(khstring) template_arguments;
    khAstExpression* optional_base_type;
    kharray(khAst) content;
} khAstClass;

khAstClass khAstClass_copy(khAstClass* class_v);
void khAstClass_delete(khAstClass* class_v);
khstring khAstClass_string(khAstClass* class_v, char32_t* origin);


typedef struct {
    bool is_incase;
    khstring name;
    kharray(khstring) template_arguments;
    kharray(khAst) content;
} khAstStruct;

khAstStruct khAstStruct_copy(khAstStruct* struct_v);
void khAstStruct_delete(khAstStruct* struct_v);
khstring khAstStruct_string(khAstStruct* struct_v, char32_t* origin);


typedef struct {
    khstring name;
    kharray(khstring) members;
} khAstEnum;

khAstEnum khAstEnum_copy(khAstEnum* enum_v);
void khAstEnum_delete(khAstEnum* enum_v);
khstring khAstEnum_string(khAstEnum* enum_v, char32_t* origin);


typedef struct {
    bool is_incase;
    khstring name;
    khAstExpression expression;
} khAstAlias;

khAstAlias khAstAlias_copy(khAstAlias* alias);
void khAstAlias_delete(khAstAlias* alias);
khstring khAstAlias_string(khAstAlias* alias, char32_t* origin);


typedef struct {
    kharray(khAstExpression) branch_conditions;
    kharray(kharray(khAst)) branch_contents;
    kharray(khAst) else_content;
} khAstIfBranch;

khAstIfBranch khAstIfBranch_copy(khAstIfBranch* if_branch);
void khAstIfBranch_delete(khAstIfBranch* if_branch);
khstring khAstIfBranch_string(khAstIfBranch* if_branch, char32_t* origin);


typedef struct {
    khAstExpression condition;
    kharray(khAst) content;
} khAstWhileLoop;

khAstWhileLoop khAstWhileLoop_copy(khAstWhileLoop* while_loop);
void khAstWhileLoop_delete(khAstWhileLoop* while_loop);
khstring khAstWhileLoop_string(khAstWhileLoop* while_loop, char32_t* origin);


typedef struct {
    khAstExpression condition;
    kharray(khAst) content;
} khAstDoWhileLoop;

khAstDoWhileLoop khAstDoWhileLoop_copy(khAstDoWhileLoop* do_while_loop);
void khAstDoWhileLoop_delete(khAstDoWhileLoop* do_while_loop);
khstring khAstDoWhileLoop_string(khAstDoWhileLoop* do_while_loop, char32_t* origin);


typedef struct {
    kharray(khstring) iterators;
    khAstExpression iteratee;
    kharray(khAst) content;
} khAstForLoop;

khAstForLoop khAstForLoop_copy(khAstForLoop* for_loop);
void khAstForLoop_delete(khAstForLoop* for_loop);
khstring khAstForLoop_string(khAstForLoop* for_loop, char32_t* origin);


typedef struct {
    kharray(khAstExpression) values;
} khAstReturn;

khAstReturn khAstReturn_copy(khAstReturn* return_v);
void khAstReturn_delete(khAstReturn* return_v);
khstring khAstReturn_string(khAstReturn* return_v, char32_t* origin);


struct _khAst {
    char32_t* begin;
    char32_t* end;

    khAstType type;
    union {
        khAstVariable variable;
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
        khAstReturn return_v;
    };
};

khAst khAst_copy(khAst* ast);
void khAst_delete(khAst* ast);
khstring khAst_string(khAst* ast, char32_t* origin);


#ifdef __cplusplus
}
#endif
