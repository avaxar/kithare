/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#include <stdlib.h>

#include <kithare/ast.h>
#include <kithare/string.h>


khArray(char32_t) khAstType_string(khAstType type) {
    switch (type) {
        case khAstType_EXPRESSION:
            return kh_string(U"expression");

        case khAstType_IMPORT:
            return kh_string(U"import");
        case khAstType_INCLUDE:
            return kh_string(U"include");
        case khAstType_FUNCTION:
            return kh_string(U"function");
        case khAstType_CLASS:
            return kh_string(U"class");
        case khAstType_STRUCT:
            return kh_string(U"struct");
        case khAstType_ENUM:
            return kh_string(U"enum");
        case khAstType_ALIAS:
            return kh_string(U"alias");

        case khAstType_IF_BRANCH:
            return kh_string(U"if_branch");
        case khAstType_WHILE_LOOP:
            return kh_string(U"while_loop");
        case khAstType_DO_WHILE_LOOP:
            return kh_string(U"do_while_loop");
        case khAstType_FOR_LOOP:
            return kh_string(U"for_loop");
        case khAstType_FOR_EACH_LOOP:
            return kh_string(U"for_each_loop");
        case khAstType_BREAK:
            return kh_string(U"break");
        case khAstType_CONTINUE:
            return kh_string(U"continue");
        case khAstType_RETURN:
            return kh_string(U"return");

        default:
            return kh_string(U"[unknown]");
    }
}

khArray(char32_t) khAstExpressionType_string(khAstExpressionType type) {
    switch (type) {
        case khAstExpressionType_IDENTIFIER:
            return kh_string(U"identifier");
        case khAstExpressionType_CHAR:
            return kh_string(U"char");
        case khAstExpressionType_STRING:
            return kh_string(U"string");
        case khAstExpressionType_BUFFER:
            return kh_string(U"buffer");
        case khAstExpressionType_BYTE:
            return kh_string(U"byte");
        case khAstExpressionType_INTEGER:
            return kh_string(U"integer");
        case khAstExpressionType_UINTEGER:
            return kh_string(U"uinteger");
        case khAstExpressionType_FLOAT:
            return kh_string(U"float");
        case khAstExpressionType_DOUBLE:
            return kh_string(U"double");
        case khAstExpressionType_IFLOAT:
            return kh_string(U"ifloat");
        case khAstExpressionType_IDOUBLE:
            return kh_string(U"idouble");

        case khAstExpressionType_TUPLE:
            return kh_string(U"tuple");
        case khAstExpressionType_ARRAY:
            return kh_string(U"array");
        case khAstExpressionType_DICT:
            return kh_string(U"dict");

        case khAstExpressionType_UNARY:
            return kh_string(U"unary");
        case khAstExpressionType_BINARY:
            return kh_string(U"binary");
        case khAstExpressionType_TERNARY:
            return kh_string(U"ternary");
        case khAstExpressionType_COMPARISON:
            return kh_string(U"comparison");
        case khAstExpressionType_CALL:
            return kh_string(U"call");
        case khAstExpressionType_INDEX:
            return kh_string(U"index");

        case khAstExpressionType_VARIABLE_DECLARATION:
            return kh_string(U"variable_declaration");
        case khAstExpressionType_LAMBDA:
            return kh_string(U"lambda");
        case khAstExpressionType_SCOPE:
            return kh_string(U"scope");
        case khAstExpressionType_TEMPLATIZE:
            return kh_string(U"templatize");

        default:
            return kh_string(U"[unknown]");
    }
}

khAstTuple khAstTuple_copy(khAstTuple* tuple) {
    return (khAstTuple){.values = khArray_copy(&tuple->values, khAstExpression_copy)};
}

void khAstTuple_delete(khAstTuple* tuple) {
    khArray_delete(&tuple->values);
}

khArray(char32_t) khAstTuple_string(khAstTuple* tuple);

khAstArray khAstArray_copy(khAstArray* array) {
    return (khAstArray){.values = khArray_copy(&array->values, khAstExpression_copy)};
}

void khAstArray_delete(khAstArray* array) {
    khArray_delete(&array->values);
}

khArray(char32_t) khAstArray_string(khAstArray* array);

khAstDict khAstDict_copy(khAstDict* dict) {
    return (khAstDict){.keys = khArray_copy(&dict->keys, khAstExpression_copy),
                       .values = khArray_copy(&dict->values, khAstExpression_copy)};
}

void khAstDict_delete(khAstDict* dict) {
    khArray_delete(&dict->keys);
    khArray_delete(&dict->values);
}

khArray(char32_t) khAstDict_string(khAstDict* dict);

khArray(char32_t) khAstUnaryExpressionType_string(khAstUnaryExpressionType type) {
    switch (type) {
        case khAstUnaryExpressionType_POSITIVE:
            return kh_string(U"positive");
        case khAstUnaryExpressionType_NEGATIVE:
            return kh_string(U"negative");

        case khAstUnaryExpressionType_PRE_INCREMENT:
            return kh_string(U"pre_increment");
        case khAstUnaryExpressionType_PRE_DECREMENT:
            return kh_string(U"pre_decrement");
        case khAstUnaryExpressionType_POST_INCREMENT:
            return kh_string(U"post_increment");
        case khAstUnaryExpressionType_POST_DECREMENT:
            return kh_string(U"post_decrement");

        case khAstUnaryExpressionType_NOT:
            return kh_string(U"not");
        case khAstUnaryExpressionType_BIT_NOT:
            return kh_string(U"bit_not");

        default:
            return kh_string(U"[unknown]");
    }
}

khAstUnaryExpression khAstUnaryExpression_copy(khAstUnaryExpression* unary_exp) {
    khAstExpression* operand = (khAstExpression*)malloc(sizeof(khAstExpression));
    *operand = khAstExpression_copy(unary_exp->operand);

    return (khAstUnaryExpression){.type = unary_exp->type, .operand = operand};
}

void khAstUnaryExpression_delete(khAstUnaryExpression* unary_exp) {
    khAstExpression_delete(unary_exp->operand);
}

khArray(char32_t) khAstUnaryExpression_string(khAstUnaryExpression* unary_exp);

khArray(char32_t) khAstBinaryExpressionType_string(khAstBinaryExpressionType type) {
    switch (type) {
        case khAstBinaryExpressionType_ADD:
            return kh_string(U"add");
        case khAstBinaryExpressionType_SUB:
            return kh_string(U"sub");
        case khAstBinaryExpressionType_MUL:
            return kh_string(U"mul");
        case khAstBinaryExpressionType_DIV:
            return kh_string(U"div");
        case khAstBinaryExpressionType_MOD:
            return kh_string(U"mod");
        case khAstBinaryExpressionType_POW:
            return kh_string(U"pow");
        case khAstBinaryExpressionType_DOT:
            return kh_string(U"dot");

        case khAstBinaryExpressionType_IADD:
            return kh_string(U"iadd");
        case khAstBinaryExpressionType_ISUB:
            return kh_string(U"isub");
        case khAstBinaryExpressionType_IMUL:
            return kh_string(U"imul");
        case khAstBinaryExpressionType_IDIV:
            return kh_string(U"idiv");
        case khAstBinaryExpressionType_IMOD:
            return kh_string(U"imod");
        case khAstBinaryExpressionType_IPOW:
            return kh_string(U"ipow");
        case khAstBinaryExpressionType_IDOT:
            return kh_string(U"idot");

        case khAstBinaryExpressionType_ASSIGN:
            return kh_string(U"assign");

        case khAstBinaryExpressionType_AND:
            return kh_string(U"and");
        case khAstBinaryExpressionType_OR:
            return kh_string(U"or");
        case khAstBinaryExpressionType_XOR:
            return kh_string(U"xor");

        case khAstBinaryExpressionType_BIT_AND:
            return kh_string(U"bit_and");
        case khAstBinaryExpressionType_BIT_OR:
            return kh_string(U"bit_or");
        case khAstBinaryExpressionType_BIT_XOR:
            return kh_string(U"bit_xor");
        case khAstBinaryExpressionType_BIT_LSHIFT:
            return kh_string(U"bit_lshift");
        case khAstBinaryExpressionType_BIT_RSHIFT:
            return kh_string(U"bit_rshift");

        case khAstBinaryExpressionType_IBIT_AND:
            return kh_string(U"ibit_and");
        case khAstBinaryExpressionType_IBIT_OR:
            return kh_string(U"ibit_or");
        case khAstBinaryExpressionType_IBIT_XOR:
            return kh_string(U"ibit_xor");
        case khAstBinaryExpressionType_IBIT_LSHIFT:
            return kh_string(U"ibit_lshift");
        case khAstBinaryExpressionType_IBIT_RSHIFT:
            return kh_string(U"ibit_rshift");

        default:
            return kh_string(U"[unknown]");
    }
}

khAstBinaryExpression khAstBinaryExpression_copy(khAstBinaryExpression* binary_exp) {
    khAstExpression* left = (khAstExpression*)malloc(sizeof(khAstExpression));
    *left = khAstExpression_copy(binary_exp->left);

    khAstExpression* right = (khAstExpression*)malloc(sizeof(khAstExpression));
    *right = khAstExpression_copy(binary_exp->right);

    return (khAstBinaryExpression){.type = binary_exp->type, .left = left, .right = right};
}

void khAstBinaryExpression_delete(khAstBinaryExpression* binary_exp) {
    khAstExpression_delete(binary_exp->left);
    khAstExpression_delete(binary_exp->right);
}

khArray(char32_t) khAstBinaryExpression_string(khAstBinaryExpression* binary_exp);

khAstTernaryExpression khAstTernaryExpression_copy(khAstTernaryExpression* ternary_exp) {
    khAstExpression* condition = (khAstExpression*)malloc(sizeof(khAstExpression));
    *condition = khAstExpression_copy(ternary_exp->condition);

    khAstExpression* value = (khAstExpression*)malloc(sizeof(khAstExpression));
    *value = khAstExpression_copy(ternary_exp->value);

    khAstExpression* otherwise = (khAstExpression*)malloc(sizeof(khAstExpression));
    *otherwise = khAstExpression_copy(ternary_exp->otherwise);

    return (khAstTernaryExpression){.condition = condition, .value = value, .otherwise = otherwise};
}
void khAstTernaryExpression_delete(khAstTernaryExpression* ternary_exp) {
    khAstExpression_delete(ternary_exp->condition);
    khAstExpression_delete(ternary_exp->value);
    khAstExpression_delete(ternary_exp->otherwise);
}

khArray(char32_t) khAstTernaryExpression_string(khAstTernaryExpression* ternary_exp);

khArray(char32_t) khAstComparisonExpressionType_string(khAstComparisonExpressionType type) {
    switch (type) {
        case khAstComparisonExpressionType_EQUAL:
            return kh_string(U"equal");
        case khAstComparisonExpressionType_NOT_EQUAL:
            return kh_string(U"not_equal");
        case khAstComparisonExpressionType_LESS:
            return kh_string(U"less");
        case khAstComparisonExpressionType_MORE:
            return kh_string(U"more");
        case khAstComparisonExpressionType_ELESS:
            return kh_string(U"eless");
        case khAstComparisonExpressionType_EMORE:
            return kh_string(U"emore");

        default:
            return kh_string(U"[unknown]");
    }
}

khAstComparisonExpression khAstComparisonExpression_copy(khAstComparisonExpression* comparison_exp) {
    return (khAstComparisonExpression){
        .operations = khArray_copy(&comparison_exp->operations, NULL),
        .operands = khArray_copy(&comparison_exp->operands, khAstExpression_copy)};
}

void khAstComparisonExpression_delete(khAstComparisonExpression* comparison_exp) {
    khArray_delete(&comparison_exp->operations);
    khArray_delete(&comparison_exp->operands);
}

khArray(char32_t) khAstComparisonExpression_string(khAstComparisonExpression* comparison_exp);

khAstCallExpression khAstCallExpression_copy(khAstCallExpression* call_exp) {
    khAstExpression* callee = (khAstExpression*)malloc(sizeof(khAstExpression));
    *callee = khAstExpression_copy(call_exp->callee);

    return (khAstCallExpression){.callee = callee,
                                 .arguments = khArray_copy(&call_exp->arguments, khAstExpression_copy)};
}

void khAstCallExpression_delete(khAstCallExpression* call_exp) {
    khAstExpression_delete(call_exp->callee);
    khArray_delete(&call_exp->arguments);
}

khArray(char32_t) khAstCallExpression_string(khAstCallExpression* call_exp);

khAstIndexExpression khAstIndexExpression_copy(khAstIndexExpression* index_exp) {
    khAstExpression* indexee = (khAstExpression*)malloc(sizeof(khAstExpression));
    *indexee = khAstExpression_copy(index_exp->indexee);

    return (khAstIndexExpression){
        .indexee = indexee, .arguments = khArray_copy(&index_exp->arguments, khAstExpression_copy)};
}

void khAstIndexExpression_delete(khAstIndexExpression* index_exp) {
    khAstExpression_delete(index_exp->indexee);
    khArray_delete(&index_exp->arguments);
}

khArray(char32_t) khAstIndexExpression_string(khAstIndexExpression* index_exp);

khAstVariableDeclaration khAstVariableDeclaration_copy(khAstVariableDeclaration* declaration) {
    khAstExpression* optional_type = NULL;
    if (declaration->optional_type != NULL) {
        optional_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_type = khAstExpression_copy(declaration->optional_type);
    }

    khAstExpression* optional_initializer = NULL;
    if (declaration->optional_initializer != NULL) {
        optional_initializer = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_initializer = khAstExpression_copy(declaration->optional_initializer);
    }

    return (khAstVariableDeclaration){.optional_type = optional_type,
                                      .name = khArray_copy(&declaration->name, NULL),
                                      .optional_initializer = optional_initializer};
}

void khAstVariableDeclaration_delete(khAstVariableDeclaration* declaration) {
    if (declaration->optional_type != NULL) {
        khAstExpression_delete(declaration->optional_type);
    }
    khArray_delete(&declaration->name);
    if (declaration->optional_initializer != NULL) {
        khAstExpression_delete(declaration->optional_initializer);
    }
}

khArray(char32_t) khAstVariableDeclaration_string(khAstVariableDeclaration* declaration);

khAstLambdaExpression khAstLambdaExpression_copy(khAstLambdaExpression* lambda) {
    khAstVariableDeclaration* optional_variadic_argument = NULL;
    if (lambda->optional_variadic_argument != NULL) {
        optional_variadic_argument =
            (khAstVariableDeclaration*)malloc(sizeof(khAstVariableDeclaration));
        *optional_variadic_argument = khAstVariableDeclaration_copy(lambda->optional_variadic_argument);
    }

    khAstExpression* optional_return_type = NULL;
    if (lambda->optional_return_type != NULL) {
        optional_return_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_return_type = khAstExpression_copy(lambda->optional_return_type);
    }

    return (khAstLambdaExpression){.arguments =
                                       khArray_copy(&lambda->arguments, khAstVariableDeclaration_copy),
                                   .optional_variadic_argument = optional_variadic_argument,
                                   .optional_return_type = optional_return_type,
                                   .content = khArray_copy(&lambda->content, khAst_copy)};
}

void khAstLambdaExpression_delete(khAstLambdaExpression* lambda) {
    khArray_delete(&lambda->arguments);
    khAstVariableDeclaration_delete(lambda->optional_variadic_argument);
    khAstExpression_delete(lambda->optional_return_type);
    khArray_delete(&lambda->content);
}

khArray(char32_t) khAstLambdaExpression_string(khAstLambdaExpression* lambda);

khAstScopeExpression khAstScopeExpression_copy(khAstScopeExpression* scope_exp) {
    khAstExpression* value = (khAstExpression*)malloc(sizeof(khAstExpression));
    *value = khAstExpression_copy(scope_exp->value);

    khArray(khArray(char32_t)) scope_names = khArray_new(khArray(char32_t), NULL);
    for (size_t i = 0; i < khArray_size(&scope_exp->scope_names); i++) {
        khArray_append(&scope_names, khArray_copy(&scope_exp->scope_names[i], NULL));
    }

    return (khAstScopeExpression){.value = value, .scope_names = scope_names};
}

void khAstScopeExpression_delete(khAstScopeExpression* scope_exp) {
    khAstExpression_delete(scope_exp->value);
    for (size_t i = 0; i < khArray_size(&scope_exp->scope_names); i++) {
        khArray_delete(&scope_exp->scope_names[i]);
    }
    khArray_delete(&scope_exp->scope_names);
}

khArray(char32_t) khAstScopeExpression_string(khAstScopeExpression* scope_exp);

khAstTemplatizeExpression khAstTemplatizeExpression_copy(khAstTemplatizeExpression* templatize_exp) {
    khAstExpression* value = (khAstExpression*)malloc(sizeof(khAstExpression));
    *value = khAstExpression_copy(templatize_exp->value);

    return (khAstTemplatizeExpression){
        .value = value,
        .template_arguments = khArray_copy(&templatize_exp->template_arguments, khAstExpression_copy)};
}

void khAstTemplatizeExpression_delete(khAstTemplatizeExpression* templatize_exp) {
    khAstExpression_delete(templatize_exp->value);
    khArray_delete(&templatize_exp->template_arguments);
}

khArray(char32_t) khAstTemplatizeExpression_string(khAstTemplatizeExpression* templatize_exp);

khAstExpression khAstExpression_copy(khAstExpression* expression) {
    khAstExpression copy = *expression;

    switch (expression->type) {
        case khAstExpressionType_IDENTIFIER:
            copy.identifier = khArray_copy(&expression->identifier, NULL);
            break;
        case khAstExpressionType_STRING:
            copy.string = khArray_copy(&expression->string, NULL);
            break;
        case khAstExpressionType_BUFFER:
            copy.buffer = khArray_copy(&expression->buffer, NULL);
            break;

        case khAstExpressionType_TUPLE:
            copy.tuple = khAstTuple_copy(&expression->tuple);
            break;
        case khAstExpressionType_ARRAY:
            copy.array = khAstArray_copy(&expression->array);
            break;
        case khAstExpressionType_DICT:
            copy.dict = khAstDict_copy(&expression->dict);
            break;

        case khAstExpressionType_UNARY:
            copy.unary = khAstUnaryExpression_copy(&expression->unary);
            break;
        case khAstExpressionType_BINARY:
            copy.binary = khAstBinaryExpression_copy(&expression->binary);
            break;
        case khAstExpressionType_TERNARY:
            copy.ternary = khAstTernaryExpression_copy(&expression->ternary);
            break;
        case khAstExpressionType_COMPARISON:
            copy.comparison = khAstComparisonExpression_copy(&expression->comparison);
            break;
        case khAstExpressionType_CALL:
            copy.call = khAstCallExpression_copy(&expression->call);
            break;
        case khAstExpressionType_INDEX:
            copy.index = khAstIndexExpression_copy(&expression->index);
            break;

        case khAstExpressionType_VARIABLE_DECLARATION:
            copy.variable_declaration =
                khAstVariableDeclaration_copy(&expression->variable_declaration);
            break;
        case khAstExpressionType_LAMBDA:
            copy.lambda = khAstLambdaExpression_copy(&expression->lambda);
            break;
        case khAstExpressionType_SCOPE:
            copy.scope = khAstScopeExpression_copy(&expression->scope);
            break;
        case khAstExpressionType_TEMPLATIZE:
            copy.templatize = khAstTemplatizeExpression_copy(&expression->templatize);
            break;

        default:
            break;
    }

    return copy;
}

void khAstExpression_delete(khAstExpression* expression) {
    switch (expression->type) {
        case khAstExpressionType_IDENTIFIER:
            khArray_delete(&expression->identifier);
            break;
        case khAstExpressionType_STRING:
            khArray_delete(&expression->string);
            break;
        case khAstExpressionType_BUFFER:
            khArray_delete(&expression->buffer);
            break;

        case khAstExpressionType_TUPLE:
            khAstTuple_delete(&expression->tuple);
            break;
        case khAstExpressionType_ARRAY:
            khAstArray_delete(&expression->array);
            break;
        case khAstExpressionType_DICT:
            khAstDict_delete(&expression->dict);
            break;

        case khAstExpressionType_UNARY:
            khAstUnaryExpression_delete(&expression->unary);
            break;
        case khAstExpressionType_BINARY:
            khAstBinaryExpression_delete(&expression->binary);
            break;
        case khAstExpressionType_TERNARY:
            khAstTernaryExpression_delete(&expression->ternary);
            break;
        case khAstExpressionType_COMPARISON:
            khAstComparisonExpression_delete(&expression->comparison);
            break;
        case khAstExpressionType_CALL:
            khAstCallExpression_delete(&expression->call);
            break;
        case khAstExpressionType_INDEX:
            khAstIndexExpression_delete(&expression->index);
            break;

        case khAstExpressionType_VARIABLE_DECLARATION:
            khAstVariableDeclaration_delete(&expression->variable_declaration);
            break;
        case khAstExpressionType_LAMBDA:
            khAstLambdaExpression_delete(&expression->lambda);
            break;
        case khAstExpressionType_SCOPE:
            khAstScopeExpression_delete(&expression->scope);
            break;
        case khAstExpressionType_TEMPLATIZE:
            khAstTemplatizeExpression_delete(&expression->templatize);

        default:
            break;
    }
}

khArray(char32_t) khAstExpression_string(khAstExpression* expression);

khAstImport khAstImport_copy(khAstImport* import_v) {
    khArray(khArray(char32_t)) path = khArray_new(khArray(char32_t), NULL);
    for (size_t i = 0; i < khArray_size(&import_v->path); i++) {
        khArray_append(&path, khArray_copy(&import_v->path[i], NULL));
    }

    khArray(char32_t)* optional_alias = NULL;
    if (import_v->optional_alias) {
        optional_alias = (khArray(char32_t)*)malloc(sizeof(khArray(char32_t)));
        *optional_alias = khArray_copy(import_v->optional_alias, NULL);
    }

    return (khAstImport){
        .path = path, .relative = import_v->relative, .optional_alias = optional_alias};
}

void khAstImport_delete(khAstImport* import_v) {
    for (size_t i = 0; i < khArray_size(&import_v->path); i++) {
        khArray_delete(&import_v->path[i]);
    }
    khArray_delete(&import_v->path);

    if (import_v->optional_alias != NULL) {
        khArray_delete(import_v->optional_alias);
    }
}

khArray(char32_t) khAstImport_string(khAstImport* import_v);

khAstInclude khAstInclude_copy(khAstInclude* include) {
    khArray(khArray(char32_t)) path = khArray_new(khArray(char32_t), NULL);
    for (size_t i = 0; i < khArray_size(&include->path); i++) {
        khArray_append(&path, khArray_copy(&include->path[i], NULL));
    }

    return (khAstInclude){.path = path, .relative = include->relative};
}

void khAstInclude_delete(khAstInclude* include) {
    for (size_t i = 0; i < khArray_size(&include->path); i++) {
        khArray_delete(&include->path[i]);
    }
    khArray_delete(&include->path);
}

khArray(char32_t) khAstInclude_string(khAstInclude* include);

khAstFunction khAstFunction_copy(khAstFunction* function) {
    khAstVariableDeclaration* optional_variadic_argument = NULL;
    if (function->optional_variadic_argument != NULL) {
        optional_variadic_argument =
            (khAstVariableDeclaration*)malloc(sizeof(khAstVariableDeclaration));
        *optional_variadic_argument =
            khAstVariableDeclaration_copy(function->optional_variadic_argument);
    }

    khAstExpression* optional_return_type = NULL;
    if (function->optional_return_type != NULL) {
        optional_return_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_return_type = khAstExpression_copy(function->optional_return_type);
    }

    return (khAstFunction){.name_point = khAstExpression_copy(&function->name_point),
                           .arguments =
                               khArray_copy(&function->arguments, khAstVariableDeclaration_copy),
                           .optional_variadic_argument = optional_variadic_argument,
                           .optional_return_type = optional_return_type,
                           .content = khArray_copy(&function->content, khAst_copy)};
}

void khAstFunction_delete(khAstFunction* function) {
    khAstExpression_delete(&function->name_point);
    khArray_delete(&function->arguments);
    if (function->optional_variadic_argument != NULL) {
        khAstVariableDeclaration_delete(function->optional_variadic_argument);
    }
    if (function->optional_return_type != NULL) {
        khAstExpression_delete(function->optional_return_type);
    }
    khArray_delete(&function->content);
}

khArray(char32_t) khAstFunction_string(khAstFunction* function);

khAstClass khAstClass_copy(khAstClass* class_v) {
    khArray(khArray(char32_t)) template_arguments = khArray_new(khArray(char32_t), NULL);
    for (size_t i = 0; i < khArray_size(&class_v->template_arguments); i++) {
        khArray_append(&template_arguments, khArray_copy(&class_v->template_arguments[i], NULL));
    }

    return (khAstClass){.name = khArray_copy(&class_v->name, NULL),
                        .template_arguments = template_arguments,
                        .content = khArray_copy(&class_v->content, khAst_copy)};
}

void khAstClass_delete(khAstClass* class_v) {
    khArray_delete(&class_v->name);
    for (size_t i = 0; i < khArray_size(&class_v->template_arguments); i++) {
        khArray_delete(&class_v->template_arguments[i]);
    }
    khArray_delete(&class_v->template_arguments);
    khArray_delete(&class_v->content);
}

khArray(char32_t) khAstClass_string(khAstClass* class_v);

khAstStruct khAstStruct_copy(khAstStruct* struct_v) {
    khArray(khArray(char32_t)) template_arguments = khArray_new(khArray(char32_t), NULL);
    for (size_t i = 0; i < khArray_size(&struct_v->template_arguments); i++) {
        khArray_append(&template_arguments, khArray_copy(&struct_v->template_arguments[i], NULL));
    }

    return (khAstStruct){.name = khArray_copy(&struct_v->name, NULL),
                         .template_arguments = template_arguments,
                         .content = khArray_copy(&struct_v->content, khAst_copy)};
}

void khAstStruct_delete(khAstStruct* struct_v) {
    khArray_delete(&struct_v->name);
    for (size_t i = 0; i < khArray_size(&struct_v->template_arguments); i++) {
        khArray_delete(&struct_v->template_arguments[i]);
    }
    khArray_delete(&struct_v->template_arguments);
    khArray_delete(&struct_v->content);
}

khArray(char32_t) khAstStruct_string(khAstStruct* struct_v);

khAstEnum khAstEnum_copy(khAstEnum* enum_v) {
    khArray(khArray(char32_t)) members = khArray_new(khArray(char32_t), NULL);
    for (size_t i = 0; i < khArray_size(&enum_v->members); i++) {
        khArray_append(&members, khArray_copy(&enum_v->members[i], NULL));
    }

    return (khAstEnum){.name = khArray_copy(&enum_v->name, NULL),
                       .members = members,
                       .values = khArray_copy(&enum_v->values, NULL)};
}

void khAstEnum_delete(khAstEnum* enum_v) {
    khArray_delete(&enum_v->name);
    for (size_t i = 0; i < khArray_size(&enum_v->members); i++) {
        khArray_delete(&enum_v->members[i]);
    }
    khArray_delete(&enum_v->members);
    khArray_delete(&enum_v->values);
}

khArray(char32_t) khAstEnum_string(khAstEnum* enum_v);

khAstAlias khAstAlias_copy(khAstAlias* alias) {
    return (khAstAlias){.name = khArray_copy(&alias->name, NULL),
                        .expression = khAstExpression_copy(&alias->expression)};
}

void khAstAlias_delete(khAstAlias* alias) {
    khArray_delete(&alias->name);
    khAstExpression_delete(&alias->expression);
}

khArray(char32_t) khAstAlias_string(khAstAlias* alias);

khAstIfBranch khAstIfBranch_copy(khAstIfBranch* if_branch) {
    khArray(khArray(khAst)) branch_contents = khArray_new(khArray(khAst), NULL);
    for (size_t i = 0; i < khArray_size(&if_branch->branch_contents); i++) {
        khArray_append(&branch_contents, khArray_copy(&if_branch->branch_contents[i], khAst_copy));
    }

    return (khAstIfBranch){.branch_conditions =
                               khArray_copy(&if_branch->branch_conditions, khAstExpression_copy),
                           .branch_contents = branch_contents,
                           .else_content = khArray_copy(&if_branch->else_content, khAst_copy)};
}

void khAstIfBranch_delete(khAstIfBranch* if_branch) {
    khArray_delete(&if_branch->branch_conditions);
    for (size_t i = 0; i < khArray_size(&if_branch->branch_contents); i++) {
        khArray_delete(&if_branch->branch_contents[i]);
    }
    khArray_delete(&if_branch->branch_contents);
    khArray_delete(&if_branch->else_content);
}

khArray(char32_t) khAstIfBranch_string(khAstIfBranch* if_branch);

khAstWhileLoop khAstWhileLoop_copy(khAstWhileLoop* while_loop) {
    return (khAstWhileLoop){.condition = khAstExpression_copy(&while_loop->condition),
                            .content = khArray_copy(&while_loop->content, khAst_copy)};
}

void khAstWhileLoop_delete(khAstWhileLoop* while_loop) {
    khAstExpression_delete(&while_loop->condition);
    khArray_delete(&while_loop->content);
}

khArray(char32_t) khAstWhileLoop_string(khAstWhileLoop* while_loop);

khAstDoWhileLoop khAstDoWhileLoop_copy(khAstDoWhileLoop* do_while_loop) {
    return (khAstDoWhileLoop){.condition = khAstExpression_copy(&do_while_loop->condition),
                              .content = khArray_copy(&do_while_loop->content, khAst_copy)};
}

void khAstDoWhileLoop_delete(khAstDoWhileLoop* do_while_loop) {
    khAstExpression_delete(&do_while_loop->condition);
    khArray_delete(&do_while_loop->content);
}

khArray(char32_t) khAstDoWhileLoop_string(khAstDoWhileLoop* do_while_loop);

khAstForLoop khAstForLoop_copy(khAstForLoop* for_loop) {
    return (khAstForLoop){.initial_expression = khAstExpression_copy(&for_loop->initial_expression),
                          .loop_condition = khAstExpression_copy(&for_loop->loop_condition),
                          .update_expression = khAstExpression_copy(&for_loop->update_expression),
                          .content = khArray_copy(&for_loop->content, khAst_copy)};
}

void khAstForLoop_delete(khAstForLoop* for_loop) {
    khAstExpression_delete(&for_loop->initial_expression);
    khAstExpression_delete(&for_loop->loop_condition);
    khAstExpression_delete(&for_loop->update_expression);
    khArray_delete(&for_loop->content);
}

khArray(char32_t) khAstForLoop_string(khAstForLoop* for_loop);

khAstForEachLoop khAstForEachLoop_copy(khAstForEachLoop* for_each_loop) {
    return (khAstForEachLoop){.iterators =
                                  khArray_copy(&for_each_loop->iterators, khAstExpression_copy),
                              .iteratee = khAstExpression_copy(&for_each_loop->iteratee),
                              .content = khArray_copy(&for_each_loop->content, khAst_copy)};
}

void khAstForEachLoop_delete(khAstForEachLoop* for_each_loop) {
    khArray_delete(&for_each_loop->iterators);
    khAstExpression_delete(&for_each_loop->iteratee);
    khArray_delete(&for_each_loop->content);
}

khArray(char32_t) khAstForEachLoop_string(khAstForEachLoop* for_each_loop);

khAstBreak khAstBreak_copy(khAstBreak* break_v) {
    return *break_v;
}

void khAstBreak_delete(khAstBreak* break_v) {}

khArray(char32_t) khAstBreak_string(khAstBreak* break_v);

khAstContinue khAstContinue_copy(khAstContinue* continue_v) {
    return *continue_v;
}

void khAstContinue_delete(khAstContinue* continue_v) {}

khArray(char32_t) khAstContinue_string(khAstContinue* continue_v);

khAstReturn khAstReturn_copy(khAstReturn* return_v) {
    return (khAstReturn){.values = khArray_copy(&return_v->values, khAstExpression_copy)};
}

void khAstReturn_delete(khAstReturn* return_v) {
    khArray_delete(&return_v->values);
}

khArray(char32_t) khAstReturn_string(khAstReturn* return_v);

khAst khAst_copy(khAst* ast) {
    khAst copy = *ast;

    switch (ast->type) {
        case khAstType_EXPRESSION:
            copy.expression = khAstExpression_copy(&ast->expression);
            break;

        case khAstType_IMPORT:
            copy.import_v = khAstImport_copy(&ast->import_v);
            break;
        case khAstType_INCLUDE:
            copy.include = khAstInclude_copy(&ast->include);
            break;
        case khAstType_FUNCTION:
            copy.function = khAstFunction_copy(&ast->function);
            break;
        case khAstType_CLASS:
            copy.class_v = khAstClass_copy(&ast->class_v);
            break;
        case khAstType_STRUCT:
            copy.struct_v = khAstStruct_copy(&ast->struct_v);
            break;
        case khAstType_ENUM:
            copy.enum_v = khAstEnum_copy(&ast->enum_v);
            break;
        case khAstType_ALIAS:
            copy.alias = khAstAlias_copy(&ast->alias);
            break;

        case khAstType_IF_BRANCH:
            copy.if_branch = khAstIfBranch_copy(&ast->if_branch);
            break;
        case khAstType_WHILE_LOOP:
            copy.while_loop = khAstWhileLoop_copy(&ast->while_loop);
            break;
        case khAstType_DO_WHILE_LOOP:
            copy.do_while_loop = khAstDoWhileLoop_copy(&ast->do_while_loop);
            break;
        case khAstType_FOR_LOOP:
            copy.for_loop = khAstForLoop_copy(&ast->for_loop);
            break;
        case khAstType_FOR_EACH_LOOP:
            copy.for_each_loop = khAstForEachLoop_copy(&ast->for_each_loop);
            break;
        case khAstType_BREAK:
            copy.break_v = khAstBreak_copy(&ast->break_v);
            break;
        case khAstType_CONTINUE:
            copy.continue_v = khAstContinue_copy(&ast->continue_v);
            break;
        case khAstType_RETURN:
            copy.return_v = khAstReturn_copy(&ast->return_v);
            break;

        default:
            break;
    }

    return copy;
}

void khAst_delete(khAst* ast) {
    switch (ast->type) {
        case khAstType_EXPRESSION:
            khAstExpression_delete(&ast->expression);
            break;

        case khAstType_IMPORT:
            khAstImport_delete(&ast->import_v);
            break;
        case khAstType_INCLUDE:
            khAstInclude_delete(&ast->include);
            break;
        case khAstType_FUNCTION:
            khAstFunction_delete(&ast->function);
            break;
        case khAstType_CLASS:
            khAstClass_delete(&ast->class_v);
            break;
        case khAstType_STRUCT:
            khAstStruct_delete(&ast->struct_v);
            break;
        case khAstType_ENUM:
            khAstEnum_delete(&ast->enum_v);
            break;
        case khAstType_ALIAS:
            khAstAlias_delete(&ast->alias);
            break;

        case khAstType_IF_BRANCH:
            khAstIfBranch_delete(&ast->if_branch);
            break;
        case khAstType_WHILE_LOOP:
            khAstWhileLoop_delete(&ast->while_loop);
            break;
        case khAstType_DO_WHILE_LOOP:
            khAstDoWhileLoop_delete(&ast->do_while_loop);
            break;
        case khAstType_FOR_LOOP:
            khAstForLoop_delete(&ast->for_loop);
            break;
        case khAstType_FOR_EACH_LOOP:
            khAstForEachLoop_delete(&ast->for_each_loop);
            break;
        case khAstType_BREAK:
            khAstBreak_delete(&ast->break_v);
            break;
        case khAstType_CONTINUE:
            khAstContinue_delete(&ast->continue_v);
            break;
        case khAstType_RETURN:
            khAstReturn_delete(&ast->return_v);
            break;

        default:
            break;
    }
}

khArray(char32_t) khAst_string(khAst* ast);
