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
#define KH_RAISE_ERROR(msg, n) throw kh::LexException(msg, line_n, char_line + n, i + n, n > 0 ? chAt(i + n) : ' ')

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
            if (chAt(i) == '\n') {
                line_n++;
                char_line = 0;
            }
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
                            switch (chAt(i + 3)) {
                                /* Hex character escape */
                            case 'x':
                            case 'X':
                            {
                                std::string hex_str;
                                for (size_t j = 4; j < 6; j++) {
                                    if (kh::isHex(chAt(i + j)))
                                        hex_str += (char)chAt(i + j);
                                    else
                                        KH_RAISE_ERROR(U"Expected a hexadecimal digit", j);
                                }

                                uint8_t hex = std::stoul(hex_str, nullptr, 16);
                                if (chAt(i + 6) == '\'') {
                                    kh::TokenValue value;
                                    value.integer = hex;
                                    tokens.emplace_back(
                                        kh::TokenType::INTEGER,
                                        value,
                                        char_line,
                                        line_n
                                    );
                                    i += 6;
                                    char_line += 6;
                                }
                                else
                                    KH_RAISE_ERROR(U"Expected a closing single quote", 6);
                            } break;

                            /* Other character escapes */
                            case '0':
                            case 'n':
                            case 'r':
                            case 't':
                            case 'v':
                            case '\\':
                            case '\'':
                                if (chAt(i + 4) == '\'') {
                                    kh::TokenValue value;

                                    switch (chAt(i + 3)) {
                                    case '0':
                                        value.integer = '\0';
                                        break;
                                    case 'n':
                                        value.integer = '\n';
                                        break;
                                    case 'r':
                                        value.integer = '\r';
                                        break;
                                    case 't':
                                        value.integer = '\t';
                                        break;
                                    case 'v':
                                        value.integer = '\v';
                                        break;
                                    case '\\':
                                        value.integer = '\\';
                                        break;
                                    case '\'':
                                        value.integer = '\'';
                                        break;
                                    }

                                    tokens.emplace_back(
                                        kh::TokenType::INTEGER,
                                        value,
                                        char_line,
                                        line_n
                                    );

                                    i += 4;
                                    char_line += 4;
                                }
                                else
                                    KH_RAISE_ERROR(U"Expected a closing single quote", 4);
                                break;

                            default:
                                KH_RAISE_ERROR(U"Unknown escape character", 3);
                            }
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

                                    i += 3;
                                    char_line += 3;
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
                        i++;
                        char_line++;
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

                        i++;
                        char_line++;
                        continue;
                    }

                    /* Handles octal numbers */
                    else if (chAt(i + 1) == 'o' || chAt(i + 1) == 'O') {
                        state = kh::TokenizeState::OCTAL;

                        if (!kh::isOct(chAt(i + 2)))
                            KH_RAISE_ERROR(U"Was expecting an octal digit at", 2);

                        i++;
                        char_line++;
                        continue;
                    }

                    /* Handles binary numbers */
                    else if (chAt(i + 1) == 'b' || chAt(i + 1) == 'B') {
                        state = kh::TokenizeState::BIN;

                        if (!kh::isBin(chAt(i + 2)))
                            KH_RAISE_ERROR(U"Was expecting a binary digit at", 2);

                        i++;
                        char_line++;
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
                    /* Possible char escape */
                    if (chAt(i + 1) == '\\') {
                        switch (chAt(i + 2)) {
                            /* Hex escapes */
                        case 'x': case 'X': {
                            std::string hex_str;
                            for (size_t j = 3; j < 5; j++) {
                                if (kh::isHex(chAt(i + j)))
                                    hex_str += (char)chAt(i + j);
                                else
                                    KH_RAISE_ERROR(U"Expected a hexadecimal digit", j);
                            }

                            uint8_t hex = std::stoul(hex_str, nullptr, 16);
                            if (chAt(i + 5) == '\'') {
                                kh::TokenValue value;
                                value.character = hex;
                                tokens.emplace_back(
                                    kh::TokenType::CHARACTER,
                                    value,
                                    char_line,
                                    line_n
                                );

                                i += 5;
                                char_line += 5;
                            }
                            else
                                KH_RAISE_ERROR(U"Expected a closing single quote", 5);
                        } break;

                            /* 2 bytes unicode escape */
                        case 'u': {
                            std::string hex_str;
                            for (size_t j = 3; j < 7; j++) {
                                if (kh::isHex(chAt(i + j)))
                                    hex_str += (char)chAt(i + j);
                                else
                                    KH_RAISE_ERROR(U"Expected a hexadecimal digit", j);
                            }

                            uint16_t hex = std::stoul(hex_str, nullptr, 16);
                            if (chAt(i + 7) == '\'') {
                                kh::TokenValue value;
                                value.character = hex;
                                tokens.emplace_back(
                                    kh::TokenType::CHARACTER,
                                    value,
                                    char_line,
                                    line_n
                                );

                                i += 7;
                                char_line += 7;
                            }
                            else
                                KH_RAISE_ERROR(U"Expected a closing single quote", 7);
                        } break;

                            /* 4 bytes unicode escape */
                        case 'U': {
                            std::string hex_str;
                            for (size_t j = 3; j < 11; j++) {
                                if (kh::isHex(chAt(i + j)))
                                    hex_str += (char)chAt(i + j);
                                else
                                    KH_RAISE_ERROR(U"Expected a hexadecimal digit", j);
                            }

                            uint32_t hex = std::stoul(hex_str, nullptr, 16);
                            if (chAt(i + 11) == '\'') {
                                kh::TokenValue value;
                                value.character = hex;
                                tokens.emplace_back(
                                    kh::TokenType::CHARACTER,
                                    value,
                                    char_line,
                                    line_n
                                );

                                i += 11;
                                char_line += 11;
                            }
                            else
                                KH_RAISE_ERROR(U"Expected a closing single quote", 11);
                        } break;

                            /* Other character escapes */
                        case '0':
                        case 'n':
                        case 'r':
                        case 't':
                        case 'v':
                        case '\\':
                        case '\'':
                            if (chAt(i + 3) == '\'') {
                                kh::TokenValue value;

                                switch (chAt(i + 2)) {
                                case '0':
                                    value.character = '\0';
                                    break;
                                case 'n':
                                    value.character = '\n';
                                    break;
                                case 'r':
                                    value.character = '\r';
                                    break;
                                case 't':
                                    value.character = '\t';
                                    break;
                                case 'v':
                                    value.character = '\v';
                                    break;
                                case '\\':
                                    value.character = '\\';
                                    break;
                                case '\'':
                                    value.character = '\'';
                                    break;
                                }

                                tokens.emplace_back(
                                    kh::TokenType::CHARACTER,
                                    value,
                                    char_line,
                                    line_n
                                );

                                i += 3;
                                char_line += 3;
                            }
                            else
                                KH_RAISE_ERROR(U"Expected a closing single quote", 3);
                            break;

                        default:
                            KH_RAISE_ERROR(U"Unknown escape character", 2);
                        }
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

                                i += 2;
                                char_line += 2;
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
                        i++;
                        char_line++;
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
                        i++;
                        char_line++;
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
                        i++;
                        char_line++;
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
                        i++; char_line++;
                    }
                    else if (chAt(i + 1) == '/') {
                        state = kh::TokenizeState::IN_INLINE_COMMENT;
                        i++; char_line++;
                        continue;
                    }
                    else if (chAt(i + 1) == '*') {
                        state = kh::TokenizeState::IN_MULTIPLE_LINE_COMMENT;
                        i++; char_line++;
                        continue;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '%': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::MOD;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::IMOD;
                        i++; char_line++;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '^': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::POW;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::IPOW;
                        i++; char_line++;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '=': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::ASSIGN;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::EQUAL;
                        i++; char_line++;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '<': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::LESS;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::LESS_EQUAL;
                        i++; char_line++;
                    }
                    else if (chAt(i + 1) == '<') {
                        value.operator_type = kh::Operator::BIT_LSHIFT;
                        i++; char_line++;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '>': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::MORE;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::MORE_EQUAL;
                        i++; char_line++;
                    }
                    else if (chAt(i + 1) == '>') {
                        value.operator_type = kh::Operator::BIT_RSHIFT;
                        i++; char_line++;
                    }
                    else if (chAt(i + 1) == ']') {
                        value.symbol_type = kh::Symbol::TEMPLATE_CLOSE;
                        i++; char_line++;

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
                        i++; char_line++;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '&': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::BIT_AND;

                    if (chAt(i + 1) == '&') {
                        value.operator_type = kh::Operator::AND;
                        i++; char_line++;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '|': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::BIT_OR;

                    if (chAt(i + 1) == '|') {
                        value.operator_type = kh::Operator::OR;
                        i++; char_line++;
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
                        i++; char_line++;

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
                i--;
                char_line--;
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
                i--;
                char_line--;
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
                    KH_RAISE_ERROR(U"Was expecting a digit after the decimal point", 0);

                kh::TokenValue value;
                value.floating = std::stod(kh::encodeUtf8(temp_str));
                tokens.emplace_back(
                    kh::TokenType::FLOATING,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
                i--;
                char_line--;
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
                i--;
                char_line--;
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
                i--;
                char_line--;
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
                i--;
                char_line--;
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
                    switch (chAt(i + 1)) {
                        /* Hex character escape */
                    case 'x':
                    case 'X': {
                        std::string hex_str;
                        for (size_t j = 2; j < 4; j++) {
                            if (kh::isHex(chAt(i + j)))
                                hex_str += (char)chAt(i + j);
                            else
                                KH_RAISE_ERROR(U"Expected a hexadecimal digit", j);
                        }

                        temp_buf.push_back(std::stoul(hex_str, nullptr, 16));
                        i += 3;
                        char_line += 3;
                    } break;

                        /* Other character escapes */
                    case '0':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    case '\\':
                    case '"': {
                        uint8_t value;

                        switch (chAt(i + 1)) {
                        case '0':
                            value = '\0';
                            break;
                        case 'n':
                            value = '\n';
                            break;
                        case 'r':
                            value = '\r';
                            break;
                        case 't':
                            value = '\t';
                            break;
                        case 'v':
                            value = '\v';
                            break;
                        case '\\':
                            value = '\\';
                            break;
                        case '"':
                            value = '"';
                            break;
                        }

                        temp_buf.push_back(value);
                        i += 1;
                        char_line += 1;
                    } break;

                    case '\n':
                        i += 1;
                        char_line = 0;
                        line_n++;
                        break;

                    default:
                        KH_RAISE_ERROR(U"Unknown escape character", 1);
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
                tokens.emplace_back(kh::TokenType::STRING, value, char_line, line_n);

                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == '\n') {
                KH_RAISE_ERROR(U"Unclosed string before new line", 0);
            }
            else {
                /* Possible character escape */
                if (chAt(i) == '\\') {
                    switch (chAt(i + 1)) {
                        /* Hex character escape */
                    case 'x': case 'X': {
                        std::string hex_str;
                        for (size_t j = 2; j < 4; j++) {
                            if (kh::isHex(chAt(i + j)))
                                hex_str += (char)chAt(i + j);
                            else
                                KH_RAISE_ERROR(U"Expected a hexadecimal digit", j);
                        }

                        temp_str += std::stoul(hex_str, nullptr, 16);
                        i += 3;
                        char_line += 3;
                    } break;

                        /* 2 bytes unicode escape */
                    case 'u': {
                        std::string hex_str;
                        for (size_t j = 2; j < 6; j++) {
                            if (kh::isHex(chAt(i + j)))
                                hex_str += (char)chAt(i + j);
                            else
                                KH_RAISE_ERROR(U"Expected a hexadecimal digit", j);
                        }

                        temp_str += std::stoul(hex_str, nullptr, 16);
                        i += 5;
                        char_line += 5;
                    } break;

                        /* 4 bytes unicode escape */
                    case 'U': {
                        std::string hex_str;
                        for (size_t j = 2; j < 10; j++) {
                            if (kh::isHex(chAt(i + j)))
                                hex_str += (char)chAt(i + j);
                            else
                                KH_RAISE_ERROR(U"Expected a hexadecimal digit", j);
                        }

                        temp_str += std::stoul(hex_str, nullptr, 16);
                        i += 9;
                        char_line += 9;
                    } break;

                        /* Other character escapes */
                    case '0':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    case '\\':
                    case '"': {
                        uint8_t value;

                        switch (chAt(i + 1)) {
                        case '0':
                            value = '\0';
                            break;
                        case 'n':
                            value = '\n';
                            break;
                        case 'r':
                            value = '\r';
                            break;
                        case 't':
                            value = '\t';
                            break;
                        case 'v':
                            value = '\v';
                            break;
                        case '\\':
                            value = '\\';
                            break;
                        case '"':
                            value = '"';
                            break;
                        }

                        temp_str += value;
                        i += 1;
                        char_line += 1;
                    } break;

                    case '\n':
                        i += 1;
                        char_line = 0;
                        line_n++;
                        break;

                    default:
                        KH_RAISE_ERROR(U"Unknown escape character", 1);
                    }
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
                char_line = 0;
                line_n++;
            }
            continue;

            /* Passing through until the multiple line comment is closed */
        case kh::TokenizeState::IN_MULTIPLE_LINE_COMMENT:
            if (chAt(i) == '*' && chAt(i + 1) == '/') {
                /* Close comment */

                state = kh::TokenizeState::NONE;
                i++;
                char_line++;
            }
            else if (chAt(i) == '\n') {
                /* Still not forgetting the line count */

                char_line = 0;
                line_n++;
            }
            continue;

        default:
            /* How did we get here? */
            KH_RAISE_ERROR(U"Got an unknown tokenize state", 0);
        }
    }
    return tokens;
}
