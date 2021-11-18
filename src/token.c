/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/string.h>
#include <kithare/token.h>


khToken khToken_copy(khToken* token) {
    khTokenValue token_value = token->value;

    switch (token->type) {
        case khTokenType_IDENTIFIER:
            token_value.identifier = khArray_char_copy(&token->value.identifier);
            break;

        case khTokenType_STRING:
            token_value.string = khArray_char_copy(&token->value.string);
            break;

        case khTokenType_BUFFER:
            token_value.buffer = khArray_byte_copy(&token->value.buffer);
            break;

        default:
            break;
    }

    return (khToken){.type = token->type, .value = token_value};
}

void khToken_delete(khToken* token) {
    switch (token->type) {
        case khTokenType_IDENTIFIER:
            khArray_char_delete(&token->value.identifier);
            break;

        case khTokenType_STRING:
            khArray_char_delete(&token->value.string);
            break;

        case khTokenType_BUFFER:
            khArray_byte_delete(&token->value.buffer);
            break;

        default:
            break;
    }
}

khArray_char khKeywordToken_string(khKeywordToken keyword) {
    switch (keyword) {
        case khKeywordToken_IMPORT:
            return kh_string("import");
        case khKeywordToken_INCLUDE:
            return kh_string("include");
        case khKeywordToken_AS:
            return kh_string("as");
        case khKeywordToken_TRY:
            return kh_string("try");
        case khKeywordToken_DEF:
            return kh_string("def");
        case khKeywordToken_CLASS:
            return kh_string("class");
        case khKeywordToken_STRUCT:
            return kh_string("struct");
        case khKeywordToken_ENUM:
            return kh_string("enum");
        case khKeywordToken_ALIAS:
            return kh_string("alias");

        case khKeywordToken_REF:
            return kh_string("ref");
        case khKeywordToken_PUBLIC:
            return kh_string("public");
        case khKeywordToken_PRIVATE:
            return kh_string("private");
        case khKeywordToken_STATIC:
            return kh_string("static");

        case khKeywordToken_IF:
            return kh_string("if");
        case khKeywordToken_ELIF:
            return kh_string("elif");
        case khKeywordToken_ELSE:
            return kh_string("else");
        case khKeywordToken_FOR:
            return kh_string("for");
        case khKeywordToken_WHILE:
            return kh_string("while");
        case khKeywordToken_DO:
            return kh_string("do");
        case khKeywordToken_BREAK:
            return kh_string("break");
        case khKeywordToken_CONTINUE:
            return kh_string("continue");
        case khKeywordToken_RETURN:
            return kh_string("return");

        default:
            return kh_string("[unknown]");
    }
}

khArray_char khDelimiterToken_string(khDelimiterToken delimiter) {
    switch (delimiter) {
        case khDelimiterToken_DOT:
            return kh_string(".");
        case khDelimiterToken_COMMA:
            return kh_string(",");
        case khDelimiterToken_COLON:
            return kh_string(":");
        case khDelimiterToken_SEMICOLON:
            return kh_string(";");
        case khDelimiterToken_EXCLAMATION:
            return kh_string("!");

        case khDelimiterToken_PARENTHESES_OPEN:
            return kh_string("(");
        case khDelimiterToken_PARENTHESES_CLOSE:
            return kh_string(")");
        case khDelimiterToken_CURLY_BRACKET_OPEN:
            return kh_string("{");
        case khDelimiterToken_CURLY_BRACKET_CLOSE:
            return kh_string("}");
        case khDelimiterToken_SQUARE_BRACKET_OPEN:
            return kh_string("[");
        case khDelimiterToken_SQUARE_BRACKET_CLOSE:
            return kh_string("]");

        default:
            return kh_string("[unknown]");
    }
}

khArray_char khOperatorToken_string(khOperatorToken operator_v) {
    switch (operator_v) {
        case khOperatorToken_ADD:
            return kh_string("+");
        case khOperatorToken_SUB:
            return kh_string("-");
        case khOperatorToken_MUL:
            return kh_string("*");
        case khOperatorToken_DIV:
            return kh_string("/");
        case khOperatorToken_MODULO:
            return kh_string("%");
        case khOperatorToken_POWER:
            return kh_string("^");

        case khOperatorToken_IADD:
            return kh_string("+=");
        case khOperatorToken_ISUB:
            return kh_string("-=");
        case khOperatorToken_IMUL:
            return kh_string("*=");
        case khOperatorToken_IDIV:
            return kh_string("/=");
        case khOperatorToken_IMODULO:
            return kh_string("%=");
        case khOperatorToken_IPOWER:
            return kh_string("^=");

        case khOperatorToken_ASSIGN:
            return kh_string("=");
        case khOperatorToken_ID:
            return kh_string("@");

        case khOperatorToken_INCREMENT:
            return kh_string("++");
        case khOperatorToken_DECREMENT:
            return kh_string("--");

        case khOperatorToken_EQUALS:
            return kh_string("==");
        case khOperatorToken_NOT_EQUAL:
            return kh_string("!=");
        case khOperatorToken_LESS:
            return kh_string("<");
        case khOperatorToken_MORE:
            return kh_string(">");
        case khOperatorToken_ELESS:
            return kh_string("<=");
        case khOperatorToken_EMORE:
            return kh_string(">=");

        case khOperatorToken_NOT:
            return kh_string("not");
        case khOperatorToken_AND:
            return kh_string("and");
        case khOperatorToken_OR:
            return kh_string("or");
        case khOperatorToken_XOR:
            return kh_string("xor");

        case khOperatorToken_BIT_NOT:
            return kh_string("~");
        case khOperatorToken_BIT_AND:
            return kh_string("&");
        case khOperatorToken_BIT_OR:
            return kh_string("|");
        case khOperatorToken_BIT_LSHIFT:
            return kh_string("<<");
        case khOperatorToken_BIT_RSHIFT:
            return kh_string(">>");

        case khOperatorToken_IBIT_AND:
            return kh_string("&=");
        case khOperatorToken_IBIT_OR:
            return kh_string("|=");
        case khOperatorToken_IBIT_XOR:
            return kh_string("~=");
        case khOperatorToken_IBIT_LSHIFT:
            return kh_string("<<=");
        case khOperatorToken_IBIT_RSHIFT:
            return kh_string(">>=");

        default:
            return kh_string("[unknown]");
    }
}

khArray_char khTokenType_string(khTokenType type) {
    switch (type) {
        case khTokenType_NONE:
            return kh_string("none");
        case khTokenType_COMMENT:
            return kh_string("comment");

        case khTokenType_IDENTIFIER:
            return kh_string("identifier");
        case khTokenType_KEYWORD:
            return kh_string("keyword");
        case khTokenType_DELIMITER:
            return kh_string("delimiter");
        case khTokenType_OPERATOR:
            return kh_string("operator");

        case khTokenType_CHAR:
            return kh_string("char");
        case khTokenType_STRING:
            return kh_string("string");
        case khTokenType_BUFFER:
            return kh_string("buffer");

        case khTokenType_SBYTE:
            return kh_string("sbyte");
        case khTokenType_BYTE:
            return kh_string("byte");
        case khTokenType_SHORT:
            return kh_string("short");
        case khTokenType_USHORT:
            return kh_string("ushort");
        case khTokenType_INT:
            return kh_string("int");
        case khTokenType_UINT:
            return kh_string("uint");
        case khTokenType_LONG:
            return kh_string("long");
        case khTokenType_ULONG:
            return kh_string("ulong");

        case khTokenType_FLOAT:
            return kh_string("float");
        case khTokenType_IFLOAT:
            return kh_string("ifloat");
        case khTokenType_DOUBLE:
            return kh_string("double");
        case khTokenType_IDOUBLE:
            return kh_string("idouble");

        default:
            return kh_string("[unknown]");
    }
}

khArray_char khToken_string(khToken* token) {
    khArray_char string = khTokenType_string(token->type);
    // C, please add U"" strings, ffs
    khArray_char_push(&string, ' ');
    khArray_char_push(&string, ':');
    khArray_char_push(&string, ' ');

    khArray_char value;

    switch (token->type) {
        case khTokenType_IDENTIFIER:
            value = khArray_char_copy(&token->value.identifier);
            break;
        case khTokenType_KEYWORD:
            value = khKeywordToken_string(token->value.keyword);
            break;
        case khTokenType_DELIMITER:
            value = khDelimiterToken_string(token->value.delimiter);
            break;
        case khTokenType_OPERATOR:
            value = khOperatorToken_string(token->value.operator_v);
            break;

        case khTokenType_CHAR:
            khArray_char_push(&string, '\'');
            value = kh_escapeChar(token->value.char_v);
            khArray_char_push(&value, '\'');
            break;
        case khTokenType_STRING:
            value = kh_quoteString(&token->value.string);
            break;
        case khTokenType_BUFFER:
            value = kh_quoteBuffer(&token->value.buffer);
            break;

        case khTokenType_SBYTE:
            value = kh_intToString(token->value.sbyte_v, 10);
            break;
        case khTokenType_BYTE:
            value = kh_uintToString(token->value.byte_v, 10);
            break;
        case khTokenType_SHORT:
            value = kh_intToString(token->value.short_v, 10);
            break;
        case khTokenType_USHORT:
            value = kh_uintToString(token->value.ushort_v, 10);
            break;
        case khTokenType_INT:
            value = kh_intToString(token->value.int_v, 10);
            break;
        case khTokenType_UINT:
            value = kh_uintToString(token->value.uint_v, 10);
            break;
        case khTokenType_LONG:
            value = kh_intToString(token->value.long_v, 10);
            break;
        case khTokenType_ULONG:
            value = kh_uintToString(token->value.ulong_v, 10);
            break;

        case khTokenType_FLOAT:
            value = kh_floatToString(token->value.float_v, 4, 10);
            break;
        case khTokenType_IFLOAT:
            value = kh_floatToString(token->value.ifloat_v, 4, 10);
            khArray_char_push(&value, 'i');
            break;
        case khTokenType_DOUBLE:
            value = kh_floatToString(token->value.double_v, 4, 10);
            break;
        case khTokenType_IDOUBLE:
            value = kh_floatToString(token->value.idouble_v, 4, 10);
            khArray_char_push(&value, 'i');
            break;

        default:
            khArray_char_pop(&string, 3);
            value = khArray_char_new();
            break;
    }

    khArray_char_concatenate(&string, &value);
    khArray_char_delete(&value);
    return string;
}
