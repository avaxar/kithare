/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
 */

#include <math.h>
#include <string.h>
#include <wctype.h>

#include <kithare/core/error.h>
#include <kithare/core/lexer.h>
#include <kithare/lib/buffer.h>
#include <kithare/lib/string.h>


static inline void raiseError(char32_t* ptr, const char32_t* message) {
    kh_raiseError((khError){.type = khErrorType_LEXER, .message = khstring_new(message), .data = ptr});
}

static inline uint8_t digitOf(char32_t chr) {
    // Regular decimal characters
    if (chr >= U'0' && chr <= U'9') {
        return chr - U'0';
    }
    // Uppercase and lowercase hex characters
    else if (chr >= U'A' && chr <= U'Z') {
        return chr - U'A' + 10;
    }
    else if (chr >= U'a' && chr <= U'z') {
        return chr - U'a' + 10;
    }
    // 0xFF (255) as an unrecognized character
    else {
        return 0xFF;
    }
}


kharray(khToken) kh_lexicate(khstring* string) {
    kharray(khToken) tokens = kharray_new(khToken, khToken_delete);
    char32_t* cursor = *string;

    do {
        kharray_append(&tokens, kh_lexToken(&cursor));
    } while (tokens[kharray_size(&tokens) - 1].type != khTokenType_EOF);
    kharray_pop(&tokens, 1);

    return tokens;
}


khToken kh_lexToken(char32_t** cursor) {
    // Skips any whitespace
    while (iswspace(**cursor)) {
        // Special case for newline
        if (**cursor == U'\n') {
            (*cursor)++;
            return khToken_fromNewline(*cursor - 1, *cursor);
        }
        else {
            (*cursor)++;
        }
    }

    char32_t* begin = *cursor;

    if (iswalpha(**cursor) || **cursor == U'_') {
        if (**cursor == U'b' || **cursor == U'B') {
            (*cursor)++;

            switch (**cursor) {
                // Byte characters: b'X'
                case U'\'': {
                    // Don't put kh_lexChar call in khToken_fromByte, as the evaluation of arguments
                    // aren't set by the C standard
                    char32_t chr = kh_lexChar(cursor, true, true);
                    return khToken_fromByte(chr, begin, *cursor);
                }

                // Buffers: b"1234"
                case U'"': {
                    khstring string = kh_lexString(cursor, true);
                    khbuffer buffer = khbuffer_new("");
                    kharray_reserve(&buffer, khstring_size(&string));

                    for (char32_t* chr = string; chr < string + khstring_size(&string); chr++) {
                        khbuffer_append(&buffer, *chr);
                    }

                    khstring_delete(&string);
                    return khToken_fromBuffer(buffer, begin, *cursor);
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
    else {
        switch (**cursor) {
            case U'\'': {
                char32_t chr = kh_lexChar(cursor, true, false);
                return khToken_fromChar(chr, begin, *cursor);
            }

            case U'"': {
                khstring string = kh_lexString(cursor, false);
                return khToken_fromString(string, begin, *cursor);
            }

            case U'#':
                (*cursor)++;
                for (; !(**cursor == U'\n' || **cursor == U'\0'); (*cursor)++) {}
                if (**cursor == U'\n') {
                    (*cursor)++;
                }
                return khToken_fromComment(begin, *cursor);

            default:
                return kh_lexSymbol(cursor);
        }
    }
}

khToken kh_lexWord(char32_t** cursor) {
    char32_t* begin = *cursor;

    // Passes through alphanumeric or underscore characters in a row
    while (iswalnum(**cursor) || **cursor == U'_') {
        (*cursor)++;
    }

    khstring identifier = khstring_new(U"");
    kharray_memory(&identifier, begin, *cursor - begin, NULL);

#define CASE_OPERATOR(STRING, OPERATOR)                        \
    if (khstring_equalCstring(&identifier, STRING)) {          \
        khstring_delete(&identifier);                          \
        return khToken_fromOperator(OPERATOR, begin, *cursor); \
    }

    CASE_OPERATOR(U"not", khOperatorToken_NOT);
    CASE_OPERATOR(U"and", khOperatorToken_AND);
    CASE_OPERATOR(U"or", khOperatorToken_OR);
    CASE_OPERATOR(U"xor", khOperatorToken_XOR);

#define CASE_KEYWORD(STRING, KEYWORD)                        \
    if (khstring_equalCstring(&identifier, STRING)) {        \
        khstring_delete(&identifier);                        \
        return khToken_fromKeyword(KEYWORD, begin, *cursor); \
    }

    CASE_KEYWORD(U"import", khKeywordToken_IMPORT);
    CASE_KEYWORD(U"include", khKeywordToken_INCLUDE);
    CASE_KEYWORD(U"as", khKeywordToken_AS);
    CASE_KEYWORD(U"def", khKeywordToken_DEF);
    CASE_KEYWORD(U"class", khKeywordToken_CLASS);
    CASE_KEYWORD(U"inherits", khKeywordToken_INHERITS);
    CASE_KEYWORD(U"struct", khKeywordToken_STRUCT);
    CASE_KEYWORD(U"enum", khKeywordToken_ENUM);
    CASE_KEYWORD(U"alias", khKeywordToken_ALIAS);

    CASE_KEYWORD(U"ref", khKeywordToken_REF);
    CASE_KEYWORD(U"wild", khKeywordToken_WILD);
    CASE_KEYWORD(U"incase", khKeywordToken_INCASE);
    CASE_KEYWORD(U"static", khKeywordToken_STATIC);

    CASE_KEYWORD(U"if", khKeywordToken_IF);
    CASE_KEYWORD(U"elif", khKeywordToken_ELIF);
    CASE_KEYWORD(U"else", khKeywordToken_ELSE);
    CASE_KEYWORD(U"for", khKeywordToken_FOR);
    CASE_KEYWORD(U"in", khKeywordToken_IN);
    CASE_KEYWORD(U"while", khKeywordToken_WHILE);
    CASE_KEYWORD(U"do", khKeywordToken_DO);
    CASE_KEYWORD(U"break", khKeywordToken_BREAK);
    CASE_KEYWORD(U"continue", khKeywordToken_CONTINUE);
    CASE_KEYWORD(U"return", khKeywordToken_RETURN);

#undef CASE_OPERATOR
#undef CASE_KEYWORD

    return khToken_fromIdentifier(identifier, begin, *cursor);
}

khToken kh_lexNumber(char32_t** cursor) {
    char32_t* begin = *cursor;

    if (digitOf(**cursor) > 9) {
        (*cursor)++;
        raiseError(*cursor, U"expecting a decimal number, from 0 to 9");
        return khToken_fromInvalid(*cursor, *cursor);
    }

    uint8_t base = 10;
    if (**cursor == U'0') {
        (*cursor)++;

        switch (*(*cursor)++) {
            // Binary, 0b...
            case U'b':
            case U'B':
                base = 2;
                break;

            // Octal, 0o...
            case U'o':
            case U'O':
                base = 8;
                break;

            // Hexadecimal, 0x...
            case U'x':
            case U'X':
                base = 16;
                break;

            default:
                (*cursor) -= 2;
                break;
        }
    }

    char32_t* origin = *cursor;
    bool had_overflowed;
    uint64_t integer = kh_lexInt(cursor, base, -1, &had_overflowed);

    // When it didn't lex any characters (presumably because it's out of base)
    if (*cursor == origin) {
        switch (base) {
            case 2:
                raiseError(*cursor, U"expecting a binary number, either 0 or 1");
                break;

            case 8:
                raiseError(*cursor, U"expecting an octal number, from 0 to 7");
                break;

            case 10:
                raiseError(*cursor, U"expecting a decimal number, from 0 to 9");
                break;

            case 16:
                raiseError(*cursor, U"expecting a hexadecimal number, from 0 to 9 or A to F");
                break;
        }

        return khToken_fromInvalid(begin, *cursor);
    }
    // If it was a floating point
    else if (**cursor == U'e' || **cursor == U'E' || **cursor == U'p' || **cursor == U'P' ||
             **cursor == U'f' || **cursor == U'F' || **cursor == 'j' || **cursor == 'J' ||
             **cursor == 'i' || **cursor == 'I' || **cursor == U'.') {
        *cursor = origin;
        double floating = kh_lexFloat(cursor, base);

        switch (*(*cursor)++) {
            // Explicit float
            case U'f':
            case U'F':
                return khToken_fromFloat(floating, begin, *cursor);

            // Imaginary float
            case U'j':
            case U'J':
                return khToken_fromIfloat(floating, begin, *cursor);

            // Imaginary double
            case U'i':
            case U'I':
                return khToken_fromIdouble(floating, begin, *cursor);

            // Defaults to a double, without any suffixes
            default:
                (*cursor)--;
                return khToken_fromDouble(floating, begin, *cursor);
        }
    }
    else if (had_overflowed) {
        raiseError(*cursor - 1, U"integer constant must not exceed 2^64");
        return khToken_fromInvalid(begin, *cursor);
    }
    else if (**cursor == U'u' || **cursor == U'U') {
        (*cursor)++;
        return khToken_fromUinteger(integer, begin, *cursor);
    }
    else {
        if (integer > (1ull << 63ull) - 1ull) {
            return khToken_fromUinteger(integer, begin, *cursor);
        }
        else {
            return khToken_fromInteger(integer, begin, *cursor);
        }
    }
}


khToken kh_lexSymbol(char32_t** cursor) {
    char32_t* begin = *cursor;

#define CASE_DELIMITER(CHR, DELIMITER) \
    case CHR:                          \
        return khToken_fromDelimiter(DELIMITER, begin, *cursor)

    switch (*(*cursor)++) {
        // Pretty repetitive stuff here. Macros are utilized
        CASE_DELIMITER(',', khDelimiterToken_COMMA);
        CASE_DELIMITER(':', khDelimiterToken_COLON);
        CASE_DELIMITER(';', khDelimiterToken_SEMICOLON);

        CASE_DELIMITER('(', khDelimiterToken_PARENTHESIS_OPEN);
        CASE_DELIMITER(')', khDelimiterToken_PARENTHESIS_CLOSE);
        CASE_DELIMITER('{', khDelimiterToken_CURLY_BRACKET_OPEN);
        CASE_DELIMITER('}', khDelimiterToken_CURLY_BRACKET_CLOSE);
        CASE_DELIMITER('[', khDelimiterToken_SQUARE_BRACKET_OPEN);
        CASE_DELIMITER(']', khDelimiterToken_SQUARE_BRACKET_CLOSE);

        case U'.':
            if ((*cursor)[0] == U'.' && (*cursor)[1] == U'.') {
                *cursor += 2;
                return khToken_fromDelimiter(khDelimiterToken_ELLIPSIS, begin, *cursor);
            }
            else {
                return khToken_fromDelimiter(khDelimiterToken_DOT, begin, *cursor);
            }

        // Down here are where many multiple-character-operators are handled
        case U'+':
            switch (*(*cursor)++) {
                case U'+':
                    return khToken_fromOperator(khOperatorToken_INCREMENT, begin, *cursor);

                case U'=':
                    return khToken_fromOperator(khOperatorToken_IADD, begin, *cursor);

                default:
                    (*cursor)--;
                    return khToken_fromOperator(khOperatorToken_ADD, begin, *cursor);
            }

        case U'-':
            switch (*(*cursor)++) {
                case U'-':
                    return khToken_fromOperator(khOperatorToken_DECREMENT, begin, *cursor);

                case U'=':
                    return khToken_fromOperator(khOperatorToken_ISUB, begin, *cursor);

                case U'>':
                    return khToken_fromDelimiter(khDelimiterToken_ARROW, begin, *cursor);

                default:
                    (*cursor)--;
                    return khToken_fromOperator(khOperatorToken_SUB, begin, *cursor);
            }

        case U'*':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IMUL, begin, *cursor);
            }
            else {
                return khToken_fromOperator(khOperatorToken_MUL, begin, *cursor);
            }

        case U'/':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IDIV, begin, *cursor);
            }
            else {
                return khToken_fromOperator(khOperatorToken_DIV, begin, *cursor);
            }

        case U'%':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IMOD, begin, *cursor);
            }
            else {
                return khToken_fromOperator(khOperatorToken_MOD, begin, *cursor);
            }

        case U'^':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IPOW, begin, *cursor);
            }
            else {
                return khToken_fromOperator(khOperatorToken_POW, begin, *cursor);
            }

        case U'@':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IDOT, begin, *cursor);
            }
            else {
                return khToken_fromOperator(khOperatorToken_DOT, begin, *cursor);
            }

        case U'=':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_EQUAL, begin, *cursor);
            }
            else {
                return khToken_fromOperator(khOperatorToken_ASSIGN, begin, *cursor);
            }

        case U'!':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_NOT_EQUAL, begin, *cursor);
            }
            else {
                return khToken_fromDelimiter(khDelimiterToken_EXCLAMATION, begin, *cursor);
            }

        case U'<':
            switch (*(*cursor)++) {
                case U'=':
                    return khToken_fromOperator(khOperatorToken_ELESS, begin, *cursor);

                case U'<':
                    if (**cursor == U'=') {
                        (*cursor)++;
                        return khToken_fromOperator(khOperatorToken_IBIT_LSHIFT, begin, *cursor);
                    }
                    else {
                        return khToken_fromOperator(khOperatorToken_BIT_LSHIFT, begin, *cursor);
                    }

                default:
                    (*cursor)--;
                    return khToken_fromOperator(khOperatorToken_LESS, begin, *cursor);
            }

        case U'>':
            switch (*(*cursor)++) {
                case U'=':
                    return khToken_fromOperator(khOperatorToken_EMORE, begin, *cursor);

                case U'<':
                    if (**cursor == U'=') {
                        (*cursor)++;
                        return khToken_fromOperator(khOperatorToken_IBIT_RSHIFT, begin, *cursor);
                    }
                    else {
                        return khToken_fromOperator(khOperatorToken_BIT_RSHIFT, begin, *cursor);
                    }

                default:
                    (*cursor)--;
                    return khToken_fromOperator(khOperatorToken_MORE, begin, *cursor);
            }

        case U'~':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IBIT_XOR, begin, *cursor);
            }
            else {
                // It's also khOperatorToken_BIT_XOR
                return khToken_fromOperator(khOperatorToken_BIT_OR, begin, *cursor);
            }

        case U'&':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IBIT_AND, begin, *cursor);
            }
            else {
                return khToken_fromOperator(khOperatorToken_BIT_AND, begin, *cursor);
            }

        case U'|':
            if (**cursor == U'=') {
                (*cursor)++;
                return khToken_fromOperator(khOperatorToken_IBIT_OR, begin, *cursor);
            }
            else {
                return khToken_fromOperator(khOperatorToken_BIT_OR, begin, *cursor);
            }

        // Null-terminator, string end
        case U'\0':
            (*cursor)--;
            return khToken_fromEof(begin, *cursor);

        default:
            raiseError(*cursor - 1, U"unknown character");
            return khToken_fromInvalid(begin, *cursor);
    }
#undef CASE_DELIMITER
}

char32_t kh_lexChar(char32_t** cursor, bool with_quotes, bool is_byte) {
    char32_t chr = 0;

    if (with_quotes) {
        if (**cursor == U'\'') {
            (*cursor)++;
        }
        else {
            raiseError(*cursor, U"expecting a single quote opening for a character");
        }
    }

    // Handle escapes
    if (**cursor == U'\\') {
        (*cursor)++;

        switch (*(*cursor)++) {
            // Handle many single character escapes
            case U'0':
                chr = U'\0';
                break;
            case U'n':
                chr = U'\n';
                break;
            case U'r':
                chr = U'\r';
                break;
            case U't':
                chr = U'\t';
                break;
            case U'v':
                chr = U'\v';
                break;
            case U'b':
                chr = U'\b';
                break;
            case U'a':
                chr = U'\a';
                break;
            case U'f':
                chr = U'\f';
                break;
            case U'\\':
                chr = U'\\';
                break;
            case U'\'':
                chr = U'\'';
                break;
            case U'"':
                chr = U'"';
                break;

            // \xAA
            case U'x': {
                char32_t* origin = *cursor;

                chr = kh_lexInt(cursor, 16, 2, NULL);
                if (*cursor != origin + 2) {
                    raiseError(*cursor,
                               U"expecting 2 hexadecimal digits for 1 byte character, from 0 to 9 or "
                               U"A to F");
                }

                break;
            }

            // \uAABB
            case U'u': {
                if (is_byte) {
                    raiseError(
                        *cursor - 1,
                        U"only allowing one byte characters, 2 byte unicode escapes are not allowed");
                    break;
                }

                char32_t* origin = *cursor;

                chr = kh_lexInt(cursor, 16, 4, NULL);
                if (*cursor != origin + 4) {
                    raiseError(
                        *cursor,
                        U"expecting 4 hexadecimal digits for 2 byte unicode character, from 0 to 9 or "
                        U"A to F");
                }

                break;
            }

            // \UAABBCCDD
            case U'U': {
                if (is_byte) {
                    raiseError(
                        *cursor - 1,
                        U"only allowing one byte characters, 4 byte unicode escapes are not allowed");
                    break;
                }

                char32_t* origin = *cursor;

                chr = kh_lexInt(cursor, 16, 8, NULL);
                if (*cursor != origin + 8) {
                    raiseError(
                        *cursor,
                        U"expecting 8 hexadecimal digits for 4 byte unicode character, from 0 to 9 or "
                        U"A to F");
                }

                break;
            }

            // Unexpected null-terminator
            case U'\0':
                (*cursor)--;
                raiseError(*cursor, U"expecting a backslash escape character, met with a dead end");
                return chr;

            // Unrecognized escape character
            default:
                raiseError(*cursor - 1, U"unknown backslash escape character");
                break;
        }
    }
    else {
        chr = **cursor;

        switch (**cursor) {
            // Encourage users to use U'\'' instead
            case U'\'':
                if (with_quotes) {
                    raiseError(*cursor, U"a character cannot be closed empty, did you mean U'\\''");
                }
                break;

            // Encourage users to use U'\n' instead
            case U'\n':
                raiseError(*cursor, U"a newline instead of an inline character, did you mean U'\\n'");
                break;

            // Unexpected null-terminator
            case U'\0':
                raiseError(*cursor, U"expecting a character, met with a dead end");
                return chr;

            default:
                if (is_byte && chr > 255) {
                    raiseError(*cursor,
                               U"only allowing one byte characters, unicode character is forbidden");
                }
                break;
        }

        (*cursor)++;
    }

    if (with_quotes) {
        if (**cursor == U'\'') {
            (*cursor)++;
        }
        else {
            raiseError(*cursor, U"expecting a single quote closing of the character");
        }
    }

    return chr;
}

khstring kh_lexString(char32_t** cursor, bool is_buffer) {
    khstring string = khstring_new(U"");
    bool multiline = false;

    if (**cursor == U'"') {
        (*cursor)++;

        // For multiline strings, uses triple double quotes
        if ((*cursor)[0] == U'"' && (*cursor)[1] == U'"') {
            *cursor += 2;
            multiline = true;
        }
    }
    else {
        raiseError(*cursor, U"expecting a double quote for a string");
    }

    while (true) {
        switch (**cursor) {
            // End string
            case U'"':
                if (multiline) {
                    if ((*cursor)[1] == U'"' && (*cursor)[2] == U'"') {
                        *cursor += 3;
                        return string;
                    }
                    else {
                        (*cursor)++;
                        khstring_append(&string, U'"');
                    }
                }
                else {
                    (*cursor)++;
                    return string;
                }
                break;

            // Explicitly handle U'\n', separate from kh_lexChar
            case U'\n':
                (*cursor)++;
                if (multiline) {
                    khstring_append(&string, U'\n');
                }
                else {
                    raiseError(*cursor - 1,
                               U"a newline instead of an inline character, use U'\\n' or a multiline "
                               U"string instead");
                }
                break;

            // Unexpected null-terminator
            case U'\0':
                raiseError(*cursor, U"expecting a character, met with a dead end");
                return string;

            // Use kh_lexChar for other character encounters
            default:
                khstring_append(&string, kh_lexChar(cursor, false, is_buffer));
                break;
        }
    }

    return string;
}

uint64_t kh_lexInt(char32_t** cursor, uint8_t base, size_t max_length, bool* had_overflowed) {
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

double kh_lexFloat(char32_t** cursor, uint8_t base) {
    double result = 0;

    // The same implementation of kh_lexInt is used here. The reason of not using kh_lexInt directly
    // is to avoid any integer overflows
    while (digitOf(**cursor) < base) {
        result *= base;
        result += digitOf(**cursor);
        (*cursor)++;
    }

    // Where the main show begins
    if (**cursor == U'.') {
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
        case U'e':
        case U'E':
            (*cursor)++;

            // Negative exponent
            if (**cursor == U'-') {
                (*cursor)++;

                result *= pow(10, -kh_lexInt(cursor, 10, -1, &had_overflowed));
                if (had_overflowed) {
                    result = 0.0;
                }
            }
            else {
                if (**cursor == U'+') {
                    (*cursor)++;
                }

                result *= pow(10, kh_lexInt(cursor, 10, -1, &had_overflowed));
                if (had_overflowed) {
                    result = INFINITY;
                }
            }

            break;

        // 2p3 -> 2 * 2^3 -> 16
        case U'p':
        case U'P':
            (*cursor)++;

            // Negative exponent
            if (**cursor == U'-') {
                (*cursor)++;

                result *= pow(2, -kh_lexInt(cursor, 10, -1, &had_overflowed));
                if (had_overflowed) {
                    result = 0.0;
                }
            }
            else {
                if (**cursor == U'+') {
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
