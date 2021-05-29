#include <kithare/parser.hpp>


std::u32string kh::LexException::format() const {
    return this->what + U" at index " + kh::repr(this->index);
}

std::u32string kh::ParseException::format() const {
    return this->what + U" at index " + kh::repr(this->index);
}

kh::Parser::Parser() {}

kh::Parser::Parser(const std::u32string& _source) : source(_source) {}

kh::Parser::Parser(const std::vector<kh::Token>& _tokens) : tokens(_tokens) {}

kh::Parser::~Parser() {}

void kh::Parser::cleanExceptions() {
    size_t last_index = -1;
    std::vector<kh::ParseException> cleaned_exceptions;
    cleaned_exceptions.reserve(this->parse_exceptions.size());

    for (kh::ParseException& exc : this->parse_exceptions) {
        if (last_index != exc.index)
            cleaned_exceptions.push_back(exc);

        last_index = exc.index;
    }

    this->parse_exceptions = cleaned_exceptions;
}
