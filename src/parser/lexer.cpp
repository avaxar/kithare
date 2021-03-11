/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/parser/lexer.cpp
* Defines include/parser/lexer.hpp and declare+define several internal helper stuff.
*/

#include <cwctype>
#include <functional>

#include "utility/string.hpp"

#include "parser/lexer.hpp"

/* Helper to raise error at a file */
#define KH_RAISE_ERROR(msg, n) throw kh::LexException(msg, line_n, char_line + n, i + n, n >= 0 ? chAt(i + n) : ' ')

#define INC_LINE       \
    do {               \
        line_n++;      \
        char_line = 0; \
    } while (false)

#define INC_I(_n)        \
    do {                 \
        i += _n;         \
        char_line += _n; \
    } while (false)

#define _TOKEN_PLACE(ttype) tokens.emplace_back(ttype, value, char_line, line_n)                                                   \

#define HANDLE_HEX_INTO_HEXSTR(_start, _len)                    \
    std::string hex_str;                                        \
    for (size_t j = _start; j < _start + _len; j++) {           \
        if (kh::isHex(chAt(i + j)))                             \
            hex_str += (char)chAt(i + j);                       \
        else                                                    \
            KH_RAISE_ERROR(U"Expected a hexadecimal digit", j); \
    }                                                           \
    INC_I(_start + _len)

#define _PLACE_HEXSTR_AS_TYPE(_start, _len, _var, ttype) \
    if (chAt(i + _start + _len) == '\'') {               \
        _var = std::stoul(hex_str, nullptr, 16);         \
        _TOKEN_PLACE(ttype);                             \
    }                                                    \
    else                                                 \
        KH_RAISE_ERROR(U"Expected a closing single quote", 0)

#define PLACE_HEXSTR_AS_INT(_start, _len) \
    _PLACE_HEXSTR_AS_TYPE(_start, _len, value.integer, kh::TokenType::INTEGER)

#define PLACE_HEXSTR_AS_CHAR(_start, _len) \
    _PLACE_HEXSTR_AS_TYPE(_start, _len, value.character, kh::TokenType::CHARACTER)

#define _HANDLE_ESCAPE(chr, echr, _val, _len, code) \
    case chr:                                       \
        _val = echr;                                \
        code;                                       \
        INC_I(_len);                                \
        break;

#define _HANDLE_ESCAPE_1(chr, echr, _val, _ttype, _len)               \
    _HANDLE_ESCAPE(chr, echr, _val, _len,                             \
        if (chAt(i + _len) != '\'')                                   \
            KH_RAISE_ERROR(U"Expected a closing single quote", _len); \
        _TOKEN_PLACE(_ttype);                                         \
    )

#define HANDLE_ESCAPES_1(_val, _valc, _len)           \
    _HANDLE_ESCAPE_1('0', '\0', _val, _valc, _len)    \
    _HANDLE_ESCAPE_1('n', '\n', _val, _valc, _len)    \
    _HANDLE_ESCAPE_1('r', '\r', _val, _valc, _len)    \
    _HANDLE_ESCAPE_1('t', '\t', _val, _valc, _len)    \
    _HANDLE_ESCAPE_1('v', '\v', _val, _valc, _len)    \
    _HANDLE_ESCAPE_1('\\', '\\', _val, _valc, _len)   \
    _HANDLE_ESCAPE_1('"', '\"', _val, _valc, _len)    \
    _HANDLE_ESCAPE_1('\'', '\'', _val, _valc, _len)   \
    default:                                                   \
        KH_RAISE_ERROR(U"Unknown escape character", _len - 1); \
    }

#define HANDLE_ESCAPES_2(code)                 \
    _HANDLE_ESCAPE('0', '\0', value, 1, code)  \
    _HANDLE_ESCAPE('n', '\n', value, 1, code)  \
    _HANDLE_ESCAPE('r', '\r', value, 1, code)  \
    _HANDLE_ESCAPE('t', '\t', value, 1, code)  \
    _HANDLE_ESCAPE('v', '\v', value, 1, code)  \
    _HANDLE_ESCAPE('\\', '\\', value, 1, code) \
    _HANDLE_ESCAPE('"', '\"', value, 1, code)  \
    _HANDLE_ESCAPE('\'', '\'', value, 1, code) \
    default:                                   \
        KH_RAISE_ERROR(U"Unknown escape character", 1); \
    }

namespace kh {
    enum class TokenizeState {
        NONE, IDENTIFIER,
        INTEGER, FLOATING, HEX, OCTAL, BIN,
        IN_BUF, IN_STR,
        IN_INLINE_COMMENT, IN_MULTIPLE_LINE_COMMENT
    };

    inline bool isDec(const char32_t chr) {
        return U'0' <= chr && chr <= U'9';
    }

    inline bool isBin(const char32_t chr) {
        return chr == U'0' || chr == U'1';
    }

    inline bool isOct(const char32_t chr) {
        return U'0' <= chr && chr <= U'7';
    }

    inline bool isHex(const char32_t chr) {
        return 
            (U'0' <= chr && chr <= U'9') ||
            (U'a' <= chr && chr <= U'f') ||
            (U'A' <= chr && chr <= U'F');
    }
}

/// <summary>
/// Lexicate/tokenize a source string into a list/std::vector of tokens.
/// </summary>
/// <param name="source">Input source string</param>
/// <returns></returns>
std::vector<kh::Token> kh::lex(const std::u32string& source) {
    std::vector<kh::Token> tokens;
    kh::TokenizeState state = kh::TokenizeState::NONE;

    std::u32string temp_str;
    std::string temp_buf;

    size_t line_n = 1, char_line = 1;

    /* Lambda function which accesses the string, and throws an error directly to the console if it had passed the length */
    const std::function<const char32_t(const size_t)> chAt = [&](const size_t index) -> const char32_t {
        if (index < source.size())
            return source[index];
        else if (index == source.size())
            return '\n';
        else {
            size_t i = index;
            KH_RAISE_ERROR(U"Unexpected EOF", -1);
        }
    };

    for (size_t i = 0; i <= source.size(); (i++, char_line++)) {
        switch (state) {
        case kh::TokenizeState::NONE:
            temp_str.clear();
            temp_buf.clear();

            /* Skips whitespace (An exception for newlines which reset the character count and increments the line count) */
            if (chAt(i) == '\n')
                INC_LINE;

            else if (std::iswspace(chAt(i)) > 0)
                continue;

            /* Possible identifier start */
            else if (std::iswalpha(chAt(i)) > 0 || chAt(i) == '_') {
                /* Possible start of a byte-string/byte-string constant */
                if (chAt(i) == 'b' || chAt(i) == 'B') {
                    /* Possible byte-char */
                    if (chAt(i + 1) == '\'') {
                        /* Possible char-escape */
                        if (chAt(i + 2) == '\\') {
                            kh::TokenValue value;
                            switch (chAt(i + 3)) {
                                /* Hex character escape */
                            case 'x':
                            case 'X':
                            {
                                HANDLE_HEX_INTO_HEXSTR(4, 2);
                                PLACE_HEXSTR_AS_INT(4, 2);
                            } break;

                            HANDLE_ESCAPES_1(value.integer, kh::TokenType::INTEGER, 4)
                        }
                        /* Plain byte-char without character escapes */
                        else {
                            /* Some edge cases */
                            if (chAt(i + 2) == '\'')
                                KH_RAISE_ERROR(U"No character inserted", 2);
                            else if (chAt(i + 1) == '\n')
                                KH_RAISE_ERROR(U"New line before byte character closing", 2);
                            else {
                                if (chAt(i + 3) == '\'') {
                                    if (chAt(i + 2) > 255)
                                        KH_RAISE_ERROR(U"A non-byte sized character", 2);

                                    kh::TokenValue value;
                                    value.integer = chAt(i + 2);
                                    tokens.emplace_back(
                                        kh::TokenType::INTEGER,
                                        value,
                                        char_line,
                                        line_n
                                    );

                                    INC_I(3);
                                }
                                else
                                    KH_RAISE_ERROR(U"Expected a closing single quote", 3);
                            }
                        }
                        continue;
                    }

                    /* Possible byte-string/buffer */
                    else if (chAt(i + 1) == '"') {
                        state = kh::TokenizeState::IN_BUF;
                        INC_I(1);
                        continue;
                    }
                }

                /* If it's not a byte-string/byte-char, it's just a normal identifier */
                state = kh::TokenizeState::IDENTIFIER;
                temp_str = chAt(i);
            }

            /* Starts with a decimal value, possible number constant */
            else if (kh::isDec(chAt(i))) {
                /* Likely to use other number base */
                if (chAt(i) == '0') {
                    /* Handles hex numbers */
                    if (chAt(i + 1) == 'x' || chAt(i + 1) == 'X') {
                        state = kh::TokenizeState::HEX;

                        if (!kh::isHex(chAt(i + 2)))
                            KH_RAISE_ERROR(U"Was expecting a hexadecimal digit", 2);

                        INC_I(1);
                        continue;
                    }

                    /* Handles octal numbers */
                    else if (chAt(i + 1) == 'o' || chAt(i + 1) == 'O') {
                        state = kh::TokenizeState::OCTAL;

                        if (!kh::isOct(chAt(i + 2)))
                            KH_RAISE_ERROR(U"Was expecting an octal digit at", 2);

                        INC_I(1);
                        continue;
                    }

                    /* Handles binary numbers */
                    else if (chAt(i + 1) == 'b' || chAt(i + 1) == 'B') {
                        state = kh::TokenizeState::BIN;

                        if (!kh::isBin(chAt(i + 2)))
                            KH_RAISE_ERROR(U"Was expecting a binary digit at", 2);

                        INC_I(1);
                        continue;
                    }
                }

                state = kh::TokenizeState::INTEGER;
                temp_str = chAt(i);
            }

            else
                switch (chAt(i)) {
                    /* Possible character */
                case '\'': {
                    kh::TokenValue value;
                    /* Possible char escape */
                    if (chAt(i + 1) == '\\') {
                        switch (chAt(i + 2)) {
                            /* Hex escapes */
                        case 'x': case 'X': {
                            HANDLE_HEX_INTO_HEXSTR(3, 2);
                            PLACE_HEXSTR_AS_CHAR(3, 2);
                        } break;

                        /* 2 bytes unicode escape */
                        case 'u': {
                            HANDLE_HEX_INTO_HEXSTR(3, 4);
                            PLACE_HEXSTR_AS_CHAR(3, 4);
                        } break;

                        /* 4 bytes unicode escape */
                        case 'U': {
                            HANDLE_HEX_INTO_HEXSTR(3, 8);
                            PLACE_HEXSTR_AS_CHAR(3, 8);
                        } break;

                        HANDLE_ESCAPES_1(value.character, kh::TokenType::CHARACTER, 3)
                    }
                    else {
                        /* Edge cases */
                        if (chAt(i + 1) == '\'')
                            KH_RAISE_ERROR(U"No character inserted", 1);
                        else if (chAt(i + 1) == '\n')
                            KH_RAISE_ERROR(U"New line before character closing", 1);
                        else {
                            if (chAt(i + 2) == '\'') {
                                kh::TokenValue value;
                                value.character = chAt(i + 1);
                                tokens.emplace_back(
                                    kh::TokenType::CHARACTER,
                                    value,
                                    char_line,
                                    line_n
                                );

                                INC_I(2);
                            }
                            else
                                KH_RAISE_ERROR(U"Expected a closing single quote", 2);
                        }
                    }
                    continue;
                } break;

                    /* Possible string */
                case '"':
                    state = kh::TokenizeState::IN_STR;
                    break;

                    /* Possible operator, also checks for overlapping/multi-character operators */
                case '+': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::ADD;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::IADD;
                        INC_I(1);
                    }

                    tokens.emplace_back(
                        kh::TokenType::OPERATOR,
                        value,
                        char_line,
                        line_n
                    );
                } break;

                case '-': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::SUB;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::ISUB;
                        INC_I(1);
                    }

                    tokens.emplace_back(
                        kh::TokenType::OPERATOR,
                        value,
                        char_line,
                        line_n
                    );
                } break;

                case '*': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::MUL;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::IMUL;
                        INC_I(1);
                    }
                    else if (chAt(i + 1) == '/')
                        KH_RAISE_ERROR(U"Unexpected comment close", 0);

                    tokens.emplace_back(
                        kh::TokenType::OPERATOR,
                        value,
                        char_line,
                        line_n
                    );
                } break;

                case '/': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::DIV;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::IDIV;
                        INC_I(1);
                    }
                    else if (chAt(i + 1) == '/') {
                        state = kh::TokenizeState::IN_INLINE_COMMENT;
                        INC_I(1);
                        continue;
                    }
                    else if (chAt(i + 1) == '*') {
                        state = kh::TokenizeState::IN_MULTIPLE_LINE_COMMENT;
                        INC_I(1);
                        continue;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '%': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::MOD;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::IMOD;
                        INC_I(1);
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '^': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::POW;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::IPOW;
                        INC_I(1);
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '=': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::ASSIGN;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::EQUAL;
                        INC_I(1);
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '<': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::LESS;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::LESS_EQUAL;
                        INC_I(1);
                    }
                    else if (chAt(i + 1) == '<') {
                        value.operator_type = kh::Operator::BIT_LSHIFT;
                        INC_I(1);
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '>': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::MORE;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::MORE_EQUAL;
                        INC_I(1);
                    }
                    else if (chAt(i + 1) == '>') {
                        value.operator_type = kh::Operator::BIT_RSHIFT;
                        INC_I(1);
                    }
                    else if (chAt(i + 1) == ']') {
                        value.symbol_type = kh::Symbol::TEMPLATE_CLOSE;
                        INC_I(1);

                        tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                        continue;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '!': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::NOT;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::NOT_EQUAL;
                        INC_I(1);
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '&': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::BIT_AND;

                    if (chAt(i + 1) == '&') {
                        value.operator_type = kh::Operator::AND;
                        INC_I(1);
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '|': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::BIT_OR;

                    if (chAt(i + 1) == '|') {
                        value.operator_type = kh::Operator::OR;
                        INC_I(1);
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '~': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::BIT_NOT;
                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '#': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::SIZEOF;
                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '@': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::ADDRESS;
                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case ';': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::SEMICOLON;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case '.': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::DOT;

                    if (kh::isDec(chAt(i + 1))) {
                        state = kh::TokenizeState::FLOATING;
                        temp_str = U"0.";
                        continue;
                    }

                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case ',': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::COMMA;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case '?': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::QUESTION;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case ':': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::COLON;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case '$': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::DOLLAR;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case '(': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::PARENTHESES_OPEN;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case ')': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::PARENTHESES_CLOSE;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case '{': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::CURLY_OPEN;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case '}': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::CURLY_CLOSE;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case '[': {
                    kh::TokenValue value;

                    if (chAt(i + 1) == '<') {
                        value.symbol_type = kh::Symbol::TEMPLATE_OPEN;
                        INC_I(1);

                        tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                        continue;
                    }

                    value.symbol_type = kh::Symbol::SQUARE_OPEN;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case ']': {
                    kh::TokenValue value;

                    value.symbol_type = kh::Symbol::SQUARE_CLOSE;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                default:
                    KH_RAISE_ERROR(U"Unrecognised character", 0);
                }
            continue;

            /* Follows the identifier's characters */
        case kh::TokenizeState::IDENTIFIER:
            /* Checks if it's still a valid identifier character */
            if (std::iswalpha(chAt(i)) > 0 || kh::isDec(chAt(i)) || chAt(i) == '_')
                temp_str += chAt(i);
            else {
                /* If it's not, reset the state and appends the concatenated identifier characters as a token */
                kh::TokenValue value;
                value.identifier = temp_str;
                tokens.emplace_back(
                    kh::TokenType::IDENTIFIER,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
                INC_I(-1);
            }
            continue;

            /* Checks for an integer */
        case kh::TokenizeState::INTEGER:
            if (kh::isDec(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'u' || chAt(i) == 'U') {
                /* Is unsigned */
                kh::TokenValue value;
                value.uinteger = std::stoull(kh::encodeUtf8(temp_str));
                tokens.emplace_back(
                    kh::TokenType::UINTEGER,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                /* Is imaginary */
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::encodeUtf8(temp_str));
                tokens.emplace_back(
                    kh::TokenType::IMAGINARY,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == '.') {
                /* Checks it as a floating point */
                temp_str += chAt(i);
                state = kh::TokenizeState::FLOATING;
            }
            else {
                kh::TokenValue value;
                value.integer = std::stoll(kh::encodeUtf8(temp_str));
                tokens.emplace_back(
                    kh::TokenType::INTEGER,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
                INC_I(-1);
            }
            continue;

            /* Checks floating point numbers */
        case kh::TokenizeState::FLOATING:
            if (kh::isDec(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                /* Is imaginary */
                if (temp_str.back() == '.')
                    temp_str.pop_back();

                kh::TokenValue value;
                value.imaginary = std::stod(kh::encodeUtf8(temp_str));
                tokens.emplace_back(
                    kh::TokenType::IMAGINARY,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
            }
            else {
                /* An artifact from how integers were checked that was transferred as a floating point with an invalid character after . */
                if (temp_str.back() == '.')
                    temp_str.pop_back();

                kh::TokenValue value;
                value.floating = std::stod(kh::encodeUtf8(temp_str));
                tokens.emplace_back(
                    kh::TokenType::FLOATING,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
                INC_I(-1);
            }
            continue;

            /* Checks hex integers */
        case kh::TokenizeState::HEX:
            if (kh::isHex(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'u' || chAt(i) == 'U') {
                /* Is unsigned */
                kh::TokenValue value;
                value.uinteger = std::stoull(kh::encodeUtf8(temp_str), nullptr, 16);
                tokens.emplace_back(
                    kh::TokenType::UINTEGER,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                /* Is imaginary */
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::encodeUtf8(temp_str), nullptr, 16);
                tokens.emplace_back(
                    kh::TokenType::IMAGINARY,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else {
                kh::TokenValue value;
                value.integer = std::stoull(kh::encodeUtf8(temp_str), nullptr, 16);
                tokens.emplace_back(
                    kh::TokenType::INTEGER,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
                INC_I(-1);
            }
            continue;

            /* Checks octal integers */
        case kh::TokenizeState::OCTAL:
            if (kh::isOct(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'u' || chAt(i) == 'U') {
                /* Is unsigned */
                kh::TokenValue value;
                value.uinteger = std::stoull(kh::encodeUtf8(temp_str), nullptr, 8);
                tokens.emplace_back(
                    kh::TokenType::UINTEGER,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                /* Is imaginary */
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::encodeUtf8(temp_str), nullptr, 8);
                tokens.emplace_back(
                    kh::TokenType::IMAGINARY,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else {
                kh::TokenValue value;
                value.integer = std::stoull(kh::encodeUtf8(temp_str), nullptr, 8);
                tokens.emplace_back(
                    kh::TokenType::INTEGER,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
                INC_I(-1);
            }
            continue;

            /* Checks binary integers */
        case kh::TokenizeState::BIN:
            if (kh::isBin(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'u' || chAt(i) == 'U') {
                /* Is unsigned */
                kh::TokenValue value;
                value.uinteger = std::stoull(kh::encodeUtf8(temp_str), nullptr, 2);
                tokens.emplace_back(
                    kh::TokenType::UINTEGER,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                /* Is imaginary */
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::encodeUtf8(temp_str), nullptr, 2);
                tokens.emplace_back(
                    kh::TokenType::IMAGINARY,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else {
                kh::TokenValue value;
                value.integer = std::stoull(kh::encodeUtf8(temp_str), nullptr, 2);
                tokens.emplace_back(
                    kh::TokenType::INTEGER,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
                INC_I(-1);
            }
            continue;

            /* Checks for a byte-string/buffer */
        case kh::TokenizeState::IN_BUF:
            if (chAt(i) == '"') {
                /* End buffer */

                kh::TokenValue value;
                value.buffer = temp_buf;
                tokens.emplace_back(
                    kh::TokenType::BUFFER,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == '\n') {
                KH_RAISE_ERROR(U"Unclosed buffer string before new line", 0);
            }
            else {
                /* Possible character escape */
                if (chAt(i) == '\\') {
                    uint8_t value;
                    switch (chAt(i + 1)) {
                        /* Hex character escape */
                    case 'x':
                    case 'X': {
                        HANDLE_HEX_INTO_HEXSTR(2, 2);
                        temp_buf.push_back(std::stoul(hex_str, nullptr, 16));
                        INC_I(-1);
                    } break;

                    case '\n':
                        INC_I(1);
                        INC_LINE;
                        break;

                    /* Other character escapes */
                    HANDLE_ESCAPES_2(temp_buf.push_back(value))
                }
                else {
                    if (chAt(i) > 255)
                        KH_RAISE_ERROR(U"A non-byte sized character", 0);

                    temp_buf.push_back(chAt(i));
                }
            }
            continue;

            /* Checks for a string */
        case kh::TokenizeState::IN_STR:
            if (chAt(i) == '"') {
                /* End string */

                kh::TokenValue value;
                value.string = temp_str;
                tokens.emplace_back(kh::TokenType::STRING, value, char_line, line_n);

                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == '\n') {
                KH_RAISE_ERROR(U"Unclosed string before new line", 0);
            }
            else {
                /* Possible character escape */
                if (chAt(i) == '\\') {
                    uint8_t value;
                    switch (chAt(i + 1)) {
                        /* Hex character escape */
                    case 'x': case 'X': {
                        HANDLE_HEX_INTO_HEXSTR(2, 2);
                        temp_str += std::stoul(hex_str, nullptr, 16);
                        INC_I(-1);
                    } break;

                    /* 2 bytes unicode escape */
                    case 'u': {
                        HANDLE_HEX_INTO_HEXSTR(2, 4);
                        temp_str += std::stoul(hex_str, nullptr, 16);
                        INC_I(-1);
                    } break;

                    /* 4 bytes unicode escape */
                    case 'U': {
                        HANDLE_HEX_INTO_HEXSTR(2, 8);
                        temp_str += std::stoul(hex_str, nullptr, 16);
                        INC_I(-1);
                    } break;

                    case '\n':
                        INC_I(1);
                        INC_LINE;
                        break;

                    /* Other character escapes */
                    HANDLE_ESCAPES_2(temp_str += value)
                }
                else
                    temp_str += chAt(i);
            }
            continue;

            /* Passing through until the inline comment is done */
        case kh::TokenizeState::IN_INLINE_COMMENT:
            if (chAt(i) == '\n') {
                /* End inline comment */
                state = kh::TokenizeState::NONE;
                INC_LINE;
            }
            continue;

            /* Passing through until the multiple line comment is closed */
        case kh::TokenizeState::IN_MULTIPLE_LINE_COMMENT:
            if (chAt(i) == '*' && chAt(i + 1) == '/') {
                /* Close comment */
                state = kh::TokenizeState::NONE;
                INC_I(1);
            }
            else if (chAt(i) == '\n') {
                /* Still not forgetting the line count */
                INC_LINE;
            }
            continue;

        default:
            /* How did we get here? */
            KH_RAISE_ERROR(U"Got an unknown tokenize state", 0);
        }
    }
    return tokens;
}
