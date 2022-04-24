/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
 */

#include <stdlib.h>

#include <kithare/core/ast.h>
#include <kithare/lib/string.h>


khstring khAstType_string(khAstType type) {
    switch (type) {
        case khAstType_INVALID:
            return khstring_new(U"invalid");

        case khAstType_EXPRESSION:
            return khstring_new(U"expression");

        case khAstType_IMPORT:
            return khstring_new(U"import");
        case khAstType_INCLUDE:
            return khstring_new(U"include");
        case khAstType_FUNCTION:
            return khstring_new(U"function");
        case khAstType_CLASS:
            return khstring_new(U"class");
        case khAstType_STRUCT:
            return khstring_new(U"struct");
        case khAstType_ENUM:
            return khstring_new(U"enum");
        case khAstType_ALIAS:
            return khstring_new(U"alias");

        case khAstType_IF_BRANCH:
            return khstring_new(U"if_branch");
        case khAstType_WHILE_LOOP:
            return khstring_new(U"while_loop");
        case khAstType_DO_WHILE_LOOP:
            return khstring_new(U"do_while_loop");
        case khAstType_FOR_LOOP:
            return khstring_new(U"for_loop");
        case khAstType_FOR_EACH_LOOP:
            return khstring_new(U"for_each_loop");
        case khAstType_BREAK:
            return khstring_new(U"break");
        case khAstType_CONTINUE:
            return khstring_new(U"continue");
        case khAstType_RETURN:
            return khstring_new(U"return");

        default:
            return khstring_new(U"unknown");
    }
}


khstring khAstExpressionType_string(khAstExpressionType type) {
    switch (type) {
        case khAstExpressionType_INVALID:
            return khstring_new(U"invalid");

        case khAstExpressionType_IDENTIFIER:
            return khstring_new(U"identifier");
        case khAstExpressionType_CHAR:
            return khstring_new(U"char");
        case khAstExpressionType_STRING:
            return khstring_new(U"string");
        case khAstExpressionType_BUFFER:
            return khstring_new(U"buffer");
        case khAstExpressionType_BYTE:
            return khstring_new(U"byte");
        case khAstExpressionType_INTEGER:
            return khstring_new(U"integer");
        case khAstExpressionType_UINTEGER:
            return khstring_new(U"uinteger");
        case khAstExpressionType_FLOAT:
            return khstring_new(U"float");
        case khAstExpressionType_DOUBLE:
            return khstring_new(U"double");
        case khAstExpressionType_IFLOAT:
            return khstring_new(U"ifloat");
        case khAstExpressionType_IDOUBLE:
            return khstring_new(U"idouble");

        case khAstExpressionType_TUPLE:
            return khstring_new(U"tuple");
        case khAstExpressionType_ARRAY:
            return khstring_new(U"array");
        case khAstExpressionType_DICT:
            return khstring_new(U"dict");

        case khAstExpressionType_UNARY:
            return khstring_new(U"unary");
        case khAstExpressionType_BINARY:
            return khstring_new(U"binary");
        case khAstExpressionType_TERNARY:
            return khstring_new(U"ternary");
        case khAstExpressionType_COMPARISON:
            return khstring_new(U"comparison");
        case khAstExpressionType_CALL:
            return khstring_new(U"call");
        case khAstExpressionType_INDEX:
            return khstring_new(U"index");

        case khAstExpressionType_VARIABLE_DECLARATION:
            return khstring_new(U"variable_declaration");
        case khAstExpressionType_LAMBDA:
            return khstring_new(U"lambda");
        case khAstExpressionType_SCOPE:
            return khstring_new(U"scope");
        case khAstExpressionType_TEMPLATIZE:
            return khstring_new(U"templatize");

        default:
            return khstring_new(U"unknown");
    }
}


khAstTuple khAstTuple_copy(khAstTuple* tuple) {
    return (khAstTuple){.values = kharray_copy(&tuple->values, khAstExpression_copy)};
}

void khAstTuple_delete(khAstTuple* tuple) {
    kharray_delete(&tuple->values);
}

khstring khAstTuple_string(khAstTuple* tuple, char32_t* origin) {
    khstring string = khstring_new(U"{\"values\": [");

    for (size_t i = 0; i < kharray_size(&tuple->values); i++) {
        khstring expression_str = khAstExpression_string(&tuple->values[i], origin);
        khstring_concatenate(&string, &expression_str);
        khstring_delete(&expression_str);

        if (i < kharray_size(&tuple->values) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstArray khAstArray_copy(khAstArray* array) {
    return (khAstArray){.values = kharray_copy(&array->values, khAstExpression_copy)};
}

void khAstArray_delete(khAstArray* array) {
    kharray_delete(&array->values);
}

khstring khAstArray_string(khAstArray* array, char32_t* origin) {
    khstring string = khstring_new(U"{\"values\": [");

    for (size_t i = 0; i < kharray_size(&array->values); i++) {
        khstring expression_str = khAstExpression_string(&array->values[i], origin);
        khstring_concatenate(&string, &expression_str);
        khstring_delete(&expression_str);

        if (i < kharray_size(&array->values) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstDict khAstDict_copy(khAstDict* dict) {
    return (khAstDict){.keys = kharray_copy(&dict->keys, khAstExpression_copy),
                       .values = kharray_copy(&dict->values, khAstExpression_copy)};
}

void khAstDict_delete(khAstDict* dict) {
    kharray_delete(&dict->keys);
    kharray_delete(&dict->values);
}

khstring khAstDict_string(khAstDict* dict, char32_t* origin) {
    khstring string = khstring_new(U"{\"keys\": [");

    for (size_t i = 0; i < kharray_size(&dict->keys); i++) {
        khstring expression_str = khAstExpression_string(&dict->keys[i], origin);
        khstring_concatenate(&string, &expression_str);
        khstring_delete(&expression_str);

        if (i < kharray_size(&dict->keys) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"values\": [");

    for (size_t i = 0; i < kharray_size(&dict->values); i++) {
        khstring expression_str = khAstExpression_string(&dict->values[i], origin);
        khstring_concatenate(&string, &expression_str);
        khstring_delete(&expression_str);

        if (i < kharray_size(&dict->values) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khstring khAstUnaryExpressionType_string(khAstUnaryExpressionType type) {
    switch (type) {
        case khAstUnaryExpressionType_POSITIVE:
            return khstring_new(U"positive");
        case khAstUnaryExpressionType_NEGATIVE:
            return khstring_new(U"negative");

        case khAstUnaryExpressionType_PRE_INCREMENT:
            return khstring_new(U"pre_increment");
        case khAstUnaryExpressionType_PRE_DECREMENT:
            return khstring_new(U"pre_decrement");
        case khAstUnaryExpressionType_POST_INCREMENT:
            return khstring_new(U"post_increment");
        case khAstUnaryExpressionType_POST_DECREMENT:
            return khstring_new(U"post_decrement");

        case khAstUnaryExpressionType_NOT:
            return khstring_new(U"not");
        case khAstUnaryExpressionType_BIT_NOT:
            return khstring_new(U"bit_not");

        default:
            return khstring_new(U"unknown");
    }
}


khAstUnaryExpression khAstUnaryExpression_copy(khAstUnaryExpression* unary_exp) {
    khAstExpression* operand = (khAstExpression*)malloc(sizeof(khAstExpression));
    *operand = khAstExpression_copy(unary_exp->operand);

    return (khAstUnaryExpression){.type = unary_exp->type, .operand = operand};
}

void khAstUnaryExpression_delete(khAstUnaryExpression* unary_exp) {
    khAstExpression_delete(unary_exp->operand);
    free(unary_exp->operand);
}

khstring khAstUnaryExpression_string(khAstUnaryExpression* unary_exp, char32_t* origin) {
    khstring string = khstring_new(U"{\"type\": ");
    khstring type_str = khAstUnaryExpressionType_string(unary_exp->type);
    khstring quoted_type = khstring_quote(&type_str);
    khstring_concatenate(&string, &quoted_type);
    khstring_delete(&type_str);
    khstring_delete(&quoted_type);

    khstring_concatenateCstring(&string, U", \"operand\": ");
    khstring operand_str = khAstExpression_string(unary_exp->operand, origin);
    khstring_concatenate(&string, &operand_str);
    khstring_delete(&operand_str);

    khstring_concatenateCstring(&string, U"}");
    return string;
}


khstring khAstBinaryExpressionType_string(khAstBinaryExpressionType type) {
    switch (type) {
        case khAstBinaryExpressionType_ADD:
            return khstring_new(U"add");
        case khAstBinaryExpressionType_SUB:
            return khstring_new(U"sub");
        case khAstBinaryExpressionType_MUL:
            return khstring_new(U"mul");
        case khAstBinaryExpressionType_DIV:
            return khstring_new(U"div");
        case khAstBinaryExpressionType_MOD:
            return khstring_new(U"mod");
        case khAstBinaryExpressionType_POW:
            return khstring_new(U"pow");
        case khAstBinaryExpressionType_DOT:
            return khstring_new(U"dot");

        case khAstBinaryExpressionType_IADD:
            return khstring_new(U"iadd");
        case khAstBinaryExpressionType_ISUB:
            return khstring_new(U"isub");
        case khAstBinaryExpressionType_IMUL:
            return khstring_new(U"imul");
        case khAstBinaryExpressionType_IDIV:
            return khstring_new(U"idiv");
        case khAstBinaryExpressionType_IMOD:
            return khstring_new(U"imod");
        case khAstBinaryExpressionType_IPOW:
            return khstring_new(U"ipow");
        case khAstBinaryExpressionType_IDOT:
            return khstring_new(U"idot");

        case khAstBinaryExpressionType_ASSIGN:
            return khstring_new(U"assign");

        case khAstBinaryExpressionType_AND:
            return khstring_new(U"and");
        case khAstBinaryExpressionType_OR:
            return khstring_new(U"or");
        case khAstBinaryExpressionType_XOR:
            return khstring_new(U"xor");

        case khAstBinaryExpressionType_BIT_AND:
            return khstring_new(U"bit_and");
        case khAstBinaryExpressionType_BIT_OR:
            return khstring_new(U"bit_or");
        case khAstBinaryExpressionType_BIT_XOR:
            return khstring_new(U"bit_xor");
        case khAstBinaryExpressionType_BIT_LSHIFT:
            return khstring_new(U"bit_lshift");
        case khAstBinaryExpressionType_BIT_RSHIFT:
            return khstring_new(U"bit_rshift");

        case khAstBinaryExpressionType_IBIT_AND:
            return khstring_new(U"ibit_and");
        case khAstBinaryExpressionType_IBIT_OR:
            return khstring_new(U"ibit_or");
        case khAstBinaryExpressionType_IBIT_XOR:
            return khstring_new(U"ibit_xor");
        case khAstBinaryExpressionType_IBIT_LSHIFT:
            return khstring_new(U"ibit_lshift");
        case khAstBinaryExpressionType_IBIT_RSHIFT:
            return khstring_new(U"ibit_rshift");

        default:
            return khstring_new(U"unknown");
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
    free(binary_exp->left);
    khAstExpression_delete(binary_exp->right);
    free(binary_exp->right);
}

khstring khAstBinaryExpression_string(khAstBinaryExpression* binary_exp, char32_t* origin) {
    khstring string = khstring_new(U"{\"type\": ");
    khstring type_str = khAstBinaryExpressionType_string(binary_exp->type);
    khstring quoted_type = khstring_quote(&type_str);
    khstring_concatenate(&string, &quoted_type);
    khstring_delete(&type_str);
    khstring_delete(&quoted_type);

    khstring_concatenateCstring(&string, U", \"left\": ");
    khstring left_str = khAstExpression_string(binary_exp->left, origin);
    khstring_concatenate(&string, &left_str);
    khstring_delete(&left_str);

    khstring_concatenateCstring(&string, U", \"right\": ");
    khstring right_str = khAstExpression_string(binary_exp->right, origin);
    khstring_concatenate(&string, &right_str);
    khstring_delete(&right_str);

    khstring_concatenateCstring(&string, U"}");
    return string;
}


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
    free(ternary_exp->condition);
    khAstExpression_delete(ternary_exp->value);
    free(ternary_exp->value);
    khAstExpression_delete(ternary_exp->otherwise);
    free(ternary_exp->otherwise);
}

khstring khAstTernaryExpression_string(khAstTernaryExpression* ternary_exp, char32_t* origin) {
    khstring string = khstring_new(U"{\"condition\": ");
    khstring condition_str = khAstExpression_string(ternary_exp->condition, origin);
    khstring_concatenate(&string, &condition_str);
    khstring_delete(&condition_str);

    khstring_concatenateCstring(&string, U", \"value\": ");
    khstring value_str = khAstExpression_string(ternary_exp->value, origin);
    khstring_concatenate(&string, &value_str);
    khstring_delete(&value_str);

    khstring_concatenateCstring(&string, U", \"otherwise\": ");
    khstring otherwise_str = khAstExpression_string(ternary_exp->otherwise, origin);
    khstring_concatenate(&string, &otherwise_str);
    khstring_delete(&otherwise_str);

    khstring_concatenateCstring(&string, U"}");
    return string;
}


khstring khAstComparisonExpressionType_string(khAstComparisonExpressionType type) {
    switch (type) {
        case khAstComparisonExpressionType_EQUAL:
            return khstring_new(U"equal");
        case khAstComparisonExpressionType_NOT_EQUAL:
            return khstring_new(U"not_equal");
        case khAstComparisonExpressionType_LESS:
            return khstring_new(U"less");
        case khAstComparisonExpressionType_MORE:
            return khstring_new(U"more");
        case khAstComparisonExpressionType_ELESS:
            return khstring_new(U"eless");
        case khAstComparisonExpressionType_EMORE:
            return khstring_new(U"emore");

        default:
            return khstring_new(U"unknown");
    }
}


khAstComparisonExpression khAstComparisonExpression_copy(khAstComparisonExpression* comparison_exp) {
    return (khAstComparisonExpression){
        .operations = kharray_copy(&comparison_exp->operations, NULL),
        .operands = kharray_copy(&comparison_exp->operands, khAstExpression_copy)};
}

void khAstComparisonExpression_delete(khAstComparisonExpression* comparison_exp) {
    kharray_delete(&comparison_exp->operations);
    kharray_delete(&comparison_exp->operands);
}

khstring khAstComparisonExpression_string(khAstComparisonExpression* comparison_exp, char32_t* origin) {
    khstring string = khstring_new(U"{\"operations\": [");
    for (size_t i = 0; i < kharray_size(&comparison_exp->operations); i++) {
        khstring operation_str = khAstComparisonExpressionType_string(comparison_exp->operations[i]);
        khstring quoted_operation = khstring_quote(&operation_str);
        khstring_concatenate(&string, &quoted_operation);
        khstring_delete(&operation_str);
        khstring_delete(&quoted_operation);

        if (i < kharray_size(&comparison_exp->operations) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"operands\": [");
    for (size_t i = 0; i < kharray_size(&comparison_exp->operands); i++) {
        khstring operand_str = khAstExpression_string(&comparison_exp->operands[i], origin);
        khstring_concatenate(&string, &operand_str);
        khstring_delete(&operand_str);

        if (i < kharray_size(&comparison_exp->operands) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstCallExpression khAstCallExpression_copy(khAstCallExpression* call_exp) {
    khAstExpression* callee = (khAstExpression*)malloc(sizeof(khAstExpression));
    *callee = khAstExpression_copy(call_exp->callee);

    return (khAstCallExpression){.callee = callee,
                                 .arguments = kharray_copy(&call_exp->arguments, khAstExpression_copy)};
}

void khAstCallExpression_delete(khAstCallExpression* call_exp) {
    khAstExpression_delete(call_exp->callee);
    free(call_exp->callee);
    kharray_delete(&call_exp->arguments);
}

khstring khAstCallExpression_string(khAstCallExpression* call_exp, char32_t* origin) {
    khstring string = khstring_new(U"{\"callee\": ");
    khstring callee_str = khAstExpression_string(call_exp->callee, origin);
    khstring_concatenate(&string, &callee_str);
    khstring_delete(&callee_str);

    khstring_concatenateCstring(&string, U", \"arguments\": [");
    for (size_t i = 0; i < kharray_size(&call_exp->arguments); i++) {
        khstring argument_str = khAstExpression_string(&call_exp->arguments[i], origin);
        khstring_concatenate(&string, &argument_str);
        khstring_delete(&argument_str);

        if (i < kharray_size(&call_exp->arguments) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstIndexExpression khAstIndexExpression_copy(khAstIndexExpression* index_exp) {
    khAstExpression* indexee = (khAstExpression*)malloc(sizeof(khAstExpression));
    *indexee = khAstExpression_copy(index_exp->indexee);

    return (khAstIndexExpression){
        .indexee = indexee, .arguments = kharray_copy(&index_exp->arguments, khAstExpression_copy)};
}

void khAstIndexExpression_delete(khAstIndexExpression* index_exp) {
    khAstExpression_delete(index_exp->indexee);
    free(index_exp->indexee);
    kharray_delete(&index_exp->arguments);
}

khstring khAstIndexExpression_string(khAstIndexExpression* index_exp, char32_t* origin) {
    khstring string = khstring_new(U"{\"indexee\": ");
    khstring indexee_str = khAstExpression_string(index_exp->indexee, origin);
    khstring_concatenate(&string, &indexee_str);
    khstring_delete(&indexee_str);

    khstring_concatenateCstring(&string, U", \"arguments\": [");
    for (size_t i = 0; i < kharray_size(&index_exp->arguments); i++) {
        khstring argument_str = khAstExpression_string(&index_exp->arguments[i], origin);
        khstring_concatenate(&string, &argument_str);
        khstring_delete(&argument_str);

        if (i < kharray_size(&index_exp->arguments) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


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

    return (khAstVariableDeclaration){.is_static = declaration->is_static,
                                      .is_wild = declaration->is_wild,
                                      .is_ref = declaration->is_ref,
                                      .name = khstring_copy(&declaration->name),
                                      .optional_type = optional_type,
                                      .optional_initializer = optional_initializer};
}

void khAstVariableDeclaration_delete(khAstVariableDeclaration* declaration) {
    khstring_delete(&declaration->name);
    if (declaration->optional_type != NULL) {
        khAstExpression_delete(declaration->optional_type);
        free(declaration->optional_type);
    }
    if (declaration->optional_initializer != NULL) {
        khAstExpression_delete(declaration->optional_initializer);
        free(declaration->optional_initializer);
    }
}

khstring khAstVariableDeclaration_string(khAstVariableDeclaration* declaration, char32_t* origin) {
    khstring string = khstring_new(U"{\"is_static\": ");
    khstring_concatenateCstring(&string, declaration->is_static ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"is_wild\": ");
    khstring_concatenateCstring(&string, declaration->is_wild ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"is_ref\": ");
    khstring_concatenateCstring(&string, declaration->is_ref ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"name\": ");
    khstring quoted_name = khstring_quote(&declaration->name);
    khstring_concatenate(&string, &quoted_name);
    khstring_delete(&quoted_name);

    khstring_concatenateCstring(&string, U", \"optional_type\": ");
    if (declaration->optional_type != NULL) {
        khstring optional_type_str = khAstExpression_string(declaration->optional_type, origin);
        khstring_concatenate(&string, &optional_type_str);
        khstring_delete(&optional_type_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"optional_initializer\": ");
    if (declaration->optional_initializer != NULL) {
        khstring optional_initializer_str =
            khAstExpression_string(declaration->optional_initializer, origin);
        khstring_concatenate(&string, &optional_initializer_str);
        khstring_delete(&optional_initializer_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U"}");
    return string;
}


khAstLambdaExpression khAstLambdaExpression_copy(khAstLambdaExpression* lambda) {
    khAstExpression* optional_variadic_argument = NULL;
    if (lambda->optional_variadic_argument != NULL) {
        optional_variadic_argument = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_variadic_argument = khAstExpression_copy(lambda->optional_variadic_argument);
    }

    khAstExpression* optional_return_type = NULL;
    if (lambda->optional_return_type != NULL) {
        optional_return_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_return_type = khAstExpression_copy(lambda->optional_return_type);
    }

    return (khAstLambdaExpression){.arguments = kharray_copy(&lambda->arguments, khAstExpression_copy),
                                   .optional_variadic_argument = optional_variadic_argument,
                                   .is_return_type_ref = lambda->is_return_type_ref,
                                   .optional_return_type = optional_return_type,
                                   .content = kharray_copy(&lambda->content, khAst_copy)};
}

void khAstLambdaExpression_delete(khAstLambdaExpression* lambda) {
    kharray_delete(&lambda->arguments);
    if (lambda->optional_variadic_argument != NULL) {
        khAstExpression_delete(lambda->optional_variadic_argument);
        free(lambda->optional_variadic_argument);
    }
    if (lambda->optional_return_type != NULL) {
        khAstExpression_delete(lambda->optional_return_type);
        free(lambda->optional_return_type);
    }
    kharray_delete(&lambda->content);
}

khstring khAstLambdaExpression_string(khAstLambdaExpression* lambda, char32_t* origin) {
    khstring string = khstring_new(U"{\"arguments\": [");
    for (size_t i = 0; i < kharray_size(&lambda->arguments); i++) {
        khstring argument_str = khAstExpression_string(&lambda->arguments[i], origin);
        khstring_concatenate(&string, &argument_str);
        khstring_delete(&argument_str);

        if (i < kharray_size(&lambda->arguments) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"optional_variadic_argument\": ");
    if (lambda->optional_variadic_argument != NULL) {
        khstring optional_variadic_argument_str =
            khAstExpression_string(lambda->optional_variadic_argument, origin);
        khstring_concatenate(&string, &optional_variadic_argument_str);
        khstring_delete(&optional_variadic_argument_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"is_return_type_ref\": ");
    khstring_concatenateCstring(&string, lambda->is_return_type_ref ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"optional_return_type\": ");
    if (lambda->optional_return_type != NULL) {
        khstring optional_return_type_str =
            khAstExpression_string(lambda->optional_return_type, origin);
        khstring_concatenate(&string, &optional_return_type_str);
        khstring_delete(&optional_return_type_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"content\": [");
    for (size_t i = 0; i < kharray_size(&lambda->content); i++) {
        khstring statement_str = khAst_string(&lambda->content[i], origin);
        khstring_concatenate(&string, &statement_str);
        khstring_delete(&statement_str);

        if (i < kharray_size(&lambda->content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstScopeExpression khAstScopeExpression_copy(khAstScopeExpression* scope_exp) {
    khAstExpression* value = (khAstExpression*)malloc(sizeof(khAstExpression));
    *value = khAstExpression_copy(scope_exp->value);

    kharray(khstring) scope_names = kharray_new(khstring, khstring_delete);
    for (size_t i = 0; i < kharray_size(&scope_exp->scope_names); i++) {
        kharray_append(&scope_names, khstring_copy(&scope_exp->scope_names[i]));
    }

    return (khAstScopeExpression){.value = value, .scope_names = scope_names};
}

void khAstScopeExpression_delete(khAstScopeExpression* scope_exp) {
    khAstExpression_delete(scope_exp->value);
    free(scope_exp->value);
    kharray_delete(&scope_exp->scope_names);
}

khstring khAstScopeExpression_string(khAstScopeExpression* scope_exp, char32_t* origin) {
    khstring string = khstring_new(U"{\"value\": ");
    khstring value_str = khAstExpression_string(scope_exp->value, origin);
    khstring_concatenate(&string, &value_str);
    khstring_delete(&value_str);

    khstring_concatenateCstring(&string, U", \"scope_names\": [");
    for (size_t i = 0; i < kharray_size(&scope_exp->scope_names); i++) {
        khstring quoted_scope_name = khstring_quote(&scope_exp->scope_names[i]);
        khstring_concatenate(&string, &quoted_scope_name);
        khstring_delete(&quoted_scope_name);

        if (i < kharray_size(&scope_exp->scope_names) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstFunctionTypeExpression
khAstFunctionTypeExpression_copy(khAstFunctionTypeExpression* function_type) {
    khAstExpression* optional_return_type = NULL;
    if (function_type->optional_return_type != NULL) {
        optional_return_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_return_type = khAstExpression_copy(function_type->optional_return_type);
    }

    return (khAstFunctionTypeExpression){
        .are_arguments_refs = kharray_copy(&function_type->are_arguments_refs, NULL),
        .argument_types = kharray_copy(&function_type->argument_types, khAstExpression_copy),
        .is_return_type_ref = function_type->is_return_type_ref,
        .optional_return_type = optional_return_type};
}

void khAstFunctionTypeExpression_delete(khAstFunctionTypeExpression* function_type) {
    kharray_delete(&function_type->are_arguments_refs);
    kharray_delete(&function_type->argument_types);

    if (function_type->optional_return_type != NULL) {
        khAstExpression_delete(function_type->optional_return_type);
        free(function_type->optional_return_type);
    }
}

khstring khAstFunctionTypeExpression_string(khAstFunctionTypeExpression* function_type,
                                            char32_t* origin) {
    khstring string = khstring_new(U"{\"are_arguments_refs\": [");
    for (size_t i = 0; i < kharray_size(&function_type->are_arguments_refs); i++) {
        khstring_concatenateCstring(&string, function_type->are_arguments_refs[i] ? U"true" : U"false");

        if (i < kharray_size(&function_type->are_arguments_refs) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"argument_types\": [");
    for (size_t i = 0; i < kharray_size(&function_type->argument_types); i++) {
        khstring argument_type_str = khAstExpression_string(&function_type->argument_types[i], origin);
        khstring_concatenate(&string, &argument_type_str);
        khstring_delete(&argument_type_str);

        if (i < kharray_size(&function_type->argument_types) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"is_return_type_ref\": ");
    khstring_concatenateCstring(&string, function_type->is_return_type_ref ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"optional_return_type\": ");
    khstring return_type_str = khAstExpression_string(function_type->optional_return_type, origin);
    khstring_concatenate(&string, &return_type_str);
    khstring_delete(&return_type_str);

    khstring_concatenateCstring(&string, U"}");
    return string;
}


khAstTemplatizeExpression khAstTemplatizeExpression_copy(khAstTemplatizeExpression* templatize_exp) {
    khAstExpression* value = (khAstExpression*)malloc(sizeof(khAstExpression));
    *value = khAstExpression_copy(templatize_exp->value);

    return (khAstTemplatizeExpression){
        .value = value,
        .template_arguments = kharray_copy(&templatize_exp->template_arguments, khAstExpression_copy)};
}

void khAstTemplatizeExpression_delete(khAstTemplatizeExpression* templatize_exp) {
    khAstExpression_delete(templatize_exp->value);
    free(templatize_exp->value);
    kharray_delete(&templatize_exp->template_arguments);
}

khstring khAstTemplatizeExpression_string(khAstTemplatizeExpression* templatize_exp, char32_t* origin) {
    khstring string = khstring_new(U"{\"value\": ");
    khstring value_str = khAstExpression_string(templatize_exp->value, origin);
    khstring_concatenate(&string, &value_str);
    khstring_delete(&value_str);

    khstring_concatenateCstring(&string, U", \"template_arguments\": [");
    for (size_t i = 0; i < kharray_size(&templatize_exp->template_arguments); i++) {
        khstring template_argument_str =
            khAstExpression_string(&templatize_exp->template_arguments[i], origin);
        khstring_concatenate(&string, &template_argument_str);
        khstring_delete(&template_argument_str);

        if (i < kharray_size(&templatize_exp->template_arguments) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstExpression khAstExpression_copy(khAstExpression* expression) {
    khAstExpression copy = *expression;

    switch (expression->type) {
        case khAstExpressionType_IDENTIFIER:
            copy.identifier = khstring_copy(&expression->identifier);
            break;
        case khAstExpressionType_STRING:
            copy.string = khstring_copy(&expression->string);
            break;
        case khAstExpressionType_BUFFER:
            copy.buffer = khbuffer_copy(&expression->buffer);
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
        case khAstExpressionType_FUNCTION_TYPE:
            copy.function_type = khAstFunctionTypeExpression_copy(&expression->function_type);
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
            khstring_delete(&expression->identifier);
            break;
        case khAstExpressionType_STRING:
            khstring_delete(&expression->string);
            break;
        case khAstExpressionType_BUFFER:
            khbuffer_delete(&expression->buffer);
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
        case khAstExpressionType_FUNCTION_TYPE:
            khAstFunctionTypeExpression_delete(&expression->function_type);
            break;
        case khAstExpressionType_TEMPLATIZE:
            khAstTemplatizeExpression_delete(&expression->templatize);

        default:
            break;
    }
}

khstring khAstExpression_string(khAstExpression* expression, char32_t* origin) {
    khstring string = khstring_new(U"{\"type\": ");
    khstring type_str = khAstExpressionType_string(expression->type);
    khstring quoted_type = khstring_quote(&type_str);
    khstring_concatenate(&string, &quoted_type);
    khstring_delete(&type_str);
    khstring_delete(&quoted_type);

    khstring_concatenateCstring(&string, U", \"begin\": ");
    if (expression->begin != NULL) {
        khstring begin_str = kh_uintToString(expression->begin - origin, 10);
        khstring_concatenate(&string, &begin_str);
        khstring_delete(&begin_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"end\": ");
    if (expression->end != NULL) {
        khstring end_str = kh_uintToString(expression->end - origin, 10);
        khstring_concatenate(&string, &end_str);
        khstring_delete(&end_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"value\": ");
    switch (expression->type) {
        case khAstExpressionType_IDENTIFIER: {
            khstring quoted_identfier = khstring_quote(&expression->identifier);
            khstring_concatenate(&string, &quoted_identfier);
            khstring_delete(&quoted_identfier);
        } break;
        case khAstExpressionType_CHAR: {
            khstring_append(&string, U'\"');
            khstring escaped_char = kh_escapeChar(expression->char_v);
            khstring_concatenate(&string, &escaped_char);
            khstring_delete(&escaped_char);
            khstring_append(&string, U'\"');
        } break;
        case khAstExpressionType_STRING: {
            khstring quoted_string = khstring_quote(&expression->string);
            khstring_concatenate(&string, &quoted_string);
            khstring_delete(&quoted_string);
        } break;
        case khAstExpressionType_BUFFER: {
            khstring quoted_buffer = khbuffer_quote(&expression->buffer);
            khstring_concatenate(&string, &quoted_buffer);
            khstring_delete(&quoted_buffer);
        } break;
        case khAstExpressionType_BYTE: {
            khstring byte_str = kh_uintToString(expression->byte, 10);
            khstring_concatenate(&string, &byte_str);
            khstring_delete(&byte_str);
        } break;
        case khAstExpressionType_INTEGER: {
            khstring integer_str = kh_intToString(expression->integer, 10);
            khstring_concatenate(&string, &integer_str);
            khstring_delete(&integer_str);
        } break;
        case khAstExpressionType_UINTEGER: {
            khstring uinteger_str = kh_uintToString(expression->uinteger, 10);
            khstring_concatenate(&string, &uinteger_str);
            khstring_delete(&uinteger_str);
        } break;
        case khAstExpressionType_FLOAT: {
            khstring float_str = kh_floatToString(expression->float_v, 8, 10);
            khstring_concatenate(&string, &float_str);
            khstring_delete(&float_str);
        } break;
        case khAstExpressionType_DOUBLE: {
            khstring double_str = kh_floatToString(expression->double_v, 16, 10);
            khstring_concatenate(&string, &double_str);
            khstring_delete(&double_str);
        } break;
        case khAstExpressionType_IFLOAT: {
            khstring ifloat_str = kh_floatToString(expression->ifloat, 8, 10);
            khstring_concatenate(&string, &ifloat_str);
            khstring_delete(&ifloat_str);
        } break;
        case khAstExpressionType_IDOUBLE: {
            khstring idouble_str = kh_floatToString(expression->idouble, 16, 10);
            khstring_concatenate(&string, &idouble_str);
            khstring_delete(&idouble_str);
        } break;

        case khAstExpressionType_TUPLE: {
            khstring tuple_str = khAstTuple_string(&expression->tuple, origin);
            khstring_concatenate(&string, &tuple_str);
            khstring_delete(&tuple_str);
        } break;
        case khAstExpressionType_ARRAY: {
            khstring array_str = khAstArray_string(&expression->array, origin);
            khstring_concatenate(&string, &array_str);
            khstring_delete(&array_str);
        } break;
        case khAstExpressionType_DICT: {
            khstring dict_str = khAstDict_string(&expression->dict, origin);
            khstring_concatenate(&string, &dict_str);
            khstring_delete(&dict_str);
        } break;

        case khAstExpressionType_UNARY: {
            khstring unary_str = khAstUnaryExpression_string(&expression->unary, origin);
            khstring_concatenate(&string, &unary_str);
            khstring_delete(&unary_str);
        } break;
        case khAstExpressionType_BINARY: {
            khstring binary_str = khAstBinaryExpression_string(&expression->binary, origin);
            khstring_concatenate(&string, &binary_str);
            khstring_delete(&binary_str);
        } break;
        case khAstExpressionType_TERNARY: {
            khstring ternary_str = khAstTernaryExpression_string(&expression->ternary, origin);
            khstring_concatenate(&string, &ternary_str);
            khstring_delete(&ternary_str);
        } break;
        case khAstExpressionType_COMPARISON: {
            khstring comparison_str = khAstComparisonExpression_string(&expression->comparison, origin);
            khstring_concatenate(&string, &comparison_str);
            khstring_delete(&comparison_str);
        } break;
        case khAstExpressionType_CALL: {
            khstring call_str = khAstCallExpression_string(&expression->call, origin);
            khstring_concatenate(&string, &call_str);
            khstring_delete(&call_str);
        } break;
        case khAstExpressionType_INDEX: {
            khstring index_str = khAstIndexExpression_string(&expression->index, origin);
            khstring_concatenate(&string, &index_str);
            khstring_delete(&index_str);
        } break;

        case khAstExpressionType_VARIABLE_DECLARATION: {
            khstring declaration_str =
                khAstVariableDeclaration_string(&expression->variable_declaration, origin);
            khstring_concatenate(&string, &declaration_str);
            khstring_delete(&declaration_str);
        } break;
        case khAstExpressionType_LAMBDA: {
            khstring lambda_str = khAstLambdaExpression_string(&expression->lambda, origin);
            khstring_concatenate(&string, &lambda_str);
            khstring_delete(&lambda_str);
        } break;
        case khAstExpressionType_SCOPE: {
            khstring scope_str = khAstScopeExpression_string(&expression->scope, origin);
            khstring_concatenate(&string, &scope_str);
            khstring_delete(&scope_str);
        } break;
        case khAstExpressionType_FUNCTION_TYPE: {
            khstring function_type_str =
                khAstFunctionTypeExpression_string(&expression->function_type, origin);
            khstring_concatenate(&string, &function_type_str);
            khstring_delete(&function_type_str);
        } break;
        case khAstExpressionType_TEMPLATIZE: {
            khstring templatize_str = khAstTemplatizeExpression_string(&expression->templatize, origin);
            khstring_concatenate(&string, &templatize_str);
            khstring_delete(&templatize_str);
        } break;

        default:
            khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U"}");
    return string;
}


khAstImport khAstImport_copy(khAstImport* import_v) {
    kharray(khstring) path = kharray_new(khstring, khstring_delete);
    for (size_t i = 0; i < kharray_size(&import_v->path); i++) {
        kharray_append(&path, khstring_copy(&import_v->path[i]));
    }

    khstring* optional_alias = NULL;
    if (import_v->optional_alias) {
        optional_alias = (khstring*)malloc(sizeof(khstring));
        *optional_alias = khstring_copy(import_v->optional_alias);
    }

    return (khAstImport){
        .path = path, .relative = import_v->relative, .optional_alias = optional_alias};
}

void khAstImport_delete(khAstImport* import_v) {
    kharray_delete(&import_v->path);
    if (import_v->optional_alias != NULL) {
        khstring_delete(import_v->optional_alias);
        free(import_v->optional_alias);
    }
}

khstring khAstImport_string(khAstImport* import_v, char32_t* origin) {
    khstring string = khstring_new(U"{\"path\": [");
    for (size_t i = 0; i < kharray_size(&import_v->path); i++) {
        khstring quoted_path = khstring_quote(&import_v->path[i]);
        khstring_concatenate(&string, &quoted_path);
        khstring_delete(&quoted_path);

        if (i < kharray_size(&import_v->path) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"relative\": ");
    khstring_concatenateCstring(&string, import_v->relative ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"optional_alias\": ");
    if (import_v->optional_alias != NULL) {
        khstring quoted_alias = khstring_quote(import_v->optional_alias);
        khstring_concatenate(&string, &quoted_alias);
        khstring_delete(&quoted_alias);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U"}");
    return string;
}


khAstInclude khAstInclude_copy(khAstInclude* include) {
    kharray(khstring) path = kharray_new(khstring, khstring_delete);
    for (size_t i = 0; i < kharray_size(&include->path); i++) {
        kharray_append(&path, khstring_copy(&include->path[i]));
    }

    return (khAstInclude){.path = path, .relative = include->relative};
}

void khAstInclude_delete(khAstInclude* include) {
    kharray_delete(&include->path);
}

khstring khAstInclude_string(khAstInclude* include, char32_t* origin) {
    khstring string = khstring_new(U"{\"path\": [");
    for (size_t i = 0; i < kharray_size(&include->path); i++) {
        khstring quoted_path = khstring_quote(&include->path[i]);
        khstring_concatenate(&string, &quoted_path);
        khstring_delete(&quoted_path);

        if (i < kharray_size(&include->path) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"relative\": ");
    khstring_concatenateCstring(&string, include->relative ? U"true" : U"false");

    khstring_concatenateCstring(&string, U"}");
    return string;
}


khAstFunction khAstFunction_copy(khAstFunction* function) {
    khAstExpression* optional_variadic_argument = NULL;
    if (function->optional_variadic_argument != NULL) {
        optional_variadic_argument = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_variadic_argument = khAstExpression_copy(function->optional_variadic_argument);
    }

    khAstExpression* optional_return_type = NULL;
    if (function->optional_return_type != NULL) {
        optional_return_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_return_type = khAstExpression_copy(function->optional_return_type);
    }

    return (khAstFunction){.is_incase = function->is_incase,
                           .is_static = function->is_static,
                           .name_point = khAstExpression_copy(&function->name_point),
                           .arguments = kharray_copy(&function->arguments, khAstExpression_copy),
                           .optional_variadic_argument = optional_variadic_argument,
                           .is_return_type_ref = function->is_return_type_ref,
                           .optional_return_type = optional_return_type,
                           .content = kharray_copy(&function->content, khAst_copy)};
}

void khAstFunction_delete(khAstFunction* function) {
    khAstExpression_delete(&function->name_point);
    kharray_delete(&function->arguments);
    if (function->optional_variadic_argument != NULL) {
        khAstExpression_delete(function->optional_variadic_argument);
        free(function->optional_variadic_argument);
    }
    if (function->optional_return_type != NULL) {
        khAstExpression_delete(function->optional_return_type);
        free(function->optional_return_type);
    }
    kharray_delete(&function->content);
}

khstring khAstFunction_string(khAstFunction* function, char32_t* origin) {
    khstring string = khstring_new(U"{\"is_incase\": ");
    khstring_concatenateCstring(&string, function->is_incase ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"is_static\": ");
    khstring_concatenateCstring(&string, function->is_static ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"name_point\": ");
    khstring name_point_str = khAstExpression_string(&function->name_point, origin);
    khstring_concatenate(&string, &name_point_str);
    khstring_delete(&name_point_str);

    khstring_concatenateCstring(&string, U", \"arguments\": [");
    for (size_t i = 0; i < kharray_size(&function->arguments); i++) {
        khstring argument_str = khAstExpression_string(&function->arguments[i], origin);
        khstring_concatenate(&string, &argument_str);
        khstring_delete(&argument_str);

        if (i < kharray_size(&function->arguments) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"optional_variadic_argument\": ");
    if (function->optional_variadic_argument != NULL) {
        khstring argument_str = khAstExpression_string(function->optional_variadic_argument, origin);
        khstring_concatenate(&string, &argument_str);
        khstring_delete(&argument_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"is_return_type_ref\": ");
    khstring_concatenateCstring(&string, function->is_return_type_ref ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"optional_return_type\": ");
    if (function->optional_return_type != NULL) {
        khstring argument_str = khAstExpression_string(function->optional_return_type, origin);
        khstring_concatenate(&string, &argument_str);
        khstring_delete(&argument_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"content\": [");
    for (size_t i = 0; i < kharray_size(&function->content); i++) {
        khstring content_str = khAst_string(&function->content[i], origin);
        khstring_concatenate(&string, &content_str);
        khstring_delete(&content_str);

        if (i < kharray_size(&function->content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstClass khAstClass_copy(khAstClass* class_v) {
    kharray(khstring) template_arguments = kharray_new(khstring, khstring_delete);
    for (size_t i = 0; i < kharray_size(&class_v->template_arguments); i++) {
        kharray_append(&template_arguments, khstring_copy(&class_v->template_arguments[i]));
    }

    khAstExpression* optional_base_type = NULL;
    if (class_v->optional_base_type != NULL) {
        optional_base_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_base_type = khAstExpression_copy(class_v->optional_base_type);
    }

    return (khAstClass){.is_incase = class_v->is_incase,
                        .name = khstring_copy(&class_v->name),
                        .template_arguments = template_arguments,
                        .optional_base_type = optional_base_type,
                        .content = kharray_copy(&class_v->content, khAst_copy)};
}

void khAstClass_delete(khAstClass* class_v) {
    khstring_delete(&class_v->name);
    kharray_delete(&class_v->template_arguments);
    if (class_v->optional_base_type != NULL) {
        khAstExpression_delete(class_v->optional_base_type);
        free(class_v->optional_base_type);
    }
    kharray_delete(&class_v->content);
}

khstring khAstClass_string(khAstClass* class_v, char32_t* origin) {
    khstring string = khstring_new(U"{\"is_incase\": ");
    khstring_concatenateCstring(&string, class_v->is_incase ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"name\": ");
    khstring quoted_name = khstring_quote(&class_v->name);
    khstring_concatenate(&string, &quoted_name);
    khstring_delete(&quoted_name);

    khstring_concatenateCstring(&string, U", \"template_arguments\": [");
    for (size_t i = 0; i < kharray_size(&class_v->template_arguments); i++) {
        khstring quoted_argument = khstring_quote(&class_v->template_arguments[i]);
        khstring_concatenate(&string, &quoted_argument);
        khstring_delete(&quoted_argument);

        if (i < kharray_size(&class_v->template_arguments) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"optional_base_type\": ");
    if (class_v->optional_base_type != NULL) {
        khstring base_type_str = khAstExpression_string(class_v->optional_base_type, origin);
        khstring_concatenate(&string, &base_type_str);
        khstring_delete(&base_type_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"content\": [");
    for (size_t i = 0; i < kharray_size(&class_v->content); i++) {
        khstring content_str = khAst_string(&class_v->content[i], origin);
        khstring_concatenate(&string, &content_str);
        khstring_delete(&content_str);

        if (i < kharray_size(&class_v->content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstStruct khAstStruct_copy(khAstStruct* struct_v) {
    kharray(khstring) template_arguments = kharray_new(khstring, khstring_delete);
    for (size_t i = 0; i < kharray_size(&struct_v->template_arguments); i++) {
        kharray_append(&template_arguments, khstring_copy(&struct_v->template_arguments[i]));
    }

    khAstExpression* optional_base_type = NULL;
    if (struct_v->optional_base_type != NULL) {
        optional_base_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_base_type = khAstExpression_copy(struct_v->optional_base_type);
    }

    return (khAstStruct){.is_incase = struct_v->is_incase,
                         .name = khstring_copy(&struct_v->name),
                         .template_arguments = template_arguments,
                         .optional_base_type = optional_base_type,
                         .content = kharray_copy(&struct_v->content, khAst_copy)};
}

void khAstStruct_delete(khAstStruct* struct_v) {
    khstring_delete(&struct_v->name);
    kharray_delete(&struct_v->template_arguments);
    if (struct_v->optional_base_type != NULL) {
        khAstExpression_delete(struct_v->optional_base_type);
        free(struct_v->optional_base_type);
    }
    kharray_delete(&struct_v->content);
}

khstring khAstStruct_string(khAstStruct* struct_v, char32_t* origin) {
    khstring string = khstring_new(U"{\"is_incase\": ");
    khstring_concatenateCstring(&string, struct_v->is_incase ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"name\": ");
    khstring quoted_name = khstring_quote(&struct_v->name);
    khstring_concatenate(&string, &quoted_name);
    khstring_delete(&quoted_name);

    khstring_concatenateCstring(&string, U", \"template_arguments\": [");
    for (size_t i = 0; i < kharray_size(&struct_v->template_arguments); i++) {
        khstring quoted_argument = khstring_quote(&struct_v->template_arguments[i]);
        khstring_concatenate(&string, &quoted_argument);
        khstring_delete(&quoted_argument);

        if (i < kharray_size(&struct_v->template_arguments) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"optional_base_type\": ");
    if (struct_v->optional_base_type != NULL) {
        khstring base_type_str = khAstExpression_string(struct_v->optional_base_type, origin);
        khstring_concatenate(&string, &base_type_str);
        khstring_delete(&base_type_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"content\": [");
    for (size_t i = 0; i < kharray_size(&struct_v->content); i++) {
        khstring content_str = khAst_string(&struct_v->content[i], origin);
        khstring_concatenate(&string, &content_str);
        khstring_delete(&content_str);

        if (i < kharray_size(&struct_v->content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstEnum khAstEnum_copy(khAstEnum* enum_v) {
    kharray(khstring) members = kharray_new(khstring, khstring_delete);
    for (size_t i = 0; i < kharray_size(&enum_v->members); i++) {
        kharray_append(&members, khstring_copy(&enum_v->members[i]));
    }

    return (khAstEnum){.name = khstring_copy(&enum_v->name), .members = members};
}

void khAstEnum_delete(khAstEnum* enum_v) {
    khstring_delete(&enum_v->name);
    kharray_delete(&enum_v->members);
}

khstring khAstEnum_string(khAstEnum* enum_v, char32_t* origin) {
    khstring string = khstring_new(U"{\"name\": ");
    khstring quoted_name = khstring_quote(&enum_v->name);
    khstring_concatenate(&string, &quoted_name);
    khstring_delete(&quoted_name);

    khstring_concatenateCstring(&string, U", \"members\": [");
    for (size_t i = 0; i < kharray_size(&enum_v->members); i++) {
        khstring quoted_member = khstring_quote(&enum_v->members[i]);
        khstring_concatenate(&string, &quoted_member);
        khstring_delete(&quoted_member);

        if (i < kharray_size(&enum_v->members) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstAlias khAstAlias_copy(khAstAlias* alias) {
    return (khAstAlias){.is_incase = alias->is_incase,
                        .name = khstring_copy(&alias->name),
                        .expression = khAstExpression_copy(&alias->expression)};
}

void khAstAlias_delete(khAstAlias* alias) {
    khstring_delete(&alias->name);
    khAstExpression_delete(&alias->expression);
}

khstring khAstAlias_string(khAstAlias* alias, char32_t* origin) {
    khstring string = khstring_new(U"{\"is_incase\": ");
    khstring_concatenateCstring(&string, alias->is_incase ? U"true" : U"false");

    khstring_concatenateCstring(&string, U", \"name\": ");
    khstring quoted_name = khstring_quote(&alias->name);
    khstring_concatenate(&string, &quoted_name);
    khstring_delete(&quoted_name);

    khstring_concatenateCstring(&string, U", \"expression\": ");
    khstring expression_str = khAstExpression_string(&alias->expression, origin);
    khstring_concatenate(&string, &expression_str);
    khstring_delete(&expression_str);

    khstring_concatenateCstring(&string, U"}");
    return string;
}


khAstIfBranch khAstIfBranch_copy(khAstIfBranch* if_branch) {
    kharray(kharray(khAst)) branch_contents = kharray_new(kharray(khAst), khstring_delete);
    for (size_t i = 0; i < kharray_size(&if_branch->branch_contents); i++) {
        kharray_append(&branch_contents, kharray_copy(&if_branch->branch_contents[i], khAst_copy));
    }

    return (khAstIfBranch){.branch_conditions =
                               kharray_copy(&if_branch->branch_conditions, khAstExpression_copy),
                           .branch_contents = branch_contents,
                           .else_content = kharray_copy(&if_branch->else_content, khAst_copy)};
}

void khAstIfBranch_delete(khAstIfBranch* if_branch) {
    kharray_delete(&if_branch->branch_conditions);
    kharray_delete(&if_branch->branch_contents);
    kharray_delete(&if_branch->else_content);
}

khstring khAstIfBranch_string(khAstIfBranch* if_branch, char32_t* origin) {
    khstring string = khstring_new(U"{\"branch_conditions\": [");
    for (size_t i = 0; i < kharray_size(&if_branch->branch_conditions); i++) {
        khstring branch_condition_str =
            khAstExpression_string(&if_branch->branch_conditions[i], origin);
        khstring_concatenate(&string, &branch_condition_str);
        khstring_delete(&branch_condition_str);

        if (i < kharray_size(&if_branch->branch_conditions) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"branch_contents\": [");
    for (size_t i = 0; i < kharray_size(&if_branch->branch_contents); i++) {
        khstring_append(&string, U'[');
        for (size_t j = 0; j < kharray_size(&if_branch->branch_contents[i]); j++) {
            khstring branch_content_str = khAst_string(&if_branch->branch_contents[i][j], origin);
            khstring_concatenate(&string, &branch_content_str);
            khstring_delete(&branch_content_str);

            if (j < kharray_size(&if_branch->branch_contents[i]) - 1) {
                khstring_concatenateCstring(&string, U", ");
            }
        }
        khstring_append(&string, U']');

        if (i < kharray_size(&if_branch->branch_contents) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"else_content\": [");
    for (size_t i = 0; i < kharray_size(&if_branch->else_content); i++) {
        khstring else_content_str = khAst_string(&if_branch->else_content[i], origin);
        khstring_concatenate(&string, &else_content_str);
        khstring_delete(&else_content_str);

        if (i < kharray_size(&if_branch->else_content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstWhileLoop khAstWhileLoop_copy(khAstWhileLoop* while_loop) {
    return (khAstWhileLoop){.condition = khAstExpression_copy(&while_loop->condition),
                            .content = kharray_copy(&while_loop->content, khAst_copy)};
}

void khAstWhileLoop_delete(khAstWhileLoop* while_loop) {
    khAstExpression_delete(&while_loop->condition);
    kharray_delete(&while_loop->content);
}

khstring khAstWhileLoop_string(khAstWhileLoop* while_loop, char32_t* origin) {
    khstring string = khstring_new(U"{\"condition\": ");
    khstring condition_str = khAstExpression_string(&while_loop->condition, origin);
    khstring_concatenate(&string, &condition_str);
    khstring_delete(&condition_str);

    khstring_concatenateCstring(&string, U", \"content\": [");
    for (size_t i = 0; i < kharray_size(&while_loop->content); i++) {
        khstring content_str = khAst_string(&while_loop->content[i], origin);
        khstring_concatenate(&string, &content_str);
        khstring_delete(&content_str);

        if (i < kharray_size(&while_loop->content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstDoWhileLoop khAstDoWhileLoop_copy(khAstDoWhileLoop* do_while_loop) {
    return (khAstDoWhileLoop){.condition = khAstExpression_copy(&do_while_loop->condition),
                              .content = kharray_copy(&do_while_loop->content, khAst_copy)};
}

void khAstDoWhileLoop_delete(khAstDoWhileLoop* do_while_loop) {
    khAstExpression_delete(&do_while_loop->condition);
    kharray_delete(&do_while_loop->content);
}

khstring khAstDoWhileLoop_string(khAstDoWhileLoop* do_while_loop, char32_t* origin) {
    khstring string = khstring_new(U"{\"condition\": ");
    khstring condition_str = khAstExpression_string(&do_while_loop->condition, origin);
    khstring_concatenate(&string, &condition_str);
    khstring_delete(&condition_str);

    khstring_concatenateCstring(&string, U", \"content\": [");
    for (size_t i = 0; i < kharray_size(&do_while_loop->content); i++) {
        khstring content_str = khAst_string(&do_while_loop->content[i], origin);
        khstring_concatenate(&string, &content_str);
        khstring_delete(&content_str);

        if (i < kharray_size(&do_while_loop->content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstForLoop khAstForLoop_copy(khAstForLoop* for_loop) {
    return (khAstForLoop){.initial_expression = khAstExpression_copy(&for_loop->initial_expression),
                          .loop_condition = khAstExpression_copy(&for_loop->loop_condition),
                          .update_expression = khAstExpression_copy(&for_loop->update_expression),
                          .content = kharray_copy(&for_loop->content, khAst_copy)};
}

void khAstForLoop_delete(khAstForLoop* for_loop) {
    khAstExpression_delete(&for_loop->initial_expression);
    khAstExpression_delete(&for_loop->loop_condition);
    khAstExpression_delete(&for_loop->update_expression);
    kharray_delete(&for_loop->content);
}

khstring khAstForLoop_string(khAstForLoop* for_loop, char32_t* origin) {
    khstring string = khstring_new(U"{\"initial_expression\": ");
    khstring initial_expression_str = khAstExpression_string(&for_loop->initial_expression, origin);
    khstring_concatenate(&string, &initial_expression_str);
    khstring_delete(&initial_expression_str);

    khstring_concatenateCstring(&string, U", \"loop_condition\": ");
    khstring loop_condition_str = khAstExpression_string(&for_loop->loop_condition, origin);
    khstring_concatenate(&string, &loop_condition_str);
    khstring_delete(&loop_condition_str);

    khstring_concatenateCstring(&string, U", \"update_expression\": ");
    khstring update_expression_str = khAstExpression_string(&for_loop->update_expression, origin);
    khstring_concatenate(&string, &update_expression_str);
    khstring_delete(&update_expression_str);

    khstring_concatenateCstring(&string, U", \"content\": [");
    for (size_t i = 0; i < kharray_size(&for_loop->content); i++) {
        khstring content_str = khAst_string(&for_loop->content[i], origin);
        khstring_concatenate(&string, &content_str);
        khstring_delete(&content_str);

        if (i < kharray_size(&for_loop->content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstForEachLoop khAstForEachLoop_copy(khAstForEachLoop* for_each_loop) {
    return (khAstForEachLoop){.iterators =
                                  kharray_copy(&for_each_loop->iterators, khAstExpression_copy),
                              .iteratee = khAstExpression_copy(&for_each_loop->iteratee),
                              .content = kharray_copy(&for_each_loop->content, khAst_copy)};
}

void khAstForEachLoop_delete(khAstForEachLoop* for_each_loop) {
    kharray_delete(&for_each_loop->iterators);
    khAstExpression_delete(&for_each_loop->iteratee);
    kharray_delete(&for_each_loop->content);
}

khstring khAstForEachLoop_string(khAstForEachLoop* for_each_loop, char32_t* origin) {
    khstring string = khstring_new(U"{\"iterators\": [");
    for (size_t i = 0; i < kharray_size(&for_each_loop->iterators); i++) {
        khstring iterators_str = khAstExpression_string(&for_each_loop->iterators[i], origin);
        khstring_concatenate(&string, &iterators_str);
        khstring_delete(&iterators_str);

        if (i < kharray_size(&for_each_loop->iterators) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"], \"iteratee\": ");
    khstring iteratee_str = khAstExpression_string(&for_each_loop->iteratee, origin);
    khstring_concatenate(&string, &iteratee_str);
    khstring_delete(&iteratee_str);

    khstring_concatenateCstring(&string, U", \"content\": [");
    for (size_t i = 0; i < kharray_size(&for_each_loop->content); i++) {
        khstring content_str = khAst_string(&for_each_loop->content[i], origin);
        khstring_concatenate(&string, &content_str);
        khstring_delete(&content_str);

        if (i < kharray_size(&for_each_loop->content) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


khAstReturn khAstReturn_copy(khAstReturn* return_v) {
    return (khAstReturn){.values = kharray_copy(&return_v->values, khAstExpression_copy)};
}

void khAstReturn_delete(khAstReturn* return_v) {
    kharray_delete(&return_v->values);
}

khstring khAstReturn_string(khAstReturn* return_v, char32_t* origin) {
    khstring string = khstring_new(U"{\"values\": [");
    for (size_t i = 0; i < kharray_size(&return_v->values); i++) {
        khstring values_str = khAstExpression_string(&return_v->values[i], origin);
        khstring_concatenate(&string, &values_str);
        khstring_delete(&values_str);

        if (i < kharray_size(&return_v->values) - 1) {
            khstring_concatenateCstring(&string, U", ");
        }
    }

    khstring_concatenateCstring(&string, U"]}");
    return string;
}


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
        case khAstType_RETURN:
            khAstReturn_delete(&ast->return_v);
            break;

        default:
            break;
    }
}

khstring khAst_string(khAst* ast, char32_t* origin) {
    khstring string = khstring_new(U"{\"type\": ");
    khstring type_str = khAstType_string(ast->type);
    khstring quoted_type = khstring_quote(&type_str);
    khstring_concatenate(&string, &quoted_type);
    khstring_delete(&type_str);
    khstring_delete(&quoted_type);

    khstring_concatenateCstring(&string, U", \"begin\": ");
    if (ast->begin != NULL) {
        khstring begin_str = kh_uintToString(ast->begin - origin, 10);
        khstring_concatenate(&string, &begin_str);
        khstring_delete(&begin_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"end\": ");
    if (ast->end != NULL) {
        khstring end_str = kh_uintToString(ast->end - origin, 10);
        khstring_concatenate(&string, &end_str);
        khstring_delete(&end_str);
    }
    else {
        khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U", \"value\": ");
    switch (ast->type) {
        case khAstType_EXPRESSION: {
            khstring expression_str = khAstExpression_string(&ast->expression, origin);
            khstring_concatenate(&string, &expression_str);
            khstring_delete(&expression_str);
        } break;

        case khAstType_IMPORT: {
            khstring import_str = khAstImport_string(&ast->import_v, origin);
            khstring_concatenate(&string, &import_str);
            khstring_delete(&import_str);
        } break;
        case khAstType_INCLUDE: {
            khstring include_str = khAstInclude_string(&ast->include, origin);
            khstring_concatenate(&string, &include_str);
            khstring_delete(&include_str);
        } break;
        case khAstType_FUNCTION: {
            khstring function_str = khAstFunction_string(&ast->function, origin);
            khstring_concatenate(&string, &function_str);
            khstring_delete(&function_str);
        } break;
        case khAstType_CLASS: {
            khstring class_str = khAstClass_string(&ast->class_v, origin);
            khstring_concatenate(&string, &class_str);
            khstring_delete(&class_str);
        } break;
        case khAstType_STRUCT: {
            khstring struct_str = khAstStruct_string(&ast->struct_v, origin);
            khstring_concatenate(&string, &struct_str);
            khstring_delete(&struct_str);
        } break;
        case khAstType_ENUM: {
            khstring enum_str = khAstEnum_string(&ast->enum_v, origin);
            khstring_concatenate(&string, &enum_str);
            khstring_delete(&enum_str);
        } break;
        case khAstType_ALIAS: {
            khstring alias_str = khAstAlias_string(&ast->alias, origin);
            khstring_concatenate(&string, &alias_str);
            khstring_delete(&alias_str);
        } break;

        case khAstType_IF_BRANCH: {
            khstring if_branch_str = khAstIfBranch_string(&ast->if_branch, origin);
            khstring_concatenate(&string, &if_branch_str);
            khstring_delete(&if_branch_str);
        } break;
        case khAstType_WHILE_LOOP: {
            khstring while_loop_str = khAstWhileLoop_string(&ast->while_loop, origin);
            khstring_concatenate(&string, &while_loop_str);
            khstring_delete(&while_loop_str);
        } break;
        case khAstType_DO_WHILE_LOOP: {
            khstring do_while_loop_str = khAstDoWhileLoop_string(&ast->do_while_loop, origin);
            khstring_concatenate(&string, &do_while_loop_str);
            khstring_delete(&do_while_loop_str);
        } break;
        case khAstType_FOR_LOOP: {
            khstring for_loop_str = khAstForLoop_string(&ast->for_loop, origin);
            khstring_concatenate(&string, &for_loop_str);
            khstring_delete(&for_loop_str);
        } break;
        case khAstType_FOR_EACH_LOOP: {
            khstring for_each_loop_str = khAstForEachLoop_string(&ast->for_each_loop, origin);
            khstring_concatenate(&string, &for_each_loop_str);
            khstring_delete(&for_each_loop_str);
        } break;
        case khAstType_RETURN: {
            khstring return_str = khAstReturn_string(&ast->return_v, origin);
            khstring_concatenate(&string, &return_str);
            khstring_delete(&return_str);
        } break;

        default:
            khstring_concatenateCstring(&string, U"null");
    }

    khstring_concatenateCstring(&string, U"}");
    return string;
}
