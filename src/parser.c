/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#include <kithare/lexer.h>
#include <kithare/parser.h>
#include <kithare/token.h>


// Sub-level parsing levels
static khAst sparseStatement(char32_t** cursor);
static khArray(khAst) sparseBlock(char32_t** cursor);
static khAstImport sparseImport(char32_t** cursor);
static khAstInclude sparseInclude(char32_t** cursor);
static khAstFunction sparseFunction(char32_t** cursor);
static khAstClass sparseClass(char32_t** cursor);
static khAstStruct sparseStruct(char32_t** cursor);
static khAstEnum sparseEnum(char32_t** cursor);
static khAstAlias sparseAlias(char32_t** cursor);
static khAstIfBranch sparseIfBranch(char32_t** cursor);
static khAstWhileLoop sparseWhileLoop(char32_t** cursor);
static khAstDoWhileLoop sparseDoWhileLoop(char32_t** cursor);
static khAst sparseForLoop(char32_t** cursor);
static khAstBreak sparseBreak(char32_t** cursor);
static khAstContinue sparseContinue(char32_t** cursor);
static khAstReturn sparseReturn(char32_t** cursor);

// Sub-level expression parsing levels, by precedence
#define EXPARSE_ARGS bool ignore_newline, bool filter_type
static khAstExpression exparseInplaceOperators(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseTernary(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseLogicalOr(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseLogicalXor(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseLogicalAnd(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseLogicalNot(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseComparisonOperators(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseBitwiseOr(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseBitwiseXor(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseBitwiseAnd(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseBitwiseShifts(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseAddSub(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseMulDivMod(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparsePow(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseRef(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseUnary(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseReverseUnary(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseScopeTemplatization(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseOther(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseVariableDeclaration(char32_t** cursor, bool ignore_newline);
static khAstExpression exparseLambda(char32_t** cursor, bool ignore_newline);
static khArray(khAstExpression) exparseList(char32_t** cursor, khDelimiterToken opening_delimiter,
                                            khDelimiterToken closing_delimiter, EXPARSE_ARGS);


khAst kh_parse(char32_t** cursor) {
    return sparseStatement(cursor);
}

khAstExpression kh_parseExpression(char32_t** cursor, EXPARSE_ARGS) {
    return exparseInplaceOperators(cursor, ignore_newline, filter_type);
}


// Token getter function
static inline khToken currentToken(char32_t** cursor, bool ignore_newline) {
    khArray(khLexError) errors = khArray_new(khLexError, khLexError_delete);
    char32_t* cursor_copy = *cursor;
    khToken token = kh_lex(&cursor_copy, &errors);

    // Ignoring newlines means that it would lex the next token if a newline token was encountered
    while (token.type == khTokenType_COMMENT || (token.type == khTokenType_NEWLINE && ignore_newline)) {
        khToken_delete(&token);
        *cursor = cursor_copy;
        token = kh_lex(&cursor_copy, &errors);
    }

    if (khArray_size(&errors) != 0) {
        // TODO: handle error
    }

    khArray_delete(&errors);

    return token;
}

// Basically, a sort of `next` function
static inline void skipToken(char32_t** cursor, bool ignore_newline) {
    char32_t* origin = *cursor;
    khToken token = kh_lex(cursor, NULL);

    // Ignoring newlines, lex the next token
    while (token.type == khTokenType_COMMENT || (token.type == khTokenType_NEWLINE && ignore_newline)) {
        khToken_delete(&token);
        token = kh_lex(cursor, NULL);
    }

    khToken_delete(&token);

    // To avoid being stuck at the same token
    if (origin == *cursor && **cursor != U'\0') {
        (*cursor)++;
    }
}

static khAst sparseStatement(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    char32_t* origin = token.begin;
    khAst ast = (khAst){.begin = origin, .end = *cursor, .type = khAstType_INVALID};

    if (token.type == khTokenType_KEYWORD) {
        khKeywordToken keyword = token.keyword;
        khToken_delete(&token);

        switch (keyword) {
            case khKeywordToken_IMPORT: {
                khAstImport import_v = sparseImport(cursor);
                ast = (khAst){
                    .begin = origin, .end = *cursor, .type = khAstType_IMPORT, .import_v = import_v};
                goto end;
            }

            case khKeywordToken_INCLUDE: {
                khAstInclude include = sparseInclude(cursor);
                ast = (khAst){
                    .begin = origin, .end = *cursor, .type = khAstType_INCLUDE, .include = include};
                goto end;
            }

            case khKeywordToken_AS:
                // TODO: handle error
                goto end;

            case khKeywordToken_DEF: {
                khAstFunction function = sparseFunction(cursor);
                ast = (khAst){
                    .begin = origin, .end = *cursor, .type = khAstType_FUNCTION, .function = function};
                goto end;
            }

            case khKeywordToken_CLASS: {
                khAstClass class_v = sparseClass(cursor);
                ast = (khAst){
                    .begin = origin, .end = *cursor, .type = khAstType_CLASS, .class_v = class_v};
                goto end;
            }

            case khKeywordToken_STRUCT: {
                khAstStruct struct_v = sparseStruct(cursor);
                ast = (khAst){
                    .begin = origin, .end = *cursor, .type = khAstType_STRUCT, .struct_v = struct_v};
                goto end;
            }

            case khKeywordToken_ENUM: {
                khAstEnum enum_v = sparseEnum(cursor);
                ast =
                    (khAst){.begin = origin, .end = *cursor, .type = khAstType_ENUM, .enum_v = enum_v};
                goto end;
            }

            case khKeywordToken_ALIAS: {
                khAstAlias alias = sparseAlias(cursor);
                ast = (khAst){.begin = origin, .end = *cursor, .type = khAstType_ALIAS, .alias = alias};
                goto end;
            }

            // Handling `incase` and `static` modifiers
            case khKeywordToken_INCASE:
            case khKeywordToken_STATIC: {
                bool is_incase = keyword == khKeywordToken_INCASE;
                char32_t* previous = *cursor;
                skipToken(cursor, true);
                token = currentToken(cursor, true);

                if (token.type == khTokenType_KEYWORD) {
                    khKeywordToken keyword = token.keyword;
                    khToken_delete(&token);

                    switch (keyword) {
                        case khKeywordToken_DEF: {
                            *cursor = previous;
                            khAstFunction function = sparseFunction(cursor);
                            ast = (khAst){.begin = origin,
                                          .end = *cursor,
                                          .type = khAstType_FUNCTION,
                                          .function = function};
                            goto end;
                        }

                        case khKeywordToken_CLASS: {
                            *cursor = previous;
                            khAstClass class_v = sparseClass(cursor);
                            ast = (khAst){.begin = origin,
                                          .end = *cursor,
                                          .type = khAstType_CLASS,
                                          .class_v = class_v};
                            goto end;
                        }

                        case khKeywordToken_STRUCT: {
                            *cursor = previous;
                            khAstStruct struct_v = sparseStruct(cursor);
                            ast = (khAst){.begin = origin,
                                          .end = *cursor,
                                          .type = khAstType_STRUCT,
                                          .struct_v = struct_v};
                            goto end;
                        }

                        case khKeywordToken_ALIAS: {
                            *cursor = previous;
                            khAstAlias alias = sparseAlias(cursor);
                            ast = (khAst){.begin = origin,
                                          .end = *cursor,
                                          .type = khAstType_ALIAS,
                                          .alias = alias};
                            goto end;
                        }

                        // `incase static`
                        case khKeywordToken_STATIC: {
                            // Double `static static`
                            if (!is_incase) {
                                // TODO: handle error
                            }

                            skipToken(cursor, true);
                            token = currentToken(cursor, true);
                            khKeywordToken keyword = token.keyword;
                            khToken_delete(&token);

                            switch (keyword) {
                                case khKeywordToken_DEF: {
                                    *cursor = previous;
                                    khAstFunction function = sparseFunction(cursor);
                                    ast = (khAst){.begin = origin,
                                                  .end = *cursor,
                                                  .type = khAstType_FUNCTION,
                                                  .function = function};
                                    goto end;
                                }

                                case khKeywordToken_CLASS: {
                                    *cursor = previous;
                                    khAstClass class_v = sparseClass(cursor);
                                    ast = (khAst){.begin = origin,
                                                  .end = *cursor,
                                                  .type = khAstType_CLASS,
                                                  .class_v = class_v};
                                    goto end;
                                }

                                case khKeywordToken_STRUCT: {
                                    *cursor = previous;
                                    khAstStruct struct_v = sparseStruct(cursor);
                                    ast = (khAst){.begin = origin,
                                                  .end = *cursor,
                                                  .type = khAstType_STRUCT,
                                                  .struct_v = struct_v};
                                    goto end;
                                }

                                case khKeywordToken_ALIAS: {
                                    *cursor = previous;
                                    khAstAlias alias = sparseAlias(cursor);
                                    ast = (khAst){.begin = origin,
                                                  .end = *cursor,
                                                  .type = khAstType_ALIAS,
                                                  .alias = alias};
                                    goto end;
                                }

                                default:
                                    // TODO: handle error
                                    goto end;
                            }
                        }

                        default:
                            // TODO: handle error
                            goto end;
                    }
                }
                else {
                    // TODO: handle error
                    goto end;
                }
            }

            case khKeywordToken_IF: {
                khAstIfBranch if_branch = sparseIfBranch(cursor);
                ast = (khAst){.begin = origin,
                              .end = *cursor,
                              .type = khAstType_IF_BRANCH,
                              .if_branch = if_branch};
                goto end;
            }

            case khKeywordToken_ELIF:
                // TODO: handle error
                goto end;

            case khKeywordToken_ELSE:
                // TODO: handle error
                goto end;

            case khKeywordToken_FOR:
                ast = sparseForLoop(cursor);
                goto end;

            case khKeywordToken_WHILE: {
                khAstWhileLoop while_loop = sparseWhileLoop(cursor);
                ast = (khAst){.begin = origin,
                              .end = *cursor,
                              .type = khAstType_WHILE_LOOP,
                              .while_loop = while_loop};
                goto end;
            }

            case khKeywordToken_DO: {
                khAstDoWhileLoop do_while_loop = sparseDoWhileLoop(cursor);
                ast = (khAst){.begin = origin,
                              .end = *cursor,
                              .type = khAstType_DO_WHILE_LOOP,
                              .do_while_loop = do_while_loop};
                goto end;
            }

            case khKeywordToken_BREAK: {
                khAstBreak break_v = sparseBreak(cursor);
                ast = (khAst){
                    .begin = origin, .end = *cursor, .type = khAstType_BREAK, .break_v = break_v};
                goto end;
            }

            case khKeywordToken_CONTINUE: {
                khAstContinue continue_v = sparseContinue(cursor);
                ast = (khAst){.begin = origin,
                              .end = *cursor,
                              .type = khAstType_CONTINUE,
                              .continue_v = continue_v};
                goto end;
            }

            case khKeywordToken_RETURN: {
                khAstReturn return_v = sparseReturn(cursor);
                ast = (khAst){
                    .begin = origin, .end = *cursor, .type = khAstType_RETURN, .return_v = return_v};
                goto end;
            }

            default:
                break;
        }
    }
    else if (token.type == khTokenType_EOF) {
        khToken_delete(&token);
        // TODO: handle error
        goto end;
    }

    // Parses expression in the body if none of the keywords above are found
    khAstExpression expression = kh_parseExpression(cursor, false, false);
    khToken_delete(&token);
    token = currentToken(cursor, false);

    // Ensures EOF, newline, or semicolon
    if (token.type == khTokenType_EOF || token.type == khTokenType_NEWLINE ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor, false);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        // Still skips a token, to prevent other being stuck at the same token
        skipToken(cursor, false);
        // TODO: handle error
    }

    khToken_delete(&token);
    ast = (khAst){
        .begin = origin, .end = *cursor, .type = khAstType_EXPRESSION, .expression = expression};

end:
    return ast;
}

static khArray(khAst) sparseBlock(char32_t** cursor) {
    khArray(khAst) block = khArray_new(khAst, khAst_delete);
    khToken token = currentToken(cursor, true);

    // Ensures opening bracket
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_CURLY_BRACKET_OPEN) {
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    // Gets the next khAst until closing bracket
    while (!(token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE)) {
        // To make sure it won't get into an infinite loop
        if (token.type == khTokenType_EOF) {
            khToken_delete(&token);
            goto end;
        }

        khToken_delete(&token);
        khArray_append(&block, sparseStatement(cursor));
        token = currentToken(cursor, true);
    }

    skipToken(cursor, true);
    khToken_delete(&token);

end:
    return block;
}

static khAstImport sparseImport(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstImport import_v = {.path = khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t)),
                            .relative = false,
                            .optional_alias = NULL};

    // Ensures `import` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_IMPORT) {
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    // For relative imports, `import .a_script_file_in_the_same_folder;`
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        import_v.relative = true;
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    // Minimum one identifier
    if (token.type == khTokenType_IDENTIFIER) {
        khArray_append(&import_v.path, khArray_copy(&token.identifier, NULL));
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    // Continues on
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);

        if (token.type == khTokenType_IDENTIFIER) {
            khArray_append(&import_v.path, khArray_copy(&token.identifier, NULL));
            khToken_delete(&token);
            skipToken(cursor, false);
            token = currentToken(cursor, false);
        }
        else {
            // TODO: handle error
        }
    }

    // `import something as another;`
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_AS) {
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);

        if (token.type == khTokenType_IDENTIFIER) {
            import_v.optional_alias = malloc(sizeof(khArray(char)*));
            *import_v.optional_alias = khArray_copy(&token.identifier, NULL);
            khToken_delete(&token);
            skipToken(cursor, false);
            token = currentToken(cursor, false);
        }
        else {
            // TODO: handle error
        }
    }

    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    khToken_delete(&token);

    return import_v;
}

static khAstInclude sparseInclude(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstInclude include = {.path = khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t)),
                            .relative = false};

    // Ensures `include` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_INCLUDE) {
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    // For relative includes, `include .a_script_file_in_the_same_folder;`
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        include.relative = true;
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    // Minimum one identifier
    if (token.type == khTokenType_IDENTIFIER) {
        khArray_append(&include.path, khArray_copy(&token.identifier, NULL));
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    // Continues on
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);

        if (token.type == khTokenType_IDENTIFIER) {
            khArray_append(&include.path, khArray_copy(&token.identifier, NULL));
            khToken_delete(&token);
            skipToken(cursor, false);
            token = currentToken(cursor, false);
        }
        else {
            // TODO: handle error
        }
    }

    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    khToken_delete(&token);

    return include;
}

static inline void sparseFunctionOrLambda(char32_t** cursor, khArray(khAstExpression) * arguments,
                                          khAstExpression** optional_variadic_argument,
                                          khAstExpression** optional_return_type,
                                          khArray(khAst) * content) {
    khToken token = currentToken(cursor, false);

    // Starts out by parsing the argument
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_PARENTHESES_OPEN) {
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    while (!(token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_PARENTHESES_CLOSE)) {
        // End variadic argument
        if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_ELLIPSIS) {
            skipToken(cursor, true);

            *optional_variadic_argument = malloc(sizeof(khAstExpression));
            **optional_variadic_argument = exparseVariableDeclaration(cursor, true);

            khToken_delete(&token);
            token = currentToken(cursor, true);

            if (!(token.type == khTokenType_DELIMITER &&
                  token.delimiter == khDelimiterToken_PARENTHESES_CLOSE)) {
                // TODO: handle error
            }

            break;
        }

        // Parse argument
        khArray_append(arguments, exparseVariableDeclaration(cursor, true));
        khToken_delete(&token);
        token = currentToken(cursor, true);

        if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
            khToken_delete(&token);
            skipToken(cursor, true);
            token = currentToken(cursor, true);
        }
        else if (token.type == khTokenType_DELIMITER &&
                 token.delimiter == khDelimiterToken_PARENTHESES_CLOSE) {
            // Do nothing
        }
        else if (token.type == khTokenType_EOF) {
            // TODO: handle error
            break;
        }
        else {
            // TODO: handle error
            khToken_delete(&token);
            skipToken(cursor, true);
            token = currentToken(cursor, true);
        }
    }

    khToken_delete(&token);
    skipToken(cursor, true);
    token = currentToken(cursor, true);

    // Optional return type
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_ARROW) {
        skipToken(cursor, true);

        *optional_return_type = malloc(sizeof(khAstExpression));
        **optional_return_type = kh_parseExpression(cursor, true, true);

        khToken_delete(&token);
        token = currentToken(cursor, true);
    }

    // Body
    *content = sparseBlock(cursor);

    khToken_delete(&token);
}

static khAstFunction sparseFunction(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstFunction function = {.is_incase = false,
                              .is_static = false,
                              .arguments = khArray_new(khAstExpression, khAstExpression_delete),
                              .optional_variadic_argument = NULL,
                              .optional_return_type = NULL};

    // Any specifiers: `incase static def function() { ... }`
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_INCASE) {
        function.is_incase = true;
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_STATIC) {
        function.is_static = true;
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    // Ensures `def` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_DEF) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    function.name_point = kh_parseExpression(cursor, false, true);

    sparseFunctionOrLambda(cursor, &function.arguments, &function.optional_variadic_argument,
                           &function.optional_return_type, &function.content);

    khToken_delete(&token);

    return function;
}

static inline void sparseClassOrStruct(char32_t** cursor, khArray(char32_t) * name,
                                       khArray(khArray(char32_t)) * template_arguments,
                                       khAstExpression** optional_base_type, khArray(khAst) * content) {
    khToken token = currentToken(cursor, false);

    // Ensures the name identifier of the class or struct
    if (token.type == khTokenType_IDENTIFIER) {
        *name = khArray_copy(&token.identifier, NULL);
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }
    else {
        *name = khArray_new(char32_t, NULL);
        // TODO: handle error
    }

    // Any template args
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_EXCLAMATION) {
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);

        // Single template argument: `class Name!T`
        if (token.type == khTokenType_IDENTIFIER) {
            khArray_append(template_arguments, khArray_copy(&token.identifier, NULL));
            khToken_delete(&token);
            skipToken(cursor, false);
            token = currentToken(cursor, false);
        }
        // Multiple template arguments in parentheses: `class Name!(T, U)`
        else if (token.type == khTokenType_DELIMITER &&
                 token.delimiter == khDelimiterToken_PARENTHESES_OPEN) {
            do {
                khToken_delete(&token);
                skipToken(cursor, true);
                token = currentToken(cursor, true);

                if (token.type == khTokenType_IDENTIFIER) {
                    khArray_append(template_arguments, khArray_copy(&token.identifier, NULL));
                }
                else {
                    // TODO: handle error
                }

                khToken_delete(&token);
                skipToken(cursor, true);
                token = currentToken(cursor, true);
            } while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA);

            if (token.type == khTokenType_DELIMITER &&
                token.delimiter == khDelimiterToken_PARENTHESES_CLOSE) {
                khToken_delete(&token);
                skipToken(cursor, false);
                token = currentToken(cursor, false);
            }
            else {
                // TODO: handle error
            }
        }
        else {
            // TODO: handle error
        }
    }

    // If it's inheriting something: `class Name(Base)`
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_PARENTHESES_OPEN) {
        skipToken(cursor, true);
        *optional_base_type = malloc(sizeof(khAstExpression));
        **optional_base_type = kh_parseExpression(cursor, true, true);

        khToken_delete(&token);
        token = currentToken(cursor, true);

        if (token.type == khTokenType_DELIMITER &&
            token.delimiter == khDelimiterToken_PARENTHESES_CLOSE) {
            skipToken(cursor, true);
        }
        else {
            // TODO: handle error
        }
    }

    // Parses its content
    *content = sparseBlock(cursor);

    khToken_delete(&token);
}

static khAstClass sparseClass(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstClass class_v = {.is_incase = false,
                          .name = NULL,
                          .template_arguments =
                              khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t)),
                          .optional_base_type = NULL,
                          .content = khArray_new(khAst, khAst_delete)};

    // Any specifiers: `incase class E { ... }`
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_INCASE) {
        class_v.is_incase = true;
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_STATIC) {
        // TODO: handle error
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    // Ensures `class` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_CLASS) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    sparseClassOrStruct(cursor, &class_v.name, &class_v.template_arguments, &class_v.optional_base_type,
                        &class_v.content);

    khToken_delete(&token);

    return class_v;
}

static khAstStruct sparseStruct(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstStruct struct_v = {.is_incase = false,
                            .name = NULL,
                            .template_arguments =
                                khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t)),
                            .content = khArray_new(khAst, khAst_delete)};

    // Any specifiers: `incase struct E { ... }`
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_INCASE) {
        struct_v.is_incase = true;
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_STATIC) {
        // TODO: handle error
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    // Ensures `struct` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_STRUCT) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    sparseClassOrStruct(cursor, &struct_v.name, &struct_v.template_arguments,
                        &struct_v.optional_base_type, &struct_v.content);

    khToken_delete(&token);

    return struct_v;
}

static khAstEnum sparseEnum(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstEnum enum_v = {.name = NULL,
                        .members = khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t))};

    // Ensures `enum` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ENUM) {
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    // Its name
    if (token.type == khTokenType_IDENTIFIER) {
        enum_v.name = khArray_copy(&token.identifier, NULL);
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }
    else {
        enum_v.name = khArray_new(char32_t, NULL);
        // TODO: handle error
    }

    // Its members
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_CURLY_BRACKET_OPEN) {
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, true);

        do {
            if (token.type == khTokenType_IDENTIFIER) {
                khArray_append(&enum_v.members, khArray_copy(&token.identifier, NULL));
            }
            else {
                // TODO: handle error
            }

            khToken_delete(&token);
            skipToken(cursor, true);
            token = currentToken(cursor, true);
        } while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA);

        // Ensures closing bracket at the end
        if (token.type == khTokenType_DELIMITER &&
            token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
            skipToken(cursor, true);
        }
        else {
            // TODO: handle error
        }
    }
    else {
        // TODO: handle error
    }

    khToken_delete(&token);

    return enum_v;
}

static khAstAlias sparseAlias(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstAlias alias = {.is_incase = false,
                        .name = NULL,
                        .expression = (khAstExpression){.type = khAstExpressionType_INVALID}};

    // Any specifiers
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_INCASE) {
        alias.is_incase = true;
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_STATIC) {
        // TODO: handle error
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }

    // Ensures `alias` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ALIAS) {
        khToken_delete(&token);
        skipToken(cursor, false);
        token = currentToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    // Its name
    if (token.type == khTokenType_IDENTIFIER) {
        alias.name = khArray_copy(&token.identifier, NULL);
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, true);
    }
    else {
        alias.name = khArray_new(char32_t, NULL);
        // TODO: handle error
    }

    // Its alias expression
    alias.expression = kh_parseExpression(cursor, false, false);
    token = currentToken(cursor, true);

    // Ensures a newline or a semicolon at the end
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    khToken_delete(&token);

    return alias;
}


static khAstIfBranch sparseIfBranch(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstIfBranch if_branch =
        (khAstIfBranch){.branch_conditions = khArray_new(khAstExpression, khAstExpression_delete),
                        .branch_contents = khArray_new(khArray(khAst), khArray_arrayDeleter(khAst)),
                        .else_content = khArray_new(khAst, khAst_delete)};

    // Ensures initial `if` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_IF) {
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    goto in;
    do {
        skipToken(cursor, true);
    in:
        khArray_append(&if_branch.branch_conditions, kh_parseExpression(cursor, false, false));
        khArray_append(&if_branch.branch_contents, sparseBlock(cursor));

        khToken_delete(&token);
        token = currentToken(cursor, true);

        // Loop back again if there are any `elif` keywords
    } while (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ELIF);

    // End optional `else` content
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ELSE) {
        skipToken(cursor, true);
        if_branch.else_content = sparseBlock(cursor);
    }

    khToken_delete(&token);

    return if_branch;
}

static khAstWhileLoop sparseWhileLoop(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstWhileLoop while_loop = {.condition = (khAstExpression){.type = khAstExpressionType_INVALID},
                                 .content = NULL};

    // Ensures `while` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_WHILE) {
        skipToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    // Its condition and block content
    while_loop.condition = kh_parseExpression(cursor, false, false);
    while_loop.content = sparseBlock(cursor);

    khToken_delete(&token);

    return while_loop;
}

static khAstDoWhileLoop sparseDoWhileLoop(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstDoWhileLoop do_while_loop = {
        .condition = (khAstExpression){.type = khAstExpressionType_INVALID}, .content = NULL};

    // Ensures `do` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_DO) {
        skipToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    // Its content
    do_while_loop.content = sparseBlock(cursor);

    khToken_delete(&token);
    token = currentToken(cursor, true);

    // Ensures `while` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_WHILE) {
        skipToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    // And its condition
    do_while_loop.condition = kh_parseExpression(cursor, false, false);

    khToken_delete(&token);
    token = currentToken(cursor, true);

    // Ensures a newline or a semicolon at the end
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    khToken_delete(&token);

    return do_while_loop;
}

static khAst sparseForLoop(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    char32_t* origin = token.begin;

    // Ensures `for` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_FOR) {
        skipToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    khArray(khAstExpression) expressions = khArray_new(khAstExpression, khAstExpression_delete);

    // Its iterator/initialization/condition/update expressions
    goto skip;
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
        skipToken(cursor, true);
    skip:
        khArray_append(&expressions, kh_parseExpression(cursor, false, false));
        khToken_delete(&token);
        token = currentToken(cursor, true);
    };

    // Hints that it's a for each loop
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_IN) {
        skipToken(cursor, true);
        khToken_delete(&token);

        khAstExpression iteratee = kh_parseExpression(cursor, false, false);
        khArray(khAst) content = sparseBlock(cursor);
        return (khAst){
            .begin = origin,
            .end = *cursor,
            .type = khAstType_FOR_EACH_LOOP,
            .for_each_loop = {.iterators = expressions, .iteratee = iteratee, .content = content}};
    }
    // Hints that it's just a normal C style for loop
    else if (khArray_size(&expressions) == 3) {
        khToken_delete(&token);

        khArray(khAst) content = sparseBlock(cursor);
        return (khAst){.begin = origin,
                       .end = *cursor,
                       .type = khAstType_FOR_LOOP,
                       .for_loop = {.initial_expression = expressions[0],
                                    .loop_condition = expressions[1],
                                    .update_expression = expressions[2],
                                    .content = content}};
    }
    else {
        khToken_delete(&token);
        khArray_delete(&expressions);
        // TODO: handle error
        return (khAst){.begin = origin, .end = *cursor, .type = khAstType_INVALID};
    }
}

static khAstBreak sparseBreak(char32_t** cursor) {
    khToken token = currentToken(cursor, true);

    // Ensures `break` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_BREAK) {
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    // Ensure it ends with a newline or a semicolon
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    khToken_delete(&token);

    return (khAstBreak){};
}

static khAstContinue sparseContinue(char32_t** cursor) {
    khToken token = currentToken(cursor, true);

    // Ensures `continue` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_CONTINUE) {
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    // Ensure it ends with a newline or a semicolon
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    khToken_delete(&token);

    return (khAstContinue){};
}

static khAstReturn sparseReturn(char32_t** cursor) {
    khToken token = currentToken(cursor, true);

    // Ensures `return` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_RETURN) {
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    // Instant return in a non-returning function
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        khToken_delete(&token);
        skipToken(cursor, false);

        return (khAstReturn){.values = khArray_new(khAstExpression, khAstExpression_delete)};
    }

    khArray(khAstExpression) values = khArray_new(khAstExpression, khAstExpression_delete);

    // Its return values
    goto skip;
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
        skipToken(cursor, true);
    skip:
        khToken_delete(&token);
        khArray_append(&values, kh_parseExpression(cursor, false, false));
        token = currentToken(cursor, true);
    };

    // Ensure it ends with a newline or a semicolon
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor, false);
    }
    else {
        // TODO: handle error
    }

    khToken_delete(&token);

    return (khAstReturn){.values = values};
}


// Macro to do recursive descent for binary operators in a switch statement
#define RCD_BINARY(LOWER, TOKEN_OPERATOR, OPERATOR)                                                 \
    khToken token = currentToken(cursor, ignore_newline);                                           \
    char32_t* origin = token.begin;                                                                 \
    khToken_delete(&token);                                                                         \
                                                                                                    \
    khAstExpression expression = LOWER(cursor, ignore_newline, filter_type);                        \
                                                                                                    \
    if (filter_type) {                                                                              \
        return expression;                                                                          \
    }                                                                                               \
                                                                                                    \
    token = currentToken(cursor, ignore_newline);                                                   \
                                                                                                    \
    while (token.type == khTokenType_OPERATOR && token.operator_v == TOKEN_OPERATOR) {              \
        skipToken(cursor, ignore_newline);                                                          \
                                                                                                    \
        khAstExpression* left = malloc(sizeof(khAstExpression));                                    \
        *left = expression;                                                                         \
                                                                                                    \
        khAstExpression* right = malloc(sizeof(khAstExpression));                                   \
        *right = LOWER(cursor, ignore_newline, filter_type);                                        \
                                                                                                    \
        expression = (khAstExpression){.begin = origin,                                             \
                                       .end = *cursor,                                              \
                                       .type = khAstExpressionType_BINARY,                          \
                                       .binary = {.type = OPERATOR, .left = left, .right = right}}; \
                                                                                                    \
        khToken_delete(&token);                                                                     \
        token = currentToken(cursor, ignore_newline);                                               \
    }                                                                                               \
                                                                                                    \
    khToken_delete(&token);                                                                         \
                                                                                                    \
    return expression;


// Macro to do recursive descent for a single binary operator as the whole function content
#define RCD_BINARY_CASE(LOWER, OPERATOR)                                                            \
    {                                                                                               \
        skipToken(cursor, ignore_newline);                                                          \
                                                                                                    \
        khAstExpression* left = malloc(sizeof(khAstExpression));                                    \
        *left = expression;                                                                         \
                                                                                                    \
        khAstExpression* right = malloc(sizeof(khAstExpression));                                   \
        *right = LOWER(cursor, ignore_newline, filter_type);                                        \
                                                                                                    \
        expression = (khAstExpression){.begin = origin,                                             \
                                       .end = *cursor,                                              \
                                       .type = khAstExpressionType_BINARY,                          \
                                       .binary = {.type = OPERATOR, .left = left, .right = right}}; \
                                                                                                    \
        khToken_delete(&token);                                                                     \
        token = currentToken(cursor, ignore_newline);                                               \
    }                                                                                               \
    break;


static khAstExpression exparseInplaceOperators(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseTernary(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // All inplace operators
    while (token.type == khTokenType_OPERATOR) {
        switch (token.operator_v) {
            case khOperatorToken_IADD:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IADD);
            case khOperatorToken_ISUB:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_ISUB);
            case khOperatorToken_IMUL:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IMUL);
            case khOperatorToken_IDIV:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IDIV);
            case khOperatorToken_IMOD:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IMOD);
            case khOperatorToken_IPOW:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IPOW);
            case khOperatorToken_IDOT:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IDOT);

            case khOperatorToken_ASSIGN:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_ASSIGN);

            case khOperatorToken_IBIT_AND:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IBIT_AND);
            case khOperatorToken_IBIT_OR:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IBIT_OR);
            case khOperatorToken_IBIT_XOR:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IBIT_XOR);
            case khOperatorToken_IBIT_LSHIFT:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IBIT_LSHIFT);
            case khOperatorToken_IBIT_RSHIFT:
                RCD_BINARY_CASE(exparseTernary, khAstBinaryExpressionType_IBIT_RSHIFT);

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);

    return expression;
}

static khAstExpression exparseTernary(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseLogicalOr(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // Hints that it's a ternary operation once `if` keyword is found after an expression
    while (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_IF) {
        skipToken(cursor, ignore_newline);

        // Its condition
        khAstExpression* condition = malloc(sizeof(khAstExpression));
        *condition = exparseLogicalOr(cursor, ignore_newline, filter_type);

        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);

        // Ensures the `else` keyword before the otherwise value
        if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ELSE) {
            skipToken(cursor, ignore_newline);
        }
        else {
            // TODO: handle error
        }

        // Its otherwise value
        khAstExpression* otherwise = malloc(sizeof(khAstExpression));
        *otherwise = exparseLogicalOr(cursor, ignore_newline, filter_type);

        khAstExpression* value = malloc(sizeof(khAstExpression));
        *value = expression;

        expression = (khAstExpression){
            .begin = origin,
            .end = *cursor,
            .type = khAstExpressionType_TERNARY,
            .ternary = {.value = value, .condition = condition, .otherwise = otherwise}};

        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }

    khToken_delete(&token);

    return expression;
}

static khAstExpression exparseLogicalOr(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseLogicalXor, khOperatorToken_OR, khAstBinaryExpressionType_OR);
}

static khAstExpression exparseLogicalXor(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseLogicalAnd, khOperatorToken_XOR, khAstBinaryExpressionType_XOR);
}

static khAstExpression exparseLogicalAnd(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseLogicalNot, khOperatorToken_AND, khAstBinaryExpressionType_AND);
}

static khAstExpression exparseLogicalNot(char32_t** cursor, EXPARSE_ARGS) {
    if (filter_type) {
        return exparseComparisonOperators(cursor, ignore_newline, filter_type);
    }

    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;

    // Self-explanatory
    if (token.type == khTokenType_OPERATOR && token.operator_v == khOperatorToken_NOT) {
        skipToken(cursor, ignore_newline);
        khToken_delete(&token);

        khAstExpression* expression = malloc(sizeof(khAstExpression));
        *expression = exparseLogicalNot(cursor, ignore_newline, filter_type);

        return (khAstExpression){
            .begin = origin,
            .end = *cursor,
            .type = khAstExpressionType_UNARY,
            .unary = {.type = khAstUnaryExpressionType_NOT, .operand = expression}};
    }
    else {
        khToken_delete(&token);
        return exparseComparisonOperators(cursor, ignore_newline, filter_type);
    }
}

static khAstExpression exparseComparisonOperators(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseBitwiseOr(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // If any comparison operators found, start these mess
    if (token.type == khTokenType_OPERATOR &&
        (token.operator_v == khOperatorToken_EQUAL || token.operator_v == khOperatorToken_NOT_EQUAL ||
         token.operator_v == khOperatorToken_LESS || token.operator_v == khOperatorToken_MORE ||
         token.operator_v == khOperatorToken_ELESS || token.operator_v == khOperatorToken_EMORE)) {
        khArray(khAstComparisonExpressionType) operations =
            khArray_new(khAstComparisonExpressionType, NULL);
        khArray(khAstExpression) operands = khArray_new(khAstExpression, khAstExpression_delete);
        khArray_append(&operands, expression);

        // Maps the khOperatorToken of the comparison operator to khAstComparisonExpressionType
        while (token.type == khTokenType_OPERATOR) {
            switch (token.operator_v) {
                case khOperatorToken_EQUAL:
                    khArray_append(&operations, khAstComparisonExpressionType_EQUAL);
                    break;
                case khOperatorToken_NOT_EQUAL:
                    khArray_append(&operations, khAstComparisonExpressionType_NOT_EQUAL);
                    break;
                case khOperatorToken_LESS:
                    khArray_append(&operations, khAstComparisonExpressionType_LESS);
                    break;
                case khOperatorToken_MORE:
                    khArray_append(&operations, khAstComparisonExpressionType_MORE);
                    break;
                case khOperatorToken_ELESS:
                    khArray_append(&operations, khAstComparisonExpressionType_ELESS);
                    break;
                case khOperatorToken_EMORE:
                    khArray_append(&operations, khAstComparisonExpressionType_EMORE);
                    break;

                default:
                    goto out;
            }

            skipToken(cursor, ignore_newline);
            khArray_append(&operands, exparseBitwiseOr(cursor, ignore_newline, filter_type));

            khToken_delete(&token);
            token = currentToken(cursor, ignore_newline);
        }
    out:

        expression = (khAstExpression){.begin = origin,
                                       .end = *cursor,
                                       .type = khAstExpressionType_COMPARISON,
                                       .comparison = {.operations = operations, .operands = operands}};
    }


    khToken_delete(&token);

    return expression;
}

static khAstExpression exparseBitwiseOr(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseBitwiseXor, khOperatorToken_BIT_OR, khAstBinaryExpressionType_BIT_OR);
}

static khAstExpression exparseBitwiseXor(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseBitwiseAnd, khOperatorToken_BIT_XOR, khAstBinaryExpressionType_BIT_XOR);
}

static khAstExpression exparseBitwiseAnd(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseBitwiseShifts, khOperatorToken_BIT_AND, khAstBinaryExpressionType_BIT_AND);
}

static khAstExpression exparseBitwiseShifts(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseAddSub(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // Self-explanatory
    while (token.type == khTokenType_OPERATOR) {
        switch (token.operator_v) {
            case khOperatorToken_BIT_LSHIFT:
                RCD_BINARY_CASE(exparseAddSub, khAstBinaryExpressionType_BIT_LSHIFT);
            case khOperatorToken_BIT_RSHIFT:
                RCD_BINARY_CASE(exparseAddSub, khAstBinaryExpressionType_BIT_RSHIFT);

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);

    return expression;
}

static khAstExpression exparseAddSub(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseMulDivMod(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // Self-explanatory
    while (token.type == khTokenType_OPERATOR) {
        switch (token.operator_v) {
            case khOperatorToken_ADD:
                RCD_BINARY_CASE(exparseMulDivMod, khAstBinaryExpressionType_ADD);
            case khOperatorToken_SUB:
                RCD_BINARY_CASE(exparseMulDivMod, khAstBinaryExpressionType_SUB);

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);

    return expression;
}

static khAstExpression exparseMulDivMod(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparsePow(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // Self-explanatory
    while (token.type == khTokenType_OPERATOR) {
        switch (token.operator_v) {
            case khOperatorToken_MUL:
                RCD_BINARY_CASE(exparsePow, khAstBinaryExpressionType_MUL);
            case khOperatorToken_DIV:
                RCD_BINARY_CASE(exparsePow, khAstBinaryExpressionType_DIV);
            case khOperatorToken_MOD:
                RCD_BINARY_CASE(exparsePow, khAstBinaryExpressionType_MOD);

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);

    return expression;
}

static khAstExpression exparsePow(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseRef, khOperatorToken_POW, khAstBinaryExpressionType_POW);
}

static khAstExpression exparseRef(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    bool is_ref = false;

    // Self-explanatory
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_REF) {
        skipToken(cursor, ignore_newline);
        is_ref = true;
    }

    khAstExpression expression = exparseUnary(cursor, ignore_newline, filter_type || is_ref);
    if (is_ref) {
        khAstExpression* value = malloc(sizeof(khAstExpression));
        *value = expression;
        expression = (khAstExpression){
            .begin = origin, .end = *cursor, .type = khAstExpressionType_REF, .ref = {.value = value}};
    }

    khToken_delete(&token);

    return expression;
}


// Like RCD_BINARY_CASE, but for unary operators
#define RCD_UNARY_CASE(FUNCTION, OPERATOR)                                           \
    {                                                                                \
        skipToken(cursor, ignore_newline);                                           \
        khToken_delete(&token);                                                      \
                                                                                     \
        khAstExpression* expression = malloc(sizeof(khAstExpression));               \
        *expression = exparseUnary(cursor, ignore_newline, filter_type);             \
        return (khAstExpression){                                                    \
            .begin = origin,                                                         \
            .end = *cursor,                                                          \
            .type = khAstExpressionType_UNARY,                                       \
            .unary = {.type = khAstUnaryExpressionType_NOT, .operand = expression}}; \
    }


static khAstExpression exparseUnary(char32_t** cursor, EXPARSE_ARGS) {
    if (filter_type) {
        return exparseReverseUnary(cursor, ignore_newline, filter_type);
    }

    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;

    // All same-precedence unary operators
    if (token.type == khTokenType_OPERATOR) {
        switch (token.operator_v) {
            case khOperatorToken_ADD:
                RCD_UNARY_CASE(exparseReverseUnary, khAstUnaryExpressionType_POSITIVE);
            case khOperatorToken_SUB:
                RCD_UNARY_CASE(exparseReverseUnary, khAstUnaryExpressionType_NEGATIVE);

            case khOperatorToken_INCREMENT:
                RCD_UNARY_CASE(exparseReverseUnary, khAstUnaryExpressionType_PRE_INCREMENT);
            case khOperatorToken_DECREMENT:
                RCD_UNARY_CASE(exparseReverseUnary, khAstUnaryExpressionType_PRE_DECREMENT);

            case khOperatorToken_NOT:
                RCD_UNARY_CASE(exparseReverseUnary, khAstUnaryExpressionType_NOT);
            case khOperatorToken_BIT_NOT:
                RCD_UNARY_CASE(exparseReverseUnary, khAstUnaryExpressionType_BIT_NOT);

            default:
                khToken_delete(&token);
                return exparseReverseUnary(cursor, ignore_newline, filter_type);
        }
    }
    else {
        khToken_delete(&token);
        return exparseReverseUnary(cursor, ignore_newline, filter_type);
    }
}

static khAstExpression exparseReverseUnary(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseScopeTemplatization(cursor, ignore_newline, filter_type);
    token = currentToken(cursor, ignore_newline);

    while (token.type == khTokenType_DELIMITER || token.type == khTokenType_OPERATOR) {
        switch (token.type) {
            case khTokenType_DELIMITER:
                switch (token.delimiter) {
                    case khDelimiterToken_PARENTHESES_OPEN: {
                        if (filter_type) {
                            goto out;
                        }

                        khArray(khAstExpression) arguments = exparseList(
                            cursor, khDelimiterToken_PARENTHESES_OPEN,
                            khDelimiterToken_PARENTHESES_CLOSE, ignore_newline, filter_type);

                        khAstExpression* callee = malloc(sizeof(khAstExpression));
                        *callee = expression;

                        expression =
                            (khAstExpression){.begin = origin,
                                              .end = *cursor,
                                              .type = khAstExpressionType_CALL,
                                              .call = {.callee = callee, .arguments = arguments}};

                        khToken_delete(&token);
                        token = currentToken(cursor, ignore_newline);
                    } break;

                    case khDelimiterToken_SQUARE_BRACKET_OPEN: {
                        khArray(khAstExpression) arguments = exparseList(
                            cursor, khDelimiterToken_SQUARE_BRACKET_OPEN,
                            khDelimiterToken_SQUARE_BRACKET_CLOSE, ignore_newline, filter_type);

                        khAstExpression* indexee = malloc(sizeof(khAstExpression));
                        *indexee = expression;

                        expression =
                            (khAstExpression){.begin = origin,
                                              .end = *cursor,
                                              .type = khAstExpressionType_INDEX,
                                              .index = {.indexee = indexee, .arguments = arguments}};

                        khToken_delete(&token);
                        token = currentToken(cursor, ignore_newline);
                    } break;

                    default:
                        goto out;
                }
                break;

            case khTokenType_OPERATOR:
                switch (token.operator_v) {
                    case khOperatorToken_INCREMENT: {
                        if (filter_type) {
                            goto out;
                        }

                        khAstExpression* operand = malloc(sizeof(khAstExpression));
                        *operand = expression;

                        expression =
                            (khAstExpression){.begin = origin,
                                              .end = *cursor,
                                              .type = khAstExpressionType_UNARY,
                                              .unary = {.type = khAstUnaryExpressionType_POST_INCREMENT,
                                                        .operand = operand}};

                        khToken_delete(&token);
                        skipToken(cursor, ignore_newline);
                        token = currentToken(cursor, ignore_newline);
                    } break;

                    case khOperatorToken_DECREMENT: {
                        if (filter_type) {
                            goto out;
                        }

                        khAstExpression* operand = malloc(sizeof(khAstExpression));
                        *operand = expression;

                        expression =
                            (khAstExpression){.begin = origin,
                                              .end = *cursor,
                                              .type = khAstExpressionType_UNARY,
                                              .unary = {.type = khAstUnaryExpressionType_POST_INCREMENT,
                                                        .operand = operand}};

                        khToken_delete(&token);
                        skipToken(cursor, ignore_newline);
                        token = currentToken(cursor, ignore_newline);
                    } break;

                    default:
                        goto out;
                }

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);

    return expression;
}

static khAstExpression exparseScopeTemplatization(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseOther(cursor, ignore_newline, filter_type);
    token = currentToken(cursor, ignore_newline);

    while (token.type == khTokenType_DELIMITER) {
        switch (token.delimiter) {
            case khDelimiterToken_DOT: {
                khArray(khArray(char32_t)) scope_names =
                    khArray_new(khArray(char32_t), khArray_arrayDeleter(char32_t));

                // `(expression).parses.these.scope.things`
                while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
                    khToken_delete(&token);
                    skipToken(cursor, ignore_newline);
                    token = currentToken(cursor, ignore_newline);

                    if (token.type == khTokenType_IDENTIFIER) {
                        khArray_append(&scope_names, khArray_copy(&token.identifier, NULL));

                        khToken_delete(&token);
                        skipToken(cursor, ignore_newline);
                        token = currentToken(cursor, ignore_newline);
                    }
                    else {
                        // TODO: handle error
                    }
                }

                khAstExpression* value = malloc(sizeof(khAstExpression));
                *value = expression;

                expression = (khAstExpression){.begin = origin,
                                               .end = *cursor,
                                               .type = khAstExpressionType_SCOPE,
                                               .scope = {.value = value, .scope_names = scope_names}};
            } break;

            case khDelimiterToken_EXCLAMATION: {
                khToken_delete(&token);
                skipToken(cursor, ignore_newline);
                token = currentToken(cursor, ignore_newline);

                khAstExpression* value = malloc(sizeof(khAstExpression));
                *value = expression;

                // Single identifier template argument: `Type!int`
                if (token.type == khTokenType_IDENTIFIER) {
                    khArray(khAstExpression) template_arguments =
                        khArray_new(khAstExpression, khAstExpression_delete);

                    khArray_append(
                        &template_arguments,
                        ((khAstExpression){.type = khAstExpressionType_IDENTIFIER,
                                           .identifier = khArray_copy(&token.identifier, NULL)}));

                    expression = (khAstExpression){
                        .begin = origin,
                        .end = *cursor,
                        .type = khAstExpressionType_TEMPLATIZE,
                        .templatize = {.value = value, .template_arguments = template_arguments}};
                }
                // Multiple template arguments: `Type!(int, float)`
                else if (token.type == khTokenType_DELIMITER &&
                         token.delimiter == khDelimiterToken_PARENTHESES_OPEN) {
                    khArray(khAstExpression) template_arguments =
                        exparseList(cursor, khDelimiterToken_PARENTHESES_OPEN,
                                    khDelimiterToken_PARENTHESES_CLOSE, ignore_newline, true);

                    expression = (khAstExpression){
                        .begin = origin,
                        .end = *cursor,
                        .type = khAstExpressionType_TEMPLATIZE,
                        .templatize = {.value = value, .template_arguments = template_arguments}};
                }
                else {
                    free(value);
                    // TODO: handle error
                }
            } break;

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);

    return expression;
}

static khAstExpression exparseOther(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khAstExpression expression = (khAstExpression){.type = khAstExpressionType_INVALID};

    switch (token.type) {
        case khTokenType_IDENTIFIER: {
            char32_t* initial = *cursor;

            skipToken(cursor, ignore_newline);
            khToken next_token = currentToken(cursor, ignore_newline);

            // Variable declaration: `identifier: Type = value`
            if (!filter_type && next_token.type == khTokenType_DELIMITER &&
                next_token.delimiter == khDelimiterToken_COLON) {
                *cursor = initial;
                expression = exparseVariableDeclaration(cursor, ignore_newline);
            }
            else {
                expression = (khAstExpression){.begin = origin,
                                               .end = *cursor,
                                               .type = khAstExpressionType_IDENTIFIER,
                                               .identifier = khArray_copy(&token.identifier, NULL)};
            }

            khToken_delete(&token);
            token = next_token;
        } break;

        case khTokenType_KEYWORD:
            switch (token.keyword) {
                // Lambdas
                case khKeywordToken_DEF:
                    if (filter_type) {
                        // TODO: handle error
                    }

                    expression = exparseLambda(cursor, ignore_newline);
                    break;

                // Variable declarations with specifiers
                case khKeywordToken_STATIC:
                case khKeywordToken_WILD:
                    if (filter_type) {
                        // TODO: handle error
                    }

                    expression = exparseVariableDeclaration(cursor, ignore_newline);
                    break;

                default:
                    // TODO: handle error
                    break;
            }
            break;

        case khTokenType_DELIMITER:
            switch (token.keyword) {
                // Parentheses enclosed expressions or tuples
                case khDelimiterToken_PARENTHESES_OPEN: {
                    khArray(khAstExpression) values =
                        exparseList(cursor, khDelimiterToken_PARENTHESES_OPEN,
                                    khDelimiterToken_PARENTHESES_CLOSE, ignore_newline, filter_type);

                    if (khArray_size(&values) == 1) {
                        expression = khAstExpression_copy(&values[0]);
                        khArray_delete(&values);
                    }
                    else {
                        expression = (khAstExpression){.begin = origin,
                                                       .end = *cursor,
                                                       .type = khAstExpressionType_TUPLE,
                                                       .tuple = {.values = values}};
                    }
                } break;

                // Arrays
                case khDelimiterToken_SQUARE_BRACKET_OPEN:
                    if (filter_type) {
                        // TODO: handle error
                    }

                    expression = (khAstExpression){
                        .type = khAstExpressionType_ARRAY,
                        .array = {.values = exparseList(cursor, khDelimiterToken_SQUARE_BRACKET_OPEN,
                                                        khDelimiterToken_SQUARE_BRACKET_CLOSE,
                                                        ignore_newline, filter_type)}};
                    break;

                default:
                    // TODO: handle error
                    break;
            }
            break;

            // The rest below are constants (v)

        case khTokenType_CHAR:
            if (filter_type) {
                // TODO: handle error
            }

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_CHAR,
                                           .char_v = token.char_v};
            break;

        case khTokenType_STRING:
            if (filter_type) {
                // TODO: handle error
            }

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_STRING,
                                           .string = khArray_copy(&token.string, NULL)};
            break;

        case khTokenType_BUFFER:
            if (filter_type) {
                // TODO: handle error
            }

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_BUFFER,
                                           .buffer = khArray_copy(&token.buffer, NULL)};
            break;

        case khTokenType_BYTE:
            if (filter_type) {
                // TODO: handle error
            }

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){
                .begin = origin, .end = *cursor, .type = khAstExpressionType_BYTE, .byte = token.byte};
            break;

        case khTokenType_INTEGER:
            // Integers should be able to be parsed even with `filter_type`, in order for static array
            // types to be parsed
            /* if (filter_type) {
             *     // TODO: handle error
             * }
             */

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_INTEGER,
                                           .integer = token.integer};
            break;

        case khTokenType_UINTEGER:
            // Same for unsigned integers
            /* if (filter_type) {
             *     // TODO: handle error
             * }
             */

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_UINTEGER,
                                           .uinteger = token.uinteger};
            break;

        case khTokenType_FLOAT:
            if (filter_type) {
                // TODO: handle error
            }

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_FLOAT,
                                           .float_v = token.float_v};
            break;

        case khTokenType_DOUBLE:
            if (filter_type) {
                // TODO: handle error
            }

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_DOUBLE,
                                           .double_v = token.double_v};
            break;

        case khTokenType_IFLOAT:
            if (filter_type) {
                // TODO: handle error
            }

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_IFLOAT,
                                           .ifloat = token.ifloat};
            break;

        case khTokenType_IDOUBLE:
            if (filter_type) {
                // TODO: handle error
            }

            skipToken(cursor, ignore_newline);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_IDOUBLE,
                                           .idouble = token.idouble};
            break;

        default:
            // TODO: handle error
            break;
    }

    khToken_delete(&token);

    return expression;
}

static khAstExpression exparseVariableDeclaration(char32_t** cursor, bool ignore_newline) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = *cursor;
    khAstVariableDeclaration declaration = {.is_static = false,
                                            .is_wild = false,
                                            .name = NULL,
                                            .optional_type = NULL,
                                            .optional_initializer = NULL};


    // `static` specifier
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_STATIC) {
        declaration.is_static = true;
        khToken_delete(&token);
        skipToken(cursor, ignore_newline);
        token = currentToken(cursor, ignore_newline);
    }

    // `wild` specifier
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_WILD) {
        declaration.is_wild = true;
        khToken_delete(&token);
        skipToken(cursor, ignore_newline);
        token = currentToken(cursor, ignore_newline);
    }

    // Its name
    if (token.type == khTokenType_IDENTIFIER) {
        declaration.name = khArray_copy(&token.identifier, NULL);
        khToken_delete(&token);
        skipToken(cursor, ignore_newline);
        token = currentToken(cursor, ignore_newline);
    }
    else {
        declaration.name = khArray_new(char32_t, NULL);
        // TODO: handle error
    }

    // Passes through the colon
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COLON) {
        khToken_delete(&token);
        skipToken(cursor, ignore_newline);
        token = currentToken(cursor, ignore_newline);
    }
    else {
        // TODO: handle error
    }

    // If there's no assign op at first, it's a type: `name: Type`
    if (!(token.type == khTokenType_OPERATOR && token.operator_v == khOperatorToken_ASSIGN)) {
        declaration.optional_type = malloc(sizeof(khAstExpression));
        *declaration.optional_type = kh_parseExpression(cursor, ignore_newline, true);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }

    // Optional initializer
    if (token.type == khTokenType_OPERATOR && token.operator_v == khOperatorToken_ASSIGN) {
        skipToken(cursor, ignore_newline);
        declaration.optional_initializer = malloc(sizeof(khAstExpression));
        *declaration.optional_initializer = kh_parseExpression(cursor, ignore_newline, false);
    }

    khToken_delete(&token);
    return (khAstExpression){.begin = origin,
                             .end = *cursor,
                             .type = khAstExpressionType_VARIABLE_DECLARATION,
                             .variable_declaration = declaration};
}

static khAstExpression exparseLambda(char32_t** cursor, bool ignore_newline) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = *cursor;
    khAstLambdaExpression lambda = {.arguments = khArray_new(khAstExpression, khAstExpression_delete),
                                    .optional_variadic_argument = NULL,
                                    .optional_return_type = NULL};

    // Ensures `def` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_DEF) {
        skipToken(cursor, ignore_newline);
    }
    else {
        // TODO: handle error
    }

    sparseFunctionOrLambda(cursor, &lambda.arguments, &lambda.optional_variadic_argument,
                           &lambda.optional_return_type, &lambda.content);

    khToken_delete(&token);
    return (khAstExpression){
        .begin = origin, .end = *cursor, .type = khAstExpressionType_LAMBDA, .lambda = lambda};
}

static khArray(khAstExpression) exparseList(char32_t** cursor, khDelimiterToken opening_delimiter,
                                            khDelimiterToken closing_delimiter, EXPARSE_ARGS) {
    khArray(khAstExpression) expressions = khArray_new(khAstExpression, khAstExpression_delete);
    khToken token = currentToken(cursor, ignore_newline);

    // Ensures the opening delimiter
    if (token.type == khTokenType_DELIMITER && token.delimiter == opening_delimiter) {
        khToken_delete(&token);
        skipToken(cursor, true);
        token = currentToken(cursor, true);
    }
    else {
        // TODO: handle error
    }

    // Instant close
    if (token.type == khTokenType_DELIMITER && token.delimiter == closing_delimiter) {
        skipToken(cursor, true);
    }
    else {
        while (true) {
            // Avoid infinite loop
            if (token.type == khTokenType_EOF) {
                break;
            }

            khArray_append(&expressions, kh_parseExpression(cursor, true, filter_type));

            khToken_delete(&token);
            token = currentToken(cursor, true);

            // Do nothing after a comma
            if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
                skipToken(cursor, true);
            }
            // Breaks out of the loop after list close
            else if (token.type == khTokenType_DELIMITER && token.delimiter == closing_delimiter) {
                skipToken(cursor, true);
                break;
            }
            else {
                // TODO: handle error
                skipToken(cursor, true);
            }
        }
    }

    khToken_delete(&token);

    return expressions;
}
