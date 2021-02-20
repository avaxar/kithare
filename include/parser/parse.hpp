#pragma once

#include "parser/ast.hpp"
#include "lexer/token.hpp"
#include "lexer/lex.hpp"


extern bool SILENT_COMPILATION;

namespace kh {
    class Parser {
    private:
        std::vector<kh::AstExpression*> ast;
		std::vector<kh::Token> lex_tokens;
		kh::String filename;

		size_t it = 0;
		inline kh::Token tAt(const size_t index) {
			if (index >= this->lex_tokens.size()) {
				if (!SILENT_COMPILATION)
					std::wcerr << "Reached EOF while searching for a token in \"" << this->filename << "\"\n";
				std::exit(1);
			}
			else
				return this->lex_tokens[index];
		}

		Parser(const kh::Parser& copy) {}
	public:
		Parser(const kh::String& source, const kh::String& file_name);
		Parser(const std::vector<kh::Token>& tokens, const kh::String& file_name);
		~Parser();
		
		void parse();

		inline std::vector<kh::AstExpression*> getAst() { return this->ast; }
		inline std::vector<kh::Token> getTokens() const { return this->lex_tokens; }
		inline kh::String getFileName() const { return this->filename; }
	};
}
