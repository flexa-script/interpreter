#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "token_constants.hpp"

namespace lexer {

	class Token {
	public:
		LexTokenType type;
		std::string value;
		unsigned int row;
		unsigned int col;

		Token(LexTokenType type, const std::string& value, unsigned int row = 0, unsigned int col = 0);
		Token();

		static const std::string& token_image(LexTokenType type);

		static bool is_assignment_op(const std::string& op);
		static bool is_assignment_collection_op(const std::string& op);
		static bool is_assignment_int_op(const std::string& op);
		static bool is_assignment_int_ex_op(const std::string& op);
		static bool is_assignment_float_op(const std::string& op);

		static bool is_expression_collection_op(const std::string& op);
		static bool is_expression_int_op(const std::string& op);
		static bool is_expression_int_ex_op(const std::string& op);
		static bool is_expression_float_op(const std::string& op);

		static bool is_equality_op(const std::string& op);
		static bool is_relational_op(const std::string& op);
		static bool is_collection_op(const std::string& op);
		static bool is_int_op(const std::string& op);
		static bool is_int_ex_op(const std::string& op);
		static bool is_float_op(const std::string& op);
	};

};

#endif // !TOKEN_HPP
