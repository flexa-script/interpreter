#include "lexer.hpp"

#include <stack>
#include <stdexcept>

using namespace core;
using namespace core::parser;

Lexer::Lexer(const std::string& name, const std::string& source)
	: source(source), name(name) {
	tokenize();
}

Lexer::Lexer(const std::string& name, const std::string& source, size_t start_row, size_t start_col)
	: source(source), name(name), current_row(start_row), current_col(start_col) {
	tokenize();
}

Lexer::~Lexer() = default;

void Lexer::tokenize() {
	current_index = -1;

	advance();

	while (has_next()) {
		if (is_space()) {
			// ignore unuseful characters
			advance();
		}
		else if (current_char == '/' && (next_char == '/' || next_char == '*')) {
			process_comment();
		}
		else if (current_char == '\'') {
			start_col = current_col;
			tokens.push_back(process_char());
		}
		else if (current_char == '"') {
			start_col = current_col;
			tokens.push_back(process_string());
		}
		else if (current_char == '`') {
			start_col = current_col;
			process_multiline_string();
		}
		else if (std::isalpha(current_char) || current_char == '_') {
			start_col = current_col;
			tokens.push_back(process_identifier());
		}
		else if (std::isdigit(current_char) || current_char == '.' && std::isdigit(next_char)) {
			start_col = current_col;
			tokens.push_back(process_number());
		}
		else {
			start_col = current_col;
			tokens.push_back(process_symbol());
		}
	}

	tokens.push_back(Token(LexTokenType::TK_EOF, "EOF", current_col, current_row));
}

Token Lexer::process_comment() {
	std::string comment;
	bool is_block = false;
	int row = current_row;
	int col = current_col;

	comment += current_char;
	advance();

	if (current_char == '*') {
		is_block = true;
	}

	do {
		comment += current_char;
		advance();
	} while (has_next() && (is_block && (current_char != '/' || before_char != '*') || !is_block && current_char != '\n'));

	comment += current_char;
	advance();

	return Token(LexTokenType::TK_COMMENT, comment, row, col);
}

Token Lexer::process_string() {
	std::string str;
	bool spec = false;

	str += current_char;
	advance();

	do {
		if (current_char == '\n') {
			throw std::runtime_error(msg_header() + "missing terminating '\"' character");
		}

		if (!spec) {
			if (current_char == '\\') {
				spec = true;
				str += current_char;
			}
			else if (current_char == '"') {
				break;
			}
			else {
				str += current_char;
			}
		}
		else {
			str += current_char;
			spec = false;
		}
		advance();
	} while (has_next());

	str += current_char;
	advance();

	return Token(LexTokenType::TK_STRING_LITERAL, str, current_row, start_col);
}

size_t Lexer::find_mlv_closer(const std::string expr) {
	size_t level = 0;
	for (size_t i = 0; i < expr.size(); ++i) {
		if (level == 0 && expr[i] == '}') {
			return i;
		}
		if (expr[i] == '{') {
			++level;
		}
		if (expr[i] == '}') {
			--level;
		}
	}
	throw std::runtime_error("unterminated string literal");
}

void Lexer::process_multiline_string() {
	std::string str;
	bool spec = false;
	
	if (next_char == '\n') {
		advance();
	}

	str = '"';
	advance();

	do {
		if (!spec) {
			if (current_char == '\\') {
				spec = true;
				if (next_char != '\n') {
					str += current_char;
				}
			}
			else if (current_char == '`') {
				break;
			}
			else if (current_char == '$'
				&& next_char == '{') {
				// it's an interpolation, so closes the current string
				str += '"';
				tokens.push_back(Token(LexTokenType::TK_STRING_LITERAL, str, current_row, start_col));

				// skips ${ advancing to the start of the interpolation
				advance();
				advance();

				// get subexpression
				auto new_idx = current_index;
				auto sub_src = source.substr(current_index);
				auto lidx = find_mlv_closer(sub_src);
				new_idx += lidx;
				sub_src = sub_src.substr(0, lidx);
				// process the interpolation
				auto sub_lex = Lexer(name, sub_src, current_row, current_col);
				// encapsulate the interpolation in a string cast and concatenate to previous string
				tokens.push_back(Token(TK_ADDITIVE_OP, "+", current_row, start_col));
				tokens.push_back(Token(TK_STRING_TYPE, "string", current_row, start_col));
				tokens.push_back(Token(TK_LEFT_BRACKET, "(", current_row, start_col));
				// add tokens from sub_lex to current tokens
				for (auto t : sub_lex.tokens) {
					if (t.type != TK_EOF) {
						tokens.push_back(t);
					}
				}
				tokens.push_back(Token(TK_RIGHT_BRACKET, ")", current_row, start_col));
				tokens.push_back(Token(TK_ADDITIVE_OP, "+", current_row, start_col));
				// advance current index to the end of the interpolation
				while (current_index < new_idx) {
					advance();
				}
				// reset str to the new string
				str = '"';
			}
			else {
				str += current_char;
			}
		}
		else {
			if (current_char != '\n') {
				str += current_char;
			}
			spec = false;
		}
		advance();
	} while (has_next());

	// optional newline, if present, remove it
	if (before_char == '\n') {
		str = str.substr(0, str.size() - 1);
	}

	str += '"';

	tokens.push_back(Token(LexTokenType::TK_STRING_LITERAL, str, current_row, start_col));

	advance();
}

Token Lexer::process_char() {
	std::string chr;

	chr += current_char;
	advance();
	if (has_next() && current_char == '\\') {
		chr += current_char;
		advance();
	}
	chr += current_char;
	advance();
	if (has_next() && current_char != '\'') {
		throw std::runtime_error(msg_header() + "missing terminating ' character");
	}
	chr += current_char;
	advance();

	return Token(LexTokenType::TK_CHAR_LITERAL, chr, current_row, start_col);
}

Token Lexer::process_special_number() {
	std::string number;
	bool bin = false;
	bool oct = false;
	bool dec = false;
	bool hex = false;

	number += current_char;
	advance();

	switch (std::tolower(current_char))
	{
	case 'b':
		bin = true;
		break;
	case 'o':
		oct = true;
		break;
	case 'd':
		dec = true;
		break;
	case 'x':
		hex = true;
		break;
	default:
		break;
	}

	number += current_char;
	advance();

	while (has_next() &&
		((bin && (current_char == '0' || current_char == '1'))
		|| (oct && current_char >= '0' && current_char <= '7')
		|| (dec && std::isdigit(current_char))
		|| (hex && (std::isdigit(current_char)
			|| current_char >= 'a' && current_char <= 'f'
			|| current_char >= 'A' && current_char <= 'F')))) {
		number += current_char;
		advance();
	}

	return Token(TK_INT_LITERAL, number, current_row, start_col);
}

Token Lexer::process_number() {
	std::string number;
	LexTokenType type;
	bool has_dot = false;

	if (current_char == '0'
		&& (std::tolower(next_char) == 'b'
			|| std::tolower(next_char) == 'o'
			|| std::tolower(next_char) == 'd'
			|| std::tolower(next_char) == 'x')) {
		return process_special_number();
	}

	while (has_next() && (std::isdigit(current_char) || current_char == '.')) {
		if (current_char == '.') {
			if (has_dot) {
				throw std::runtime_error(msg_header() + "found '" + current_char + "' defining float");
			}
			has_dot = true;
		}
		number += current_char;
		advance();
	}

	if (std::tolower(current_char) == 'e') {
		has_dot = true;
		number += current_char;
		advance();
		if (current_char == '+' || current_char == '-') {
			number += current_char;
			advance();
		}
		while (has_next() && std::isdigit(current_char)) {
			number += current_char;
			advance();
		}
	}

	if (has_dot) {
		type = TK_FLOAT_LITERAL;
	}
	else if (std::tolower(current_char) == 'f') {
		type = TK_FLOAT_LITERAL;
		advance();
	}
	else {
		type = TK_INT_LITERAL;
	}

	return Token(type, number, current_row, start_col);
}

Token Lexer::process_identifier() {
	std::string identifier;
	LexTokenType type = LexTokenType::TK_ERROR;

	while (has_next() && (std::isalnum(current_char) || current_char == '_')) {
		identifier += current_char;
		advance();
	}

	for (size_t i = 0; i < TOKEN_IMAGE.size(); ++i) {
		if (identifier == TOKEN_IMAGE.at((LexTokenType)i)) {
			type = (LexTokenType)i;
			break;
		}
	}

	if (type == LexTokenType::TK_ERROR) {
		if (identifier == "true" || identifier == "false") {
			type = TK_BOOL_LITERAL;
		}
		else {
			type = TK_IDENTIFIER;
		}
	}

	return Token(type, identifier, current_row, start_col);
}

Token Lexer::process_symbol() {
	char symbol;
	std::string str_symbol;
	LexTokenType type;
	bool is_unary = false;
	bool found = false;
	bool left_c = false;

	symbol = current_char;
	str_symbol = current_char;
	advance();

	switch (symbol) {
	case '-':
		if (current_char == '-') {
			is_unary = true;
			str_symbol += current_char;
			advance();
		}
	case '+': // let fallthrough
		if (current_char == '+') {
			is_unary = true;
			str_symbol += current_char;
			advance();
		}
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
		}
		type = is_unary ? TK_INCREMENT_OP : TK_ADDITIVE_OP;
		break;

	case '~':
		type = TK_NOT;
		break;

	case '*':
		if (current_char == '*') {
			found = true;
			str_symbol += current_char;
			advance();
		}
	case '/': // let fallthrough
		if (current_char == '%' && !found) {
			str_symbol += current_char;
			advance();
		}
	case '%': // let fallthrough
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
		}
		type = TK_MULTIPLICATIVE_OP;
		break;

	case '&':
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
		}
		type = TK_BITWISE_AND;
		break;

	case '^':
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
		}
		type = TK_BITWISE_XOR;
		break;

	case '|':
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
		}
		type = TK_BITWISE_OR;
		break;

	case '<':
		left_c = true;
		if (current_char == '<') {
			found = true;
			str_symbol += current_char;
			advance();
		}
	case '>': // let fallthrough
		if (current_char == '>' && !found) {
			found = true;
			str_symbol += current_char;
			advance();
		}
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
			if (current_char == '>' && left_c && !found) {
				str_symbol += current_char;
				advance();
				type = TK_THREE_WAY_OP;
				break;
			}
		}
		type = found ? TK_BITWISE_SHIFT : TK_RELATIONAL_OP;
		break;

	case '=':
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
			type = TK_EQUALITY_OP;
		}
		else {
			type = TK_EQUALS;
		}
		break;

	case '!':
		if (current_char != '=') {
			throw std::runtime_error(msg_header() + "expected '='");
		}
		str_symbol += current_char;
		advance();
		type = TK_EQUALITY_OP;
		break;

	case '{':
		type = TK_LEFT_CURLY;
		break;

	case '}':
		type = TK_RIGHT_CURLY;
		break;

	case '[':
		type = TK_LEFT_BRACE;
		break;

	case ']':
		type = TK_RIGHT_BRACE;
		break;

	case '(':
		type = TK_LEFT_BRACKET;
		break;

	case ')':
		type = TK_RIGHT_BRACKET;
		break;

	case ',':
		type = TK_COMMA;
		break;

	case ';':
		type = TK_SEMICOLON;
		break;

	case ':':
		if (current_char == ':') {
			str_symbol += current_char;
			advance();
			type = TK_LIB_ACESSOR_OP;
		}
		else {
			type = TK_COLON;
		}
		break;

	case '.':
		if (current_char == '.') {
			str_symbol += current_char;
			advance();
			if (current_char != '.') {
				throw std::runtime_error(msg_header() + "expected '.'");
			}
			str_symbol += current_char;
			advance();
			type = TK_ELLIPSIS;
		}
		else {
			type = TK_DOT;
		}
		break;

	case '?':
		type = TK_QMARK;
		break;

	case '$':
		type = TK_DSIGN;
		break;

	default:
		type = TK_ERROR;
	}

	return Token(type, str_symbol, current_row, start_col);
}

bool Lexer::has_next() {
	return current_index < source.length();
}

bool Lexer::is_space() {
	return std::isspace(current_char) || current_char == '\t' || current_char == '\r' || current_char == '\n';
}

void Lexer::advance() {
	if (current_char == '\n') {
		current_col = 1;
		++current_row;
	}
	else {
		++current_col;
	}
	before_char = current_char;
	current_char = source[++current_index];
	if (has_next()) {
		next_char = source[current_index + 1];
	}
}

Token Lexer::next_token() {
	if (current_token < tokens.size()) {
		return tokens[current_token++];
	}
	else {
		std::string error = "final token surpassed";
		return Token(TK_ERROR, error);
	}
}

std::string Lexer::msg_header() {
	return "(LERR) " + name + '[' + std::to_string(current_row) + ':' + std::to_string(current_col) + "]: ";
}
