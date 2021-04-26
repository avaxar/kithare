/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * src/parser/lexer.cpp
 * Defines include/parser/lexer.hpp and declare+define several internal helper stuff.
 */

#include "parser/lexer.hpp"

/* Helper to raise error at a file */
#define KH_RAISE_ERROR(msg, n) throw kh::LexException(msg, i + n)

/* Use this macro to export a variable hex_str from a given start and len relative
 * to the file index */
#define HANDLE_HEX_INTO_HEXSTR(_start, _len)                    \
    std::string hex_str;                                        \
    for (size_t j = _start; j < _start + _len; j++) {           \
        if (kh::isHex(chAt(i + j)))                             \
            hex_str += (char)chAt(i + j);                       \
        else                                                    \
            KH_RAISE_ERROR(U"Expected a hexadecimal digit", j); \
    }                                                           \
    i += _start + _len

/* Helper macro */
#define _PLACE_HEXSTR_AS_TYPE(_var, ttype)               \
    if (chAt(i) == '\'') {                               \
        _var = std::stoul(hex_str, nullptr, 16);         \
        tokens.emplace_back(start, i + 1, ttype, value); \
    }                                                    \
    else                                                 \
        KH_RAISE_ERROR(U"Expected a closing single quote", 0)

/* Place a hex_str as an integer into tokens stack */
#define PLACE_HEXSTR_AS_INT() _PLACE_HEXSTR_AS_TYPE(value.integer, kh::TokenType::INTEGER)

/* Place a hex_str as a character into tokens stack */
#define PLACE_HEXSTR_AS_CHAR() _PLACE_HEXSTR_AS_TYPE(value.character, kh::TokenType::CHARACTER)

/* Helper macro */
#define _HANDLE_ESCAPE(chr, echr, _val, _len, code) \
    case chr:                                       \
        _val = echr;                                \
        code;                                       \
        i += _len;                                  \
        break;

/* Helper macro */
#define _HANDLE_ESCAPE_1(chr, echr, _val, _ttype, _len)                          \
    _HANDLE_ESCAPE(chr, echr, _val, _len,                                        \
                   if (chAt(i + _len) != '\'')                                   \
                       KH_RAISE_ERROR(U"Expected a closing single quote", _len); \
                   tokens.emplace_back(start, i + _len + 1, _ttype, value);)

/* Use this to handle string escapes from a switch statement. This is used to handle
 * escapes into byte/unicode characters */
#define HANDLE_ESCAPES_1(_val, _valc, _len)         \
    _HANDLE_ESCAPE_1('0', '\0', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('n', '\n', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('r', '\r', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('t', '\t', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('v', '\v', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('b', '\b', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('f', '\f', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('\\', '\\', _val, _valc, _len) \
    _HANDLE_ESCAPE_1('"', '\"', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('\'', '\'', _val, _valc, _len) \
    default:                                        \
        KH_RAISE_ERROR(U"Unknown escape character", _len - 1);

/* Use this to handle string escapes from a switch statement. This is used to handle
 * escapes into byte/unicode strings */
#define HANDLE_ESCAPES_2(code)                 \
    _HANDLE_ESCAPE('0', '\0', value, 1, code)  \
    _HANDLE_ESCAPE('n', '\n', value, 1, code)  \
    _HANDLE_ESCAPE('r', '\r', value, 1, code)  \
    _HANDLE_ESCAPE('t', '\t', value, 1, code)  \
    _HANDLE_ESCAPE('v', '\v', value, 1, code)  \
    _HANDLE_ESCAPE('b', '\b', value, 1, code)  \
    _HANDLE_ESCAPE('f', '\f', value, 1, code)  \
    _HANDLE_ESCAPE('\\', '\\', value, 1, code) \
    _HANDLE_ESCAPE('"', '\"', value, 1, code)  \
    _HANDLE_ESCAPE('\'', '\'', value, 1, code) \
    default:                                   \
        KH_RAISE_ERROR(U"Unknown escape character", 1);

/* Handle a simple symbol from a switch block */
#define HANDLE_SIMPLE_SYMBOL(sym, name)                                \
    case sym: {                                                        \
        kh::TokenValue val;                                            \
        val.symbol_type = name;                                        \
        tokens.emplace_back(start, i + 1, kh::TokenType::SYMBOL, val); \
    } break;

/* Handle a simple operator from a switch block */
#define HANDLE_SIMPLE_OP(sym, name)                                      \
    case sym: {                                                          \
        kh::TokenValue val;                                              \
        val.operator_type = name;                                        \
        tokens.emplace_back(start, i + 1, kh::TokenType::OPERATOR, val); \
    } break;

/* Handle a combination of two operators as a single operator from a switch block */
#define HANDLE_OP_COMBO(sym, name, sym2, name2)                          \
    case sym: {                                                          \
        kh::TokenValue val;                                              \
        val.operator_type = name;                                        \
        if (chAt(i + 1) == sym2) {                                       \
            val.operator_type = name2;                                   \
            i++;                                                         \
        }                                                                \
        tokens.emplace_back(start, i + 1, kh::TokenType::OPERATOR, val); \
    } break;


namespace kh {
    enum class TokenizeState {
        NONE,
        IDENTIFIER,
        INTEGER,
        FLOATING,
        HEX,
        OCTAL,
        BIN,
        IN_BUF,
        IN_MULTILINE_BUF,
        IN_STR,
        IN_MULTILINE_STR,
        IN_INLINE_COMMENT,
        IN_MULTIPLE_LINE_COMMENT
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
        return (U'0' <= chr && chr <= U'9') || (U'a' <= chr && chr <= U'f') ||
               (U'A' <= chr && chr <= U'F');
    }
}

std::vector<kh::Token> kh::lex(const std::u32string& source, const bool lex_comments) {
    std::vector<kh::Token> tokens;
    kh::TokenizeState state = kh::TokenizeState::NONE;

    size_t start = 0;
    std::u32string temp_str;
    std::string temp_buf;

    /* Lambda function which accesses the string, and throws an error directly to the console if it
     * had passed the length */
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

    for (size_t i = 0; i <= source.size(); i++) {
        switch (state) {
            case kh::TokenizeState::NONE:
                start = i;
                temp_str.clear();
                temp_buf.clear();

                /* Skips whitespace and newlines */
                if (chAt(i) == '\n' || std::iswspace(chAt(i)) > 0)
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
                                    case 'X': {
                                        HANDLE_HEX_INTO_HEXSTR(4, 2);
                                        PLACE_HEXSTR_AS_INT();
                                    } break;

                                        HANDLE_ESCAPES_1(value.integer, kh::TokenType::INTEGER, 4)
                                }
                            }
                            else if (chAt(i + 2) == '\'') {
                                /* No Character inserted, like b''. Treat it like a 0 */
                                kh::TokenValue value;
                                value.integer = 0;
                                tokens.emplace_back(start, i + 3, kh::TokenType::INTEGER, value);

                                i += 2;
                            }
                            else if (chAt(i + 2) == '\n')
                                KH_RAISE_ERROR(U"New line before byte character closing", 2);

                            /* Plain byte-char without character escapes */
                            else if (chAt(i + 3) == '\'') {
                                if (chAt(i + 2) > 255)
                                    KH_RAISE_ERROR(U"A non-byte sized character", 2);

                                kh::TokenValue value;
                                value.integer = chAt(i + 2);
                                tokens.emplace_back(start, i + 4, kh::TokenType::INTEGER, value);

                                i += 3;
                            }
                            else
                                KH_RAISE_ERROR(U"Expected a closing single quote", 3);
                            continue;
                        }
                        /* Possible byte-string/buffer */
                        else if (chAt(i + 1) == '"') {
                            if (chAt(i + 2) == '"' && chAt(i + 3) == '"') {
                                state = kh::TokenizeState::IN_MULTILINE_BUF;
                                i += 3;
                            }
                            else {
                                state = kh::TokenizeState::IN_BUF;
                                i += 1;
                            }
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
                        switch (chAt(i + 1)) {
                            case 'x':
                            case 'X': {
                                state = kh::TokenizeState::HEX;
                                if (!kh::isHex(chAt(i + 2)))
                                    KH_RAISE_ERROR(U"Was expecting a hexadecimal digit", 2);

                                i++;
                                continue;
                            }

                            /* Handles octal numbers */
                            case 'o':
                            case 'O': {
                                state = kh::TokenizeState::OCTAL;
                                if (!kh::isOct(chAt(i + 2)))
                                    KH_RAISE_ERROR(U"Was expecting an octal digit at", 2);

                                i++;
                                continue;
                            }

                            /* Handles binary numbers */
                            case 'b':
                            case 'B': {
                                state = kh::TokenizeState::BIN;
                                if (!kh::isBin(chAt(i + 2)))
                                    KH_RAISE_ERROR(U"Was expecting a binary digit at", 2);

                                i++;
                                continue;
                            }
                        }
                    }

                    state = kh::TokenizeState::INTEGER;
                    temp_str = chAt(i);
                }

                else {
                    switch (chAt(i)) {
                        /* Possible character */
                        case '\'': {
                            kh::TokenValue value;
                            /* Possible char escape */
                            if (chAt(i + 1) == '\\') {
                                switch (chAt(i + 2)) {
                                    /* Hex escapes */
                                    case 'x':
                                    case 'X': {
                                        HANDLE_HEX_INTO_HEXSTR(3, 2);
                                        PLACE_HEXSTR_AS_CHAR();
                                    } break;

                                    /* 2 bytes unicode escape */
                                    case 'u': {
                                        HANDLE_HEX_INTO_HEXSTR(3, 4);
                                        PLACE_HEXSTR_AS_CHAR();
                                    } break;

                                    /* 4 bytes unicode escape */
                                    case 'U': {
                                        HANDLE_HEX_INTO_HEXSTR(3, 8);
                                        PLACE_HEXSTR_AS_CHAR();
                                    } break;

                                        HANDLE_ESCAPES_1(value.character, kh::TokenType::CHARACTER, 3);
                                }
                            }
                            else if (chAt(i + 1) == '\'') {
                                /* No Character inserted, like ''. Treat it like a \0 */
                                kh::TokenValue value;
                                value.character = 0;
                                tokens.emplace_back(start, i + 2, kh::TokenType::CHARACTER, value);

                                i += 1;
                            }
                            else if (chAt(i + 1) == '\n')
                                KH_RAISE_ERROR(U"New line before character closing", 1);
                            else if (chAt(i + 2) == '\'') {
                                kh::TokenValue value;
                                value.character = chAt(i + 1);
                                tokens.emplace_back(start, i + 3, kh::TokenType::CHARACTER, value);
                                i += 2;
                            }
                            else
                                KH_RAISE_ERROR(U"Expected a closing single quote", 2);
                            continue;
                        } break;

                        /* Possible string */
                        case '"':
                            if (chAt(i + 1) == '"' && chAt(i + 2) == '"') {
                                state = kh::TokenizeState::IN_MULTILINE_STR;
                                i += 2;
                            }
                            else
                                state = kh::TokenizeState::IN_STR;
                            break;

                            /* Operator handling */
                            HANDLE_OP_COMBO('+', kh::Operator::ADD, '=', kh::Operator::IADD);
                            HANDLE_OP_COMBO('-', kh::Operator::SUB, '=', kh::Operator::ISUB);
                            HANDLE_OP_COMBO('%', kh::Operator::MOD, '=', kh::Operator::IMOD);
                            HANDLE_OP_COMBO('^', kh::Operator::POW, '=', kh::Operator::IPOW);
                            HANDLE_OP_COMBO('=', kh::Operator::ASSIGN, '=', kh::Operator::EQUAL);
                            HANDLE_OP_COMBO('!', kh::Operator::NOT, '=', kh::Operator::NOT_EQUAL);
                            HANDLE_OP_COMBO('&', kh::Operator::BIT_AND, '&', kh::Operator::AND);
                            HANDLE_OP_COMBO('|', kh::Operator::BIT_OR, '|', kh::Operator::OR);
                            HANDLE_SIMPLE_OP('~', kh::Operator::BIT_NOT);
                            HANDLE_SIMPLE_OP('#', kh::Operator::SIZEOF);
                            HANDLE_SIMPLE_OP('@', kh::Operator::ADDRESS);

                            /* Symbol handling */
                            HANDLE_SIMPLE_SYMBOL(';', kh::Symbol::SEMICOLON);
                            HANDLE_SIMPLE_SYMBOL(',', kh::Symbol::COMMA);
                            HANDLE_SIMPLE_SYMBOL(':', kh::Symbol::COLON);
                            HANDLE_SIMPLE_SYMBOL('(', kh::Symbol::PARENTHESES_OPEN);
                            HANDLE_SIMPLE_SYMBOL(')', kh::Symbol::PARENTHESES_CLOSE);
                            HANDLE_SIMPLE_SYMBOL('{', kh::Symbol::CURLY_OPEN);
                            HANDLE_SIMPLE_SYMBOL('}', kh::Symbol::CURLY_CLOSE);
                            HANDLE_SIMPLE_SYMBOL('[', kh::Symbol::SQUARE_OPEN);
                            HANDLE_SIMPLE_SYMBOL(']', kh::Symbol::SQUARE_CLOSE);

                        /* Some operators and symbols have more complicated handling, and
                         * those are not macro-ised */
                        case '*': {
                            kh::TokenValue value;
                            value.operator_type = kh::Operator::MUL;

                            if (chAt(i + 1) == '=') {
                                value.operator_type = kh::Operator::IMUL;
                                i++;
                            }
                            else if (chAt(i + 1) == '/')
                                KH_RAISE_ERROR(U"Unexpected comment close", 0);

                            tokens.emplace_back(start, i + 1, kh::TokenType::OPERATOR, value);
                        } break;

                        case '/': {
                            kh::TokenValue value;
                            value.operator_type = kh::Operator::DIV;

                            if (chAt(i + 1) == '=') {
                                value.operator_type = kh::Operator::IDIV;
                                i++;
                            }
                            else if (chAt(i + 1) == '/') {
                                state = kh::TokenizeState::IN_INLINE_COMMENT;
                                i++;
                                continue;
                            }
                            else if (chAt(i + 1) == '*') {
                                state = kh::TokenizeState::IN_MULTIPLE_LINE_COMMENT;
                                i++;
                                continue;
                            }

                            tokens.emplace_back(start, i + 1, kh::TokenType::OPERATOR, value);
                        } break;

                        case '<': {
                            kh::TokenValue value;
                            value.operator_type = kh::Operator::LESS;

                            if (chAt(i + 1) == '=') {
                                value.operator_type = kh::Operator::LESS_EQUAL;
                                i++;
                            }
                            else if (chAt(i + 1) == '<') {
                                value.operator_type = kh::Operator::BIT_LSHIFT;
                                i++;
                            }

                            tokens.emplace_back(start, i + 1, kh::TokenType::OPERATOR, value);
                        } break;

                        case '>': {
                            kh::TokenValue value;
                            value.operator_type = kh::Operator::MORE;

                            if (chAt(i + 1) == '=') {
                                value.operator_type = kh::Operator::MORE_EQUAL;
                                i++;
                            }
                            else if (chAt(i + 1) == '>') {
                                value.operator_type = kh::Operator::BIT_RSHIFT;
                                i++;
                            }

                            tokens.emplace_back(start, i + 1, kh::TokenType::OPERATOR, value);
                        } break;

                        case '.': {
                            kh::TokenValue value;
                            value.symbol_type = kh::Symbol::DOT;

                            if (kh::isDec(chAt(i + 1))) {
                                state = kh::TokenizeState::FLOATING;
                                temp_str = U"0.";
                                continue;
                            }

                            tokens.emplace_back(start, i + 1, kh::TokenType::SYMBOL, value);
                        } break;

                        default:
                            KH_RAISE_ERROR(U"Unrecognised character", 0);
                    }
                }
                continue;

                /* Follows the identifier's characters */
            case kh::TokenizeState::IDENTIFIER:
                /* Checks if it's still a valid identifier character */
                if (std::iswalpha(chAt(i)) > 0 || kh::isDec(chAt(i)) || chAt(i) == '_')
                    temp_str += chAt(i);
                else {
                    /* If it's not, reset the state and appends the concatenated identifier
                     * characters as a token */
                    kh::TokenValue value;
                    value.identifier = temp_str;
                    tokens.emplace_back(start, i, kh::TokenType::IDENTIFIER, value);

                    state = kh::TokenizeState::NONE;
                    i--;
                }
                continue;

                /* Checks for an integer */
            case kh::TokenizeState::INTEGER:
                if (kh::isDec(chAt(i)))
                    temp_str += chAt(i);
                else if (chAt(i) == 'u' || chAt(i) == 'U') {
                    /* Is unsigned */
                    kh::TokenValue value;
                    try {
                        value.uinteger = std::stoull(kh::encodeUtf8(temp_str));
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Unsigned integer too large to be interpret", 0);
                    }
                    tokens.emplace_back(start, i + 1, kh::TokenType::UINTEGER, value);

                    state = kh::TokenizeState::NONE;
                }
                else if (chAt(i) == 'i' || chAt(i) == 'I') {
                    /* Is imaginary */
                    kh::TokenValue value;
                    try {
                        value.imaginary = std::stoull(kh::encodeUtf8(temp_str));
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Imaginary integer too large to be interpret", 0);
                    }
                    tokens.emplace_back(start, i + 1, kh::TokenType::IMAGINARY, value);

                    state = kh::TokenizeState::NONE;
                }
                else if (chAt(i) == '.') {
                    /* Checks it as a floating point */
                    temp_str += chAt(i);
                    state = kh::TokenizeState::FLOATING;
                }
                else {
                    kh::TokenValue value;
                    try {
                        value.integer = std::stoll(kh::encodeUtf8(temp_str));
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Integer too large to be interpret", -1);
                    }
                    tokens.emplace_back(start, i, kh::TokenType::INTEGER, value);

                    state = kh::TokenizeState::NONE;
                    i--;
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
                    tokens.emplace_back(start, i + 1, kh::TokenType::IMAGINARY, value);

                    state = kh::TokenizeState::NONE;
                }
                else {
                    /* An artifact from how integers were checked that was transferred as a floating
                     * point with an invalid character after . */
                    if (temp_str.back() == '.')
                        KH_RAISE_ERROR(U"Was expecting a digit after the decimal point", 0);

                    kh::TokenValue value;
                    value.floating = std::stod(kh::encodeUtf8(temp_str));
                    tokens.emplace_back(start, i, kh::TokenType::FLOATING, value);

                    state = kh::TokenizeState::NONE;
                    i--;
                }
                continue;

                /* Checks hex integers */
            case kh::TokenizeState::HEX:
                if (kh::isHex(chAt(i)))
                    temp_str += chAt(i);
                else if (chAt(i) == 'u' || chAt(i) == 'U') {
                    /* Is unsigned */
                    kh::TokenValue value;
                    try {
                        value.uinteger = std::stoull(kh::encodeUtf8(temp_str), nullptr, 16);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Unsigned hex integer too large to be interpret", 0);
                    }
                    tokens.emplace_back(start, i + 1, kh::TokenType::UINTEGER, value);

                    state = kh::TokenizeState::NONE;
                }
                else if (chAt(i) == 'i' || chAt(i) == 'I') {
                    /* Is imaginary */
                    kh::TokenValue value;
                    try {
                        value.imaginary = std::stoull(kh::encodeUtf8(temp_str), nullptr, 16);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Imaginary hex integer too large to be interpret", 0);
                    }
                    tokens.emplace_back(start, i + 1, kh::TokenType::IMAGINARY, value);

                    state = kh::TokenizeState::NONE;
                }
                else {
                    kh::TokenValue value;
                    try {
                        value.integer = std::stoull(kh::encodeUtf8(temp_str), nullptr, 16);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Hex integer too large to be interpret", -1);
                    }
                    tokens.emplace_back(start, i, kh::TokenType::INTEGER, value);

                    state = kh::TokenizeState::NONE;
                    i--;
                }
                continue;

                /* Checks octal integers */
            case kh::TokenizeState::OCTAL:
                if (kh::isOct(chAt(i)))
                    temp_str += chAt(i);
                else if (chAt(i) == 'u' || chAt(i) == 'U') {
                    /* Is unsigned */
                    kh::TokenValue value;
                    try {
                        value.uinteger = std::stoull(kh::encodeUtf8(temp_str), nullptr, 8);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Unsigned octal integer too large to be interpret", 0);
                    }
                    tokens.emplace_back(start, i + 1, kh::TokenType::UINTEGER, value);

                    state = kh::TokenizeState::NONE;
                }
                else if (chAt(i) == 'i' || chAt(i) == 'I') {
                    /* Is imaginary */
                    kh::TokenValue value;
                    try {
                        value.imaginary = std::stoull(kh::encodeUtf8(temp_str), nullptr, 8);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Imaginary octal integer too large to be interpret", 0);
                    }
                    tokens.emplace_back(start, i + 1, kh::TokenType::IMAGINARY, value);

                    state = kh::TokenizeState::NONE;
                }
                else {
                    kh::TokenValue value;
                    try {
                        value.integer = std::stoull(kh::encodeUtf8(temp_str), nullptr, 8);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Octal integer too large to be interpret", -1);
                    }
                    tokens.emplace_back(start, i, kh::TokenType::INTEGER, value);

                    state = kh::TokenizeState::NONE;
                    i--;
                }
                continue;

                /* Checks binary integers */
            case kh::TokenizeState::BIN:
                if (kh::isBin(chAt(i)))
                    temp_str += chAt(i);

                else if (chAt(i) == 'u' || chAt(i) == 'U') {
                    /* Is unsigned */
                    kh::TokenValue value;
                    try {
                        value.uinteger = std::stoull(kh::encodeUtf8(temp_str), nullptr, 2);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Unsigned binary integer too large to be interpret", 0);
                    }
                    tokens.emplace_back(start, i + 1, kh::TokenType::UINTEGER, value);

                    state = kh::TokenizeState::NONE;
                }
                else if (chAt(i) == 'i' || chAt(i) == 'I') {
                    /* Is imaginary */
                    kh::TokenValue value;
                    try {
                        value.imaginary = std::stoull(kh::encodeUtf8(temp_str), nullptr, 2);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Imaginary binary integer too large to be interpret", 0);
                    }
                    tokens.emplace_back(start, i + 1, kh::TokenType::IMAGINARY, value);

                    state = kh::TokenizeState::NONE;
                }
                else {
                    kh::TokenValue value;
                    try {
                        value.integer = std::stoull(kh::encodeUtf8(temp_str), nullptr, 2);
                    }
                    catch (...) {
                        KH_RAISE_ERROR(U"Binary integer too large to be interpret", -1);
                    }
                    tokens.emplace_back(start, i, kh::TokenType::INTEGER, value);

                    state = kh::TokenizeState::NONE;
                    i--;
                }
                continue;

                /* Checks for a byte-string/buffer */
            case kh::TokenizeState::IN_BUF:
                if (chAt(i) == '"') {
                    /* End buffer */

                    kh::TokenValue value;
                    value.buffer = temp_buf;
                    tokens.emplace_back(start, i + 1, kh::TokenType::BUFFER, value);

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
                                i--;
                            } break;

                            case '\n':
                                i++;
                                break;

                                /* Other character escapes */
                                HANDLE_ESCAPES_2(temp_buf.push_back(value));
                        }
                    }
                    else {
                        if (chAt(i) > 255)
                            KH_RAISE_ERROR(U"A non-byte sized character", 0);

                        temp_buf.push_back(chAt(i));
                    }
                }
                continue;

                /* Checks for a multiline byte-string/buffer */
            case kh::TokenizeState::IN_MULTILINE_BUF:
                if (chAt(i) == '"' && chAt(i + 1) == '"' && chAt(i + 2) == '"') {
                    /* End buffer */
                    kh::TokenValue value;
                    value.buffer = temp_buf;
                    tokens.emplace_back(start, i + 3, kh::TokenType::BUFFER, value);

                    state = kh::TokenizeState::NONE;
                    i += 2;
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
                                i--;
                            } break;

                            case '\n':
                                i++;
                                break;

                                /* Other character escapes */
                                HANDLE_ESCAPES_2(temp_buf.push_back(value));
                        }
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
                    tokens.emplace_back(start, i + 1, kh::TokenType::STRING, value);

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
                            case 'x':
                            case 'X': {
                                HANDLE_HEX_INTO_HEXSTR(2, 2);
                                temp_str += std::stoul(hex_str, nullptr, 16);
                                i--;
                            } break;

                            /* 2 bytes unicode escape */
                            case 'u': {
                                HANDLE_HEX_INTO_HEXSTR(2, 4);
                                temp_str += std::stoul(hex_str, nullptr, 16);
                                i--;
                            } break;

                            /* 4 bytes unicode escape */
                            case 'U': {
                                HANDLE_HEX_INTO_HEXSTR(2, 8);
                                temp_str += std::stoul(hex_str, nullptr, 16);
                                i--;
                            } break;

                            case '\n':
                                i++;
                                break;

                                /* Other character escapes */
                                HANDLE_ESCAPES_2(temp_str += value)
                        }
                    }
                    else
                        temp_str += chAt(i);
                }
                continue;

            /* Checks for a multiline string */
            case kh::TokenizeState::IN_MULTILINE_STR:
                if (chAt(i) == '"' && chAt(i + 1) == '"' && chAt(i + 2) == '"') {
                    /* End string */
                    kh::TokenValue value;
                    value.string = temp_str;
                    tokens.emplace_back(start, i + 3, kh::TokenType::STRING, value);

                    state = kh::TokenizeState::NONE;
                    i += 2;
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
                                temp_str += std::stoul(hex_str, nullptr, 16);
                                i--;
                            } break;

                            /* 2 bytes unicode escape */
                            case 'u': {
                                HANDLE_HEX_INTO_HEXSTR(2, 4);
                                temp_str += std::stoul(hex_str, nullptr, 16);
                                i--;
                            } break;

                            /* 4 bytes unicode escape */
                            case 'U': {
                                HANDLE_HEX_INTO_HEXSTR(2, 8);
                                temp_str += std::stoul(hex_str, nullptr, 16);
                                i--;
                            } break;

                            case '\n':
                                i++;
                                break;

                                /* Other character escapes */
                                HANDLE_ESCAPES_2(temp_str += value)
                        }
                    }
                    else
                        temp_str += chAt(i);
                }
                continue;

            /* Passing through until the inline comment is done */
            case kh::TokenizeState::IN_INLINE_COMMENT:
                if (chAt(i) == '\n') {
                    state = kh::TokenizeState::NONE;

                    if (lex_comments)
                        tokens.emplace_back(start, i + 1, kh::TokenType::COMMENT, kh::TokenValue());
                }
                continue;

            /* Passing through until the multiple line comment is closed */
            case kh::TokenizeState::IN_MULTIPLE_LINE_COMMENT:
                if (chAt(i) == '*' && chAt(i + 1) == '/') {
                    /* Close comment */
                    state = kh::TokenizeState::NONE;

                    if (lex_comments)
                        tokens.emplace_back(start, i + 2, kh::TokenType::COMMENT, kh::TokenValue());

                    i++;
                }
                continue;

            default:
                /* How did we get here? */
                KH_RAISE_ERROR(U"Got an unknown tokenize state", 0);
        }
    }
    /* We were expecting to be in a tokenize state, but got EOF, so throw error.
     * This usually happens if the user has forgotten to close a multiline comment,
     * string or buffer */
    if (state != kh::TokenizeState::NONE)
        throw kh::LexException(U"Got unexpected EOF", source.size());

    return tokens;
}
