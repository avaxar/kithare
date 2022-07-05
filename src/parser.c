/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
 */

#include <kithare/core/error.h>
#include <kithare/core/lexer.h>
#include <kithare/core/parser.h>
#include <kithare/core/token.h>


static inline void raiseError(char32_t* ptr, const char32_t* message) {
    kh_raiseError((khError){.type = khErrorType_PARSER, .message = khstring_new(message), .data = ptr});
}

// Token getter function
static inline khToken currentToken(char32_t** cursor, bool ignore_newline) {
    char32_t* cursor_copy = *cursor;
    khToken token = kh_lexToken(&cursor_copy);

    // Ignoring newlines means that it would lex the next token if a newline token was encountered
    while (token.type == khTokenType_COMMENT || (token.type == khTokenType_NEWLINE && ignore_newline)) {
        khToken_delete(&token);
        *cursor = cursor_copy;
        token = kh_lexToken(&cursor_copy);
    }

    return token;
}

// Basically, a sort of `next` function
static inline void skipToken(char32_t** cursor) {
    char32_t* origin = *cursor;
    khToken token = kh_lexToken(cursor);

    // Ignoring comments, lex the next token
    while (token.type == khTokenType_COMMENT) {
        khToken_delete(&token);
        token = kh_lexToken(cursor);
    }

    khToken_delete(&token);

    // To avoid being stuck at the same token
    if (origin == *cursor && **cursor != U'\0') {
        (*cursor)++;
    }
}

static inline bool isEnd(char32_t** cursor) {
    char32_t* cursor_copy = *cursor;
    khToken token = kh_lexToken(&cursor_copy);

    // Ignoring newlines and comments
    while (token.type == khTokenType_COMMENT || token.type == khTokenType_NEWLINE) {
        khToken_delete(&token);
        token = kh_lexToken(&cursor_copy);
    }

    if (token.type == khTokenType_EOF) {
        khToken_delete(&token);
        return true;
    }
    else {
        khToken_delete(&token);
        return false;
    }
}


kharray(khAstStatement) kh_parse(khstring* string) {
    kharray(khAstStatement) statements = kharray_new(khAstStatement, khAstStatement_delete);
    char32_t* cursor = *string;

    while (!isEnd(&cursor)) {
        kharray_append(&statements, kh_parseStatement(&cursor));
    }

    return statements;
}


// Sub-level parsing levels
static kharray(khAstStatement) sparseBlock(char32_t** cursor);
static void sparseSpecifiers(char32_t** cursor, bool allow_incase, bool* is_incase, bool allow_static,
                             bool* is_static, bool ignore_newline);
static khAstVariable sparseVariable(char32_t** cursor, bool no_static, bool ignore_newline);
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
static khAstForLoop sparseForLoop(char32_t** cursor);
static void sparseBreak(char32_t** cursor);
static void sparseContinue(char32_t** cursor);
static khAstReturn sparseReturn(char32_t** cursor);

// Sub-level expression parsing levels, by lowest to highest precedence
#define EXPARSE_ARGS bool ignore_newline, bool filter_type
static khAstExpression exparseIpAssignmentOperators(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseTernary(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseLogicalOr(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseLogicalXor(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseLogicalAnd(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseLogicalNot(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseComparisonOperators(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseRange(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseBitwiseOr(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseBitwiseXor(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseBitwiseAnd(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseBitwiseShifts(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseAddSub(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseMulDivModDot(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseUnary(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparsePow(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseReverseUnary(char32_t** cursor, EXPARSE_ARGS);
static khAstExpression exparseOther(char32_t** cursor, EXPARSE_ARGS);

static khAstExpression exparseSignature(char32_t** cursor, bool ignore_newline);
static khAstExpression exparseLambda(char32_t** cursor, bool ignore_newline);
static khAstExpression exparseDict(char32_t** cursor, bool ignore_newline);
static kharray(khAstExpression) exparseList(char32_t** cursor, khDelimiterToken opening_delimiter,
                                            khDelimiterToken closing_delimiter, EXPARSE_ARGS);


khAstStatement kh_parseStatement(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    char32_t* origin = token.begin;
    khAstStatement statement =
        (khAstStatement){.begin = origin, .end = *cursor, .type = khAstStatementType_INVALID};

    if (token.type == khTokenType_KEYWORD) {
        switch (token.keyword) {
            case khKeywordToken_IMPORT: {
                khAstImport import_v = sparseImport(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_IMPORT,
                                             .import_v = import_v};
                goto end;
            }

            case khKeywordToken_INCLUDE: {
                khAstInclude include = sparseInclude(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_INCLUDE,
                                             .include = include};
                goto end;
            }

            case khKeywordToken_AS:
                raiseError(token.begin, U"unexpected keyword");
                skipToken(cursor);
                khToken_delete(&token);
                goto end;

            case khKeywordToken_DEF: {
                khAstFunction function = sparseFunction(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_FUNCTION,
                                             .function = function};
                goto end;
            }

            case khKeywordToken_CLASS: {
                khAstClass class_v = sparseClass(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_CLASS,
                                             .class_v = class_v};
                goto end;
            }

            case khKeywordToken_STRUCT: {
                khAstStruct struct_v = sparseStruct(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_STRUCT,
                                             .struct_v = struct_v};
                goto end;
            }

            case khKeywordToken_ENUM: {
                khAstEnum enum_v = sparseEnum(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){
                    .begin = origin, .end = *cursor, .type = khAstStatementType_ENUM, .enum_v = enum_v};
                goto end;
            }

            case khKeywordToken_ALIAS: {
                khAstAlias alias = sparseAlias(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){
                    .begin = origin, .end = *cursor, .type = khAstStatementType_ALIAS, .alias = alias};
                goto end;
            }

            // Should be variable declarations
            case khKeywordToken_WILD:
            case khKeywordToken_REF: {
                goto out;
            }

            // Handling `incase` and `static` modifiers
            case khKeywordToken_INCASE:
            case khKeywordToken_STATIC: {
                char32_t* previous = *cursor;
                sparseSpecifiers(cursor, true, NULL, true, NULL, true);
                khToken_delete(&token);
                token = currentToken(cursor, true);
                *cursor = previous;

                if (token.type == khTokenType_KEYWORD) {
                    switch (token.keyword) {
                        case khKeywordToken_DEF: {
                            khAstFunction function = sparseFunction(cursor);
                            khToken_delete(&token);
                            statement = (khAstStatement){.begin = origin,
                                                         .end = *cursor,
                                                         .type = khAstStatementType_FUNCTION,
                                                         .function = function};
                            goto end;
                        }

                        case khKeywordToken_CLASS: {
                            khAstClass class_v = sparseClass(cursor);
                            khToken_delete(&token);
                            statement = (khAstStatement){.begin = origin,
                                                         .end = *cursor,
                                                         .type = khAstStatementType_CLASS,
                                                         .class_v = class_v};
                            goto end;
                        }

                        case khKeywordToken_STRUCT: {
                            khAstStruct struct_v = sparseStruct(cursor);
                            khToken_delete(&token);
                            statement = (khAstStatement){.begin = origin,
                                                         .end = *cursor,
                                                         .type = khAstStatementType_STRUCT,
                                                         .struct_v = struct_v};
                            goto end;
                        }

                        case khKeywordToken_ALIAS: {
                            khAstAlias alias = sparseAlias(cursor);
                            khToken_delete(&token);
                            statement = (khAstStatement){.begin = origin,
                                                         .end = *cursor,
                                                         .type = khAstStatementType_ALIAS,
                                                         .alias = alias};
                            goto end;
                        }

                        // Might be a variable declaration; might be an expression.
                        case khKeywordToken_WILD:
                        case khKeywordToken_REF:
                        default: {
                            khToken_delete(&token);
                            token = currentToken(cursor, true);
                            goto out;
                        }
                    }
                }
                // Same goes with above
                else {
                    khToken_delete(&token);
                    token = currentToken(cursor, true);
                    goto out;
                }
            }

            case khKeywordToken_IF: {
                khAstIfBranch if_branch = sparseIfBranch(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_IF_BRANCH,
                                             .if_branch = if_branch};
                goto end;
            }

            case khKeywordToken_ELIF:
                raiseError(token.begin, U"no following if statement to have an elif statement");
                skipToken(cursor);
                khToken_delete(&token);
                goto end;

            case khKeywordToken_ELSE:
                raiseError(token.begin, U"no following if statement to have an else statement");
                skipToken(cursor);
                khToken_delete(&token);
                goto end;

            case khKeywordToken_FOR: {
                khAstForLoop for_loop = sparseForLoop(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_FOR_LOOP,
                                             .for_loop = for_loop};
                goto end;
            }

            case khKeywordToken_WHILE: {
                khAstWhileLoop while_loop = sparseWhileLoop(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_WHILE_LOOP,
                                             .while_loop = while_loop};
                goto end;
            }

            case khKeywordToken_DO: {
                khAstDoWhileLoop do_while_loop = sparseDoWhileLoop(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_DO_WHILE_LOOP,
                                             .do_while_loop = do_while_loop};
                goto end;
            }

            case khKeywordToken_BREAK: {
                sparseBreak(cursor);
                khToken_delete(&token);
                statement =
                    (khAstStatement){.begin = origin, .end = *cursor, .type = khAstStatementType_BREAK};
                goto end;
            }

            case khKeywordToken_CONTINUE: {
                sparseContinue(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){
                    .begin = origin, .end = *cursor, .type = khAstStatementType_CONTINUE};
                goto end;
            }

            case khKeywordToken_RETURN: {
                khAstReturn return_v = sparseReturn(cursor);
                khToken_delete(&token);
                statement = (khAstStatement){.begin = origin,
                                             .end = *cursor,
                                             .type = khAstStatementType_RETURN,
                                             .return_v = return_v};
                goto end;
            }

            default:
                goto out;
        }
    }
    else if (token.type == khTokenType_EOF) {
        raiseError(token.begin, U"expecting a statement, met with a dead end");
        khToken_delete(&token);
        goto end;
    }
out:
    // These keywords except `incase` indicate a variable declaration
    if (token.type == khTokenType_KEYWORD &&
        (token.keyword == khKeywordToken_REF || token.keyword == khKeywordToken_WILD ||
         token.keyword == khKeywordToken_INCASE || token.keyword == khKeywordToken_STATIC)) {
        khAstVariable variable = sparseVariable(cursor, false, false);
        statement = (khAstStatement){
            .begin = origin, .end = *cursor, .type = khAstStatementType_VARIABLE, .variable = variable};

        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    // Tests out whether this might be a specifierless variable declaration
    else if (token.type == khTokenType_IDENTIFIER) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);

        if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COLON) {
            *cursor = origin;
            khAstVariable variable = sparseVariable(cursor, false, false);
            statement = (khAstStatement){.begin = origin,
                                         .end = *cursor,
                                         .type = khAstStatementType_VARIABLE,
                                         .variable = variable};
        }
        else {
            *cursor = origin;
            khAstExpression expression = kh_parseExpression(cursor, false, false);
            statement = (khAstStatement){.begin = origin,
                                         .end = *cursor,
                                         .type = khAstStatementType_EXPRESSION,
                                         .expression = expression};
        }

        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    // Parses expression in the body if none of the keywords above are found
    else {
        khAstExpression expression = kh_parseExpression(cursor, false, false);
        statement = (khAstStatement){.begin = origin,
                                     .end = *cursor,
                                     .type = khAstStatementType_EXPRESSION,
                                     .expression = expression};

        khToken_delete(&token);
        token = currentToken(cursor, false);
    }

    // Ensures EOF, newline, or semicolon
    if (token.type == khTokenType_EOF || token.type == khTokenType_NEWLINE ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        // Still skips a token, to prevent other being stuck at the same token
        skipToken(cursor);
        raiseError(token.begin, U"expecting a newline or a semicolon");
    }

    khToken_delete(&token);

end:
    return statement;
}

static kharray(khAstStatement) sparseBlock(char32_t** cursor) {
    kharray(khAstStatement) block = kharray_new(khAstStatement, khAstStatement_delete);
    khToken token = currentToken(cursor, true);

    // Ensures opening bracket
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_CURLY_BRACKET_OPEN) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);
    }
    else {
        raiseError(token.begin, U"expecting an opening curly bracket");
    }

    // Gets the next statement until closing bracket
    while (!(token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE)) {
        // To make sure it won't get into an infinite loop
        if (token.type == khTokenType_EOF) {
            raiseError(token.begin, U"expecting a statement, met with a dead end");
            khToken_delete(&token);
            goto end;
        }

        kharray_append(&block, kh_parseStatement(cursor));
        khToken_delete(&token);
        token = currentToken(cursor, true);
    }

    skipToken(cursor);
    khToken_delete(&token);

end:
    return block;
}

static void sparseSpecifiers(char32_t** cursor, bool allow_incase, bool* is_incase, bool allow_static,
                             bool* is_static, bool ignore_newline) {
    khToken token = currentToken(cursor, ignore_newline);

    if (is_incase) {
        *is_incase = false;
    }
    if (is_static) {
        *is_static = false;
    }

    while (token.type == khTokenType_KEYWORD) {
        switch (token.keyword) {
            case khKeywordToken_INCASE:
                if (!allow_incase) {
                    raiseError(token.begin, U"the `incase` keyword is not allowed here");
                }
                else if (is_incase) {
                    *is_incase = true;
                }

                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, ignore_newline);
                break;

            case khKeywordToken_STATIC:
                if (!allow_static) {
                    raiseError(token.begin, U"the `static` keyword is not allowed here");
                }
                else if (is_static) {
                    *is_static = true;
                }

                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, ignore_newline);
                break;

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);
}

static khAstVariable sparseVariable(char32_t** cursor, bool no_static, bool ignore_newline) {
    khToken token = currentToken(cursor, ignore_newline);
    khAstVariable variable = {.is_static = false,
                              .is_wild = false,
                              .is_ref = false,
                              .name = NULL,
                              .optional_type = NULL,
                              .optional_initializer = NULL};


    if (!no_static) {
        // `static` specifier
        sparseSpecifiers(cursor, false, NULL, true, &variable.is_static, ignore_newline);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }

    // `wild` specifier
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_WILD) {
        variable.is_wild = true;
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }

    // `ref` specifier
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_REF) {
        variable.is_ref = true;
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }

    // Its name
    if (token.type == khTokenType_IDENTIFIER) {
        variable.name = khstring_copy(&token.identifier);
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }
    else {
        variable.name = khstring_new(U"");
        raiseError(token.begin, U"expecting a name for the variable in the declaration");
    }

    // Passes through the colon
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COLON) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }
    else {
        raiseError(token.begin, U"expecting a colon to separate the name and the type of the variable");
    }

    // If there's no assign op at first, it's a type: `name: Type`
    if (!(token.type == khTokenType_OPERATOR && token.operator_v == khOperatorToken_ASSIGN)) {
        variable.optional_type = malloc(sizeof(khAstExpression));
        *variable.optional_type = kh_parseExpression(cursor, ignore_newline, true);

        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }

    // Optional initializer
    if (token.type == khTokenType_OPERATOR && token.operator_v == khOperatorToken_ASSIGN) {
        skipToken(cursor);
        variable.optional_initializer = malloc(sizeof(khAstExpression));
        *variable.optional_initializer = kh_parseExpression(cursor, ignore_newline, false);
    }

    khToken_delete(&token);
    return variable;
}

static khAstImport sparseImport(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstImport import_v = {
        .path = kharray_new(khstring, khstring_delete), .relative = false, .optional_alias = NULL};

    // Ensures `import` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_IMPORT) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting an `import` keyword");
    }

    // For relative imports, `import .a_script_file_in_the_same_folder`
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        skipToken(cursor);
        import_v.relative = true;
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }

    // Minimum one identifier
    if (token.type == khTokenType_IDENTIFIER) {
        kharray_append(&import_v.path, khstring_copy(&token.identifier));
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    // Directory import, `import .`
    else if (import_v.relative) {
        goto finish;
    }
    else {
        raiseError(token.begin, U"expecting something to import");
    }

    // Continues on
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);

        if (token.type == khTokenType_IDENTIFIER) {
            kharray_append(&import_v.path, khstring_copy(&token.identifier));
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, false);
        }
        else {
            raiseError(token.begin, U"expecting another identifier");
        }
    }

finish:
    // `import something as another`
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_AS) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);

        if (token.type == khTokenType_IDENTIFIER) {
            import_v.optional_alias = malloc(sizeof(kharray(char)*));
            *import_v.optional_alias = khstring_copy(&token.identifier);
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, false);
        }
        else {
            raiseError(token.begin, U"expecting an identifier to alias the imported module as");
        }
    }

    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        raiseError(token.begin, U"expecting a newline or a semicolon");
    }

    khToken_delete(&token);
    return import_v;
}

static khAstInclude sparseInclude(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstInclude include = {.path = kharray_new(khstring, khstring_delete), .relative = false};

    // Ensures `include` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_INCLUDE) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting an `include` keyword");
    }

    // For relative includes, `include .a_script_file_in_the_same_folder`
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        include.relative = true;
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }

    // Minimum one identifier
    if (token.type == khTokenType_IDENTIFIER) {
        kharray_append(&include.path, khstring_copy(&token.identifier));
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    // Directory include, `include .`
    else if (include.relative) {
        goto finish;
    }
    else {
        raiseError(token.begin, U"expecting something to include");
    }

    // Continues on
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);

        if (token.type == khTokenType_IDENTIFIER) {
            kharray_append(&include.path, khstring_copy(&token.identifier));
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, false);
        }
        else {
            raiseError(token.begin, U"expecting another identifier");
        }
    }

finish:
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        raiseError(token.begin, U"expecting a newline or a semicolon");
    }

    khToken_delete(&token);
    return include;
}

static inline void sparseFunctionOrLambda(char32_t** cursor, kharray(khAstVariable) * arguments,
                                          khAstVariable** optional_variadic_argument,
                                          bool* is_return_type_ref,
                                          khAstExpression** optional_return_type,
                                          kharray(khAstStatement) * block) {
    khToken token = currentToken(cursor, false);

    // Starts out by parsing the argument
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_PARENTHESIS_OPEN) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);
    }
    else {
        raiseError(token.begin, U"expecting an opening parenthesis for the arguments");
    }

    while (!(token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_PARENTHESIS_CLOSE)) {
        // Variadic argument as the end
        if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_ELLIPSIS) {
            skipToken(cursor);

            *optional_variadic_argument = malloc(sizeof(khAstVariable));
            **optional_variadic_argument = sparseVariable(cursor, true, true);

            khToken_delete(&token);
            token = currentToken(cursor, true);

            if (!(token.type == khTokenType_DELIMITER &&
                  token.delimiter == khDelimiterToken_PARENTHESIS_CLOSE)) {
                raiseError(token.begin, U"expecting a closing parenthesis after the variadic argument");
            }

            break;
        }

        // Parse argument
        kharray_append(arguments, sparseVariable(cursor, true, true));
        khToken_delete(&token);
        token = currentToken(cursor, true);

        if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, true);
        }
        else if (token.type == khTokenType_DELIMITER &&
                 token.delimiter == khDelimiterToken_PARENTHESIS_CLOSE) {
            // Do nothing
        }
        else if (token.type == khTokenType_EOF) {
            raiseError(token.begin,
                       U"expecting a comma with another argument or a closing parenthesis, "
                       U"met with a dead end");
            break;
        }
        else {
            raiseError(token.begin,
                       U"expecting a comma with another argument or a closing parenthesis");
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, true);
        }
    }

    skipToken(cursor);
    khToken_delete(&token);
    token = currentToken(cursor, true);

    // Optional return type
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_ARROW) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);

        if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_REF) {
            *is_return_type_ref = true;
            skipToken(cursor);
        }
        else {
            *is_return_type_ref = false;
        }

        *optional_return_type = malloc(sizeof(khAstExpression));
        **optional_return_type = kh_parseExpression(cursor, true, true);

        khToken_delete(&token);
        token = currentToken(cursor, true);
    }

    // Body
    *block = sparseBlock(cursor);

    khToken_delete(&token);
}

static khAstFunction sparseFunction(char32_t** cursor) {
    khAstFunction function = {.is_incase = false,
                              .is_static = false,
                              .identifiers = kharray_new(khstring, khstring_delete),
                              .template_arguments = kharray_new(khstring, khstring_delete),
                              .arguments = kharray_new(khAstVariable, khAstVariable_delete),
                              .optional_variadic_argument = NULL,
                              .is_return_type_ref = false,
                              .optional_return_type = NULL,
                              .block = NULL};

    // Any specifiers: `incase static def function() { ... }`
    sparseSpecifiers(cursor, true, &function.is_incase, true, &function.is_static, true);

    khToken token = currentToken(cursor, true);

    // Ensures `def` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_DEF) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting a `def` keyword");
    }

    // Identifiers
    goto in;
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_DOT) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);
    in:
        if (token.type == khTokenType_IDENTIFIER) {
            kharray_append(&function.identifiers, khstring_copy(&token.identifier));
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, false);
        }
        else {
            raiseError(token.begin, U"expecting an identifier or name of the function");
        }
    }

    // Any template args
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_EXCLAMATION) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);

        // Single template argument: `def name!T`
        if (token.type == khTokenType_IDENTIFIER) {
            kharray_append(&function.template_arguments, khstring_copy(&token.identifier));
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, false);
        }
        // Multiple template arguments in parentheses: `def name!(T, U)`
        else if (token.type == khTokenType_DELIMITER &&
                 token.delimiter == khDelimiterToken_PARENTHESIS_OPEN) {
            do {
                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, true);

                if (token.type == khTokenType_IDENTIFIER) {
                    kharray_append(&function.template_arguments, khstring_copy(&token.identifier));
                }
                else {
                    raiseError(token.begin, U"expecting the name for a template argument");
                }

                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, true);
            } while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA);

            if (token.type == khTokenType_DELIMITER &&
                token.delimiter == khDelimiterToken_PARENTHESIS_CLOSE) {
                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, false);
            }
            else {
                raiseError(token.begin, U"expecting a closing parenthesis");
            }
        }
        else {
            raiseError(token.begin, U"expecting template argument(s)");
        }
    }

    // This will take care of the rest, including arguments and body
    sparseFunctionOrLambda(cursor, &function.arguments, &function.optional_variadic_argument,
                           &function.is_return_type_ref, &function.optional_return_type,
                           &function.block);

    khToken_delete(&token);
    return function;
}

static inline void sparseClassOrStruct(char32_t** cursor, khstring* name,
                                       kharray(khstring) * template_arguments,
                                       khAstExpression** optional_base_type,
                                       kharray(khAstStatement) * block) {
    khToken token = currentToken(cursor, false);

    // Ensures the name identifier of the class or struct
    if (token.type == khTokenType_IDENTIFIER) {
        *name = khstring_copy(&token.identifier);
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        *name = khstring_new(U"");
        raiseError(token.begin, U"expecting a name for the type");
    }

    // Any template args
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_EXCLAMATION) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);

        // Single template argument: `class Name!T`
        if (token.type == khTokenType_IDENTIFIER) {
            kharray_append(template_arguments, khstring_copy(&token.identifier));
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, false);
        }
        // Multiple template arguments in parentheses: `class Name!(T, U)`
        else if (token.type == khTokenType_DELIMITER &&
                 token.delimiter == khDelimiterToken_PARENTHESIS_OPEN) {
            do {
                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, true);

                if (token.type == khTokenType_IDENTIFIER) {
                    kharray_append(template_arguments, khstring_copy(&token.identifier));
                }
                else {
                    raiseError(token.begin, U"expecting the name for a template argument");
                }

                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, true);
            } while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA);

            if (token.type == khTokenType_DELIMITER &&
                token.delimiter == khDelimiterToken_PARENTHESIS_CLOSE) {
                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, false);
            }
            else {
                raiseError(token.begin, U"expecting a closing parenthesis");
            }
        }
        else {
            raiseError(token.begin, U"expecting template argument(s)");
        }
    }

    // If a class is inheriting something: `class Name inherits Base`
    if (optional_base_type != NULL && token.type == khTokenType_KEYWORD &&
        token.keyword == khKeywordToken_INHERITS) {
        skipToken(cursor);
        *optional_base_type = malloc(sizeof(khAstExpression));
        **optional_base_type = kh_parseExpression(cursor, true, true);

        khToken_delete(&token);
        token = currentToken(cursor, true);
    }

    // Parses its block
    *block = sparseBlock(cursor);

    khToken_delete(&token);
}

static khAstClass sparseClass(char32_t** cursor) {
    khAstClass class_v = {.is_incase = false,
                          .name = NULL,
                          .template_arguments = kharray_new(khstring, khstring_delete),
                          .optional_base_type = NULL,
                          .block = NULL};

    // Any specifiers: `incase class E { ... }`
    sparseSpecifiers(cursor, true, &class_v.is_incase, false, NULL, true);

    khToken token = currentToken(cursor, true);

    // Ensures `class` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_CLASS) {
        skipToken(cursor);
    }
    else {
        raiseError(token.begin, U"expecting a `class` keyword");
    }

    sparseClassOrStruct(cursor, &class_v.name, &class_v.template_arguments, &class_v.optional_base_type,
                        &class_v.block);

    khToken_delete(&token);
    return class_v;
}

static khAstStruct sparseStruct(char32_t** cursor) {
    khAstStruct struct_v = {.is_incase = false,
                            .name = NULL,
                            .template_arguments = kharray_new(khstring, khstring_delete),
                            .block = NULL};

    // Any specifiers: `incase struct E { ... }`
    sparseSpecifiers(cursor, true, &struct_v.is_incase, false, NULL, true);

    khToken token = currentToken(cursor, true);

    // Ensures `struct` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_STRUCT) {
        skipToken(cursor);
    }
    else {
        raiseError(token.begin, U"expecting a `struct` keyword");
    }

    sparseClassOrStruct(cursor, &struct_v.name, &struct_v.template_arguments, NULL, &struct_v.block);

    khToken_delete(&token);
    return struct_v;
}

static khAstEnum sparseEnum(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstEnum enum_v = {.name = NULL, .members = kharray_new(khstring, khstring_delete)};

    // No specifiers at all
    sparseSpecifiers(cursor, false, NULL, false, NULL, true);

    // Ensures `enum` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ENUM) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting an `enum` keyword");
    }

    // Its name
    if (token.type == khTokenType_IDENTIFIER) {
        enum_v.name = khstring_copy(&token.identifier);
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        enum_v.name = khstring_new(U"");
        raiseError(token.begin, U"expecting a name for the enum type");
    }

    // Its members
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_CURLY_BRACKET_OPEN) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);

        do {
            if (token.type == khTokenType_IDENTIFIER) {
                kharray_append(&enum_v.members, khstring_copy(&token.identifier));
            }
            else {
                raiseError(token.begin, U"expecting a member name");
            }

            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, true);
        } while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA);

        // Ensures closing bracket at the end
        if (token.type == khTokenType_DELIMITER &&
            token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
            skipToken(cursor);
        }
        else {
            raiseError(token.begin,
                       U"expecting a comma with another member or a closing curly bracket");
        }
    }
    else {
        raiseError(token.begin, U"expecting a `class` keyword");
    }

    khToken_delete(&token);
    return enum_v;
}

static khAstAlias sparseAlias(char32_t** cursor) {
    khAstAlias alias = {.is_incase = false,
                        .name = NULL,
                        .expression = (khAstExpression){
                            .begin = NULL, .end = NULL, .type = khAstExpressionType_INVALID}};

    // Any specifiers
    sparseSpecifiers(cursor, true, &alias.is_incase, false, NULL, true);

    khToken token = currentToken(cursor, true);

    // Ensures `alias` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ALIAS) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting an `alias` keyword");
    }

    // Its name
    if (token.type == khTokenType_IDENTIFIER) {
        alias.name = khstring_copy(&token.identifier);
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);
    }
    else {
        alias.name = khstring_new(U"");
        raiseError(token.begin, U"expecting a name for the alias");
    }

    // Its alias expression
    alias.expression = kh_parseExpression(cursor, false, false);
    token = currentToken(cursor, false);

    // Ensures a newline or a semicolon at the end
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        raiseError(token.begin, U"expecting a newline or a semicolon");
    }

    khToken_delete(&token);
    return alias;
}


static khAstIfBranch sparseIfBranch(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstIfBranch if_branch = (khAstIfBranch){
        .branch_conditions = kharray_new(khAstExpression, khAstExpression_delete),
        .branch_blocks = kharray_new(kharray(khAstStatement), kharray_arrayDeleter(khAstStatement)),
        .else_block = kharray_new(khAstStatement, khAstStatement_delete)};

    // Ensures initial `if` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_IF) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting an `if` keyword");
    }

    goto in;
    while (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ELIF) {
        skipToken(cursor);
    in:
        kharray_append(&if_branch.branch_conditions, kh_parseExpression(cursor, false, false));
        kharray_append(&if_branch.branch_blocks, sparseBlock(cursor));

        khToken_delete(&token);
        token = currentToken(cursor, true);

        // Loop back again if there are any `elif` keywords
    }

    // End optional `else` block
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ELSE) {
        skipToken(cursor);
        if_branch.else_block = sparseBlock(cursor);
    }

    khToken_delete(&token);
    return if_branch;
}

static khAstWhileLoop sparseWhileLoop(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstWhileLoop while_loop = {
        .condition = (khAstExpression){.begin = NULL, .end = NULL, .type = khAstExpressionType_INVALID},
        .block = NULL};

    // Ensures `while` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_WHILE) {
        skipToken(cursor);
    }
    else {
        raiseError(token.begin, U"expecting a `while` keyword");
    }

    // Its condition and block block
    while_loop.condition = kh_parseExpression(cursor, false, false);
    while_loop.block = sparseBlock(cursor);

    khToken_delete(&token);
    return while_loop;
}

static khAstDoWhileLoop sparseDoWhileLoop(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstDoWhileLoop do_while_loop = {
        .condition = (khAstExpression){.begin = NULL, .end = NULL, .type = khAstExpressionType_INVALID},
        .block = NULL};

    // Ensures `do` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_DO) {
        skipToken(cursor);
    }
    else {
        raiseError(token.begin, U"expecting a `do` keyword");
    }

    // Its block
    do_while_loop.block = sparseBlock(cursor);

    khToken_delete(&token);
    token = currentToken(cursor, true);

    // Ensures `while` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_WHILE) {
        skipToken(cursor);
    }
    else {
        raiseError(token.begin, U"expecting a `while` keyword");
    }

    // And its condition
    do_while_loop.condition = kh_parseExpression(cursor, false, false);

    khToken_delete(&token);
    token = currentToken(cursor, false);

    // Ensures a newline or a semicolon at the end
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        raiseError(token.begin, U"expecting a newline or a semicolon");
    }

    khToken_delete(&token);
    return do_while_loop;
}

static khAstForLoop sparseForLoop(char32_t** cursor) {
    khToken token = currentToken(cursor, true);
    khAstForLoop for_loop = {
        .iterators = kharray_new(khstring, khstring_delete),
        .iteratee = (khAstExpression){.begin = NULL, .end = NULL, .type = khAstExpressionType_INVALID},
        .block = NULL};

    // Ensures `for` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_FOR) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting a `for` keyword");
    }

    // Iterator variables
    goto in;
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);
    in:
        if (token.type == khTokenType_IDENTIFIER) {
            kharray_append(&for_loop.iterators, khstring_copy(&token.identifier));
            skipToken(cursor);
            khToken_delete(&token);
            token = currentToken(cursor, false);
        }
        else {
            raiseError(token.begin, U"expecting an iterator variable name");
        }
    }

    // Ensures `in` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_IN) {
        skipToken(cursor);
    }
    else {
        raiseError(token.begin, U"expecting an `in` keyword");
    }

    // Iteratee expression and block block
    for_loop.iteratee = kh_parseExpression(cursor, false, false);
    for_loop.block = sparseBlock(cursor);

    khToken_delete(&token);
    return for_loop;
}

static void sparseBreak(char32_t** cursor) {
    khToken token = currentToken(cursor, true);

    // Ensures `break` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_BREAK) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting a `break` keyword");
    }

    // Ensure it ends with a newline or a semicolon
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        raiseError(token.begin, U"expecting a newline or a semicolon");
    }

    khToken_delete(&token);
}

static void sparseContinue(char32_t** cursor) {
    khToken token = currentToken(cursor, true);

    // Ensures `continue` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_CONTINUE) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting a `continue` keyword");
    }

    // Ensure it ends with a newline or a semicolon
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        raiseError(token.begin, U"expecting a newline or a semicolon");
    }

    khToken_delete(&token);
}

static khAstReturn sparseReturn(char32_t** cursor) {
    khToken token = currentToken(cursor, true);

    // Ensures `return` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_RETURN) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, false);
    }
    else {
        raiseError(token.begin, U"expecting a `return` keyword");
    }

    // Instant return in a non-returning function
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);

        khToken_delete(&token);
        return (khAstReturn){.values = kharray_new(khAstExpression, khAstExpression_delete)};
    }

    kharray(khAstExpression) values = kharray_new(khAstExpression, khAstExpression_delete);

    // Its return values
    goto skip;
    while (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
        skipToken(cursor);
    skip:
        khToken_delete(&token);
        kharray_append(&values, kh_parseExpression(cursor, false, false));
        token = currentToken(cursor, false);
    };

    // Ensure it ends with a newline or a semicolon
    if (token.type == khTokenType_NEWLINE || token.type == khTokenType_EOF ||
        (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_SEMICOLON)) {
        skipToken(cursor);
    }
    else if (token.type == khTokenType_DELIMITER &&
             token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        // Do nothing
    }
    else {
        raiseError(token.begin, U"expecting a newline or a semicolon");
    }

    khToken_delete(&token);
    return (khAstReturn){.values = values};
}


// Macro to do recursive descent for a single binary operator as the whole function block
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
        skipToken(cursor);                                                                          \
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
    return expression;


// Macro to do recursive descent for binary operators in a switch statement
#define RCD_BINARY_CASE(LOWER, OPERATOR)                                                            \
    {                                                                                               \
        skipToken(cursor);                                                                          \
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


khAstExpression kh_parseExpression(char32_t** cursor, EXPARSE_ARGS) {
    return exparseIpAssignmentOperators(cursor, ignore_newline, filter_type);
}

static khAstExpression exparseIpAssignmentOperators(char32_t** cursor, EXPARSE_ARGS) {
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
            // These LOWER macro arguments are set to this very function itself,
            // in order to parse inplace/assignment operators from right to left.
            case khOperatorToken_ASSIGN:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_ASSIGN);

            case khOperatorToken_IP_ADD:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_ADD);
            case khOperatorToken_IP_SUB:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_SUB);
            case khOperatorToken_IP_MUL:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_MUL);
            case khOperatorToken_IP_DIV:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_DIV);
            case khOperatorToken_IP_MOD:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_MOD);
            case khOperatorToken_IP_POW:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_POW);
            case khOperatorToken_IP_DOT:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_DOT);

            case khOperatorToken_IP_BIT_AND:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_BIT_AND);
            case khOperatorToken_IP_BIT_OR:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_BIT_OR);
            case khOperatorToken_IP_BIT_XOR:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_BIT_XOR);
            case khOperatorToken_IP_BIT_LSHIFT:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_BIT_LSHIFT);
            case khOperatorToken_IP_BIT_RSHIFT:
                RCD_BINARY_CASE(exparseIpAssignmentOperators, khAstBinaryExpressionType_IP_BIT_RSHIFT);

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
        skipToken(cursor);

        // Its condition
        khAstExpression* condition = malloc(sizeof(khAstExpression));
        *condition = exparseLogicalOr(cursor, ignore_newline, filter_type);

        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);

        // Ensures the `else` keyword before the otherwise value
        if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_ELSE) {
            skipToken(cursor);
        }
        else {
            raiseError(token.begin, U"expecting an `else` keyword after the condition");
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
        skipToken(cursor);

        khAstExpression* expression = malloc(sizeof(khAstExpression));
        *expression = exparseLogicalNot(cursor, ignore_newline, filter_type);

        khToken_delete(&token);
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

    khAstExpression expression = exparseRange(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // If any comparison operators found, start this mess
    if (token.type == khTokenType_OPERATOR &&
        (token.operator_v == khOperatorToken_EQUAL || token.operator_v == khOperatorToken_UNEQUAL ||
         token.operator_v == khOperatorToken_LESS || token.operator_v == khOperatorToken_GREATER ||
         token.operator_v == khOperatorToken_LESS_EQUAL ||
         token.operator_v == khOperatorToken_GREATER_EQUAL)) {
        kharray(khAstComparisonExpressionType) operations =
            kharray_new(khAstComparisonExpressionType, NULL);
        kharray(khAstExpression) operands = kharray_new(khAstExpression, khAstExpression_delete);
        kharray_append(&operands, expression);

        // Maps the khOperatorToken of the comparison operator to khAstComparisonExpressionType
        while (token.type == khTokenType_OPERATOR) {
            switch (token.operator_v) {
                case khOperatorToken_EQUAL:
                    kharray_append(&operations, khAstComparisonExpressionType_EQUAL);
                    break;
                case khOperatorToken_UNEQUAL:
                    kharray_append(&operations, khAstComparisonExpressionType_UNEQUAL);
                    break;
                case khOperatorToken_LESS:
                    kharray_append(&operations, khAstComparisonExpressionType_LESS);
                    break;
                case khOperatorToken_GREATER:
                    kharray_append(&operations, khAstComparisonExpressionType_GREATER);
                    break;
                case khOperatorToken_LESS_EQUAL:
                    kharray_append(&operations, khAstComparisonExpressionType_LESS_EQUAL);
                    break;
                case khOperatorToken_GREATER_EQUAL:
                    kharray_append(&operations, khAstComparisonExpressionType_GREATER_EQUAL);
                    break;

                default:
                    goto out;
            }

            skipToken(cursor);
            kharray_append(&operands, exparseRange(cursor, ignore_newline, filter_type));

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

static khAstExpression exparseRange(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseBitwiseOr, khOperatorToken_RANGE, khAstBinaryExpressionType_RANGE);
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

    khAstExpression expression = exparseMulDivModDot(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // Self-explanatory
    while (token.type == khTokenType_OPERATOR) {
        switch (token.operator_v) {
            case khOperatorToken_ADD:
                RCD_BINARY_CASE(exparseMulDivModDot, khAstBinaryExpressionType_ADD);
            case khOperatorToken_SUB:
                RCD_BINARY_CASE(exparseMulDivModDot, khAstBinaryExpressionType_SUB);

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);
    return expression;
}

static khAstExpression exparseMulDivModDot(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseUnary(cursor, ignore_newline, filter_type);

    if (filter_type) {
        return expression;
    }

    token = currentToken(cursor, ignore_newline);

    // Self-explanatory
    while (token.type == khTokenType_OPERATOR) {
        switch (token.operator_v) {
            case khOperatorToken_MUL:
                RCD_BINARY_CASE(exparseUnary, khAstBinaryExpressionType_MUL);
            case khOperatorToken_DIV:
                RCD_BINARY_CASE(exparseUnary, khAstBinaryExpressionType_DIV);
            case khOperatorToken_MOD:
                RCD_BINARY_CASE(exparseUnary, khAstBinaryExpressionType_MOD);
            case khOperatorToken_DOT:
                RCD_BINARY_CASE(exparseUnary, khAstBinaryExpressionType_DOT);

            default:
                goto out;
        }
    }
out:

    khToken_delete(&token);
    return expression;
}


// Like RCD_BINARY_CASE, but for unary operators
#define RCD_UNARY_CASE(FUNCTION, OPERATOR)                                            \
    {                                                                                 \
        skipToken(cursor);                                                            \
                                                                                      \
        khAstExpression* expression = malloc(sizeof(khAstExpression));                \
        *expression = exparseUnary(cursor, ignore_newline, filter_type);              \
                                                                                      \
        khToken_delete(&token);                                                       \
        return (khAstExpression){.begin = origin,                                     \
                                 .end = *cursor,                                      \
                                 .type = khAstExpressionType_UNARY,                   \
                                 .unary = {.type = OPERATOR, .operand = expression}}; \
    }


static khAstExpression exparseUnary(char32_t** cursor, EXPARSE_ARGS) {
    if (filter_type) {
        return exparsePow(cursor, ignore_newline, filter_type);
    }

    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;

    // All same-precedence unary operators
    if (token.type == khTokenType_OPERATOR) {
        switch (token.operator_v) {
            case khOperatorToken_ADD:
                RCD_UNARY_CASE(exparsePow, khAstUnaryExpressionType_POSITIVE);
            case khOperatorToken_SUB:
                RCD_UNARY_CASE(exparsePow, khAstUnaryExpressionType_NEGATIVE);

            case khOperatorToken_NOT:
                RCD_UNARY_CASE(exparsePow, khAstUnaryExpressionType_NOT);
            case khOperatorToken_BIT_NOT:
                RCD_UNARY_CASE(exparsePow, khAstUnaryExpressionType_BIT_NOT);

            default:
                khToken_delete(&token);
                return exparsePow(cursor, ignore_newline, filter_type);
        }
    }
    else {
        khToken_delete(&token);
        return exparsePow(cursor, ignore_newline, filter_type);
    }
}

static khAstExpression exparsePow(char32_t** cursor, EXPARSE_ARGS) {
    RCD_BINARY(exparseReverseUnary, khOperatorToken_POW, khAstBinaryExpressionType_POW);
}

static khAstExpression exparseReverseUnary(char32_t** cursor, EXPARSE_ARGS) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = token.begin;
    khToken_delete(&token);

    khAstExpression expression = exparseOther(cursor, ignore_newline, filter_type);
    token = currentToken(cursor, ignore_newline);

    while (token.type == khTokenType_DELIMITER || token.type == khTokenType_OPERATOR) {
        switch (token.type) {
            case khTokenType_DELIMITER:
                switch (token.delimiter) {
                    case khDelimiterToken_PARENTHESIS_OPEN: {
                        if (filter_type) {
                            goto out;
                        }

                        kharray(khAstExpression) arguments = exparseList(
                            cursor, khDelimiterToken_PARENTHESIS_OPEN,
                            khDelimiterToken_PARENTHESIS_CLOSE, ignore_newline, filter_type);

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
                        kharray(khAstExpression) arguments = exparseList(
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

                    case khDelimiterToken_DOT: {
                        kharray(khstring) scope_names = kharray_new(khstring, khstring_delete);

                        // `(expression).parses.these.scope.things`
                        while (token.type == khTokenType_DELIMITER &&
                               token.delimiter == khDelimiterToken_DOT) {
                            skipToken(cursor);
                            khToken_delete(&token);
                            token = currentToken(cursor, ignore_newline);

                            if (token.type == khTokenType_IDENTIFIER) {
                                kharray_append(&scope_names, khstring_copy(&token.identifier));

                                skipToken(cursor);
                                khToken_delete(&token);
                                token = currentToken(cursor, ignore_newline);
                            }
                            else {
                                raiseError(token.begin, U"expecting an identifier to scope into");
                            }
                        }

                        khAstExpression* value = malloc(sizeof(khAstExpression));
                        *value = expression;

                        expression =
                            (khAstExpression){.begin = origin,
                                              .end = *cursor,
                                              .type = khAstExpressionType_SCOPE,
                                              .scope = {.value = value, .scope_names = scope_names}};
                    } break;

                    case khDelimiterToken_EXCLAMATION: {
                        skipToken(cursor);
                        khToken_delete(&token);
                        token = currentToken(cursor, ignore_newline);

                        khAstExpression* value = malloc(sizeof(khAstExpression));
                        *value = expression;

                        // Single identifier template argument: `Type!int`
                        if (token.type == khTokenType_IDENTIFIER) {
                            kharray(khAstExpression) template_arguments =
                                kharray_new(khAstExpression, khAstExpression_delete);

                            kharray_append(
                                &template_arguments,
                                ((khAstExpression){.begin = token.begin,
                                                   .end = token.end,
                                                   .type = khAstExpressionType_IDENTIFIER,
                                                   .identifier = khstring_copy(&token.identifier)}));

                            expression = (khAstExpression){
                                .begin = origin,
                                .end = *cursor,
                                .type = khAstExpressionType_TEMPLATIZE,
                                .templatize = {.value = value,
                                               .template_arguments = template_arguments}};

                            skipToken(cursor);
                            khToken_delete(&token);
                            token = currentToken(cursor, ignore_newline);
                        }
                        // Multiple template arguments: `Type!(int, float)`
                        else if (token.type == khTokenType_DELIMITER &&
                                 token.delimiter == khDelimiterToken_PARENTHESIS_OPEN) {
                            kharray(khAstExpression) template_arguments =
                                exparseList(cursor, khDelimiterToken_PARENTHESIS_OPEN,
                                            khDelimiterToken_PARENTHESIS_CLOSE, ignore_newline, true);

                            expression = (khAstExpression){
                                .begin = origin,
                                .end = *cursor,
                                .type = khAstExpressionType_TEMPLATIZE,
                                .templatize = {.value = value,
                                               .template_arguments = template_arguments}};

                            khToken_delete(&token);
                            token = currentToken(cursor, ignore_newline);
                        }
                        else {
                            free(value);
                            raiseError(token.begin, U"expecting a type argument for templatizing");
                        }
                    } break;

                    default:
                        goto out;
                }
                break;

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
    khAstExpression expression =
        (khAstExpression){.begin = NULL, .end = NULL, .type = khAstExpressionType_INVALID};

    switch (token.type) {
        case khTokenType_IDENTIFIER: {
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_IDENTIFIER,
                                           .identifier = khstring_copy(&token.identifier)};

            skipToken(cursor);
        } break;

        case khTokenType_KEYWORD: {
            // 2 cases
            if (token.keyword == khKeywordToken_DEF) {
                char32_t* initial = *cursor;
                skipToken(&initial);
                khToken next_token = currentToken(&initial, ignore_newline);

                // A function signature
                if (next_token.type == khTokenType_DELIMITER &&
                    next_token.delimiter == khDelimiterToken_EXCLAMATION) {
                    expression = exparseSignature(cursor, ignore_newline);
                }
                // A lambda
                else {
                    if (filter_type) {
                        raiseError(token.begin, U"expecting a type, not a lambda");
                    }

                    expression = exparseLambda(cursor, ignore_newline);
                }

                khToken_delete(&next_token);
            }
            else {
                raiseError(token.begin, U"unexpected keyword in an expression");
                skipToken(cursor);
            }
        } break;

        case khTokenType_DELIMITER:
            switch (token.keyword) {
                // Parentheses enclosed expressions or tuples
                case khDelimiterToken_PARENTHESIS_OPEN: {
                    kharray(khAstExpression) values =
                        exparseList(cursor, khDelimiterToken_PARENTHESIS_OPEN,
                                    khDelimiterToken_PARENTHESIS_CLOSE, ignore_newline, filter_type);

                    if (kharray_size(&values) == 1) {
                        expression = khAstExpression_copy(&values[0]);
                        kharray_delete(&values);
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
                        raiseError(token.begin, U"expecting a type, not an array");
                    }

                    expression = (khAstExpression){
                        .type = khAstExpressionType_ARRAY,
                        .array = {.values = exparseList(cursor, khDelimiterToken_SQUARE_BRACKET_OPEN,
                                                        khDelimiterToken_SQUARE_BRACKET_CLOSE,
                                                        ignore_newline, filter_type)}};
                    break;

                // Dicts
                case khDelimiterToken_CURLY_BRACKET_OPEN:
                    if (filter_type) {
                        raiseError(token.begin, U"expecting a type, not a dict");
                    }

                    expression = exparseDict(cursor, ignore_newline);
                    break;

                default:
                    raiseError(token.begin, U"unexpected token in an expression");
                    skipToken(cursor);
                    break;
            }
            break;

            // The rest below are constants (v)

        case khTokenType_CHAR:
            if (filter_type) {
                raiseError(token.begin, U"expecting a type, not a character");
            }

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_CHAR,
                                           .char_v = token.char_v};
            break;

        case khTokenType_STRING:
            if (filter_type) {
                raiseError(token.begin, U"expecting a type, not a string");
            }

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_STRING,
                                           .string = khstring_copy(&token.string)};
            break;

        case khTokenType_BUFFER:
            if (filter_type) {
                raiseError(token.begin, U"expecting a type, not a buffer");
            }

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_BUFFER,
                                           .buffer = khbuffer_copy(&token.buffer)};
            break;

        case khTokenType_BYTE:
            if (filter_type) {
                raiseError(token.begin, U"expecting a type, not a byte");
            }

            skipToken(cursor);
            expression = (khAstExpression){
                .begin = origin, .end = *cursor, .type = khAstExpressionType_BYTE, .byte = token.byte};
            break;

        case khTokenType_INTEGER:
            // Integers should be able to be parsed even with `filter_type`, in order for static array
            // types to be parsed
            /* if (filter_type) {
             *     raiseError(token.begin, U"expecting a type, not an integer");
             * }
             */

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_INTEGER,
                                           .integer = token.integer};
            break;

        case khTokenType_UINTEGER:
            // Same for unsigned integers
            /* if (filter_type) {
             *     raiseError(token.begin, U"expecting a type, not an unsigned integer");
             * }
             */

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_UINTEGER,
                                           .uinteger = token.uinteger};
            break;

        case khTokenType_FLOAT:
            if (filter_type) {
                raiseError(token.begin, U"expecting a type, not a floating-point number");
            }

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_FLOAT,
                                           .float_v = token.float_v};
            break;

        case khTokenType_DOUBLE:
            if (filter_type) {
                raiseError(token.begin, U"expecting a type, not a double floating-point number");
            }

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_DOUBLE,
                                           .double_v = token.double_v};
            break;

        case khTokenType_IFLOAT:
            if (filter_type) {
                raiseError(token.begin, U"expecting a type, not an imaginary floating-point number");
            }

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_IFLOAT,
                                           .ifloat = token.ifloat};
            break;

        case khTokenType_IDOUBLE:
            if (filter_type) {
                raiseError(token.begin,
                           U"expecting a type, not an imaginary double floating-point number");
            }

            skipToken(cursor);
            expression = (khAstExpression){.begin = origin,
                                           .end = *cursor,
                                           .type = khAstExpressionType_IDOUBLE,
                                           .idouble = token.idouble};
            break;

        case khTokenType_NEWLINE:
            raiseError(token.begin, U"unexpected newline in an expression");
            skipToken(cursor);
            break;

        case khTokenType_EOF:
            raiseError(token.begin, U"expecting an expression, met with a dead end");
            break;

        default:
            raiseError(token.begin, U"unexpected token in an expression");
            skipToken(cursor);
            break;
    }

    khToken_delete(&token);
    return expression;
}

static khAstExpression exparseSignature(char32_t** cursor, bool ignore_newline) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = *cursor;
    khAstSignature signature = {.are_arguments_refs = kharray_new(bool, NULL),
                                .argument_types = kharray_new(khAstExpression, khAstExpression_delete),
                                .is_return_type_ref = false,
                                .optional_return_type = NULL};

    // Ensures `def` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_DEF) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }
    else {
        raiseError(token.begin, U"expecting a `def` keyword");
    }

    // Ensures an exclamation mark
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_EXCLAMATION) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }
    else {
        raiseError(token.begin, U"expecting an exclamation mark");
    }

    // Ensures an opening parenthesis
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_PARENTHESIS_OPEN) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);
    }
    else {
        raiseError(token.begin, U"expecting an opening parenthesis");
    }

    // Instant close
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_PARENTHESIS_CLOSE) {
        skipToken(cursor);
    }
    else {
        while (true) {
            // Handles `ref` arguments
            if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_REF) {
                kharray_append(&signature.are_arguments_refs, true);
                skipToken(cursor);
            }
            else {
                kharray_append(&signature.are_arguments_refs, false);
            }

            // Argument type
            kharray_append(&signature.argument_types, kh_parseExpression(cursor, true, true));
            khToken_delete(&token);
            token = currentToken(cursor, true);

            // Do nothing after a comma
            if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, true);
            }
            // Breaks out of the loop after closing the arglist
            else if (token.type == khTokenType_DELIMITER &&
                     token.delimiter == khDelimiterToken_PARENTHESIS_CLOSE) {
                skipToken(cursor);
                break;
            }
            else {
                raiseError(token.begin,
                           U"expecting a comma or another argument type after an argument type");
                break;
            }
        }
    }

    // Optional return type
    khToken_delete(&token);
    token = currentToken(cursor, ignore_newline);
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_ARROW) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);

        // Handles `ref` return type
        if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_REF) {
            signature.is_return_type_ref = true;
            skipToken(cursor);
        }

        // Return type itself
        signature.optional_return_type = malloc(sizeof(khAstExpression));
        *signature.optional_return_type = kh_parseExpression(cursor, ignore_newline, true);
    }

    khToken_delete(&token);
    return (khAstExpression){
        .begin = origin, .end = *cursor, .type = khAstExpressionType_SIGNATURE, .signature = signature};
}

static khAstExpression exparseLambda(char32_t** cursor, bool ignore_newline) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = *cursor;
    khAstLambda lambda = {.arguments = kharray_new(khAstVariable, khAstVariable_delete),
                          .optional_variadic_argument = NULL,
                          .is_return_type_ref = false,
                          .optional_return_type = NULL,
                          .block = NULL};

    // Ensures `def` keyword
    if (token.type == khTokenType_KEYWORD && token.keyword == khKeywordToken_DEF) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, ignore_newline);
    }
    else {
        raiseError(token.begin, U"expecting a `def` keyword");
    }

    sparseFunctionOrLambda(cursor, &lambda.arguments, &lambda.optional_variadic_argument,
                           &lambda.is_return_type_ref, &lambda.optional_return_type, &lambda.block);

    khToken_delete(&token);
    return (khAstExpression){
        .begin = origin, .end = *cursor, .type = khAstExpressionType_LAMBDA, .lambda = lambda};
}

static khAstExpression exparseDict(char32_t** cursor, bool ignore_newline) {
    khToken token = currentToken(cursor, ignore_newline);
    char32_t* origin = *cursor;
    khAstDict dict = {.keys = kharray_new(khAstExpression, khAstExpression_delete),
                      .values = kharray_new(khAstExpression, khAstExpression_delete)};

    // Ensures an opening curly bracket
    if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_CURLY_BRACKET_OPEN) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);
    }
    else {
        raiseError(token.begin, U"expecting an opening curly bracket");
    }

    // Instant close
    if (token.type == khTokenType_DELIMITER &&
        token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
        skipToken(cursor);
    }
    else {
        while (true) {
            // Key
            kharray_append(&dict.keys, kh_parseExpression(cursor, true, false));
            khToken_delete(&token);
            token = currentToken(cursor, true);

            // Ensures a colon
            if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COLON) {
                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, true);
            }
            else {
                raiseError(token.begin, U"expecting a colon after the key for its value pair");
            }

            // Value
            kharray_append(&dict.values, kh_parseExpression(cursor, true, false));
            khToken_delete(&token);
            token = currentToken(cursor, true);

            // Do nothing after a comma
            if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, true);
            }
            // Breaks out of the loop after dict close
            else if (token.type == khTokenType_DELIMITER &&
                     token.delimiter == khDelimiterToken_CURLY_BRACKET_CLOSE) {
                skipToken(cursor);
                break;
            }
            else {
                raiseError(
                    token.begin,
                    U"expecting a comma with another pair of key and value or a closing delimiter");
                break;
            }
        }
    }

    khToken_delete(&token);
    return (khAstExpression){
        .begin = origin, .end = *cursor, .type = khAstExpressionType_DICT, .dict = dict};
}

static kharray(khAstExpression) exparseList(char32_t** cursor, khDelimiterToken opening_delimiter,
                                            khDelimiterToken closing_delimiter, EXPARSE_ARGS) {
    kharray(khAstExpression) expressions = kharray_new(khAstExpression, khAstExpression_delete);
    khToken token = currentToken(cursor, ignore_newline);

    // Ensures the opening delimiter
    if (token.type == khTokenType_DELIMITER && token.delimiter == opening_delimiter) {
        skipToken(cursor);
        khToken_delete(&token);
        token = currentToken(cursor, true);
    }
    else {
        raiseError(token.begin, U"expecting an opening delimiter");
    }

    // Instant close
    if (token.type == khTokenType_DELIMITER && token.delimiter == closing_delimiter) {
        skipToken(cursor);
    }
    else {
        while (true) {
            // Expression
            kharray_append(&expressions, kh_parseExpression(cursor, true, filter_type));
            khToken_delete(&token);
            token = currentToken(cursor, true);

            // Do nothing after a comma
            if (token.type == khTokenType_DELIMITER && token.delimiter == khDelimiterToken_COMMA) {
                skipToken(cursor);
                khToken_delete(&token);
                token = currentToken(cursor, true);
            }
            // Breaks out of the loop after list close
            else if (token.type == khTokenType_DELIMITER && token.delimiter == closing_delimiter) {
                skipToken(cursor);
                break;
            }
            else {
                raiseError(token.begin,
                           U"expecting a comma with another element or a closing delimiter");
                break;
            }
        }
    }

    khToken_delete(&token);

    return expressions;
}
