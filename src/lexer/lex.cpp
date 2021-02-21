#include "lexer/lex.hpp"


std::vector<kh::Token> kh::lex(const kh::String& source, const kh::String& file_name) {
    std::vector<kh::Token> tokens;
    kh::TokenizeState state = kh::TokenizeState::NONE;

    kh::String temp_str;
    std::vector<uint8> temp_buf;

    size_t line_n = 1, char_line = 1;

    const std::function<const uint32(const size_t)> chAt = [&](const size_t index) -> const uint32 {
        if (index < source.size())
            return source[index];
        else if (index == source.size())
            return '\n';
        else 
            KH_RAISE_ERROR("Unexpected EOF");
    };

    for (size_t i = 0; i <= source.size(); (i++, char_line++)) {
        switch (state) {
        case kh::TokenizeState::NONE:
            temp_str = kh::String();
            if (!temp_buf.empty())
                temp_buf.clear();

            if (chAt(i) == '\n') {
                line_n++;
                char_line = 0;
            }
            else if (std::iswspace(chAt(i)) > 0)
                continue;
            else if (std::iswalpha(chAt(i)) > 0 || chAt(i) == '_') {
                if (chAt(i) == 'b' || chAt(i) == 'B') {
                    if (chAt(i + 1) == '\'') {
                        if (chAt(i + 2) == '\\') {
                            switch (chAt(i + 3)) {
                            case 'x':
                            case 'X':
                            {
                                std::string hex_str;
                                for (size_t j = 4; j < 6; j++) {
                                    if (kh::isHex(chAt(i + j)))
                                        hex_str += (char)chAt(i + j);
                                    else
                                        KH_RAISE_INFO_ERROR("Expected a hexadecimal digit", j);
                                }

                                uint8 hex = std::stoul(hex_str, nullptr, 16);
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
                                    KH_RAISE_INFO_ERROR("Expected a closing single quote", 6);
                            } break;

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
                                    KH_RAISE_INFO_ERROR("Expected a closing single quote", 4);
                                break;

                            default:
                                KH_RAISE_INFO_ERROR("Unknown escape character", 3);
                            }
                        }
                        else {
                            if (chAt(i + 2) == '\'') { KH_RAISE_INFO_ERROR("No character inserted", 2) }
                            else if (chAt(i + 1) == '\n') { KH_RAISE_INFO_ERROR("New line before byte character closing", 2) }
                            else {
                                if (chAt(i + 3) == '\'') {
                                    if (chAt(i + 2) > 255)
                                        KH_RAISE_INFO_ERROR("A non-byte sized character", 2);

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
                                    KH_RAISE_INFO_ERROR("Expected a closing single quote", 3);
                            }
                        }
                        continue;
                    }
                    else if (chAt(i + 1) == '"') {
                        state = kh::TokenizeState::IN_BUF;
                        i++;
                        char_line++;
                        continue;
                    }
                }

                state = kh::TokenizeState::IDENTIFIER;
                temp_str = chAt(i);
            }

            else if (kh::isDec(chAt(i))) {
                if (chAt(i) == '0') {
                    if (chAt(i + 1) == 'x' || chAt(i + 1) == 'X') {
                        state = kh::TokenizeState::HEX;

                        if (!kh::isHex(chAt(i + 2)))
                            KH_RAISE_INFO_ERROR("Was expecting a hexadecimal digit", 2);

                        i++;
                        char_line++;
                        continue;
                    }
                    else if (chAt(i + 1) == 'o' || chAt(i + 1) == 'O') {
                        state = kh::TokenizeState::OCTAL;

                        if (!kh::isOct(chAt(i + 2)))
                            KH_RAISE_INFO_ERROR("Was expecting an octal digit at", 2);

                        i++;
                        char_line++;
                        continue;
                    }
                    else if (chAt(i + 1) == 'b' || chAt(i + 1) == 'B') {
                        state = kh::TokenizeState::BIN;

                        if (!kh::isBin(chAt(i + 2)))
                            KH_RAISE_INFO_ERROR("Was expecting a binary digit at", 2);

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
                case '\'': {
                    if (chAt(i + 1) == '\\') {
                        switch (chAt(i + 2)) {
                        case 'x': case 'X': {
                            std::string hex_str;
                            for (size_t j = 3; j < 5; j++) {
                                if (kh::isHex(chAt(i + j)))
                                    hex_str += (char)chAt(i + j);
                                else
                                    KH_RAISE_INFO_ERROR("Expected a hexadecimal digit", j);
                            }

                            uint8 hex = std::stoul(hex_str, nullptr, 16);
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
                                KH_RAISE_INFO_ERROR("Expected a closing single quote", 5);
                        } break;

                        case 'u': {
                            std::string hex_str;
                            for (size_t j = 3; j < 7; j++) {
                                if (kh::isHex(chAt(i + j)))
                                    hex_str += (char)chAt(i + j);
                                else
                                    KH_RAISE_INFO_ERROR("Expected a hexadecimal digit", j);
                            }

                            uint16 hex = std::stoul(hex_str, nullptr, 16);
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
                                KH_RAISE_INFO_ERROR("Expected a closing single quote", 7);
                        } break;

                        case 'U': {
                            std::string hex_str;
                            for (size_t j = 3; j < 11; j++) {
                                if (kh::isHex(chAt(i + j)))
                                    hex_str += (char)chAt(i + j);
                                else
                                    KH_RAISE_INFO_ERROR("Expected a hexadecimal digit", j);
                            }

                            uint32 hex = std::stoul(hex_str, nullptr, 16);
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
                                KH_RAISE_INFO_ERROR("Expected a closing single quote", 11);
                        } break;

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
                                KH_RAISE_INFO_ERROR("Expected a closing single quote", 3);
                            break;

                        default:
                            KH_RAISE_INFO_ERROR("Unknown escape character", 2);
                        }
                    }
                    else {
                        if (chAt(i + 1) == '\'') { KH_RAISE_INFO_ERROR("No character inserted", 1); }
                        else if (chAt(i + 1) == '\n') { KH_RAISE_INFO_ERROR("New line before character closing", 1) }
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
                                KH_RAISE_INFO_ERROR("Expected a closing single quote", 2);
                        }
                    }
                    continue;
                } break;
                
                case '"':
                    state = kh::TokenizeState::IN_STR;
                    break;

                case '+': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::ADD;

                    if (chAt(i + 1) == '=') {
                        value.operator_type = kh::Operator::IADD;
                        i++;
                        char_line++;
                    }
                    else if (chAt(i + 1) == '+') {
                        value.operator_type = kh::Operator::INCREMENT;
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
                    else if (chAt(i + 1) == '-') {
                        value.operator_type = kh::Operator::DECREMENT;
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
                        KH_RAISE_INFO_ERROR("Unexpected comment close", 0);

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

                case '.': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::SCOPE;

                    if (kh::isDec(chAt(i + 1))) {
                        state = kh::TokenizeState::FLOATING;
                        temp_str = kh::toString(L"0.");
                        continue;
                    }

                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '?': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::TERNARY_IF;
                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case ':': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::TERNARY_ELSE;
                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case '$': {
                    kh::TokenValue value;
                    value.operator_type = kh::Operator::STACK_INST;
                    tokens.emplace_back(kh::TokenType::OPERATOR, value, char_line, line_n);
                } break;

                case ';': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::SEMICOLON;
                    tokens.emplace_back(kh::TokenType::SYMBOL, value, char_line, line_n);
                } break;

                case ',': {
                    kh::TokenValue value;
                    value.symbol_type = kh::Symbol::COMMA;
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
                    KH_RAISE_INFO_ERROR("Unrecognised character", 0);
                }
            continue;

        case kh::TokenizeState::IDENTIFIER:
            if (std::iswalpha(chAt(i)) > 0 || kh::isDec(chAt(i)) || chAt(i) == '_')
                temp_str += chAt(i);
            else {
                kh::TokenValue value;
                value.identifier_name = temp_str;
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

        case kh::TokenizeState::INTEGER:
            if (kh::isDec(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'u' || chAt(i) == 'U') {
                kh::TokenValue value;
                value.unsigned_integer = std::stoull(kh::fromStringW(temp_str));
                tokens.emplace_back(
                    kh::TokenType::UNSIGNED_INTEGER,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::fromStringW(temp_str));
                tokens.emplace_back(
                    kh::TokenType::IMAGINARY,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == '.') {
                temp_str += chAt(i);
                state = kh::TokenizeState::FLOATING;
            }
            else {
                kh::TokenValue value;
                value.integer = std::stoll(kh::fromStringW(temp_str));
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

        case kh::TokenizeState::FLOATING:
            if (kh::isDec(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                if (temp_str.back() == '.')
                    temp_str.pop_back();

                kh::TokenValue value;
                value.imaginary = std::stod(kh::fromStringW(temp_str));
                tokens.emplace_back(
                    kh::TokenType::IMAGINARY,
                    value,
                    char_line,
                    line_n
                );

                state = kh::TokenizeState::NONE;
            }
            else {
                if (temp_str.back() == '.')
                    temp_str.pop_back();

                kh::TokenValue value;
                value.floating = std::stod(kh::fromStringW(temp_str));
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

        case kh::TokenizeState::HEX:
            if (kh::isHex(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'u' || chAt(i) == 'U') {
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::fromStringW(temp_str), nullptr, 16);
                tokens.emplace_back(
                    kh::TokenType::UNSIGNED_INTEGER,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::fromStringW(temp_str), nullptr, 16);
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
                value.integer = std::stoull(kh::fromStringW(temp_str), nullptr, 16);
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

        case kh::TokenizeState::OCTAL:
            if (kh::isOct(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'u' || chAt(i) == 'U') {
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::fromStringW(temp_str), nullptr, 8);
                tokens.emplace_back(
                    kh::TokenType::UNSIGNED_INTEGER,
                    value,
                    char_line,
                    line_n
                );
                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::fromStringW(temp_str), nullptr, 8);
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
                value.integer = std::stoull(kh::fromStringW(temp_str), nullptr, 8);
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

        case kh::TokenizeState::BIN:
            if (kh::isBin(chAt(i)))
                temp_str += chAt(i);
            else if (chAt(i) == 'i' || chAt(i) == 'I') {
                kh::TokenValue value;
                value.imaginary = std::stoull(kh::fromStringW(temp_str), nullptr, 2);
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
                value.integer = std::stoull(kh::fromStringW(temp_str), nullptr, 2);
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

        case kh::TokenizeState::IN_BUF:
            if (chAt(i) == '"') {
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
                KH_RAISE_INFO_ERROR("Unclosed buffer string before new line", 0);
            }
            else {
                if (chAt(i) == '\\') {
                    switch (chAt(i + 1)) {
                    case 'x':
                    case 'X': {
                        std::string hex_str;
                        for (size_t j = 2; j < 4; j++) {
                            if (kh::isHex(chAt(i + j)))
                                hex_str += (char)chAt(i + j);
                            else
                                KH_RAISE_INFO_ERROR("Expected a hexadecimal digit", j);
                        }

                        temp_buf.push_back(std::stoul(hex_str, nullptr, 16));
                        i += 3;
                        char_line += 3;
                    } break;

                    case '0':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    case '\\':
                    case '"': {
                        uint8 value;

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
                        KH_RAISE_INFO_ERROR("Unknown escape character", 1);
                    }
                }
                else {
                    if (chAt(i) > 255)
                        KH_RAISE_INFO_ERROR("A non-byte sized character", 0);

                    temp_buf.push_back(chAt(i));
                }
            }
            continue;

        case kh::TokenizeState::IN_STR:
            if (chAt(i) == '"') {
                kh::TokenValue value;
                value.string = temp_str;
                tokens.emplace_back(kh::TokenType::STRING, value, char_line, line_n);

                state = kh::TokenizeState::NONE;
            }
            else if (chAt(i) == '\n') {
                KH_RAISE_INFO_ERROR("Unclosed string before new line", 0);
            }
            else {
                if (chAt(i) == '\\') {
                    switch (chAt(i + 1)) {
                    case 'x': case 'X': {
                        std::string hex_str;
                        for (size_t j = 2; j < 4; j++) {
                            if (kh::isHex(chAt(i + j)))
                                hex_str += (char)chAt(i + j);
                            else
                                KH_RAISE_INFO_ERROR("Expected a hexadecimal digit", j);
                        }

                        temp_str += std::stoul(hex_str, nullptr, 16);
                        i += 3;
                        char_line += 3;
                    } break;

                    case 'u': {
                        std::string hex_str;
                        for (size_t j = 2; j < 6; j++) {
                            if (kh::isHex(chAt(i + j)))
                                hex_str += (char)chAt(i + j);
                            else
                                KH_RAISE_INFO_ERROR("Expected a hexadecimal digit", j);
                        }

                        temp_str += std::stoul(hex_str, nullptr, 16);
                        i += 5;
                        char_line += 5;
                    } break;

                    case 'U': {
                        std::string hex_str;
                        for (size_t j = 2; j < 10; j++) {
                            if (kh::isHex(chAt(i + j)))
                                hex_str += (char)chAt(i + j);
                            else
                                KH_RAISE_INFO_ERROR("Expected a hexadecimal digit", j);
                        }

                        temp_str += std::stoul(hex_str, nullptr, 16);
                        i += 9;
                        char_line += 9;
                    } break;

                    case '0':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    case '\\':
                    case '"': {
                        uint8 value;

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
                        KH_RAISE_INFO_ERROR("Unknown escape character", 1);
                    }
                }
                else
                    temp_str += chAt(i);
            }
            continue;

        case kh::TokenizeState::IN_INLINE_COMMENT:
            if (chAt(i) == '\n') {
                state = kh::TokenizeState::NONE;
                char_line = 0;
                line_n++;
            }
            continue;

        case kh::TokenizeState::IN_MULTIPLE_LINE_COMMENT:
            if (chAt(i) == '*' && chAt(i + 1) == '/') {
                state = kh::TokenizeState::NONE;
                i++;
                char_line++;
            }
            else if (chAt(i) == '\n') {
                char_line = 0;
                line_n++;
            }
            continue;

        default:
            /* How did we get here? */
            KH_RAISE_ERROR("Got an unknown tokenize state");
        }
    }
    return tokens;
}
