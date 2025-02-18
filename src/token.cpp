#include "token.hpp"

using namespace lexer;

Token::Token(LexTokenType type, const std::string& value, unsigned int row, unsigned int col)
	: type(type), value(value), row(row), col(col) {}

Token::Token()
	: type(LexTokenType::TOK_ERROR), value(""), row(0), col(0) {}

const std::string& Token::token_image(LexTokenType type) {
	return TOKEN_IMAGE[type];
}

// assignment

bool Token::is_assignment_op(const std::string& op) {
	return is_assignment_collection_op(op)
		|| is_assignment_float_op(op)
		|| is_assignment_int_op(op)
		|| is_assignment_int_ex_op(op);
}

bool Token::is_assignment_collection_op(const std::string& op) {
	return op == "=" || op == "+=";
}

bool Token::is_assignment_float_op(const std::string& op) {
	return is_assignment_collection_op(op)
		|| op == "-=" || op == "*=" || op == "/="
		|| op == "**=" || op == "/%=" || op == "%=";
}

bool Token::is_assignment_int_op(const std::string& op) {
	return is_assignment_float_op(op)
		|| is_assignment_int_ex_op(op);
}

bool Token::is_assignment_int_ex_op(const std::string& op) {
	return op == "|=" || op == "^=" || op == "&="
		|| op == "<<=" || op == ">>=";
}

// expression

bool Token::is_expression_collection_op(const std::string& op) {
	return op == "+";
}

bool Token::is_expression_int_op(const std::string& op) {
	return is_expression_float_op(op)
		|| is_expression_int_ex_op(op);
}

bool Token::is_expression_int_ex_op(const std::string& op) {
	return op == "|" || op == "^" || op == "&"
		|| op == "<<" || op == ">>";
}

bool Token::is_expression_float_op(const std::string& op) {
	return is_expression_collection_op(op)
		|| op == "-" || op == "*" || op == "/"
		|| op == "**" || op == "/%" || op == "%";
}

// is operator

bool Token::is_equality_op(const std::string& op) {
	return op == "==" || op == "!=";
}

bool Token::is_relational_op(const std::string& op) {
	return op == "<=" || op == ">="
		|| op == "<" || op == ">"
		|| op == "<=>";
}

bool Token::is_int_op(const std::string& op) {
	return is_assignment_int_op(op)
		|| is_expression_int_op(op);
}

bool Token::is_int_ex_op(const std::string& op) {
	return is_assignment_int_ex_op(op)
		|| is_expression_int_ex_op(op);
}

bool Token::is_float_op(const std::string& op) {
	return is_assignment_float_op(op)
		|| is_expression_float_op(op);
}

bool Token::is_collection_op(const std::string& op) {
	return is_assignment_collection_op(op)
		|| is_expression_collection_op(op);
}
