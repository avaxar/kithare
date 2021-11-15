/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <math.h>
#include <string.h>
#include <wctype.h>

#include <kithare/arrays.h>
#include <kithare/lexer.h>


static inline uint8_t digitOf(char chr) {
    // Regular decimal characters
    if (chr >= '0' && chr <= '9') {
        return chr - '0';
    }
    // Uppercase and lowercase hex characters
    else if (chr >= 'A' && chr <= 'Z') {
        return chr - 'A' + 10;
    }
    else if (chr >= 'a' && chr <= 'z') {
        return chr - 'a' + 10;
    }
    // 0xFF (255) as an unrecognized character
    else {
        return 0xFF;
    }
}

khToken kh_lex(char** cursor) {
    // Skips any whitespace
    while (iswspace(kh_lexUtf8(cursor, false))) {
        kh_lexUtf8(cursor, true);
    }

    if (iswalpha(kh_lexUtf8(cursor, false))) {
        if (**cursor == 'b' || **cursor == 'B') {
            (*cursor)++;

            switch (**cursor) {
                // Byte characters: b'X'
                case '\'':
                    return khToken_fromUint8(kh_lexChar(cursor, true));

                // Buffers: b"1234"
                case '"': {
                    khArray_char string = kh_lexString(cursor);
                    khArray_byte buffer = khArray_byte_new();
                    khArray_byte_reserve(&buffer, string.size);

                    for (uint32_t* chr = string.array; chr < string.array + string.size; chr++) {
                        // TODO: handle >255 characters
                        khArray_byte_push(&buffer, *chr);
                    }

                    khArray_char_delete(&string);
                    return khToken_fromBuffer(buffer);
                }

                default:
                    (*cursor)--;
                    return kh_lexWord(cursor);
            }
        }
        else {
            return kh_lexWord(cursor);
        }
    }
    else if (digitOf(**cursor) < 10) {
        return kh_lexNumber(cursor);
    }
    else if (**cursor == '\'') {
        return khToken_fromChar(kh_lexChar(cursor, true));
    }
    else if (**cursor == '"') {
        return khToken_fromString(kh_lexString(cursor));
    }
    else {
        return kh_lexSymbol(cursor);
    }
}

khToken kh_lexWord(char** cursor) {
    char* origin = *cursor;

    // Passes through alphanumeric characters in a row
    while (iswalnum(kh_lexUtf8(cursor, false))) {
        kh_lexUtf8(cursor, true);
    }

    khArray_byte identifier = khArray_byte_fromMemory((uint8_t*)origin, *cursor - origin);
    khArray_byte_push(&identifier, '\0'); // Push a null-terminator for strcmp

#define CASE_OPERATOR(STRING, OPERATOR)                 \
    if (strcmp((char*)identifier.array, STRING) == 0) { \
        khArray_byte_delete(&identifier);               \
        return khToken_fromOperator(OPERATOR);          \
    }

    CASE_OPERATOR("not", khOperatorToken_NOT);
    CASE_OPERATOR("and", khOperatorToken_AND);
    CASE_OPERATOR("or", khOperatorToken_OR);
    CASE_OPERATOR("xor", khOperatorToken_XOR);

#define CASE_KEYWORD(STRING, KEYWORD)                   \
    if (strcmp((char*)identifier.array, STRING) == 0) { \
        khArray_byte_delete(&identifier);               \
        return khToken_fromKeyword(KEYWORD);            \
    }

    CASE_KEYWORD("import", khKeywordToken_IMPORT);
    CASE_KEYWORD("include", khKeywordToken_INCLUDE);
    CASE_KEYWORD("as", khKeywordToken_AS);
    CASE_KEYWORD("try", khKeywordToken_TRY);
    CASE_KEYWORD("def", khKeywordToken_DEF);
    CASE_KEYWORD("class", khKeywordToken_CLASS);
    CASE_KEYWORD("struct", khKeywordToken_STRUCT);
    CASE_KEYWORD("enum", khKeywordToken_ENUM);
    CASE_KEYWORD("alias", khKeywordToken_ALIAS);

    CASE_KEYWORD("ref", khKeywordToken_REF);
    CASE_KEYWORD("public", khKeywordToken_PUBLIC);
    CASE_KEYWORD("private", khKeywordToken_PRIVATE);
    CASE_KEYWORD("static", khKeywordToken_STATIC);

    CASE_KEYWORD("if", khKeywordToken_IF);
    CASE_KEYWORD("elif", khKeywordToken_ELIF);
    CASE_KEYWORD("else", khKeywordToken_ELSE);
    CASE_KEYWORD("for", khKeywordToken_FOR);
    CASE_KEYWORD("while", khKeywordToken_WHILE);
    CASE_KEYWORD("do", khKeywordToken_DO);
    CASE_KEYWORD("break", khKeywordToken_BREAK);
    CASE_KEYWORD("continue", khKeywordToken_CONTINUE);
    CASE_KEYWORD("return", khKeywordToken_RETURN);

#undef CASE_OPERATOR
#undef CASE_KEYWORD

    khArray_byte_pop(&identifier, 1); // Get rid of the extra null-terminator
    return khToken_fromIdentifier(identifier);
}

khToken kh_lexNumber(char** cursor) {
    if (digitOf(**cursor) > 9) {
        // TODO: handle error
        return khToken_fromNone();
    }

    uint8_t base = 10;
    if (**cursor == '0') {
        (*cursor)++;

        switch (*(*cursor)++) {
            // Binary, 0b...
            case 'b':
            case 'B':
                base = 2;
                break;

            // Octal, 0o...
            case 'o':
            case 'O':
                base = 8;
                break;

            // Hexadecimal, 0x...
            case 'x':
            case 'X':
                base = 16;
                break;

            default:
                (*cursor) -= 2;
                break;
        }
    }

    char* origin = *cursor;
    bool had_overflowed;
    uint64_t integer = kh_lexInt(cursor, base, -1, &had_overflowed);

    // When it didn't lex any characters (presumably because it's out of base)
    if (*cursor == origin) {
        // TODO: handle error
        return khToken_fromNone();
    }
    // If it was a floating point
    else if (**cursor == '.' || had_overflowed) {
        *cursor = origin;
        double floating = kh_lexFloat(cursor, base);

        switch (*(*cursor)++) {
            // 4.f -> float(4.0)
            case 'f':
            case 'F':
                return khToken_fromFloat(floating);

            // 4.d -> double(4.0)
            case 'd':
            case 'D':
                return khToken_fromDouble(floating);

            // Imaginary floating points
            case 'i':
            case 'I':
                switch (*(*cursor)++) {
                    // 5.if -> cfloat(0.0, 5.0)
                    case 'f':
                    case 'F':
                        return khToken_fromIfloat(floating);

                    // 5.id -> cdouble(0.0, 5.0)
                    case 'd':
                    case 'D':
                        return khToken_fromIdouble(floating);

                    // Defaults to a cdouble, without any extra suffixes
                    default:
                        (*cursor)--;
                        return khToken_fromIdouble(floating);
                }

            // Defaults to a double, without any suffixes
            default:
                (*cursor)--;
                return khToken_fromDouble(floating);
        }
    }
    else {
        switch (*(*cursor)++) {
            // 1b -> byte(1)
            case 'b':
            case 'B':
                return khToken_fromInt8(integer);

            // 1s -> short(1)
            case 's':
            case 'S':
                return khToken_fromInt16(integer);

            // 1l -> long(1)
            case 'l':
            case 'L':
                return khToken_fromInt64(integer);

            // Unsigned integers
            case 'u':
            case 'U':
                switch (*(*cursor)++) {
                    // 2ub -> ubyte(2)
                    case 'b':
                    case 'B':
                        return khToken_fromUint8(integer);

                    // 2us -> ushort(2)
                    case 's':
                    case 'S':
                        return khToken_fromUint16(integer);

                    // 2ul -> ulong(2)
                    case 'l':
                    case 'L':
                        return khToken_fromUint64(integer);

                    // Defaults to an uint32, without any extra suffixes
                    default:
                        (*cursor)--;
                        return khToken_fromUint32(integer);
                }

            // Defaults to an int32, without any suffixes
            default:
                (*cursor)--;
                return khToken_fromInt32(integer);
        }
    }
}

khToken kh_lexSymbol(char** cursor) {
#define CASE_DELIMITER(CHR, DELIMITER) \
    case CHR:                          \
        return khToken_fromDelimiter(DELIMITER)

    switch (*(*cursor)++) {
        // Pretty repetitive stuff here. Macros are utilized
        CASE_DELIMITER('.', khDelimiterToken_DOT);
        CASE_DELIMITER(',', khDelimiterToken_COMMA);
        CASE_DELIMITER(':', khDelimiterToken_COLON);
        CASE_DELIMITER(';', khDelimiterToken_SEMICOLON);

        CASE_DELIMITER('(', khDelimiterToken_PARENTHESES_OPEN);
        CASE_DELIMITER(')', khDelimiterToken_PARENTHESES_CLOSE);
        CASE_DELIMITER('{', khDelimiterToken_CURLY_BRACKET_OPEN);
        CASE_DELIMITER('}', khDelimiterToken_CURLY_BRACKET_CLOSE);
        CASE_DELIMITER('[', khDelimiterToken_SQUARE_BRACKET_OPEN);
        CASE_DELIMITER(']', khDelimiterToken_CURLY_BRACKET_CLOSE);

        // Down here are where many multiple-character-operators are handled
        case '+':
            switch (*(*cursor)++) {
                case '+':
                    return khToken_fromOperator(khOperatorToken_INCREMENT);

                case '=':
                    return khToken_fromOperator(khOperatorToken_IADD);

                default:
                    (*cursor)--;
                    return khToken_fromOperator(khOperatorToken_ADD);
            }

        case '-':
            switch (*(*cursor)++) {
                case '-':
                    return khToken_fromOperator(khOperatorToken_DECREMENT);

                case '=':
                    return khToken_fromOperator(khOperatorToken_ISUB);

                default:
                    (*cursor)--;
                    return khToken_fromOperator(khOperatorToken_SUB);
            }

        case '*':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IMUL);
            }
            else {
                return khToken_fromOperator(khOperatorToken_MUL);
            }

        case '/':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IDIV);
            }
            else {
                return khToken_fromOperator(khOperatorToken_DIV);
            }

        case '%':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IMODULO);
            }
            else {
                return khToken_fromOperator(khOperatorToken_MODULO);
            }

        case '^':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IPOWER);
            }
            else {
                return khToken_fromOperator(khOperatorToken_POWER);
            }

        case '=':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_EQUALS);
            }
            else {
                return khToken_fromOperator(khOperatorToken_ASSIGN);
            }

        case '@':
            return khToken_fromOperator(khOperatorToken_ID);

        case '!':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_NOT_EQUAL);
            }
            else {
                return khToken_fromDelimiter(khDelimiterToken_EXCLAMATION);
            }

        case '<':
            switch (*(*cursor)++) {
                case '=':
                    return khToken_fromOperator(khOperatorToken_ELESS);

                case '<':
                    if (**cursor == '=') {
                        (*cursor)++;
                        return khToken_fromOperator(khOperatorToken_IBIT_LSHIFT);
                    }
                    else {
                        return khToken_fromOperator(khOperatorToken_BIT_LSHIFT);
                    }

                default:
                    (*cursor)--;
                    return khToken_fromOperator(khOperatorToken_LESS);
            }

        case '>':
            switch (*(*cursor)++) {
                case '=':
                    return khToken_fromOperator(khOperatorToken_EMORE);

                case '<':
                    if (**cursor == '=') {
                        (*cursor)++;
                        return khToken_fromOperator(khOperatorToken_IBIT_RSHIFT);
                    }
                    else {
                        return khToken_fromOperator(khOperatorToken_BIT_RSHIFT);
                    }

                default:
                    (*cursor)--;
                    return khToken_fromOperator(khOperatorToken_MORE);
            }

        case '~':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IBIT_XOR);
            }
            else {
                // It's also khOperatorToken_BIT_XOR
                return khToken_fromOperator(khOperatorToken_BIT_OR);
            }

        case '&':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IBIT_AND);
            }
            else {
                return khToken_fromOperator(khOperatorToken_BIT_AND);
            }

        case '|':
            if (**cursor == '=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IBIT_OR);
            }
            else {
                return khToken_fromOperator(khOperatorToken_BIT_OR);
            }

        // Unexpected null-terminator
        case '\0':
            (*cursor)--;
            // TODO: handle error
            return khToken_fromNone();

        default:
            (*cursor)--;
            // TODO: handle error
            return khToken_fromNone();
    }
#undef CASE_DELIMITER
}

uint32_t kh_lexChar(char** cursor, bool with_quotes) {
    uint32_t chr = 0;

    if (with_quotes) {
        if (**cursor == '\'') {
            (*cursor)++;
        }
        else {
            // TODO: handle error
        }
    }

    // Handle escapes
    if (**cursor == '\\') {
        (*cursor)++;

        switch (*(*cursor)++) {
            // Handle many single character escapes
            case '0':
                chr = '\0';
                break;
            case 'n':
                chr = '\n';
                break;
            case 'r':
                chr = '\r';
                break;
            case 't':
                chr = '\t';
                break;
            case 'v':
                chr = '\v';
                break;
            case 'b':
                chr = '\b';
                break;
            case 'a':
                chr = '\a';
                break;
            case 'f':
                chr = '\f';
                break;
            case '\\':
                chr = '\\';
                break;
            case '\'':
                chr = '\'';
                break;
            case '"':
                chr = '"';
                break;

            // \xAA
            case 'x': {
                char* origin = *cursor;

                chr = kh_lexInt(cursor, 16, 2, NULL);
                if (*cursor != origin + 2) {
                    // TODO: handle error
                }

                break;
            }

            // \uAABB
            case 'u': {
                char* origin = *cursor;

                chr = kh_lexInt(cursor, 16, 4, NULL);
                if (*cursor != origin + 4) {
                    // TODO: handle error
                }

                break;
            }

            // \UAABBCCDD
            case 'U': {
                char* origin = *cursor;

                chr = kh_lexInt(cursor, 16, 8, NULL);
                if (*cursor != origin + 8) {
                    // TODO: handle error
                }

                break;
            }

            // Unexpected null-terminator
            case '\0':
                (*cursor)--;
                // TODO: handle error
                break;

            // Unrecognized escape character
            default:
                (*cursor)--;
                // TODO: handle error
                break;
        }
    }
    else {
        chr = kh_lexUtf8(cursor, false);

        switch (chr) {
            // Encourage users to use '\'' instead
            case '\'':
                if (with_quotes) {
                    // TODO: handle error
                }
                break;

            // Encourage users to use '\n' instead
            case '\n':
                // TODO: handle error
                break;

            // Unexpected null-terminator
            case '\0':
                // TODO: handle error
                break;
        }

        kh_lexUtf8(cursor, true); // Goes to the next character
    }

    if (with_quotes) {
        if (**cursor == '\'') {
            (*cursor)++;
        }
        else {
            // TODO: handle error
        }
    }

    return chr;
}

khArray_char kh_lexString(char** cursor) {
    khArray_char string = khArray_char_new();
    bool multiline = false;

    if (**cursor == '"') {
        (*cursor)++;

        // For multiline strings, uses triple double quotes
        if ((*cursor)[0] == '"' && (*cursor)[1] == '"') {
            *cursor += 2;
            multiline = true;
        }
    }
    else {
        // TODO: handle error
    }

    while (true) {
        switch (**cursor) {
            // End string
            case '"':
                if (multiline) {
                    if ((*cursor)[1] == '"' && (*cursor)[2] == '"') {
                        *cursor += 3;
                        return string;
                    }
                    else {
                        (*cursor)++;
                        khArray_char_push(&string, '"');
                    }
                }
                else {
                    (*cursor)++;
                    return string;
                }
                break;

            // Explicitly handle '\n', separate from kh_lexChar
            case '\n':
                if (multiline) {
                    (*cursor)++;
                    khArray_char_push(&string, '\n');
                }
                else {
                    // TODO: handle error
                }
                break;

            // Unexpected null-terminator
            case '\0':
                // TODO: handle error
                break;

            // Use kh_lexChar for other character encounters
            default:
                khArray_char_push(&string, kh_lexChar(cursor, false));
                break;
        }
    }

    return string;
}

uint32_t kh_lexUtf8(char** cursor, bool skip) {
    // Pass ASCII characters
    if ((uint8_t)(**cursor) < 128) {
        if (skip) {
            return *(*cursor)++;
        }
        else {
            return **cursor;
        }
    }

    uint32_t chr = 0;
    uint8_t continuation = 0;

    if ((**cursor & 0b11100000) == 0b11000000) {
        chr = **cursor & 0b00011111;
        continuation = 1;
    }
    else if ((**cursor & 0b11110000) == 0b11100000) {
        chr = **cursor & 0b00001111;
        continuation = 2;
    }
    else if ((**cursor & 0b11111000) == 0b11110000) {
        chr = **cursor & 0b00000111;
        continuation = 3;
    }

    char* origin = *cursor;

    for ((*cursor)++; continuation > 0; continuation--, (*cursor)++) {
        if ((**cursor & 0b11000000) != 0b10000000) {
            return -1;
        }

        chr = (chr << 6) | (**cursor & 0b00111111);
    }

    if (!skip) {
        *cursor = origin;
    }

    return chr;
}

uint64_t kh_lexInt(char** cursor, uint8_t base, size_t max_length, bool* had_overflowed) {
    uint64_t result = 0;

    if (had_overflowed != NULL) {
        *had_overflowed = false;
    }

    while (digitOf(**cursor) < base && max_length > 0) {
        uint64_t previous = result;
        result *= base; // Shift left of the base: 123 -> 1230 (decimal)
        result += digitOf(**cursor);

        // Simple overflow check
        if (result < previous && had_overflowed != NULL) {
            *had_overflowed = true;
        }

        (*cursor)++;
        max_length--;
    }

    return result;
}

double kh_lexFloat(char** cursor, uint8_t base) {
    double result = 0;

    // The same implementation of kh_lexInt is used here. The reason of not using kh_lexInt directly
    // is to avoid any integer overflows
    while (digitOf(**cursor) < base) {
        result *= base;
        result += digitOf(**cursor);
        (*cursor)++;
    }

    // Where the main show begins
    if (**cursor == '.') {
        (*cursor)++;
        double exponent = 1.l / base;

        while (digitOf(**cursor) < base) {
            result += digitOf(**cursor) * exponent;
            exponent /= base; // Shift right of the base: 0.01 -> 0.001 (decimal)
            (*cursor)++;
        }
    }

    bool had_overflowed;
    switch (**cursor) {
        // 2e3 -> 2 * 10^3 -> 2000
        case 'e':
        case 'E':
            (*cursor)++;

            // Negative exponent
            if (**cursor == '-') {
                (*cursor)++;

                result *= pow(10, -kh_lexInt(cursor, 10, -1, &had_overflowed));
                if (had_overflowed) {
                    result = 0.0;
                }
            }
            else {
                if (**cursor == '+') {
                    (*cursor)++;
                }

                result *= pow(10, kh_lexInt(cursor, 10, -1, &had_overflowed));
                if (had_overflowed) {
                    result = INFINITY;
                }
            }

            break;

        // 2p3 -> 2 * 2^3 -> 16
        case 'p':
        case 'P':
            (*cursor)++;

            // Negative exponent
            if (**cursor == '-') {
                (*cursor)++;

                result *= pow(2, -kh_lexInt(cursor, 10, -1, &had_overflowed));
                if (had_overflowed) {
                    result = 0.0;
                }
            }
            else {
                if (**cursor == '+') {
                    (*cursor)++;
                }

                result *= pow(2, kh_lexInt(cursor, 10, -1, &had_overflowed));
                if (had_overflowed) {
                    result = INFINITY;
                }
            }

            break;
    }

    return result;
}
