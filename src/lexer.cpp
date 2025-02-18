#include <stack>
#include <stdexcept>

#include "lexer.hpp"

using namespace lexer;

Lexer::Lexer(const std::string& name, const std::string& source)
	: source(source), name(name) {
	tokenize();
}

Lexer::~Lexer() = default;

void Lexer::tokenize() {
	current_index = -1;
	current_row = 1;
	current_col = 0;

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

	tokens.push_back(Token(LexTokenType::TOK_EOF, "EOF", current_col, current_row));
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

	return Token(LexTokenType::TOK_COMMENT, comment, row, col);
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

	return Token(LexTokenType::TOK_STRING_LITERAL, str, current_row, start_col);
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

	str = '"';
	advance();

	do {
		if (!spec) {
			if (current_char == '\\') {
				spec = true;
				str += current_char;
			}
			else if (current_char == '`') {
				break;
			}
			else if (current_char == '$'
				&& next_char == '{') {
				str += '"';
				tokens.push_back(Token(LexTokenType::TOK_STRING_LITERAL, str, current_row, start_col));
				advance();
				advance();
				auto new_idx = current_index;
				auto sub_src = source.substr(current_index);
				auto lidx = find_mlv_closer(sub_src);
				new_idx += lidx;
				auto sub_lex = Lexer("", sub_src.substr(0, lidx));
				tokens.push_back(Token(TOK_ADDITIVE_OP, "+", current_row, start_col));
				tokens.push_back(Token(TOK_STRING_TYPE, "string", current_row, start_col));
				tokens.push_back(Token(TOK_LEFT_BRACKET, "(", current_row, start_col));
				for (auto t : sub_lex.tokens) {
					if (t.type != TOK_EOF) {
						tokens.push_back(t);
					}
				}
				tokens.push_back(Token(TOK_RIGHT_BRACKET, ")", current_row, start_col));
				tokens.push_back(Token(TOK_ADDITIVE_OP, "+", current_row, start_col));
				while (current_index < new_idx) {
					advance();
				}
				str = '"';
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

	str += '"';

	tokens.push_back(Token(LexTokenType::TOK_STRING_LITERAL, str, current_row, start_col));

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

	return Token(LexTokenType::TOK_CHAR_LITERAL, chr, current_row, start_col);
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

	return Token(TOK_INT_LITERAL, number, current_row, start_col);
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
		type = TOK_FLOAT_LITERAL;
	}
	else if (std::tolower(current_char) == 'f') {
		type = TOK_FLOAT_LITERAL;
		advance();
	}
	else {
		type = TOK_INT_LITERAL;
	}

	return Token(type, number, current_row, start_col);
}

Token Lexer::process_identifier() {
	std::string identifier;
	LexTokenType type = LexTokenType::TOK_ERROR;

	while (has_next() && (std::isalnum(current_char) || current_char == '_')) {
		identifier += current_char;
		advance();
	}

	for (size_t i = 0; i < TOKEN_IMAGE.size(); ++i) {
		if (identifier == TOKEN_IMAGE[i]) {
			type = (LexTokenType)i;
			break;
		}
	}

	if (type == LexTokenType::TOK_ERROR) {
		if (identifier == "true" || identifier == "false") {
			type = TOK_BOOL_LITERAL;
		}
		else {
			type = TOK_IDENTIFIER;
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
		type = is_unary ? TOK_INCREMENT_OP : TOK_ADDITIVE_OP;
		break;

	case '~':
		type = TOK_NOT;
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
		type = TOK_MULTIPLICATIVE_OP;
		break;

	case '&':
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
		}
		type = TOK_BITWISE_AND;
		break;

	case '^':
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
		}
		type = TOK_BITWISE_XOR;
		break;

	case '|':
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
		}
		type = TOK_BITWISE_OR;
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
				type = TOK_THREE_WAY_OP;
				break;
			}
		}
		type = found ? TOK_BITWISE_SHIFT : TOK_RELATIONAL_OP;
		break;

	case '=':
		if (current_char == '=') {
			str_symbol += current_char;
			advance();
			type = TOK_EQUALITY_OP;
		}
		else {
			type = TOK_EQUALS;
		}
		break;

	case '!':
		if (current_char != '=') {
			throw std::runtime_error(msg_header() + "expected '='");
		}
		str_symbol += current_char;
		advance();
		type = TOK_EQUALITY_OP;
		break;

	case '{':
		type = TOK_LEFT_CURLY;
		break;

	case '}':
		type = TOK_RIGHT_CURLY;
		break;

	case '[':
		type = TOK_LEFT_BRACE;
		break;

	case ']':
		type = TOK_RIGHT_BRACE;
		break;

	case '(':
		type = TOK_LEFT_BRACKET;
		break;

	case ')':
		type = TOK_RIGHT_BRACKET;
		break;

	case ',':
		type = TOK_COMMA;
		break;

	case ';':
		type = TOK_SEMICOLON;
		break;

	case ':':
		if (current_char == ':') {
			str_symbol += current_char;
			advance();
			type = TOK_LIB_ACESSOR_OP;
		}
		else {
			type = TOK_COLON;
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
			type = TOK_ELLIPSIS;
		}
		else {
			type = TOK_DOT;
		}
		break;

	case '?':
		type = TOK_QMARK;
		break;

	case '$':
		type = TOK_DSIGN;
		break;

	default:
		type = TOK_ERROR;
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
		return Token(TOK_ERROR, error);
	}
}

std::string Lexer::msg_header() {
	return "(LERR) " + name + '[' + std::to_string(current_row) + ':' + std::to_string(current_col) + "]: ";
}
