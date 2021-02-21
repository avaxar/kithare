#include "parser/parse.hpp"


kh::Parser::Parser(const kh::String& source, const kh::String& file_name) {
    this->filename = file_name;
    this->lex_tokens = kh::lex(source, file_name);
}

kh::Parser::Parser(const std::vector<kh::Token>& tokens, const kh::String& file_name) {
    this->filename = file_name;
    this->lex_tokens = tokens;
}

kh::Parser::~Parser() {
    for (kh::AstExpression* expression : this->ast)
        if (expression != nullptr)
            delete expression;
}

void kh::Parser::parse() {
    for (this->it = 0; this->it < this->lex_tokens.size(); this->it++) {
        
    }
}
