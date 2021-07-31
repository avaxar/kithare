/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */


#include <kithare/lexer.hpp>


using namespace kh;

std::string kh::LexException::format() const {
    return this->what + " at line " + strfy((uint64_t)this->line) + " column " +
           strfy((uint64_t)this->column);
}

std::vector<Token> kh::lex(const std::u32string& source) {
    std::vector<LexException> exceptions;
    Lexer lexer{source, exceptions};
    std::vector<Token> tokens = lexer.lex();

    if (exceptions.empty()) {
        return tokens;
    }
    else {
        throw exceptions;
    }
}

/* Helper to raise error at a file */
#define KH_RAISE_ERROR(msg, n) throw LexException(msg, i + n)

/* Use this macro to export a variable hex_str from a given start and len relative
 * to the file index */
#define HANDLE_HEX_INTO_HEXSTR(_start, _len)                   \
    std::string hex_str;                                       \
    for (size_t j = _start; j < _start + _len; j++) {          \
        if (isHex(chAt(i + j)))                                \
            hex_str += (char)chAt(i + j);                      \
        else                                                   \
            KH_RAISE_ERROR("expected a hexadecimal digit", j); \
    }                                                          \
    i += _start + _len

/* Helper macro */
#define _PLACE_HEXSTR_AS_TYPE(_var, ttype)               \
    if (chAt(i) == '\'') {                               \
        _var = std::stoul(hex_str, nullptr, 16);         \
        tokens.emplace_back(start, i + 1, ttype, value); \
    }                                                    \
    else                                                 \
        KH_RAISE_ERROR("expected a closing single quote", 0)

/* Place a hex_str as an integer into tokens stack */
#define PLACE_HEXSTR_AS_INT() _PLACE_HEXSTR_AS_TYPE(value.integer, TokenType::INTEGER)

/* Place a hex_str as a character into tokens stack */
#define PLACE_HEXSTR_AS_CHAR() _PLACE_HEXSTR_AS_TYPE(value.character, TokenType::CHARACTER)

/* Helper macro */
#define _HANDLE_ESCAPE(chr, echr, _val, _len, code) \
    case chr:                                       \
        _val = echr;                                \
        code;                                       \
        i += _len;                                  \
        break;

/* Helper macro */
#define _HANDLE_ESCAPE_1(chr, echr, _val, _ttype, _len)                         \
    _HANDLE_ESCAPE(chr, echr, _val, _len,                                       \
                   if (chAt(i + _len) != '\'')                                  \
                       KH_RAISE_ERROR("expected a closing single quote", _len); \
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
    _HANDLE_ESCAPE_1('a', '\a', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('f', '\f', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('\\', '\\', _val, _valc, _len) \
    _HANDLE_ESCAPE_1('"', '\"', _val, _valc, _len)  \
    _HANDLE_ESCAPE_1('\'', '\'', _val, _valc, _len) \
    default:                                        \
        KH_RAISE_ERROR("unknown escape character", _len - 1);

/* Use this to handle string escapes from a switch statement. This is used to handle
 * escapes into byte/unicode strings */
#define HANDLE_ESCAPES_2(code)                 \
    _HANDLE_ESCAPE('0', '\0', value, 1, code)  \
    _HANDLE_ESCAPE('n', '\n', value, 1, code)  \
    _HANDLE_ESCAPE('r', '\r', value, 1, code)  \
    _HANDLE_ESCAPE('t', '\t', value, 1, code)  \
    _HANDLE_ESCAPE('v', '\v', value, 1, code)  \
    _HANDLE_ESCAPE('b', '\b', value, 1, code)  \
    _HANDLE_ESCAPE('a', '\a', value, 1, code)  \
    _HANDLE_ESCAPE('f', '\f', value, 1, code)  \
    _HANDLE_ESCAPE('\\', '\\', value, 1, code) \
    _HANDLE_ESCAPE('"', '\"', value, 1, code)  \
    _HANDLE_ESCAPE('\'', '\'', value, 1, code) \
    default:                                   \
        KH_RAISE_ERROR("unknown escape character", 1);

/* Handle a simple symbol from a switch block */
#define HANDLE_SIMPLE_SYMBOL(sym, name)                            \
    case sym: {                                                    \
        TokenValue val;                                            \
        val.symbol_type = name;                                    \
        tokens.emplace_back(start, i + 1, TokenType::SYMBOL, val); \
    } break;

/* Handle a simple operator from a switch block */
#define HANDLE_SIMPLE_OP(sym, name)                                  \
    case sym: {                                                      \
        TokenValue val;                                              \
        val.operator_type = name;                                    \
        tokens.emplace_back(start, i + 1, TokenType::OPERATOR, val); \
    } break;

/* Handle a combination of two operators as a single operator from a switch block */
#define HANDLE_OP_COMBO(sym, name, sym2, name2)                      \
    case sym: {                                                      \
        TokenValue val;                                              \
        val.operator_type = name;                                    \
        if (chAt(i + 1) == sym2) {                                   \
            val.operator_type = name2;                               \
            i++;                                                     \
        }                                                            \
        tokens.emplace_back(start, i + 1, TokenType::OPERATOR, val); \
    } break;


enum class LexState {
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

std::vector<Token> kh::Lexer::lex() {
    LexState state = LexState::NONE;
    std::vector<Token> tokens;

    size_t start = 0;
    std::u32string temp_str;
    std::string temp_buf;

    /* Lambda function which accesses the string, and throws an error directly to the console if it
     * had passed the length */
    std::function<char32_t(const size_t)> chAt = [&](const size_t index) -> char32_t {
        if (index < this->source.size()) {
            return this->source[index];
        }
        else if (index == this->source.size()) {
            return '\n';
        }
        else {
            size_t i = index;
            KH_RAISE_ERROR("unexpected end of file", -1);
        }
    };

    for (size_t i = 0; i <= this->source.size(); i++) {
        try {
            switch (state) {
                case LexState::NONE:
                    start = i;
                    temp_str.clear();
                    temp_buf.clear();

                    /* Skips whitespace and newlines */
                    if (chAt(i) == '\n' || std::iswspace(chAt(i)) > 0) {
                        continue;
                    }

                    /* Possible identifier start */
                    else if (std::iswalpha(chAt(i)) > 0 || chAt(i) == '_') {
                        /* Possible start of a byte-string/byte-string constant */
                        if (chAt(i) == 'b' || chAt(i) == 'B') {
                            /* Possible byte-char */
                            if (chAt(i + 1) == '\'') {
                                /* Possible char-escape */
                                if (chAt(i + 2) == '\\') {
                                    TokenValue value;
                                    switch (chAt(i + 3)) {
                                        /* Hex character escape */
                                        case 'x':
                                        case 'X': {
                                            HANDLE_HEX_INTO_HEXSTR(4, 2);
                                            PLACE_HEXSTR_AS_INT();
                                        } break;

                                            HANDLE_ESCAPES_1(value.integer, TokenType::INTEGER, 4)
                                    }
                                }
                                else if (chAt(i + 2) == '\'') {
                                    /* No Character inserted, like b''. Treat it like a 0 */
                                    TokenValue value;
                                    value.integer = 0;
                                    tokens.emplace_back(start, i + 3, TokenType::INTEGER, value);

                                    i += 2;
                                }
                                else if (chAt(i + 2) == '\n') {
                                    KH_RAISE_ERROR("new line before byte character closing", 2);
                                }

                                /* Plain byte-char without character escapes */
                                else if (chAt(i + 3) == '\'') {
                                    if (chAt(i + 2) > 255) {
                                        KH_RAISE_ERROR("a non-byte sized character", 2);
                                    }

                                    TokenValue value;
                                    value.integer = chAt(i + 2);
                                    tokens.emplace_back(start, i + 4, TokenType::INTEGER, value);

                                    i += 3;
                                }
                                else {
                                    KH_RAISE_ERROR("expected a closing single quote", 3);
                                }
                                continue;
                            }
                            /* Possible byte-string/buffer */
                            else if (chAt(i + 1) == '"') {
                                if (chAt(i + 2) == '"' && chAt(i + 3) == '"') {
                                    state = LexState::IN_MULTILINE_BUF;
                                    i += 3;
                                }
                                else {
                                    state = LexState::IN_BUF;
                                    i += 1;
                                }
                                continue;
                            }
                        }

                        /* If it's not a byte-string/byte-char, it's just a normal identifier */
                        state = LexState::IDENTIFIER;
                        temp_str = chAt(i);
                    }

                    /* Starts with a decimal value, possible number constant */
                    else if (isDec(chAt(i))) {
                        /* Likely to use other number base */
                        if (chAt(i) == '0') {
                            /* Handles hex numbers */
                            switch (chAt(i + 1)) {
                                case 'x':
                                case 'X': {
                                    state = LexState::HEX;
                                    if (!isHex(chAt(i + 2))) {
                                        KH_RAISE_ERROR("expected a hexadecimal digit", 2);
                                    }

                                    i++;
                                    continue;
                                }

                                    /* Handles octal numbers */
                                case 'o':
                                case 'O': {
                                    state = LexState::OCTAL;
                                    if (!isOct(chAt(i + 2))) {
                                        KH_RAISE_ERROR("expected an octal digit at", 2);
                                    }

                                    i++;
                                    continue;
                                }

                                    /* Handles binary numbers */
                                case 'b':
                                case 'B': {
                                    state = LexState::BIN;
                                    if (!isBin(chAt(i + 2))) {
                                        KH_RAISE_ERROR("expected a binary digit at", 2);
                                    }

                                    i++;
                                    continue;
                                }
                            }
                        }

                        state = LexState::INTEGER;
                        temp_str = chAt(i);
                    }

                    else {
                        switch (chAt(i)) {
                            /* Possible character */
                            case '\'': {
                                TokenValue value;
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

                                            HANDLE_ESCAPES_1(value.character, TokenType::CHARACTER, 3);
                                    }
                                }
                                else if (chAt(i + 1) == '\'') {
                                    /* No Character inserted, like ''. Treat it like a \0 */
                                    TokenValue value;
                                    value.character = 0;
                                    tokens.emplace_back(start, i + 2, TokenType::CHARACTER, value);

                                    i += 1;
                                }
                                else if (chAt(i + 1) == '\n') {
                                    KH_RAISE_ERROR("new line before character closing", 1);
                                }
                                else if (chAt(i + 2) == '\'') {
                                    TokenValue value;
                                    value.character = chAt(i + 1);
                                    tokens.emplace_back(start, i + 3, TokenType::CHARACTER, value);
                                    i += 2;
                                }
                                else {
                                    KH_RAISE_ERROR("expected a closing single quote", 2);
                                }
                                continue;
                            } break;

                                /* Possible string */
                            case '"':
                                if (chAt(i + 1) == '"' && chAt(i + 2) == '"') {
                                    state = LexState::IN_MULTILINE_STR;
                                    i += 2;
                                }
                                else {
                                    state = LexState::IN_STR;
                                }
                                break;

                                /* Operator handling */
                                HANDLE_OP_COMBO('%', Operator::MOD, '=', Operator::IMOD);
                                HANDLE_OP_COMBO('^', Operator::POW, '=', Operator::IPOW);
                                HANDLE_OP_COMBO('=', Operator::ASSIGN, '=', Operator::EQUAL);
                                HANDLE_OP_COMBO('!', Operator::NOT, '=', Operator::NOT_EQUAL);

                                HANDLE_SIMPLE_OP('&', Operator::BIT_AND);
                                HANDLE_SIMPLE_OP('|', Operator::BIT_OR);
                                HANDLE_SIMPLE_OP('~', Operator::BIT_NOT);
                                HANDLE_SIMPLE_OP('#', Operator::SIZEOF);
                                HANDLE_SIMPLE_OP('@', Operator::ADDRESS);

                                /* Symbol handling */
                                HANDLE_SIMPLE_SYMBOL(';', Symbol::SEMICOLON);
                                HANDLE_SIMPLE_SYMBOL(',', Symbol::COMMA);
                                HANDLE_SIMPLE_SYMBOL(':', Symbol::COLON);
                                HANDLE_SIMPLE_SYMBOL('(', Symbol::PARENTHESES_OPEN);
                                HANDLE_SIMPLE_SYMBOL(')', Symbol::PARENTHESES_CLOSE);
                                HANDLE_SIMPLE_SYMBOL('{', Symbol::CURLY_OPEN);
                                HANDLE_SIMPLE_SYMBOL('}', Symbol::CURLY_CLOSE);
                                HANDLE_SIMPLE_SYMBOL('[', Symbol::SQUARE_OPEN);
                                HANDLE_SIMPLE_SYMBOL(']', Symbol::SQUARE_CLOSE);

                                /* Some operators and symbols have more complicated handling, and
                                 * those are not macro-ised */
                            case '+': {
                                TokenValue value;
                                value.operator_type = Operator::ADD;

                                if (chAt(i + 1) == '=') {
                                    value.operator_type = Operator::IADD;
                                    i++;
                                }
                                else if (chAt(i + 1) == '+') {
                                    value.operator_type = Operator::INCREMENT;
                                    i++;
                                }

                                tokens.emplace_back(start, i + 1, TokenType::OPERATOR, value);
                            } break;

                            case '-': {
                                TokenValue value;
                                value.operator_type = Operator::SUB;

                                if (chAt(i + 1) == '=') {
                                    value.operator_type = Operator::ISUB;
                                    i++;
                                }
                                else if (chAt(i + 1) == '-') {
                                    value.operator_type = Operator::DECREMENT;
                                    i++;
                                }

                                tokens.emplace_back(start, i + 1, TokenType::OPERATOR, value);
                            } break;

                            case '*': {
                                TokenValue value;
                                value.operator_type = Operator::MUL;

                                if (chAt(i + 1) == '=') {
                                    value.operator_type = Operator::IMUL;
                                    i++;
                                }
                                else if (chAt(i + 1) == '/') {
                                    KH_RAISE_ERROR("unexpected comment close", 0);
                                }

                                tokens.emplace_back(start, i + 1, TokenType::OPERATOR, value);
                            } break;

                            case '/': {
                                TokenValue value;
                                value.operator_type = Operator::DIV;

                                if (chAt(i + 1) == '=') {
                                    value.operator_type = Operator::IDIV;
                                    i++;
                                }
                                else if (chAt(i + 1) == '/') {
                                    state = LexState::IN_INLINE_COMMENT;
                                    i++;
                                    continue;
                                }
                                else if (chAt(i + 1) == '*') {
                                    state = LexState::IN_MULTIPLE_LINE_COMMENT;
                                    i++;
                                    continue;
                                }

                                tokens.emplace_back(start, i + 1, TokenType::OPERATOR, value);
                            } break;

                            case '<': {
                                TokenValue value;
                                value.operator_type = Operator::LESS;

                                if (chAt(i + 1) == '=') {
                                    value.operator_type = Operator::LESS_EQUAL;
                                    i++;
                                }
                                else if (chAt(i + 1) == '<') {
                                    value.operator_type = Operator::BIT_LSHIFT;
                                    i++;
                                }

                                tokens.emplace_back(start, i + 1, TokenType::OPERATOR, value);
                            } break;

                            case '>': {
                                TokenValue value;
                                value.operator_type = Operator::MORE;

                                if (chAt(i + 1) == '=') {
                                    value.operator_type = Operator::MORE_EQUAL;
                                    i++;
                                }
                                else if (chAt(i + 1) == '>') {
                                    value.operator_type = Operator::BIT_RSHIFT;
                                    i++;
                                }

                                tokens.emplace_back(start, i + 1, TokenType::OPERATOR, value);
                            } break;

                            case '.': {
                                TokenValue value;
                                value.symbol_type = Symbol::DOT;

                                if (isDec(chAt(i + 1))) {
                                    state = LexState::FLOATING;
                                    temp_str = U"0.";
                                    continue;
                                }

                                tokens.emplace_back(start, i + 1, TokenType::SYMBOL, value);
                            } break;

                            default:
                                KH_RAISE_ERROR("unrecognized character", 0);
                        }
                    }
                    continue;

                    /* Follows the identifier's characters */
                case LexState::IDENTIFIER:
                    /* Checks if it's still a valid identifier character */
                    if (std::iswalpha(chAt(i)) > 0 || isDec(chAt(i)) || chAt(i) == '_') {
                        temp_str += chAt(i);
                    }
                    else {
                        TokenValue value;
                        if (temp_str == U"and") {
                            value.operator_type = Operator::AND;
                            tokens.emplace_back(start, i, TokenType::OPERATOR, value);
                        }
                        else if (temp_str == U"or") {
                            value.operator_type = Operator::OR;
                            tokens.emplace_back(start, i, TokenType::OPERATOR, value);
                        }
                        else {
                            /* If it's not, reset the state and appends the concatenated identifier
                             * characters as a token */
                            value.identifier = utf8Encode(temp_str);
                            tokens.emplace_back(start, i, TokenType::IDENTIFIER, value);
                        }

                        state = LexState::NONE;
                        i--;
                    }
                    continue;

                    /* Checks for an integer */
                case LexState::INTEGER:
                    if (isDec(chAt(i))) {
                        temp_str += chAt(i);
                    }
                    else if (chAt(i) == 'u' || chAt(i) == 'U') {
                        /* Is unsigned */
                        TokenValue value;
                        try {
                            value.uinteger = std::stoull(utf8Encode(temp_str));
                        }
                        catch (...) {
                            KH_RAISE_ERROR("unsigned integer too large to be interpret", 0);
                        }
                        tokens.emplace_back(start, i + 1, TokenType::UINTEGER, value);

                        state = LexState::NONE;
                    }
                    else if (chAt(i) == 'i' || chAt(i) == 'I') {
                        /* Is imaginary */
                        TokenValue value;
                        try {
                            value.imaginary = std::stoull(utf8Encode(temp_str));
                        }
                        catch (...) {
                            KH_RAISE_ERROR("imaginary integer too large to be interpret", 0);
                        }
                        tokens.emplace_back(start, i + 1, TokenType::IMAGINARY, value);

                        state = LexState::NONE;
                    }
                    else if (chAt(i) == '.') {
                        /* Checks it as a floating point */
                        temp_str += chAt(i);
                        state = LexState::FLOATING;
                    }
                    else {
                        TokenValue value;
                        try {
                            value.integer = std::stoll(utf8Encode(temp_str));
                        }
                        catch (...) {
                            KH_RAISE_ERROR("integer too large to be interpret", -1);
                        }
                        tokens.emplace_back(start, i, TokenType::INTEGER, value);

                        state = LexState::NONE;
                        i--;
                    }
                    continue;

                    /* Checks floating point numbers */
                case LexState::FLOATING:
                    if (isDec(chAt(i))) {
                        temp_str += chAt(i);
                    }
                    else if (chAt(i) == 'i' || chAt(i) == 'I') {
                        /* Is imaginary */
                        if (temp_str.back() == '.') {
                            temp_str.pop_back();
                        }

                        TokenValue value;
                        value.imaginary = std::stod(utf8Encode(temp_str));
                        tokens.emplace_back(start, i + 1, TokenType::IMAGINARY, value);

                        state = LexState::NONE;
                    }
                    else {
                        /* An artifact from how integers were checked that was transferred as a floating
                         * point with an invalid character after . */
                        if (temp_str.back() == '.') {
                            KH_RAISE_ERROR("was expecting a digit after the decimal point", 0);
                        }

                        TokenValue value;
                        value.floating = std::stod(utf8Encode(temp_str));
                        tokens.emplace_back(start, i, TokenType::FLOATING, value);

                        state = LexState::NONE;
                        i--;
                    }
                    continue;

                    /* Checks hex integers */
                case LexState::HEX:
                    if (isHex(chAt(i))) {
                        temp_str += chAt(i);
                    }
                    else if (chAt(i) == 'u' || chAt(i) == 'U') {
                        /* Is unsigned */
                        TokenValue value;
                        try {
                            value.uinteger = std::stoull(utf8Encode(temp_str), nullptr, 16);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("unsigned hex integer too large to be interpret", 0);
                        }
                        tokens.emplace_back(start, i + 1, TokenType::UINTEGER, value);

                        state = LexState::NONE;
                    }
                    else if (chAt(i) == 'i' || chAt(i) == 'I') {
                        /* Is imaginary */
                        TokenValue value;
                        try {
                            value.imaginary = std::stoull(utf8Encode(temp_str), nullptr, 16);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("imaginary hex integer too large to be interpret", 0);
                        }
                        tokens.emplace_back(start, i + 1, TokenType::IMAGINARY, value);

                        state = LexState::NONE;
                    }
                    else {
                        TokenValue value;
                        try {
                            value.integer = std::stoull(utf8Encode(temp_str), nullptr, 16);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("hex integer too large to be interpret", -1);
                        }
                        tokens.emplace_back(start, i, TokenType::INTEGER, value);

                        state = LexState::NONE;
                        i--;
                    }
                    continue;

                    /* Checks octal integers */
                case LexState::OCTAL:
                    if (isOct(chAt(i))) {
                        temp_str += chAt(i);
                    }
                    else if (chAt(i) == 'u' || chAt(i) == 'U') {
                        /* Is unsigned */
                        TokenValue value;
                        try {
                            value.uinteger = std::stoull(utf8Encode(temp_str), nullptr, 8);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("unsigned octal integer too large to be interpret", 0);
                        }
                        tokens.emplace_back(start, i + 1, TokenType::UINTEGER, value);

                        state = LexState::NONE;
                    }
                    else if (chAt(i) == 'i' || chAt(i) == 'I') {
                        /* Is imaginary */
                        TokenValue value;
                        try {
                            value.imaginary = std::stoull(utf8Encode(temp_str), nullptr, 8);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("imaginary octal integer too large to be interpret", 0);
                        }
                        tokens.emplace_back(start, i + 1, TokenType::IMAGINARY, value);

                        state = LexState::NONE;
                    }
                    else {
                        TokenValue value;
                        try {
                            value.integer = std::stoull(utf8Encode(temp_str), nullptr, 8);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("octal integer too large to be interpret", -1);
                        }
                        tokens.emplace_back(start, i, TokenType::INTEGER, value);

                        state = LexState::NONE;
                        i--;
                    }
                    continue;

                    /* Checks binary integers */
                case LexState::BIN:
                    if (isBin(chAt(i))) {
                        temp_str += chAt(i);
                    }

                    else if (chAt(i) == 'u' || chAt(i) == 'U') {
                        /* Is unsigned */
                        TokenValue value;
                        try {
                            value.uinteger = std::stoull(utf8Encode(temp_str), nullptr, 2);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("unsigned binary integer too large to be interpret", 0);
                        }
                        tokens.emplace_back(start, i + 1, TokenType::UINTEGER, value);

                        state = LexState::NONE;
                    }
                    else if (chAt(i) == 'i' || chAt(i) == 'I') {
                        /* Is imaginary */
                        TokenValue value;
                        try {
                            value.imaginary = std::stoull(utf8Encode(temp_str), nullptr, 2);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("imaginary binary integer too large to be interpret", 0);
                        }
                        tokens.emplace_back(start, i + 1, TokenType::IMAGINARY, value);

                        state = LexState::NONE;
                    }
                    else {
                        TokenValue value;
                        try {
                            value.integer = std::stoull(utf8Encode(temp_str), nullptr, 2);
                        }
                        catch (...) {
                            KH_RAISE_ERROR("binary integer too large to be interpret", -1);
                        }
                        tokens.emplace_back(start, i, TokenType::INTEGER, value);

                        state = LexState::NONE;
                        i--;
                    }
                    continue;

                    /* Checks for a byte-string/buffer */
                case LexState::IN_BUF:
                    if (chAt(i) == '"') {
                        /* End buffer */

                        TokenValue value;
                        value.buffer = temp_buf;
                        tokens.emplace_back(start, i + 1, TokenType::BUFFER, value);

                        state = LexState::NONE;
                    }
                    else if (chAt(i) == '\n') {
                        KH_RAISE_ERROR("unclosed buffer string before new line", 0);
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
                            if (chAt(i) > 255) {
                                KH_RAISE_ERROR("a non-byte sized character", 0);
                            }

                            temp_buf.push_back(chAt(i));
                        }
                    }
                    continue;

                    /* Checks for a multiline byte-string/buffer */
                case LexState::IN_MULTILINE_BUF:
                    if (chAt(i) == '"' && chAt(i + 1) == '"' && chAt(i + 2) == '"') {
                        /* End buffer */
                        TokenValue value;
                        value.buffer = temp_buf;
                        tokens.emplace_back(start, i + 3, TokenType::BUFFER, value);

                        state = LexState::NONE;
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
                            if (chAt(i) > 255) {
                                KH_RAISE_ERROR("a non-byte sized character", 0);
                            }

                            temp_buf.push_back(chAt(i));
                        }
                    }
                    continue;

                    /* Checks for a string */
                case LexState::IN_STR:
                    if (chAt(i) == '"') {
                        /* End string */
                        TokenValue value;
                        value.string = temp_str;
                        tokens.emplace_back(start, i + 1, TokenType::STRING, value);

                        state = LexState::NONE;
                    }
                    else if (chAt(i) == '\n') {
                        KH_RAISE_ERROR("unclosed string before new line", 0);
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
                        else {
                            temp_str += chAt(i);
                        }
                    }
                    continue;

                    /* Checks for a multiline string */
                case LexState::IN_MULTILINE_STR:
                    if (chAt(i) == '"' && chAt(i + 1) == '"' && chAt(i + 2) == '"') {
                        /* End string */
                        TokenValue value;
                        value.string = temp_str;
                        tokens.emplace_back(start, i + 3, TokenType::STRING, value);

                        state = LexState::NONE;
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
                        else {
                            temp_str += chAt(i);
                        }
                    }
                    continue;

                    /* Passing through until the inline comment is done */
                case LexState::IN_INLINE_COMMENT:
                    if (chAt(i) == '\n') {
                        state = LexState::NONE;
                    }
                    continue;

                    /* Passing through until the multiple line comment is closed */
                case LexState::IN_MULTIPLE_LINE_COMMENT:
                    if (chAt(i) == '*' && chAt(i + 1) == '/') {
                        /* Close comment */
                        state = LexState::NONE;
                        i++;
                    }
                    continue;

                default:
                    /* How did we get here? */
                    KH_RAISE_ERROR("got an unknown lex state (u got a bug m8)", 0);
            }
        }
        catch (const LexException& exc) {
            this->exceptions.push_back(exc);
            state = LexState::NONE;
            getLineColumn(this->source, exc.index, this->exceptions.back().column,
                          this->exceptions.back().line);
        }
    }
    /* We were expecting to be in a tokenize state, but got EOF, so throw error.
     * This usually happens if the user has forgotten to close a multiline comment,
     * string or buffer */
    if (state != LexState::NONE) {
        this->exceptions.emplace_back("unexpected end of file", this->source.size());
    }

    /* Fills in the `column` and `line` number attributes of each token */
    size_t column = 1, line = 1;
    size_t token_index = 0;
    for (size_t i = 0; i <= this->source.size(); i++, column++) {
        if (token_index >= tokens.size()) {
            break;
        }
        if (this->source[i] == '\n') {
            column = 0;
            line++;
        }

        if (tokens[token_index].begin == i) {
            tokens[token_index].column = column;
            tokens[token_index].line = line;
            token_index++;
        }
    }

    return tokens;
}
