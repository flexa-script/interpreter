#ifndef PARSER_HPP
#define PARSER_HPP

#include <stack>

#include "ast.hpp"
#include "lexer.hpp"

using namespace lexer;

namespace parser {

	class Parser {
	private:
		Lexer* lex;
		Token current_token;
		Token next_token;
		Type current_array_type = Type::T_UNDEFINED;
		std::stack<bool> consume_semicolon;

	public:
		std::string name;

	public:
		explicit Parser(const std::string& name, Lexer* lex);

		std::shared_ptr<ASTProgramNode> parse_program();

	private:
		// parse types and parameters
		Type parse_type();
		TypeDefinition parse_unpacked_type_definition();
		TypeDefinition parse_function_type_definition();
		TypeDefinition parse_declaration_type_definition(Type ptype = Type::T_UNDEFINED);

		// statement nodes
		std::shared_ptr<ASTNode> parse_program_statement();
		std::shared_ptr<ASTNamespaceManagerNode> parse_namespace_manager_statement();
		std::shared_ptr<ASTUsingNode> parse_using_statement();
		std::shared_ptr<ASTNode> parse_block_statement();
		std::shared_ptr<ASTDeclarationNode> parse_declaration_statement();
		std::shared_ptr<ASTStatementNode> parse_unpacked_declaration_statement();
		std::shared_ptr<ASTAssignmentNode> parse_assignment_statement(std::shared_ptr<ASTIdentifierNode> idnode);
		std::shared_ptr<ASTUnaryExprNode> parse_increment_expression(std::shared_ptr<ASTIdentifierNode> idnode);
		std::shared_ptr<ASTReturnNode> parse_return_statement();
		std::shared_ptr<ASTExitNode> parse_exit_statement();
		std::shared_ptr<ASTEnumNode> parse_enum_statement();
		std::shared_ptr<ASTBlockNode> parse_block();
		std::shared_ptr<ASTBlockNode> parse_struct_block();
		std::shared_ptr<ASTStatementNode> parse_struct_block_variables();
		std::shared_ptr<ASTContinueNode> parse_continue_statement();
		std::shared_ptr<ASTBreakNode> parse_break_statement();
		std::shared_ptr<ASTSwitchNode> parse_switch_statement();
		std::shared_ptr<ASTElseIfNode> parse_else_if_statement();
		std::shared_ptr<ASTIfNode> parse_if_statement();
		std::shared_ptr<ASTTryCatchNode> parse_try_catch_statement();
		std::shared_ptr<ASTThrowNode> parse_throw_statement();
		std::shared_ptr<ASTForNode> parse_for_statement();
		std::shared_ptr<ASTNode> parse_foreach_collection();
		std::shared_ptr<ASTForEachNode> parse_foreach_statement();
		std::shared_ptr<ASTWhileNode> parse_while_statement();
		std::shared_ptr<ASTDoWhileNode> parse_do_while_statement();
		std::shared_ptr<ASTLambdaFunction> parse_function_expression();
		std::shared_ptr<ASTFunctionDefinitionNode> parse_function_statement();
		std::shared_ptr<ASTFunctionDefinitionNode> parse_function_definition(const std::string& identifier);
		std::shared_ptr<ASTStructDefinitionNode> parse_struct_definition();

		// expression nodes
		std::shared_ptr<ASTExprNode> parse_statement_expression();
		std::shared_ptr<ASTExprNode> parse_expression();
		std::shared_ptr<ASTExprNode> parse_ternary_expression();
		std::shared_ptr<ASTExprNode> parse_in_expression();
		std::shared_ptr<ASTExprNode> parse_logical_or_expression();
		std::shared_ptr<ASTExprNode> parse_logical_and_expression();
		std::shared_ptr<ASTExprNode> parse_bitwise_or_expression(); 
		std::shared_ptr<ASTExprNode> parse_bitwise_xor_expression();
		std::shared_ptr<ASTExprNode> parse_bitwise_and_expression();
		std::shared_ptr<ASTExprNode> parse_equality_expression();
		std::shared_ptr<ASTExprNode> parse_relational_expression();
		std::shared_ptr<ASTExprNode> parse_spaceship_expression();
		std::shared_ptr<ASTExprNode> parse_bitwise_shift_expression();
		std::shared_ptr<ASTExprNode> parse_simple_expression();
		std::shared_ptr<ASTExprNode> parse_term();
		std::shared_ptr<ASTExprNode> parse_exponentiation();
		std::shared_ptr<ASTExprNode> parse_factor();

		// factor expression nodes
		std::shared_ptr<ASTNode> parse_identifier_statement();
		std::shared_ptr<ASTArrayConstructorNode> parse_array_constructor_node();
		std::shared_ptr<ASTStructConstructorNode> parse_struct_constructor_node(std::shared_ptr<ASTIdentifierNode> idnode);
		std::shared_ptr<ASTFunctionCallNode> parse_function_call_node(std::shared_ptr<ASTIdentifierNode> idnode);
		std::shared_ptr<ASTThisNode> parse_this_node();
		std::shared_ptr<ASTTypeCastNode> parse_type_parse_node();
		std::shared_ptr<ASTTypingNode> parse_typing_node();

		// special
		std::vector<std::shared_ptr<ASTExprNode>> parse_actual_params();
		VariableDefinition* parse_struct_var_def();
		VariableDefinition* parse_formal_param();
		VariableDefinition* parse_unpacked_formal_param();
		std::shared_ptr<ASTExprNode> parse_identifier_expression();
		std::shared_ptr<ASTIdentifierNode> parse_identifier_node();
		flx_bool parse_bool_literal();
		flx_int parse_int_literal();
		flx_float parse_float_literal();
		flx_char parse_char_literal();
		flx_string parse_string_literal();
		std::vector<std::shared_ptr<ASTExprNode>> parse_dimension_vector();
		Identifier parse_identifier();
		std::vector<Identifier> parse_identifier_vector();

		void consume_token();
		void consume_token(LexTokenType type);
		void check_current_token(LexTokenType type) const;
		void check_consume_semicolon();

		std::string msg_header() const;

	};
}

#endif // !PARSER_HPP
