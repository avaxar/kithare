#include <kithare/utf8.hpp>
#include <kithare/string.hpp>


std::u32string kh::quote(const std::u32string& str) {
    std::u32string repr_str = U"\"";
    repr_str.reserve(str.size() + str.size() / 8 + 2);

    for (const char32_t chr : str) {
        switch (chr) {
            case U'"':
                repr_str += U"\\\"";
                break;
            case U'\\':
                repr_str += U"\\\\";
                break;
            case ' ':
                repr_str += U" ";
                break;
            case U'\t':
                repr_str += U"\\t";
                break;
            case U'\v':
                repr_str += U"\\v";
                break;
            case U'\n':
                repr_str += U"\\n";
                break;
            case U'\r':
                repr_str += U"\\r";
                break;
            case U'\b':
                repr_str += U"\\b";
                break;
            case U'\f':
                repr_str += U"\\f";
                break;
            case U'\a':
                repr_str += U"\\a";
                break;
            default:
                if (chr > 32 && chr < 127)
                    repr_str += (char32_t)chr;
                else {
                    std::stringstream sstream;
                    sstream << std::hex << (uint32_t)chr;

                    if (chr <= 0xff)
                        repr_str +=
                            U"\\x" + kh::repr((sstream.str().size() == 1 ? "0" : "") + sstream.str());
                    else if (chr <= 0xfff)
                        repr_str += U"\\u0" + kh::repr(sstream.str());
                    else if (chr <= 0xffff)
                        repr_str += U"\\u" + kh::repr(sstream.str());
                    else if (chr <= 0xfffff)
                        repr_str += U"\\U000" + kh::repr(sstream.str());
                    else if (chr <= 0xffffff)
                        repr_str += U"\\U00" + kh::repr(sstream.str());
                    else if (chr <= 0xfffffff)
                        repr_str += U"\\U0" + kh::repr(sstream.str());
                    else if (chr <= 0xffffffff)
                        repr_str += U"\\U" + kh::repr(sstream.str());
                }
        }
    }

    repr_str += U'"';
    return repr_str;
}

std::u32string kh::quote(const std::string& str) {
    std::u32string repr_str = U"\"";
    repr_str.reserve(str.size() + str.size() / 8 + 2);

    for (const char chr : str) {
        switch (chr) {
            case '"':
                repr_str += U"\\\"";
                break;
            case '\\':
                repr_str += U"\\\\";
                break;
            case ' ':
                repr_str += U" ";
                break;
            case '\t':
                repr_str += U"\\t";
                break;
            case '\v':
                repr_str += U"\\v";
                break;
            case '\n':
                repr_str += U"\\n";
                break;
            case '\r':
                repr_str += U"\\r";
                break;
            case '\b':
                repr_str += U"\\b";
                break;
            case '\f':
                repr_str += U"\\f";
                break;
            case '\a':
                repr_str += U"\\a";
                break;
            default:
                if (chr > 32 && chr < 127)
                    repr_str += (char32_t)chr;
                else if (chr <= 0xff) {
                    std::stringstream sstream;
                    sstream << std::hex << (int)((uint8_t)chr);
                    repr_str +=
                        U"\\x" + kh::repr((sstream.str().size() == 1 ? "0" : "") + sstream.str());
                }
        }
    }

    repr_str += U'"';
    return repr_str;
}

void kh::getLineColumn(const std::u32string& str, const size_t index, size_t& column, size_t& line) {
    column = 0;
    line = 1;

    for (size_t i = 0; i < index + 1; i++) {
        if (str[i] == '\n') {
            column = 1;
            line++;
        }
        else
            column++;
    }
}

std::u32string kh::repr(const std::u32string& str) {
    return str;
}

std::u32string kh::repr(const std::wstring& str) {
    std::u32string str32;
    str32.reserve(str.size());

    for (const wchar_t chr : str)
        str32 += (char32_t)chr;

    return str32;
}

std::u32string kh::repr(const std::string& str) {
    return kh::decodeUtf8(str);
}

std::u32string kh::repr(const char chr) {
    char32_t chr32 = chr;
    return std::u32string(&chr32, &chr32 + 1);
}

std::u32string kh::repr(const wchar_t chr) {
    char32_t chr32 = chr;
    return std::u32string(&chr32, &chr32 + 1);
}

std::u32string kh::repr(const char32_t chr) {
    return std::u32string(&chr, &chr + 1);
}

#define KH_REPR_TO_STRING(TYPE)             \
    std::u32string kh::repr(const TYPE n) { \
        return kh::repr(std::to_string(n)); \
    }

KH_REPR_TO_STRING(int64_t);
KH_REPR_TO_STRING(uint64_t);
KH_REPR_TO_STRING(size_t);
KH_REPR_TO_STRING(float);
KH_REPR_TO_STRING(double);

std::u32string kh::repr(const std::complex<float>& n) {
    return kh::repr(n.real()) + U" + " + kh::repr(n.imag()) + U"i";
}

std::u32string kh::repr(const std::complex<double>& n) {
    return kh::repr(n.real()) + U" + " + kh::repr(n.imag()) + U"i";
}
