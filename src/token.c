/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#include <kithare/string.h>
#include <kithare/token.h>


khArray(char32_t) khTokenType_string(khTokenType type) {
    switch (type) {
        case khTokenType_INVALID:
            return kh_string(U"invalid");
        case khTokenType_EOF:
            return kh_string(U"eof");
        case khTokenType_NEWLINE:
            return kh_string(U"newline");
        case khTokenType_COMMENT:
            return kh_string(U"comment");

        case khTokenType_IDENTIFIER:
            return kh_string(U"identifier");
        case khTokenType_KEYWORD:
            return kh_string(U"keyword");
        case khTokenType_DELIMITER:
            return kh_string(U"delimiter");
        case khTokenType_OPERATOR:
            return kh_string(U"operator");

        case khTokenType_CHAR:
            return kh_string(U"char");
        case khTokenType_STRING:
            return kh_string(U"string");
        case khTokenType_BUFFER:
            return kh_string(U"buffer");

        case khTokenType_BYTE:
            return kh_string(U"byte");
        case khTokenType_INTEGER:
            return kh_string(U"integer");
        case khTokenType_UINTEGER:
            return kh_string(U"uinteger");
        case khTokenType_FLOAT:
            return kh_string(U"float");
        case khTokenType_DOUBLE:
            return kh_string(U"double");
        case khTokenType_IDOUBLE:
            return kh_string(U"idouble");
        case khTokenType_IFLOAT:
            return kh_string(U"ifloat");

        default:
            return kh_string(U"[unknown]");
    }
}


khArray(char32_t) khKeywordToken_string(khKeywordToken keyword) {
    switch (keyword) {
        case khKeywordToken_IMPORT:
            return kh_string(U"import");
        case khKeywordToken_INCLUDE:
            return kh_string(U"include");
        case khKeywordToken_AS:
            return kh_string(U"as");
        case khKeywordToken_DEF:
            return kh_string(U"def");
        case khKeywordToken_CLASS:
            return kh_string(U"class");
        case khKeywordToken_STRUCT:
            return kh_string(U"struct");
        case khKeywordToken_ENUM:
            return kh_string(U"enum");
        case khKeywordToken_ALIAS:
            return kh_string(U"alias");

        case khKeywordToken_REF:
            return kh_string(U"ref");
        case khKeywordToken_WILD:
            return kh_string(U"wild");
        case khKeywordToken_INCASE:
            return kh_string(U"incase");
        case khKeywordToken_STATIC:
            return kh_string(U"static");

        case khKeywordToken_IF:
            return kh_string(U"if");
        case khKeywordToken_ELIF:
            return kh_string(U"elif");
        case khKeywordToken_ELSE:
            return kh_string(U"else");
        case khKeywordToken_FOR:
            return kh_string(U"for");
        case khKeywordToken_IN:
            return kh_string(U"in");
        case khKeywordToken_WHILE:
            return kh_string(U"while");
        case khKeywordToken_DO:
            return kh_string(U"do");
        case khKeywordToken_BREAK:
            return kh_string(U"break");
        case khKeywordToken_CONTINUE:
            return kh_string(U"continue");
        case khKeywordToken_RETURN:
            return kh_string(U"return");

        default:
            return kh_string(U"[unknown]");
    }
}


khArray(char32_t) khDelimiterToken_string(khDelimiterToken delimiter) {
    switch (delimiter) {
        case khDelimiterToken_DOT:
            return kh_string(U".");
        case khDelimiterToken_COMMA:
            return kh_string(U",");
        case khDelimiterToken_COLON:
            return kh_string(U":");
        case khDelimiterToken_SEMICOLON:
            return kh_string(U";");
        case khDelimiterToken_EXCLAMATION:
            return kh_string(U"!");

        case khDelimiterToken_PARENTHESES_OPEN:
            return kh_string(U"(");
        case khDelimiterToken_PARENTHESES_CLOSE:
            return kh_string(U")");
        case khDelimiterToken_CURLY_BRACKET_OPEN:
            return kh_string(U"{");
        case khDelimiterToken_CURLY_BRACKET_CLOSE:
            return kh_string(U"}");
        case khDelimiterToken_SQUARE_BRACKET_OPEN:
            return kh_string(U"[");
        case khDelimiterToken_SQUARE_BRACKET_CLOSE:
            return kh_string(U"]");

        case khDelimiterToken_ARROW:
            return kh_string(U"->");
        case khDelimiterToken_ELLIPSIS:
            return kh_string(U"...");

        default:
            return kh_string(U"[unknown]");
    }
}


khArray(char32_t) khOperatorToken_string(khOperatorToken operator_v) {
    switch (operator_v) {
        case khOperatorToken_ADD:
            return kh_string(U"+");
        case khOperatorToken_SUB:
            return kh_string(U"-");
        case khOperatorToken_MUL:
            return kh_string(U"*");
        case khOperatorToken_DIV:
            return kh_string(U"/");
        case khOperatorToken_MOD:
            return kh_string(U"%");
        case khOperatorToken_POW:
            return kh_string(U"^");
        case khOperatorToken_DOT:
            return kh_string(U"@");

        case khOperatorToken_IADD:
            return kh_string(U"+=");
        case khOperatorToken_ISUB:
            return kh_string(U"-=");
        case khOperatorToken_IMUL:
            return kh_string(U"*=");
        case khOperatorToken_IDIV:
            return kh_string(U"/=");
        case khOperatorToken_IMOD:
            return kh_string(U"%=");
        case khOperatorToken_IPOW:
            return kh_string(U"^=");
        case khOperatorToken_IDOT:
            return kh_string(U"@=");

        case khOperatorToken_INCREMENT:
            return kh_string(U"++");
        case khOperatorToken_DECREMENT:
            return kh_string(U"--");

        case khOperatorToken_ASSIGN:
            return kh_string(U"=");

        case khOperatorToken_EQUAL:
            return kh_string(U"==");
        case khOperatorToken_NOT_EQUAL:
            return kh_string(U"!=");
        case khOperatorToken_LESS:
            return kh_string(U"<");
        case khOperatorToken_MORE:
            return kh_string(U">");
        case khOperatorToken_ELESS:
            return kh_string(U"<=");
        case khOperatorToken_EMORE:
            return kh_string(U">=");

        case khOperatorToken_NOT:
            return kh_string(U"not");
        case khOperatorToken_AND:
            return kh_string(U"and");
        case khOperatorToken_OR:
            return kh_string(U"or");
        case khOperatorToken_XOR:
            return kh_string(U"xor");

        case khOperatorToken_BIT_NOT:
            return kh_string(U"~");
        case khOperatorToken_BIT_AND:
            return kh_string(U"&");
        case khOperatorToken_BIT_OR:
            return kh_string(U"|");
        case khOperatorToken_BIT_LSHIFT:
            return kh_string(U"<<");
        case khOperatorToken_BIT_RSHIFT:
            return kh_string(U">>");

        case khOperatorToken_IBIT_AND:
            return kh_string(U"&=");
        case khOperatorToken_IBIT_OR:
            return kh_string(U"|=");
        case khOperatorToken_IBIT_XOR:
            return kh_string(U"~=");
        case khOperatorToken_IBIT_LSHIFT:
            return kh_string(U"<<=");
        case khOperatorToken_IBIT_RSHIFT:
            return kh_string(U">>=");

        default:
            return kh_string(U"[unknown]");
    }
}


khToken khToken_copy(khToken* token) {
    khToken copy = *token;

    switch (token->type) {
        case khTokenType_IDENTIFIER:
            copy.identifier = khArray_copy(&token->identifier, NULL);
            break;

        case khTokenType_STRING:
            copy.string = khArray_copy(&token->string, NULL);
            break;

        case khTokenType_BUFFER:
            copy.buffer = khArray_copy(&token->buffer, NULL);
            break;

        default:
            break;
    }

    return copy;
}

void khToken_delete(khToken* token) {
    switch (token->type) {
        case khTokenType_IDENTIFIER:
            khArray_delete(&token->identifier);
            break;

        case khTokenType_STRING:
            khArray_delete(&token->string);
            break;

        case khTokenType_BUFFER:
            khArray_delete(&token->buffer);
            break;

        default:
            break;
    }
}

khArray(char32_t) khToken_string(khToken* token, char32_t* origin) {
    khArray(char32_t) string = khTokenType_string(token->type);
    khArray_append(&string, U'(');

    khArray(char32_t) begin_str = kh_uintToString((token->begin - origin) / sizeof(char32_t), 10);
    khArray_concatenate(&string, &begin_str, NULL);
    khArray_delete(&begin_str);
    kh_appendCstring(&string, U", ");

    khArray(char32_t) end_str = kh_uintToString((token->end - origin) / sizeof(char32_t), 10);
    khArray_concatenate(&string, &end_str, NULL);
    khArray_delete(&end_str);
    kh_appendCstring(&string, U", ");

    khArray(char32_t) value;
    switch (token->type) {
        case khTokenType_IDENTIFIER:
            value = khArray_copy(&token->identifier, NULL);
            break;
        case khTokenType_KEYWORD:
            value = khKeywordToken_string(token->keyword);
            break;
        case khTokenType_DELIMITER:
            value = khDelimiterToken_string(token->delimiter);
            break;
        case khTokenType_OPERATOR:
            value = khOperatorToken_string(token->operator_v);
            break;

        case khTokenType_CHAR:
            khArray_append(&string, U'\'');
            value = kh_escapeChar(token->char_v);
            khArray_append(&value, U'\'');
            break;
        case khTokenType_STRING:
            value = kh_quoteString(&token->string);
            break;
        case khTokenType_BUFFER:
            value = kh_quoteBuffer(&token->buffer);
            break;

        case khTokenType_BYTE:
            kh_appendCstring(&string, U"\'");
            value = kh_escapeChar(token->byte);
            khArray_append(&value, U'\'');
            break;
        case khTokenType_INTEGER:
            value = kh_intToString(token->integer, 10);
            break;
        case khTokenType_UINTEGER:
            value = kh_uintToString(token->uinteger, 10);
            break;
        case khTokenType_FLOAT:
            value = kh_floatToString(token->float_v, 4, 10);
            break;
        case khTokenType_DOUBLE:
            value = kh_floatToString(token->double_v, 4, 10);
            break;
        case khTokenType_IFLOAT:
            value = kh_floatToString(token->ifloat, 4, 10);
            break;
        case khTokenType_IDOUBLE:
            value = kh_floatToString(token->idouble, 4, 10);
            break;

        default:
            khArray_pop(&string, 3);
            value = khArray_new(char32_t, NULL);
            break;
    }

    khArray_concatenate(&string, &value, NULL);
    khArray_delete(&value);
    khArray_append(&string, U')');

    return string;
}
