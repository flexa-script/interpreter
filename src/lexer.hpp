#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <cstdint>

#include "token.hpp"

namespace core {

	namespace parser {

		class Lexer {
		public:
			Lexer(const std::string& name, const std::string& source);
			Lexer(const std::string& name, const std::string& source, size_t start_row, size_t start_col);
			~Lexer();

			Token next_token();

		private:
			char before_char;
			char current_char;
			char next_char;
			size_t current_token = 0;
			intmax_t current_index = 0;
			size_t current_row = 1;
			size_t start_col = 0;
			size_t current_col = 0;
			std::string source;
			std::string name;
			std::vector<Token> tokens;

			void tokenize();
			bool has_next();
			bool is_space();
			void advance();
			Token process_identifier();
			Token process_special_number();
			Token process_number();
			Token process_char();
			Token process_string();
			void process_multiline_string();
			Token process_symbol();
			Token process_comment();

			std::string msg_header();

			static size_t find_mlv_closer(const std::string expr);
		};

	}

}

#endif // !LEXER_HPP
