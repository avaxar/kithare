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
        case khAstType_INVALID:
            return kh_string(U"invalid");

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
        case khAstExpressionType_INVALID:
            return kh_string(U"invalid");

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

khArray(char32_t) khAstTuple_string(khAstTuple* tuple) {
    khArray(char32_t) string = kh_string(U"(");

    for (size_t i = 0; i < khArray_size(&tuple->values); i++) {
        khArray(char32_t) value_str = khAstExpression_string(&tuple->values[i]);
        khArray_concatenate(&string, &value_str, NULL);
        khArray_delete(&value_str);

        if (i < khArray_size(&tuple->values) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    khArray_append(&string, U')');
    return string;
}


khAstArray khAstArray_copy(khAstArray* array) {
    return (khAstArray){.values = khArray_copy(&array->values, khAstExpression_copy)};
}

void khAstArray_delete(khAstArray* array) {
    khArray_delete(&array->values);
}

khArray(char32_t) khAstArray_string(khAstArray* array) {
    khArray(char32_t) string = kh_string(U"(");

    for (size_t i = 0; i < khArray_size(&array->values); i++) {
        khArray(char32_t) value_str = khAstExpression_string(&array->values[i]);
        khArray_concatenate(&string, &value_str, NULL);
        khArray_delete(&value_str);

        if (i < khArray_size(&array->values) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    khArray_append(&string, U')');
    return string;
}


khAstDict khAstDict_copy(khAstDict* dict) {
    return (khAstDict){.keys = khArray_copy(&dict->keys, khAstExpression_copy),
                       .values = khArray_copy(&dict->values, khAstExpression_copy)};
}

void khAstDict_delete(khAstDict* dict) {
    khArray_delete(&dict->keys);
    khArray_delete(&dict->values);
}

khArray(char32_t) khAstDict_string(khAstDict* dict) {
    khArray(char32_t) string = kh_string(U"(");

    for (size_t i = 0; i < khArray_size(&dict->keys); i++) {
        khArray_append(&string, U'(');

        khArray(char32_t) key_str = khAstExpression_string(&dict->keys[i]);
        khArray_concatenate(&string, &key_str, NULL);
        khArray_delete(&key_str);

        kh_appendCstring(&string, U", ");

        khArray(char32_t) value_str = khAstExpression_string(&dict->values[i]);
        khArray_concatenate(&string, &value_str, NULL);
        khArray_delete(&value_str);

        khArray_append(&string, U')');
        if (i < khArray_size(&dict->keys) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    khArray_append(&string, U')');
    return string;
}


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

khArray(char32_t) khAstUnaryExpression_string(khAstUnaryExpression* unary_exp) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) operand_str = khAstExpression_string(unary_exp->operand);
    khArray_concatenate(&string, &operand_str, NULL);
    khArray_delete(&operand_str);

    khArray_append(&string, U')');
    return string;
}


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

khArray(char32_t) khAstBinaryExpression_string(khAstBinaryExpression* binary_exp) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) left_str = khAstExpression_string(binary_exp->left);
    khArray_concatenate(&string, &left_str, NULL);
    khArray_delete(&left_str);

    kh_appendCstring(&string, U", ");

    khArray(char32_t) right_str = khAstExpression_string(binary_exp->right);
    khArray_concatenate(&string, &right_str, NULL);
    khArray_delete(&right_str);

    khArray_append(&string, U')');
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
    khAstExpression_delete(ternary_exp->value);
    khAstExpression_delete(ternary_exp->otherwise);
}

khArray(char32_t) khAstTernaryExpression_string(khAstTernaryExpression* ternary_exp) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) condition_str = khAstExpression_string(ternary_exp->condition);
    khArray_concatenate(&string, &condition_str, NULL);
    khArray_delete(&condition_str);

    kh_appendCstring(&string, U", ");

    khArray(char32_t) value_str = khAstExpression_string(ternary_exp->value);
    khArray_concatenate(&string, &value_str, NULL);
    khArray_delete(&value_str);

    kh_appendCstring(&string, U", ");

    khArray(char32_t) otherwise_str = khAstExpression_string(ternary_exp->otherwise);
    khArray_concatenate(&string, &otherwise_str, NULL);
    khArray_delete(&otherwise_str);

    khArray_append(&string, U')');
    return string;
}


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

khArray(char32_t) khAstComparisonExpression_string(khAstComparisonExpression* comparison_exp) {
    khArray(char32_t) string = kh_string(U"(");

    for (size_t i = 0; i < khArray_size(&comparison_exp->operations); i++) {
        khArray(char32_t) operand_str = khAstExpression_string(&comparison_exp->operands[i]);
        khArray_concatenate(&string, &operand_str, NULL);
        khArray_delete(&operand_str);

        kh_appendCstring(&string, U", ");

        khArray(char32_t) operation_str =
            khAstComparisonExpressionType_string(comparison_exp->operations[i]);
        khArray_concatenate(&string, &operation_str, NULL);
        khArray_delete(&operation_str);

        kh_appendCstring(&string, U", ");
    }

    if (khArray_size(&comparison_exp->operands) > 0) {
        khArray(char32_t) operand_str = khAstExpression_string(
            &comparison_exp->operands[khArray_size(&comparison_exp->operations)]);
        khArray_concatenate(&string, &operand_str, NULL);
        khArray_delete(&operand_str);
    }

    khArray_append(&string, U')');
    return string;
}


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

khArray(char32_t) khAstCallExpression_string(khAstCallExpression* call_exp) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) callee_str = khAstExpression_string(call_exp->callee);
    khArray_concatenate(&string, &callee_str, NULL);
    khArray_delete(&callee_str);

    kh_appendCstring(&string, U", ");

    khArray_append(&string, U'(');
    for (size_t i = 0; i < khArray_size(&call_exp->arguments); i++) {
        khArray(char32_t) argument_str = khAstExpression_string(&call_exp->arguments[i]);
        khArray_concatenate(&string, &argument_str, NULL);
        khArray_delete(&argument_str);

        if (i < khArray_size(&call_exp->arguments) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


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

khArray(char32_t) khAstIndexExpression_string(khAstIndexExpression* index_exp) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) indexee_str = khAstExpression_string(index_exp->indexee);
    khArray_concatenate(&string, &indexee_str, NULL);
    khArray_delete(&indexee_str);

    kh_appendCstring(&string, U", ");

    khArray_append(&string, U'(');
    for (size_t i = 0; i < khArray_size(&index_exp->arguments); i++) {
        khArray(char32_t) argument_str = khAstExpression_string(&index_exp->arguments[i]);
        khArray_concatenate(&string, &argument_str, NULL);
        khArray_delete(&argument_str);

        if (i < khArray_size(&index_exp->arguments) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

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
                                      .name = khArray_copy(&declaration->name, NULL),
                                      .optional_type = optional_type,
                                      .optional_initializer = optional_initializer};
}

void khAstVariableDeclaration_delete(khAstVariableDeclaration* declaration) {
    khArray_delete(&declaration->name);
    if (declaration->optional_type != NULL) {
        khAstExpression_delete(declaration->optional_type);
    }
    if (declaration->optional_initializer != NULL) {
        khAstExpression_delete(declaration->optional_initializer);
    }
}

khArray(char32_t) khAstVariableDeclaration_string(khAstVariableDeclaration* declaration) {
    khArray(char32_t) string = kh_string(U"(");

    kh_appendCstring(&string, declaration->is_static ? U"true, " : U"false, ");
    kh_appendCstring(&string, declaration->is_wild ? U"true, " : U"false, ");

    khArray(char32_t) name_str = kh_quoteString(&declaration->name);
    khArray_concatenate(&string, &name_str, NULL);
    khArray_delete(&name_str);

    kh_appendCstring(&string, U", ");
    if (declaration->optional_type != NULL) {
        khArray(char32_t) type_str = khAstExpression_string(declaration->optional_type);
        khArray_concatenate(&string, &type_str, NULL);
        khArray_delete(&type_str);
    }
    else {
        kh_appendCstring(&string, U"unspecified");
    }

    if (declaration->optional_initializer != NULL) {
        kh_appendCstring(&string, U", ");

        khArray(char32_t) initializer_str = khAstExpression_string(declaration->optional_initializer);
        khArray_concatenate(&string, &initializer_str, NULL);
        khArray_delete(&initializer_str);
    }

    khArray_append(&string, U')');
    return string;
}


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

khArray(char32_t) khAstLambdaExpression_string(khAstLambdaExpression* lambda) {
    khArray(char32_t) string = kh_string(U"(");

    khArray_append(&string, U'(');
    for (size_t i = 0; i < khArray_size(&lambda->arguments); i++) {
        khArray(char32_t) argument_str = khAstVariableDeclaration_string(&lambda->arguments[i]);
        khArray_concatenate(&string, &argument_str, NULL);
        khArray_delete(&argument_str);

        if (i < khArray_size(&lambda->arguments) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }
    kh_appendCstring(&string, U"), ");

    if (lambda->optional_variadic_argument != NULL) {
        khArray(char32_t) variadic_argument_str =
            khAstVariableDeclaration_string(lambda->optional_variadic_argument);
        khArray_concatenate(&string, &variadic_argument_str, NULL);
        khArray_delete(&variadic_argument_str);
    }
    else {
        kh_appendCstring(&string, U"unspecified");
    }

    kh_appendCstring(&string, U", ");
    if (lambda->optional_return_type != NULL) {
        khArray(char32_t) return_type_str = khAstExpression_string(lambda->optional_return_type);
        khArray_concatenate(&string, &return_type_str, NULL);
        khArray_delete(&return_type_str);
    }
    else {
        kh_appendCstring(&string, U"unspecified");
    }

    kh_appendCstring(&string, U", (");
    for (size_t i = 0; i < khArray_size(&lambda->content); i++) {
        khArray(char32_t) ast_str = khAst_string(&lambda->content[i]);
        khArray_concatenate(&string, &ast_str, NULL);
        khArray_delete(&ast_str);

        if (i < khArray_size(&lambda->content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


khAstScopeExpression khAstScopeExpression_copy(khAstScopeExpression* scope_exp) {
    khAstExpression* value = (khAstExpression*)malloc(sizeof(khAstExpression));
    *value = khAstExpression_copy(scope_exp->value);

    khArray(khArray(char32_t)) scope_names =
        khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t));
    for (size_t i = 0; i < khArray_size(&scope_exp->scope_names); i++) {
        khArray_append(&scope_names, khArray_copy(&scope_exp->scope_names[i], NULL));
    }

    return (khAstScopeExpression){.value = value, .scope_names = scope_names};
}

void khAstScopeExpression_delete(khAstScopeExpression* scope_exp) {
    khAstExpression_delete(scope_exp->value);
    khArray_delete(&scope_exp->scope_names);
}

khArray(char32_t) khAstScopeExpression_string(khAstScopeExpression* scope_exp) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) value_str = khAstExpression_string(scope_exp->value);
    khArray_concatenate(&string, &value_str, NULL);
    khArray_delete(&value_str);

    kh_appendCstring(&string, U", (");

    for (size_t i = 0; i < khArray_size(&scope_exp->scope_names); i++) {
        khArray(char32_t) scope_name_str = kh_quoteString(&scope_exp->scope_names[i]);
        khArray_concatenate(&string, &scope_name_str, NULL);
        khArray_delete(&scope_name_str);

        if (i < khArray_size(&scope_exp->scope_names) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }
    kh_appendCstring(&string, U"))");

    return string;
}


khAstRefExpression khAstRefExpression_copy(khAstRefExpression* ref_exp) {
    khAstExpression* value = (khAstExpression*)malloc(sizeof(khAstExpression));
    *value = khAstExpression_copy(ref_exp->value);

    return (khAstRefExpression){.value = value};
}

void khAstRefExpression_delete(khAstRefExpression* ref_exp) {
    khAstExpression_delete(ref_exp->value);
}

khArray(char32_t) khAstRefExpression_string(khAstRefExpression* ref_exp) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) value_str = khAstExpression_string(ref_exp->value);
    khArray_concatenate(&string, &value_str, NULL);
    khArray_delete(&value_str);

    khArray_append(&string, U')');
    return string;
}


khAstFunctionTypeExpression
khAstFunctionTypeExpression_copy(khAstFunctionTypeExpression* function_type) {
    khAstExpression* return_type = (khAstExpression*)malloc(sizeof(khAstExpression));
    *return_type = khAstExpression_copy(function_type->return_type);

    return (khAstFunctionTypeExpression){
        .argument_types = khArray_copy(&function_type->argument_types, khAstExpression_copy),
        .return_type = return_type};
}

void khAstFunctionTypeExpression_delete(khAstFunctionTypeExpression* function_type) {
    khArray_delete(&function_type->argument_types);
    khAstExpression_delete(function_type->return_type);
}

khArray(char32_t) khAstFunctionTypeExpression_string(khAstFunctionTypeExpression* function_type) {
    khArray(char32_t) string = kh_string(U"((");

    for (size_t i = 0; i < khArray_size(&function_type->argument_types); i++) {
        khArray(char32_t) argument_type_str = khAstExpression_string(&function_type->argument_types[i]);
        khArray_concatenate(&string, &argument_type_str, NULL);
        khArray_delete(&argument_type_str);

        if (i < khArray_size(&function_type->argument_types) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"), ");

    khArray(char32_t) return_type_str = khAstExpression_string(function_type->return_type);
    khArray_concatenate(&string, &return_type_str, NULL);
    khArray_delete(&return_type_str);

    kh_appendCstring(&string, U"))");
    khArray_append(&string, U')');
    return string;
}


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

khArray(char32_t) khAstTemplatizeExpression_string(khAstTemplatizeExpression* templatize_exp) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) value_str = khAstExpression_string(templatize_exp->value);
    khArray_concatenate(&string, &value_str, NULL);
    khArray_delete(&value_str);

    kh_appendCstring(&string, U", (");

    for (size_t i = 0; i < khArray_size(&templatize_exp->template_arguments); i++) {
        khArray(char32_t) template_argument_str =
            khAstExpression_string(&templatize_exp->template_arguments[i]);
        khArray_concatenate(&string, &template_argument_str, NULL);
        khArray_delete(&template_argument_str);

        if (i < khArray_size(&templatize_exp->template_arguments) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }
    kh_appendCstring(&string, U"))");

    return string;
}


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
        case khAstExpressionType_REF:
            copy.ref = khAstRefExpression_copy(&expression->ref);
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
        case khAstExpressionType_REF:
            khAstRefExpression_delete(&expression->ref);
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

khArray(char32_t) khAstExpression_string(khAstExpression* expression) {
    khArray(char32_t) string = khAstExpressionType_string(expression->type);

    switch (expression->type) {
        case khAstExpressionType_INVALID:
            kh_appendCstring(&string, U"()");
            break;

        case khAstExpressionType_IDENTIFIER: {
            khArray_append(&string, U'(');
            khArray(char32_t) identifier_str = kh_quoteString(&expression->identifier);
            khArray_concatenate(&string, &identifier_str, NULL);
            khArray_delete(&identifier_str);
            khArray_append(&string, U')');
        } break;
        case khAstExpressionType_CHAR: {
            kh_appendCstring(&string, U"(\'");
            khArray(char32_t) char_str = kh_escapeChar(expression->char_v);
            khArray_concatenate(&string, &char_str, NULL);
            khArray_delete(&char_str);
            kh_appendCstring(&string, U"\')");
        } break;
        case khAstExpressionType_STRING: {
            khArray_append(&string, U'(');
            khArray(char32_t) string_str = kh_quoteString(&expression->string);
            khArray_concatenate(&string, &string_str, NULL);
            khArray_delete(&string_str);
            khArray_append(&string, U')');
        } break;
        case khAstExpressionType_BUFFER: {
            khArray_append(&string, U'(');
            khArray(char32_t) buffer_str = kh_quoteBuffer(&expression->buffer);
            khArray_concatenate(&string, &buffer_str, NULL);
            khArray_delete(&buffer_str);
            khArray_append(&string, U')');
        } break;
        case khAstExpressionType_INTEGER: {
            khArray_append(&string, U'(');
            khArray(char32_t) integer_str = kh_intToString(expression->integer, 10);
            khArray_concatenate(&string, &integer_str, NULL);
            khArray_delete(&integer_str);
            khArray_append(&string, U')');
        } break;
        case khAstExpressionType_UINTEGER: {
            khArray_append(&string, U'(');
            khArray(char32_t) uinteger_str = kh_uintToString(expression->uinteger, 10);
            khArray_concatenate(&string, &uinteger_str, NULL);
            khArray_delete(&uinteger_str);
            khArray_append(&string, U')');
        } break;
        case khAstExpressionType_FLOAT: {
            khArray_append(&string, U'(');
            khArray(char32_t) float_str = kh_floatToString(expression->float_v, 4, 10);
            khArray_concatenate(&string, &float_str, NULL);
            khArray_delete(&float_str);
            khArray_append(&string, U')');
        } break;
        case khAstExpressionType_DOUBLE: {
            khArray_append(&string, U'(');
            khArray(char32_t) double_str = kh_floatToString(expression->double_v, 4, 10);
            khArray_concatenate(&string, &double_str, NULL);
            khArray_delete(&double_str);
            khArray_append(&string, U')');
        } break;
        case khAstExpressionType_IFLOAT: {
            khArray_append(&string, U'(');
            khArray(char32_t) ifloat_str = kh_floatToString(expression->ifloat, 4, 10);
            khArray_concatenate(&string, &ifloat_str, NULL);
            khArray_delete(&ifloat_str);
            khArray_append(&string, U')');
        } break;
        case khAstExpressionType_IDOUBLE: {
            khArray_append(&string, U'(');
            khArray(char32_t) idouble_str = kh_floatToString(expression->idouble, 4, 10);
            khArray_concatenate(&string, &idouble_str, NULL);
            khArray_delete(&idouble_str);
            khArray_append(&string, U')');
        } break;

        case khAstExpressionType_TUPLE: {
            khArray(char32_t) tuple_str = khAstTuple_string(&expression->tuple);
            khArray_concatenate(&string, &tuple_str, NULL);
            khArray_delete(&tuple_str);
        } break;
        case khAstExpressionType_ARRAY: {
            khArray(char32_t) array_str = khAstArray_string(&expression->array);
            khArray_concatenate(&string, &array_str, NULL);
            khArray_delete(&array_str);
        } break;
        case khAstExpressionType_DICT: {
            khArray(char32_t) dict_str = khAstDict_string(&expression->dict);
            khArray_concatenate(&string, &dict_str, NULL);
            khArray_delete(&dict_str);
        } break;

        case khAstExpressionType_UNARY: {
            khArray_delete(&string);
            string = khAstUnaryExpressionType_string(expression->unary.type);
            khArray(char32_t) unary_str = khAstUnaryExpression_string(&expression->unary);
            khArray_concatenate(&string, &unary_str, NULL);
            khArray_delete(&unary_str);
        } break;
        case khAstExpressionType_BINARY: {
            khArray_delete(&string);
            string = khAstBinaryExpressionType_string(expression->binary.type);
            khArray(char32_t) binary_str = khAstBinaryExpression_string(&expression->binary);
            khArray_concatenate(&string, &binary_str, NULL);
            khArray_delete(&binary_str);
        } break;
        case khAstExpressionType_TERNARY: {
            khArray(char32_t) ternary_str = khAstTernaryExpression_string(&expression->ternary);
            khArray_concatenate(&string, &ternary_str, NULL);
            khArray_delete(&ternary_str);
        } break;
        case khAstExpressionType_CALL: {
            khArray(char32_t) call_str = khAstCallExpression_string(&expression->call);
            khArray_concatenate(&string, &call_str, NULL);
            khArray_delete(&call_str);
        } break;
        case khAstExpressionType_INDEX: {
            khArray(char32_t) index_str = khAstIndexExpression_string(&expression->index);
            khArray_concatenate(&string, &index_str, NULL);
            khArray_delete(&index_str);
        } break;

        case khAstExpressionType_VARIABLE_DECLARATION: {
            khArray(char32_t) variable_declaration_str =
                khAstVariableDeclaration_string(&expression->variable_declaration);
            khArray_concatenate(&string, &variable_declaration_str, NULL);
            khArray_delete(&variable_declaration_str);
        } break;
        case khAstExpressionType_LAMBDA: {
            khArray(char32_t) lambda_str = khAstLambdaExpression_string(&expression->lambda);
            khArray_concatenate(&string, &lambda_str, NULL);
            khArray_delete(&lambda_str);
        } break;
        case khAstExpressionType_SCOPE: {
            khArray(char32_t) scope_str = khAstScopeExpression_string(&expression->scope);
            khArray_concatenate(&string, &scope_str, NULL);
            khArray_delete(&scope_str);
        } break;
        case khAstExpressionType_REF: {
            khArray(char32_t) ref_str = khAstRefExpression_string(&expression->ref);
            khArray_concatenate(&string, &ref_str, NULL);
            khArray_delete(&ref_str);
        } break;
        case khAstExpressionType_FUNCTION_TYPE: {
            khArray(char32_t) function_type_str =
                khAstFunctionTypeExpression_string(&expression->function_type);
            khArray_concatenate(&string, &function_type_str, NULL);
            khArray_delete(&function_type_str);
        } break;
        case khAstExpressionType_TEMPLATIZE: {
            khArray(char32_t) templatize_str =
                khAstTemplatizeExpression_string(&expression->templatize);
            khArray_concatenate(&string, &templatize_str, NULL);
            khArray_delete(&templatize_str);
        } break;

        default:
            break;
    }

    return string;
}


khAstImport khAstImport_copy(khAstImport* import_v) {
    khArray(khArray(char32_t)) path = khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t));
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
    khArray_delete(&import_v->path);
    if (import_v->optional_alias != NULL) {
        khArray_delete(import_v->optional_alias);
    }
}

khArray(char32_t) khAstImport_string(khAstImport* import_v) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) path = import_v->relative ? kh_string(U".") : khArray_new(char32_t, NULL);
    for (size_t i = 0; i < khArray_size(&import_v->path); i++) {
        khArray_concatenate(&path, &import_v->path[i], NULL);

        if (i < khArray_size(&import_v->path) - 1) {
            khArray_append(&path, U'.');
        }
    }

    khArray(char32_t) path_str = kh_quoteString(&path);
    khArray_concatenate(&string, &path_str, NULL);
    khArray_delete(&path_str);
    khArray_delete(&path);

    if (import_v->optional_alias != NULL) {
        kh_appendCstring(&string, U", ");
        khArray(char32_t) alias_str = kh_quoteString(import_v->optional_alias);
        khArray_concatenate(&string, &alias_str, NULL);
        khArray_delete(&alias_str);
    }

    khArray_append(&string, U')');
    return string;
}


khAstInclude khAstInclude_copy(khAstInclude* include) {
    khArray(khArray(char32_t)) path = khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t));
    for (size_t i = 0; i < khArray_size(&include->path); i++) {
        khArray_append(&path, khArray_copy(&include->path[i], NULL));
    }

    return (khAstInclude){.path = path, .relative = include->relative};
}

void khAstInclude_delete(khAstInclude* include) {
    khArray_delete(&include->path);
}

khArray(char32_t) khAstInclude_string(khAstInclude* include) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) path = include->relative ? kh_string(U".") : khArray_new(char32_t, NULL);
    for (size_t i = 0; i < khArray_size(&include->path); i++) {
        khArray_concatenate(&path, &include->path[i], NULL);

        if (i < khArray_size(&include->path) - 1) {
            khArray_append(&path, U'.');
        }
    }

    khArray(char32_t) path_str = kh_quoteString(&path);
    khArray_concatenate(&string, &path_str, NULL);
    khArray_delete(&path_str);
    khArray_delete(&path);

    khArray_append(&string, U')');
    return string;
}


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

    return (khAstFunction){.is_incase = function->is_incase,
                           .is_static = function->is_static,
                           .name_point = khAstExpression_copy(&function->name_point),
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

khArray(char32_t) khAstFunction_string(khAstFunction* function) {
    khArray(char32_t) string = kh_string(U"(");

    kh_appendCstring(&string, function->is_incase ? U"true, " : U"false, ");
    kh_appendCstring(&string, function->is_static ? U"true, " : U"false, ");

    khArray(char32_t) name_point_str = khAstExpression_string(&function->name_point);
    khArray_concatenate(&string, &name_point_str, NULL);
    khArray_delete(&name_point_str);

    kh_appendCstring(&string, U", (");
    for (size_t i = 0; i < khArray_size(&function->arguments); i++) {
        khArray(char32_t) argument_str = khAstVariableDeclaration_string(&function->arguments[i]);
        khArray_concatenate(&string, &argument_str, NULL);
        khArray_delete(&argument_str);

        if (i < khArray_size(&function->arguments) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }
    kh_appendCstring(&string, U"), ");

    if (function->optional_variadic_argument != NULL) {
        khArray(char32_t) variadic_argument_str =
            khAstVariableDeclaration_string(function->optional_variadic_argument);
        khArray_concatenate(&string, &variadic_argument_str, NULL);
        khArray_delete(&variadic_argument_str);
    }
    else {
        kh_appendCstring(&string, U"unspecified");
    }

    kh_appendCstring(&string, U", ");
    if (function->optional_return_type != NULL) {
        khArray(char32_t) return_type_str = khAstExpression_string(function->optional_return_type);
        khArray_concatenate(&string, &return_type_str, NULL);
        khArray_delete(&return_type_str);
    }
    else {
        kh_appendCstring(&string, U"unspecified");
    }

    kh_appendCstring(&string, U", (");
    for (size_t i = 0; i < khArray_size(&function->content); i++) {
        khArray(char32_t) ast_str = khAst_string(&function->content[i]);
        khArray_concatenate(&string, &ast_str, NULL);
        khArray_delete(&ast_str);

        if (i < khArray_size(&function->content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


khAstClass khAstClass_copy(khAstClass* class_v) {
    khArray(khArray(char32_t)) template_arguments =
        khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t));
    for (size_t i = 0; i < khArray_size(&class_v->template_arguments); i++) {
        khArray_append(&template_arguments, khArray_copy(&class_v->template_arguments[i], NULL));
    }

    khAstExpression* optional_base_type = NULL;
    if (class_v->optional_base_type != NULL) {
        optional_base_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_base_type = khAstExpression_copy(class_v->optional_base_type);
    }

    return (khAstClass){.is_incase = class_v->is_incase,
                        .name = khArray_copy(&class_v->name, NULL),
                        .template_arguments = template_arguments,
                        .optional_base_type = optional_base_type,
                        .content = khArray_copy(&class_v->content, khAst_copy)};
}

void khAstClass_delete(khAstClass* class_v) {
    khArray_delete(&class_v->name);
    khArray_delete(&class_v->template_arguments);
    if (class_v->optional_base_type != NULL) {
        khAstExpression_delete(class_v->optional_base_type);
    }
    khArray_delete(&class_v->content);
}

khArray(char32_t) khAstClass_string(khAstClass* class_v) {
    khArray(char32_t) string = kh_string(U"(");

    kh_appendCstring(&string, class_v->is_incase ? U"true, " : U"false, ");

    khArray(char32_t) name_str = kh_quoteString(&class_v->name);
    khArray_concatenate(&string, &name_str, NULL);
    khArray_delete(&name_str);

    kh_appendCstring(&string, U", (");
    for (size_t i = 0; i < khArray_size(&class_v->template_arguments); i++) {
        khArray(char32_t) template_argument_str = khArray_copy(&class_v->template_arguments[i], NULL);
        khArray_concatenate(&string, &template_argument_str, NULL);
        khArray_delete(&template_argument_str);

        if (i < khArray_size(&class_v->template_arguments) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"), ");

    if (class_v->optional_base_type != NULL) {
        khArray(char32_t) base_type_str = khAstExpression_string(class_v->optional_base_type);
        khArray_concatenate(&string, &base_type_str, NULL);
        khArray_delete(&base_type_str);
    }
    else {
        kh_appendCstring(&string, U"unspecified");
    }

    kh_appendCstring(&string, U", (");
    for (size_t i = 0; i < khArray_size(&class_v->content); i++) {
        khArray(char32_t) ast_str = khAst_string(&class_v->content[i]);
        khArray_concatenate(&string, &ast_str, NULL);
        khArray_delete(&ast_str);

        if (i < khArray_size(&class_v->content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


khAstStruct khAstStruct_copy(khAstStruct* struct_v) {
    khArray(khArray(char32_t)) template_arguments =
        khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t));
    for (size_t i = 0; i < khArray_size(&struct_v->template_arguments); i++) {
        khArray_append(&template_arguments, khArray_copy(&struct_v->template_arguments[i], NULL));
    }

    khAstExpression* optional_base_type = NULL;
    if (struct_v->optional_base_type != NULL) {
        optional_base_type = (khAstExpression*)malloc(sizeof(khAstExpression));
        *optional_base_type = khAstExpression_copy(struct_v->optional_base_type);
    }

    return (khAstStruct){.is_incase = struct_v->is_incase,
                         .name = khArray_copy(&struct_v->name, NULL),
                         .template_arguments = template_arguments,
                         .optional_base_type = optional_base_type,
                         .content = khArray_copy(&struct_v->content, khAst_copy)};
}

void khAstStruct_delete(khAstStruct* struct_v) {
    khArray_delete(&struct_v->name);
    khArray_delete(&struct_v->template_arguments);
    if (struct_v->optional_base_type != NULL) {
        khAstExpression_delete(struct_v->optional_base_type);
    }
    khArray_delete(&struct_v->content);
}

khArray(char32_t) khAstStruct_string(khAstStruct* struct_v) {
    khArray(char32_t) string = kh_string(U"(");

    kh_appendCstring(&string, struct_v->is_incase ? U"true, " : U"false, ");

    khArray(char32_t) name_str = kh_quoteString(&struct_v->name);
    khArray_concatenate(&string, &name_str, NULL);
    khArray_delete(&name_str);

    kh_appendCstring(&string, U", (");
    for (size_t i = 0; i < khArray_size(&struct_v->template_arguments); i++) {
        khArray(char32_t) template_argument_str = khArray_copy(&struct_v->template_arguments[i], NULL);
        khArray_concatenate(&string, &template_argument_str, NULL);
        khArray_delete(&template_argument_str);

        if (i < khArray_size(&struct_v->template_arguments) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"), ");

    if (struct_v->optional_base_type != NULL) {
        khArray(char32_t) base_type_str = khAstExpression_string(struct_v->optional_base_type);
        khArray_concatenate(&string, &base_type_str, NULL);
        khArray_delete(&base_type_str);
    }
    else {
        kh_appendCstring(&string, U"unspecified");
    }

    kh_appendCstring(&string, U", (");

    for (size_t i = 0; i < khArray_size(&struct_v->content); i++) {
        khArray(char32_t) ast_str = khAst_string(&struct_v->content[i]);
        khArray_concatenate(&string, &ast_str, NULL);
        khArray_delete(&ast_str);

        if (i < khArray_size(&struct_v->content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


khAstEnum khAstEnum_copy(khAstEnum* enum_v) {
    khArray(khArray(char32_t)) members = khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t));
    for (size_t i = 0; i < khArray_size(&enum_v->members); i++) {
        khArray_append(&members, khArray_copy(&enum_v->members[i], NULL));
    }

    return (khAstEnum){.name = khArray_copy(&enum_v->name, NULL), .members = members};
}

void khAstEnum_delete(khAstEnum* enum_v) {
    khArray_delete(&enum_v->name);
    khArray_delete(&enum_v->members);
}

khArray(char32_t) khAstEnum_string(khAstEnum* enum_v) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) name_str = kh_quoteString(&enum_v->name);
    khArray_concatenate(&string, &name_str, NULL);
    khArray_delete(&name_str);

    kh_appendCstring(&string, U", (");

    for (size_t i = 0; i < khArray_size(&enum_v->members); i++) {
        khArray(char32_t) member_str = kh_quoteString(&enum_v->members[i]);
        khArray_concatenate(&string, &member_str, NULL);
        khArray_delete(&member_str);

        if (i < khArray_size(&enum_v->members) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


khAstAlias khAstAlias_copy(khAstAlias* alias) {
    return (khAstAlias){.is_incase = alias->is_incase,
                        .name = khArray_copy(&alias->name, NULL),
                        .expression = khAstExpression_copy(&alias->expression)};
}

void khAstAlias_delete(khAstAlias* alias) {
    khArray_delete(&alias->name);
    khAstExpression_delete(&alias->expression);
}

khArray(char32_t) khAstAlias_string(khAstAlias* alias) {
    khArray(char32_t) string = kh_string(U"(");

    kh_appendCstring(&string, alias->is_incase ? U"true, " : U"false, ");

    khArray(char32_t) name_str = kh_quoteString(&alias->name);
    khArray_concatenate(&string, &name_str, NULL);
    khArray_delete(&name_str);

    kh_appendCstring(&string, U", ");

    khArray(char32_t) expression_str = khAstExpression_string(&alias->expression);
    khArray_concatenate(&string, &expression_str, NULL);
    khArray_delete(&expression_str);

    khArray_append(&string, U')');
    return string;
}


khAstIfBranch khAstIfBranch_copy(khAstIfBranch* if_branch) {
    khArray(khArray(khAst)) branch_contents =
        khArray_new(khArray(khAst), khArray_arrayDeleter(char32_t));
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
    khArray_delete(&if_branch->branch_contents);
    khArray_delete(&if_branch->else_content);
}

khArray(char32_t) khAstIfBranch_string(khAstIfBranch* if_branch) {
    khArray(char32_t) string = kh_string(U"(");

    for (size_t i = 0; i < khArray_size(&if_branch->branch_conditions); i++) {
        khArray_append(&string, U'(');

        khArray(char32_t) condition_str = khAstExpression_string(&if_branch->branch_conditions[i]);
        khArray_concatenate(&string, &condition_str, NULL);
        khArray_delete(&condition_str);

        kh_appendCstring(&string, U", (");

        for (size_t j = 0; j < khArray_size(&if_branch->branch_contents[i]); j++) {
            khArray(char32_t) content_str = khAst_string(&if_branch->branch_contents[i][j]);
            khArray_concatenate(&string, &content_str, NULL);
            khArray_delete(&content_str);

            if (j < khArray_size(&if_branch->branch_contents[i]) - 1) {
                kh_appendCstring(&string, U", ");
            }
        }

        kh_appendCstring(&string, U"))");

        if (i < khArray_size(&if_branch->branch_conditions) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    for (size_t i = 0; i < khArray_size(&if_branch->else_content); i++) {
        khArray(char32_t) content_str = khAst_string(&if_branch->else_content[i]);
        khArray_concatenate(&string, &content_str, NULL);
        khArray_delete(&content_str);

        if (i < khArray_size(&if_branch->else_content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    khArray_append(&string, U')');
    return string;
}


khAstWhileLoop khAstWhileLoop_copy(khAstWhileLoop* while_loop) {
    return (khAstWhileLoop){.condition = khAstExpression_copy(&while_loop->condition),
                            .content = khArray_copy(&while_loop->content, khAst_copy)};
}

void khAstWhileLoop_delete(khAstWhileLoop* while_loop) {
    khAstExpression_delete(&while_loop->condition);
    khArray_delete(&while_loop->content);
}

khArray(char32_t) khAstWhileLoop_string(khAstWhileLoop* while_loop) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) condition_str = khAstExpression_string(&while_loop->condition);
    khArray_concatenate(&string, &condition_str, NULL);
    khArray_delete(&condition_str);

    kh_appendCstring(&string, U", (");

    for (size_t i = 0; i < khArray_size(&while_loop->content); i++) {
        khArray(char32_t) content_str = khAst_string(&while_loop->content[i]);
        khArray_concatenate(&string, &content_str, NULL);
        khArray_delete(&content_str);

        if (i < khArray_size(&while_loop->content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


khAstDoWhileLoop khAstDoWhileLoop_copy(khAstDoWhileLoop* do_while_loop) {
    return (khAstDoWhileLoop){.condition = khAstExpression_copy(&do_while_loop->condition),
                              .content = khArray_copy(&do_while_loop->content, khAst_copy)};
}

void khAstDoWhileLoop_delete(khAstDoWhileLoop* do_while_loop) {
    khAstExpression_delete(&do_while_loop->condition);
    khArray_delete(&do_while_loop->content);
}

khArray(char32_t) khAstDoWhileLoop_string(khAstDoWhileLoop* do_while_loop) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) condition_str = khAstExpression_string(&do_while_loop->condition);
    khArray_concatenate(&string, &condition_str, NULL);
    khArray_delete(&condition_str);

    kh_appendCstring(&string, U", (");

    for (size_t i = 0; i < khArray_size(&do_while_loop->content); i++) {
        khArray(char32_t) content_str = khAst_string(&do_while_loop->content[i]);
        khArray_concatenate(&string, &content_str, NULL);
        khArray_delete(&content_str);

        if (i < khArray_size(&do_while_loop->content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


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

khArray(char32_t) khAstForLoop_string(khAstForLoop* for_loop) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) initial_expression_str = khAstExpression_string(&for_loop->initial_expression);
    khArray_concatenate(&string, &initial_expression_str, NULL);
    khArray_delete(&initial_expression_str);

    kh_appendCstring(&string, U", ");
    khArray(char32_t) loop_condition_str = khAstExpression_string(&for_loop->loop_condition);
    khArray_concatenate(&string, &loop_condition_str, NULL);
    khArray_delete(&loop_condition_str);

    kh_appendCstring(&string, U", ");
    khArray(char32_t) update_expression_str = khAstExpression_string(&for_loop->update_expression);
    khArray_concatenate(&string, &update_expression_str, NULL);
    khArray_delete(&update_expression_str);

    kh_appendCstring(&string, U", (");
    for (size_t i = 0; i < khArray_size(&for_loop->content); i++) {
        khArray(char32_t) content_str = khAst_string(&for_loop->content[i]);
        khArray_concatenate(&string, &content_str, NULL);
        khArray_delete(&content_str);

        if (i < khArray_size(&for_loop->content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


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

khArray(char32_t) khAstForEachLoop_string(khAstForEachLoop* for_each_loop) {
    khArray(char32_t) string = kh_string(U"((");

    for (size_t i = 0; i < khArray_size(&for_each_loop->iterators); i++) {
        khArray(char32_t) iterator_str = khAstExpression_string(&for_each_loop->iterators[i]);
        khArray_concatenate(&string, &iterator_str, NULL);
        khArray_delete(&iterator_str);

        if (i < khArray_size(&for_each_loop->iterators) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"), ");

    khArray(char32_t) iteratee_str = khAstExpression_string(&for_each_loop->iteratee);
    khArray_concatenate(&string, &iteratee_str, NULL);
    khArray_delete(&iteratee_str);

    kh_appendCstring(&string, U", (");

    for (size_t i = 0; i < khArray_size(&for_each_loop->content); i++) {
        khArray(char32_t) content_str = khAst_string(&for_each_loop->content[i]);
        khArray_concatenate(&string, &content_str, NULL);
        khArray_delete(&content_str);

        if (i < khArray_size(&for_each_loop->content) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    kh_appendCstring(&string, U"))");

    return string;
}


khAstBreak khAstBreak_copy(khAstBreak* break_v) {
    return *break_v;
}

void khAstBreak_delete(khAstBreak* break_v) {}

khArray(char32_t) khAstBreak_string(khAstBreak* break_v) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) breakings_str = kh_uintToString(break_v->breakings, 10);
    khArray_concatenate(&string, &breakings_str, NULL);
    khArray_delete(&breakings_str);

    khArray_append(&string, U')');
    return string;
}


khAstContinue khAstContinue_copy(khAstContinue* continue_v) {
    return *continue_v;
}

void khAstContinue_delete(khAstContinue* continue_v) {}

khArray(char32_t) khAstContinue_string(khAstContinue* continue_v) {
    khArray(char32_t) string = kh_string(U"(");

    khArray(char32_t) continuations_str = kh_uintToString(continue_v->continuations, 10);
    khArray_concatenate(&string, &continuations_str, NULL);
    khArray_delete(&continuations_str);

    khArray_append(&string, U')');
    return string;
}


khAstReturn khAstReturn_copy(khAstReturn* return_v) {
    return (khAstReturn){.values = khArray_copy(&return_v->values, khAstExpression_copy)};
}

void khAstReturn_delete(khAstReturn* return_v) {
    khArray_delete(&return_v->values);
}

khArray(char32_t) khAstReturn_string(khAstReturn* return_v) {
    khArray(char32_t) string = kh_string(U"(");

    for (size_t i = 0; i < khArray_size(&return_v->values); i++) {
        khArray(char32_t) value_str = khAstExpression_string(&return_v->values[i]);
        khArray_concatenate(&string, &value_str, NULL);
        khArray_delete(&value_str);

        if (i < khArray_size(&return_v->values) - 1) {
            kh_appendCstring(&string, U", ");
        }
    }

    khArray_append(&string, U')');
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

khArray(char32_t) khAst_string(khAst* ast) {
    khArray(char32_t) string = khAstType_string(ast->type);

    switch (ast->type) {
        case khAstType_INVALID:
            kh_appendCstring(&string, U"()");
            break;

        case khAstType_EXPRESSION: {
            khArray_delete(&string);
            string = khAstExpression_string(&ast->expression);
        } break;

        case khAstType_IMPORT: {
            khArray(char32_t) import_str = khAstImport_string(&ast->import_v);
            khArray_concatenate(&string, &import_str, NULL);
            khArray_delete(&import_str);
        } break;
        case khAstType_INCLUDE: {
            khArray(char32_t) include_str = khAstInclude_string(&ast->include);
            khArray_concatenate(&string, &include_str, NULL);
            khArray_delete(&include_str);
        } break;
        case khAstType_FUNCTION: {
            khArray(char32_t) function_str = khAstFunction_string(&ast->function);
            khArray_concatenate(&string, &function_str, NULL);
            khArray_delete(&function_str);
        } break;
        case khAstType_CLASS: {
            khArray(char32_t) class_str = khAstClass_string(&ast->class_v);
            khArray_concatenate(&string, &class_str, NULL);
            khArray_delete(&class_str);
        } break;
        case khAstType_STRUCT: {
            khArray(char32_t) struct_str = khAstStruct_string(&ast->struct_v);
            khArray_concatenate(&string, &struct_str, NULL);
            khArray_delete(&struct_str);
        } break;
        case khAstType_ENUM: {
            khArray(char32_t) enum_str = khAstEnum_string(&ast->enum_v);
            khArray_concatenate(&string, &enum_str, NULL);
            khArray_delete(&enum_str);
        } break;
        case khAstType_ALIAS: {
            khArray(char32_t) alias_str = khAstAlias_string(&ast->alias);
            khArray_concatenate(&string, &alias_str, NULL);
            khArray_delete(&alias_str);
        } break;

        case khAstType_IF_BRANCH: {
            khArray(char32_t) if_branch_str = khAstIfBranch_string(&ast->if_branch);
            khArray_concatenate(&string, &if_branch_str, NULL);
            khArray_delete(&if_branch_str);
        } break;
        case khAstType_WHILE_LOOP: {
            khArray(char32_t) while_loop_str = khAstWhileLoop_string(&ast->while_loop);
            khArray_concatenate(&string, &while_loop_str, NULL);
            khArray_delete(&while_loop_str);
        } break;
        case khAstType_DO_WHILE_LOOP: {
            khArray(char32_t) do_while_loop_str = khAstDoWhileLoop_string(&ast->do_while_loop);
            khArray_concatenate(&string, &do_while_loop_str, NULL);
            khArray_delete(&do_while_loop_str);
        } break;
        case khAstType_FOR_LOOP: {
            khArray(char32_t) for_loop_str = khAstForLoop_string(&ast->for_loop);
            khArray_concatenate(&string, &for_loop_str, NULL);
            khArray_delete(&for_loop_str);
        } break;
        case khAstType_FOR_EACH_LOOP: {
            khArray(char32_t) for_each_loop_str = khAstForEachLoop_string(&ast->for_each_loop);
            khArray_concatenate(&string, &for_each_loop_str, NULL);
            khArray_delete(&for_each_loop_str);
        } break;
        case khAstType_BREAK: {
            khArray(char32_t) break_str = khAstBreak_string(&ast->break_v);
            khArray_concatenate(&string, &break_str, NULL);
            khArray_delete(&break_str);
        } break;
        case khAstType_CONTINUE: {
            khArray(char32_t) continue_str = khAstContinue_string(&ast->continue_v);
            khArray_concatenate(&string, &continue_str, NULL);
            khArray_delete(&continue_str);
        } break;
        case khAstType_RETURN: {
            khArray(char32_t) return_str = khAstReturn_string(&ast->return_v);
            khArray_concatenate(&string, &return_str, NULL);
            khArray_delete(&return_str);
        } break;

        default:
            break;
    }

    return string;
}
