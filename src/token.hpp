#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

#include "token_constants.hpp"

namespace core {

	class Token {
	public:
		LexTokenType type;
		std::string value;
		size_t row;
		size_t col;

		Token(LexTokenType type, const std::string& value, size_t row = 0, size_t col = 0);
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

}

#endif // !TOKEN_HPP
