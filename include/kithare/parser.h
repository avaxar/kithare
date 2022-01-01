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

#include "array.h"
#include "ast.h"


khAst kh_parse(char32_t** cursor);
khAstExpression kh_parseExpression(char32_t** cursor, bool ignore_newline, bool filter_type);

khArray(khAst) kh_parseBlock(char32_t** cursor);
khAstImport kh_parseImport(char32_t** cursor);
khAstInclude kh_parseInclude(char32_t** cursor);
khAstFunction kh_parseFunction(char32_t** cursor);
khAstClass kh_parseClass(char32_t** cursor);
khAstStruct kh_parseStruct(char32_t** cursor);
khAstEnum kh_parseEnum(char32_t** cursor);
khAstAlias kh_parseAlias(char32_t** cursor);

khAstIfBranch kh_parseIfBranch(char32_t** cursor);
khAstWhileLoop kh_parseWhileLoop(char32_t** cursor);
khAstDoWhileLoop kh_parseDoWhileLoop(char32_t** cursor);
khAst kh_parseForLoop(char32_t** cursor);
khAstBreak kh_parseBreak(char32_t** cursor);
khAstContinue kh_parseContinue(char32_t** cursor);
khAstReturn kh_parseReturn(char32_t** cursor);

khAstExpression kh_exparseInplaceOperators(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseTernary(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseLogicalOr(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseLogicalXor(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseLogicalAnd(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseLogicalNot(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseComparisonOperators(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseBitwiseOr(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseBitwiseXor(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseBitwiseAnd(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseBitwiseShifts(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseAddSub(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseMulDivMod(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparsePow(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseRef(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseUnary(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseReverseUnary(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseScopeTemplatization(char32_t** cursor, bool ignore_newline, bool filter_type);
khAstExpression kh_exparseOther(char32_t** cursor, bool ignore_newline, bool filter_type);

khAstExpression kh_exparseVariableDeclaration(char32_t** cursor, bool ignore_newline);
khAstExpression kh_exparseLambda(char32_t** cursor, bool ignore_newline);
khArray(khAstExpression)
    kh_exparseList(char32_t** cursor, khDelimiterToken opening_delimiter,
                   khDelimiterToken closing_delimiter, bool ignore_newline, bool filter_type);


#ifdef __cplusplus
}
#endif
