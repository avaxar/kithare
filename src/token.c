/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#include <kithare/core/token.h>
#include <kithare/lib/string.h>


khstring khTokenType_string(khTokenType type) {
    switch (type) {
        case khTokenType_INVALID:
            return khstring_new(U"invalid");
        case khTokenType_EOF:
            return khstring_new(U"eof");
        case khTokenType_NEWLINE:
            return khstring_new(U"newline");
        case khTokenType_COMMENT:
            return khstring_new(U"comment");

        case khTokenType_IDENTIFIER:
            return khstring_new(U"identifier");
        case khTokenType_KEYWORD:
            return khstring_new(U"keyword");
        case khTokenType_DELIMITER:
            return khstring_new(U"delimiter");
        case khTokenType_OPERATOR:
            return khstring_new(U"operator");

        case khTokenType_CHAR:
            return khstring_new(U"char");
        case khTokenType_STRING:
            return khstring_new(U"string");
        case khTokenType_BUFFER:
            return khstring_new(U"buffer");

        case khTokenType_BYTE:
            return khstring_new(U"byte");
        case khTokenType_INTEGER:
            return khstring_new(U"integer");
        case khTokenType_UINTEGER:
            return khstring_new(U"uinteger");
        case khTokenType_FLOAT:
            return khstring_new(U"float");
        case khTokenType_DOUBLE:
            return khstring_new(U"double");
        case khTokenType_IDOUBLE:
            return khstring_new(U"idouble");
        case khTokenType_IFLOAT:
            return khstring_new(U"ifloat");

        default:
            return khstring_new(U"[unknown]");
    }
}


khstring khKeywordToken_string(khKeywordToken keyword) {
    switch (keyword) {
        case khKeywordToken_IMPORT:
            return khstring_new(U"import");
        case khKeywordToken_INCLUDE:
            return khstring_new(U"include");
        case khKeywordToken_AS:
            return khstring_new(U"as");
        case khKeywordToken_DEF:
            return khstring_new(U"def");
        case khKeywordToken_CLASS:
            return khstring_new(U"class");
        case khKeywordToken_STRUCT:
            return khstring_new(U"struct");
        case khKeywordToken_ENUM:
            return khstring_new(U"enum");
        case khKeywordToken_ALIAS:
            return khstring_new(U"alias");

        case khKeywordToken_REF:
            return khstring_new(U"ref");
        case khKeywordToken_WILD:
            return khstring_new(U"wild");
        case khKeywordToken_INCASE:
            return khstring_new(U"incase");
        case khKeywordToken_STATIC:
            return khstring_new(U"static");

        case khKeywordToken_IF:
            return khstring_new(U"if");
        case khKeywordToken_ELIF:
            return khstring_new(U"elif");
        case khKeywordToken_ELSE:
            return khstring_new(U"else");
        case khKeywordToken_FOR:
            return khstring_new(U"for");
        case khKeywordToken_IN:
            return khstring_new(U"in");
        case khKeywordToken_WHILE:
            return khstring_new(U"while");
        case khKeywordToken_DO:
            return khstring_new(U"do");
        case khKeywordToken_BREAK:
            return khstring_new(U"break");
        case khKeywordToken_CONTINUE:
            return khstring_new(U"continue");
        case khKeywordToken_RETURN:
            return khstring_new(U"return");

        default:
            return khstring_new(U"[unknown]");
    }
}


khstring khDelimiterToken_string(khDelimiterToken delimiter) {
    switch (delimiter) {
        case khDelimiterToken_DOT:
            return khstring_new(U".");
        case khDelimiterToken_COMMA:
            return khstring_new(U",");
        case khDelimiterToken_COLON:
            return khstring_new(U":");
        case khDelimiterToken_SEMICOLON:
            return khstring_new(U";");
        case khDelimiterToken_EXCLAMATION:
            return khstring_new(U"!");

        case khDelimiterToken_PARENTHESIS_OPEN:
            return khstring_new(U"(");
        case khDelimiterToken_PARENTHESIS_CLOSE:
            return khstring_new(U")");
        case khDelimiterToken_CURLY_BRACKET_OPEN:
            return khstring_new(U"{");
        case khDelimiterToken_CURLY_BRACKET_CLOSE:
            return khstring_new(U"}");
        case khDelimiterToken_SQUARE_BRACKET_OPEN:
            return khstring_new(U"[");
        case khDelimiterToken_SQUARE_BRACKET_CLOSE:
            return khstring_new(U"]");

        case khDelimiterToken_ARROW:
            return khstring_new(U"->");
        case khDelimiterToken_ELLIPSIS:
            return khstring_new(U"...");

        default:
            return khstring_new(U"[unknown]");
    }
}


khstring khOperatorToken_string(khOperatorToken operator_v) {
    switch (operator_v) {
        case khOperatorToken_ADD:
            return khstring_new(U"+");
        case khOperatorToken_SUB:
            return khstring_new(U"-");
        case khOperatorToken_MUL:
            return khstring_new(U"*");
        case khOperatorToken_DIV:
            return khstring_new(U"/");
        case khOperatorToken_MOD:
            return khstring_new(U"%");
        case khOperatorToken_POW:
            return khstring_new(U"^");
        case khOperatorToken_DOT:
            return khstring_new(U"@");

        case khOperatorToken_IADD:
            return khstring_new(U"+=");
        case khOperatorToken_ISUB:
            return khstring_new(U"-=");
        case khOperatorToken_IMUL:
            return khstring_new(U"*=");
        case khOperatorToken_IDIV:
            return khstring_new(U"/=");
        case khOperatorToken_IMOD:
            return khstring_new(U"%=");
        case khOperatorToken_IPOW:
            return khstring_new(U"^=");
        case khOperatorToken_IDOT:
            return khstring_new(U"@=");

        case khOperatorToken_INCREMENT:
            return khstring_new(U"++");
        case khOperatorToken_DECREMENT:
            return khstring_new(U"--");

        case khOperatorToken_ASSIGN:
            return khstring_new(U"=");

        case khOperatorToken_EQUAL:
            return khstring_new(U"==");
        case khOperatorToken_NOT_EQUAL:
            return khstring_new(U"!=");
        case khOperatorToken_LESS:
            return khstring_new(U"<");
        case khOperatorToken_MORE:
            return khstring_new(U">");
        case khOperatorToken_ELESS:
            return khstring_new(U"<=");
        case khOperatorToken_EMORE:
            return khstring_new(U">=");

        case khOperatorToken_NOT:
            return khstring_new(U"not");
        case khOperatorToken_AND:
            return khstring_new(U"and");
        case khOperatorToken_OR:
            return khstring_new(U"or");
        case khOperatorToken_XOR:
            return khstring_new(U"xor");

        case khOperatorToken_BIT_NOT:
            return khstring_new(U"~");
        case khOperatorToken_BIT_AND:
            return khstring_new(U"&");
        case khOperatorToken_BIT_OR:
            return khstring_new(U"|");
        case khOperatorToken_BIT_LSHIFT:
            return khstring_new(U"<<");
        case khOperatorToken_BIT_RSHIFT:
            return khstring_new(U">>");

        case khOperatorToken_IBIT_AND:
            return khstring_new(U"&=");
        case khOperatorToken_IBIT_OR:
            return khstring_new(U"|=");
        case khOperatorToken_IBIT_XOR:
            return khstring_new(U"~=");
        case khOperatorToken_IBIT_LSHIFT:
            return khstring_new(U"<<=");
        case khOperatorToken_IBIT_RSHIFT:
            return khstring_new(U">>=");

        default:
            return khstring_new(U"[unknown]");
    }
}


khToken khToken_copy(khToken* token) {
    khToken copy = *token;

    switch (token->type) {
        case khTokenType_IDENTIFIER:
            copy.identifier = khstring_copy(&token->identifier);
            break;

        case khTokenType_STRING:
            copy.string = khstring_copy(&token->string);
            break;

        case khTokenType_BUFFER:
            copy.buffer = khbuffer_copy(&token->buffer);
            break;

        default:
            break;
    }

    return copy;
}

void khToken_delete(khToken* token) {
    switch (token->type) {
        case khTokenType_IDENTIFIER:
            khstring_delete(&token->identifier);
            break;

        case khTokenType_STRING:
            khstring_delete(&token->string);
            break;

        case khTokenType_BUFFER:
            khbuffer_delete(&token->buffer);
            break;

        default:
            break;
    }
}

khstring khToken_string(khToken* token, char32_t* origin) {
    khstring string = khTokenType_string(token->type);
    khstring_append(&string, U'(');

    khstring begin_str = kh_uintToString((token->begin - origin) / sizeof(char32_t), 10);
    khstring_concatenate(&string, &begin_str);
    khstring_delete(&begin_str);
    khstring_concatenateCstring(&string, U", ");

    khstring end_str = kh_uintToString((token->end - origin) / sizeof(char32_t), 10);
    khstring_concatenate(&string, &end_str);
    khstring_delete(&end_str);
    khstring_concatenateCstring(&string, U", ");

    khstring value;
    switch (token->type) {
        case khTokenType_IDENTIFIER:
            value = khstring_copy(&token->identifier);
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
            khstring_append(&string, U'\'');
            value = kh_escapeChar(token->char_v);
            khstring_append(&value, U'\'');
            break;
        case khTokenType_STRING:
            value = khstring_quote(&token->string);
            break;
        case khTokenType_BUFFER:
            value = khbuffer_quote(&token->buffer);
            break;

        case khTokenType_BYTE:
            khstring_concatenateCstring(&string, U"\'");
            value = kh_escapeChar(token->byte);
            khstring_append(&value, U'\'');
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
            kharray_pop(&string, 3);
            value = khstring_new(U"");
            break;
    }

    khstring_concatenate(&string, &value);
    khstring_delete(&value);
    khstring_append(&string, U')');

    return string;
}
