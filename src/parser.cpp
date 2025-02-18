#include <cmath>
#include <iostream>

#include "parser.hpp"
#include "utils.hpp"
#include "visitor.hpp"

using namespace lexer;
using namespace parser;
using namespace visitor;

Parser::Parser(const std::string& name, Lexer* lex) : name(name), lex(lex) {
	current_token = lex->next_token();
	next_token = lex->next_token();
}

std::shared_ptr<ASTProgramNode> Parser::parse_program() {
	auto statements = std::vector<std::shared_ptr<ASTNode>>();
	std::string name_space = default_namespace;

	if (current_token.type == TOK_NAMESPACE) {
		consume_token(TOK_IDENTIFIER);
		name_space = current_token.value;
		consume_token(TOK_SEMICOLON);
		consume_token();
	}

	while (current_token.type != TOK_EOF) {
		statements.push_back(parse_program_statement());
		consume_token();
	}

	return std::make_shared<ASTProgramNode>(name, name_space, statements);
}

std::shared_ptr<ASTUsingNode> Parser::parse_using_statement() {
	auto library = std::vector<std::string>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	do {
		consume_token(TOK_IDENTIFIER);
		library.push_back(current_token.value);
		if (next_token.type == TOK_DOT) {
			consume_token();
		}
	} while (next_token.type == TOK_IDENTIFIER);

	consume_token(TOK_SEMICOLON);

	return std::make_shared<ASTUsingNode>(library, row, col);
}

std::shared_ptr<ASTNode> Parser::parse_program_statement() {
	switch (current_token.type) {
	case TOK_USING:
		return parse_using_statement();
	case TOK_INCLUDE:
	case TOK_EXCLUDE:
		return parse_namespace_manager_statement();
	case TOK_FUN:
		return parse_function_statement();
	default:
		consume_semicolon.push(true);
		std::shared_ptr<ASTNode> node = parse_block_statement();
		consume_semicolon.pop();
		return node;
	}
}

std::shared_ptr<ASTNode> Parser::parse_block_statement() {
	switch (current_token.type) {
	case TOK_ENUM:
		return parse_enum_statement();
	case TOK_VAR:
	case TOK_CONST:
		return parse_unpacked_declaration_statement();
	case TOK_STRUCT:
		return parse_struct_definition();
	case TOK_TRY:
		return parse_try_catch_statement();
	case TOK_THROW:
		return parse_throw_statement();
	case TOK_SWITCH:
		return parse_switch_statement();
	case TOK_IF:
		return parse_if_statement();
	case TOK_WHILE:
		return parse_while_statement();
	case TOK_DO:
		return parse_do_while_statement();
	case TOK_FOR:
		return parse_for_statement();
	case TOK_FOREACH:
		return parse_foreach_statement();
	case TOK_CONTINUE:
		return parse_continue_statement();
	case TOK_BREAK:
		return parse_break_statement();
	case TOK_EXIT:
		return parse_exit_statement();
	case TOK_RETURN:
		return parse_return_statement();
	case TOK_IDENTIFIER:
		return parse_identifier_statement();
	default:
		try {
			return parse_statement_expression();
		}
		catch (const std::exception& ex) {
			throw std::runtime_error(msg_header() + "expected statement or expression");
		}
	}
}

std::shared_ptr<ASTNamespaceManagerNode> Parser::parse_namespace_manager_statement() {
	std::string image = current_token.value;
	std::string name_space = "";
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token();
	consume_token(TOK_IDENTIFIER);
	name_space = current_token.value;
	consume_token(TOK_SEMICOLON);

	return std::make_shared<ASTNamespaceManagerNode>(image, name_space, row, col);
}

std::shared_ptr<ASTExprNode> Parser::parse_statement_expression() {
	std::shared_ptr<ASTExprNode> expr = parse_expression();
	if (next_token.type != LexTokenType::TOK_EOF) {
		check_consume_semicolon();
	}
	return expr;
}

std::shared_ptr<ASTReturnNode> Parser::parse_return_statement() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	std::shared_ptr<ASTExprNode> expr = nullptr;

	if (next_token.type != TOK_SEMICOLON) {
		consume_token();
		expr = parse_expression();
	}

	check_consume_semicolon();

	return std::make_shared<ASTReturnNode>(expr, row, col);
}

std::shared_ptr<ASTExitNode> Parser::parse_exit_statement() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_BRACKET);

	consume_token();
	std::shared_ptr<ASTExprNode> expr = parse_expression();

	consume_token(TOK_RIGHT_BRACKET);

	check_consume_semicolon();

	return std::make_shared<ASTExitNode>(expr, row, col);
}

std::shared_ptr<ASTEnumNode> Parser::parse_enum_statement() {
	auto identifiers = std::vector<std::string>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_CURLY);

	while (next_token.type != TOK_RIGHT_CURLY
		&& next_token.type != TOK_ERROR
		&& next_token.type != TOK_EOF) {
		consume_token(TOK_IDENTIFIER);
		identifiers.push_back(current_token.value);
		if (next_token.type == TOK_COMMA) {
			consume_token();
		}
	}

	consume_token(TOK_RIGHT_CURLY);
	check_consume_semicolon();

	return std::make_shared<ASTEnumNode>(identifiers, row, col);
}

std::shared_ptr<ASTBlockNode> Parser::parse_block() {
	auto statements = std::vector<std::shared_ptr<ASTNode>>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token();
	while (current_token.type != TOK_RIGHT_CURLY
		&& current_token.type != TOK_ERROR
		&& current_token.type != TOK_EOF) {
		consume_semicolon.push(true);
		statements.push_back(parse_block_statement());
		consume_semicolon.pop();
		consume_token();
	}

	if (current_token.type == TOK_RIGHT_CURLY) {
		return std::make_shared<ASTBlockNode>(statements, row, col);
	}
	throw std::runtime_error(msg_header() + "reached end of file while parsing");
}

std::shared_ptr<ASTBlockNode> Parser::parse_struct_block() {
	auto statements = std::vector<std::shared_ptr<ASTNode>>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token();
	while (current_token.type != TOK_RIGHT_CURLY
		&& current_token.type != TOK_ERROR
		&& current_token.type != TOK_EOF) {
		statements.push_back(parse_struct_block_variables());
		consume_token();
	}

	if (current_token.type == TOK_RIGHT_CURLY) {
		return std::make_shared<ASTBlockNode>(statements, row, col);
	}
	throw std::runtime_error(msg_header() + "mismatched scopes: reached end of file while parsing");
}

std::shared_ptr<ASTStatementNode> Parser::parse_struct_block_variables() {
	switch (current_token.type) {
	case TOK_VAR:
		return parse_declaration_statement();

	default:
		throw std::runtime_error(msg_header() + "invalid declaration starting with '" + current_token.value + "' encountered");
	}
}

TypeDefinition Parser::parse_declaration_type_definition(Type ptype) {
	Type type = ptype;
	Type array_type = Type::T_UNDEFINED;
	auto dim = std::vector<std::shared_ptr<ASTExprNode>>();
	std::string type_name;
	std::string type_name_space;

	if (next_token.type == TOK_COLON) {
		consume_token();
		consume_token();

		if (next_token.type == TOK_LIB_ACESSOR_OP) {
			type_name_space = current_token.value;
			consume_token();
			consume_token();
		}

		if (type == Type::T_UNDEFINED) {
			type = parse_type();
		}
		else if (type == Type::T_ARRAY) {
			current_array_type = parse_type();

			if (current_array_type == parser::Type::T_UNDEFINED
				|| current_array_type == parser::Type::T_VOID
				|| current_array_type == parser::Type::T_ARRAY) {
				current_array_type = parser::Type::T_ANY;
			}

			array_type = current_array_type;
		}

		if (current_token.type == TOK_IDENTIFIER) {
			type_name = current_token.value;
		}
	}

	if (type == Type::T_UNDEFINED) {
		type = Type::T_ANY;
	}

	return TypeDefinition(type, array_type, dim, type_name, type_name_space);
}

VariableDefinition* Parser::parse_struct_var_def() {
	bool is_rest = false;
	std::string identifier;
	Type type = Type::T_UNDEFINED;
	TypeDefinition type_def = Type::T_UNDEFINED;
	std::shared_ptr<ASTExprNode> expr_size;
	auto dim = std::vector<std::shared_ptr<ASTExprNode>>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	if (current_token.type == TOK_ELLIPSIS) {
		is_rest = true;
		consume_token(TOK_IDENTIFIER);
	}
	else {
		check_current_token(TOK_IDENTIFIER);
	}

	auto id = parse_identifier();
	identifier = id.identifier;
	dim = id.access_vector;

	if (dim.size() > 0) {
		type = Type::T_ARRAY;
	}

	type_def = parse_declaration_type_definition(type);

	if (is_rest) {
		auto ndim = std::make_shared<ASTLiteralNode<flx_int>>(0, row, col);
		if (!is_array(type_def.type)) {
			type_def.array_type = type;
			type_def.type = Type::T_ARRAY;
		}
		dim.insert(dim.begin(), ndim);
	}

	return new VariableDefinition(identifier, type_def.type, type_def.type_name,
		type_def.type_name_space, type_def.array_type, dim, nullptr, is_rest, row, col);
};

std::shared_ptr<ASTContinueNode> Parser::parse_continue_statement() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	check_consume_semicolon();

	return std::make_shared<ASTContinueNode>(row, col);
}

std::shared_ptr<ASTBreakNode> Parser::parse_break_statement() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	check_consume_semicolon();

	return std::make_shared<ASTBreakNode>(row, col);
}

std::shared_ptr<ASTSwitchNode> Parser::parse_switch_statement() {
	std::shared_ptr<ASTExprNode> condition;
	std::map<std::shared_ptr<ASTExprNode>, unsigned int> case_blocks = std::map<std::shared_ptr<ASTExprNode>, unsigned int>();
	long default_block = 0;
	std::vector<std::shared_ptr<ASTNode>> statements = std::vector<std::shared_ptr<ASTNode>>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_BRACKET);
	consume_token();
	condition = parse_expression();
	consume_token(TOK_RIGHT_BRACKET);
	consume_token(TOK_LEFT_CURLY);
	consume_token();

	while (current_token.type == TOK_CASE) {
		bool is_block = false;
		consume_token();
		std::shared_ptr<ASTExprNode> case_exrp = parse_expression();
		int start_position = statements.size();
		consume_token();
		consume_token();

		if (current_token.type == TOK_LEFT_CURLY) {
			statements.push_back(parse_block());
			consume_token();
		}
		else {
			while (current_token.type != TOK_CASE && current_token.type != TOK_DEFAULT
				&& current_token.type != TOK_RIGHT_CURLY && current_token.type != TOK_ERROR
				&& current_token.type != TOK_EOF) {
				consume_semicolon.push(true);
				statements.push_back(parse_block_statement());
				consume_semicolon.pop();
				consume_token();
			}
		}
		case_blocks.emplace(case_exrp, start_position);
	}

	if (current_token.type == TOK_DEFAULT) {
		bool is_block = false;
		default_block = statements.size();
		consume_token();
		consume_token();
		if (current_token.type == TOK_LEFT_CURLY) {
			statements.push_back(parse_block());
			consume_token();
		}
		else {
			while (current_token.type != TOK_RIGHT_CURLY && current_token.type != TOK_ERROR && current_token.type != TOK_EOF) {
				consume_semicolon.push(true);
				statements.push_back(parse_block_statement());
				consume_semicolon.pop();
				consume_token();
			}
		}
	}

	default_block = statements.size();

	return std::make_shared<ASTSwitchNode>(condition, statements, case_blocks, default_block, row, col);
}

std::shared_ptr<ASTElseIfNode> Parser::parse_else_if_statement() {
	std::shared_ptr<ASTExprNode> condition;
	std::shared_ptr<ASTBlockNode> if_block;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_BRACKET);
	consume_token();
	condition = parse_expression();
	consume_token(TOK_RIGHT_BRACKET);
	consume_token(TOK_LEFT_CURLY);
	if_block = parse_block();

	return std::make_shared<ASTElseIfNode>(condition, if_block, row, col);
}

std::shared_ptr<ASTIfNode> Parser::parse_if_statement() {
	std::shared_ptr<ASTExprNode> condition;
	std::shared_ptr<ASTBlockNode> if_block;
	std::vector<std::shared_ptr<ASTElseIfNode>> else_ifs = std::vector<std::shared_ptr<ASTElseIfNode>>();
	std::shared_ptr<ASTBlockNode> else_block = nullptr;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_BRACKET);
	consume_token();
	condition = parse_expression();
	consume_token(TOK_RIGHT_BRACKET);
	consume_token(TOK_LEFT_CURLY);
	if_block = parse_block();

	if (next_token.type == TOK_ELSE) {
		consume_token();

		if (next_token.type == TOK_IF) {
			do {
				consume_token(TOK_IF);
				else_ifs.push_back(parse_else_if_statement());
				if (next_token.type == TOK_ELSE) {
					consume_token(TOK_ELSE);
				}
			} while (next_token.type == TOK_IF);
		}

		if (current_token.type == TOK_ELSE) {
			consume_token(TOK_LEFT_CURLY);
			else_block = parse_block();
		}
	}

	return std::make_shared<ASTIfNode>(condition, if_block, else_ifs, else_block, row, col);
}

std::shared_ptr<ASTTryCatchNode> Parser::parse_try_catch_statement() {
	std::shared_ptr<ASTStatementNode> decl;
	std::shared_ptr<ASTBlockNode> try_block;
	std::shared_ptr<ASTBlockNode> catch_block;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_CURLY);
	try_block = parse_block();
	check_current_token(TOK_RIGHT_CURLY);

	consume_token(TOK_CATCH);
	consume_token(TOK_LEFT_BRACKET);
	consume_token();
	if (current_token.type == TOK_ELLIPSIS) {
		decl = std::make_shared<ASTEllipsisNode>(row, col);
	}
	else {
		check_current_token(TOK_VAR);
		consume_semicolon.push(false);
		decl = parse_unpacked_declaration_statement();
		consume_semicolon.pop();
	}
	consume_token(TOK_RIGHT_BRACKET);

	consume_token(TOK_LEFT_CURLY);
	catch_block = parse_block();
	check_current_token(TOK_RIGHT_CURLY);

	return std::make_shared<ASTTryCatchNode>(decl, try_block, catch_block, row, col);
}

std::shared_ptr<ASTThrowNode> Parser::parse_throw_statement() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	std::shared_ptr<ASTExprNode> expr;

	consume_token();
	expr = parse_expression();
	check_consume_semicolon();

	return std::make_shared<ASTThrowNode>(expr, row, col);
}

std::shared_ptr<ASTForNode> Parser::parse_for_statement() {
	std::array<std::shared_ptr<ASTNode>, 3> dci = { nullptr };
	std::shared_ptr<ASTBlockNode> block;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_BRACKET);

	for (int i = 0; i < 3; ++i) {
		if (next_token.type != TOK_SEMICOLON && next_token.type != TOK_RIGHT_BRACKET) {
			consume_token();
			consume_semicolon.push(i < 2);
			dci[i] = parse_block_statement();
			consume_semicolon.pop();

		}
		else {
			if (i < 2) {
				consume_token();
			}
		}
	}

	consume_token(TOK_RIGHT_BRACKET);
	consume_token(TOK_LEFT_CURLY);

	block = parse_block();

	return std::make_shared<ASTForNode>(dci, block, row, col);
}

std::shared_ptr<ASTNode> Parser::parse_foreach_collection() {
	std::shared_ptr<ASTNode> node = nullptr;

	consume_semicolon.push(false);
	switch (current_token.type) {
	case TOK_LEFT_CURLY:
		node = parse_array_constructor_node();
		break;
	case TOK_IDENTIFIER:
		node = parse_identifier_statement();
		break;
	default:
		throw std::runtime_error(msg_header() + "expected array");
	}
	consume_semicolon.pop();

	return node;
}

std::shared_ptr<ASTForEachNode> Parser::parse_foreach_statement() {
	std::shared_ptr<ASTStatementNode> itdecl;
	std::shared_ptr<ASTNode> collection;
	std::shared_ptr<ASTBlockNode> block;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_BRACKET);
	consume_token();
	consume_semicolon.push(false);
	check_current_token(TOK_VAR);
	itdecl = parse_unpacked_declaration_statement();
	consume_token(TOK_IN);
	consume_token();
	collection = parse_foreach_collection();
	consume_semicolon.pop();
	consume_token(TOK_RIGHT_BRACKET);
	consume_token(TOK_LEFT_CURLY);
	block = parse_block();

	return std::make_shared<ASTForEachNode>(itdecl, collection, block, row, col);
}

std::shared_ptr<ASTWhileNode> Parser::parse_while_statement() {
	std::shared_ptr<ASTExprNode> condition;
	std::shared_ptr<ASTBlockNode> block;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_BRACKET);
	consume_token();
	condition = parse_expression();
	consume_token(TOK_RIGHT_BRACKET);
	consume_token(TOK_LEFT_CURLY);
	block = parse_block();

	return std::make_shared<ASTWhileNode>(condition, block, row, col);
}

std::shared_ptr<ASTDoWhileNode> Parser::parse_do_while_statement() {
	std::shared_ptr<ASTExprNode> condition;
	std::shared_ptr<ASTBlockNode> block;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_CURLY);
	block = parse_block();
	check_current_token(TOK_RIGHT_CURLY);
	consume_token(TOK_WHILE);
	consume_token();
	condition = parse_expression();
	check_current_token(TOK_RIGHT_BRACKET);
	consume_semicolon.push(true);
	check_consume_semicolon();
	consume_semicolon.pop();

	return std::make_shared<ASTDoWhileNode>(condition, block, row, col);
}

std::shared_ptr<ASTLambdaFunction> Parser::parse_function_expression() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	consume_token();
	return std::make_shared<ASTLambdaFunction>(parse_function_definition(""), row, col);
}

std::shared_ptr<ASTFunctionDefinitionNode> Parser::parse_function_statement() {
	consume_token(TOK_IDENTIFIER);
	std::string identifier = current_token.value;
	consume_token(TOK_LEFT_BRACKET);
	return parse_function_definition(identifier);
}

TypeDefinition Parser::parse_unpacked_type_definition() {
	Type type = Type::T_STRUCT;
	std::string type_name = "";
	std::string type_name_space = "";

	if (current_token.type == TOK_COLON) {
		consume_token();
		if (next_token.type == TOK_LIB_ACESSOR_OP) {
			type_name_space = current_token.value;
			consume_token();
			consume_token();
		}
		if (current_token.type != TOK_IDENTIFIER && current_token.type != TOK_ANY_TYPE) {
			throw std::runtime_error(msg_header() + "expected struct type");
		}
		if (current_token.type == TOK_IDENTIFIER) {
			type_name = current_token.value;
		}
		type = parse_type();
	}
	else {
		type = Type::T_ANY;
	}

	return TypeDefinition(type, Type::T_UNDEFINED, std::vector<std::shared_ptr<ASTExprNode>>(), type_name, type_name_space);
}

TypeDefinition Parser::parse_function_type_definition() {
	Type type = Type::T_VOID;
	Type array_type = parser::Type::T_UNDEFINED;
	auto dim_vector = std::vector<std::shared_ptr<ASTExprNode>>();
	std::string type_name = "";
	std::string type_name_space = "";

	consume_token();
	if (current_token.type == TOK_COLON) {
		consume_token();
		if (next_token.type == TOK_LIB_ACESSOR_OP) {
			type_name_space = current_token.value;
			consume_token();
			consume_token();
		}
		if (current_token.type != TOK_BOOL_TYPE && current_token.type != TOK_INT_TYPE
			&& current_token.type != TOK_FLOAT_TYPE && current_token.type != TOK_CHAR_TYPE
			&& current_token.type != TOK_STRING_TYPE && current_token.type != TOK_IDENTIFIER
			&& current_token.type != TOK_FUNCTION_TYPE && current_token.type != TOK_VOID_TYPE
			&& current_token.type != TOK_ANY_TYPE) {
			throw std::runtime_error(msg_header() + "expected type");
		}
		if (current_token.type == TOK_IDENTIFIER) {
			type_name = current_token.value;
		}
		type = parse_type();
		consume_token();
		dim_vector = parse_dimension_vector();
		if (dim_vector.size() > 0) {
			array_type = type;
			type = Type::T_ARRAY;
			consume_token();
		}
	}
	else {
		type = Type::T_VOID;
	}

	return TypeDefinition(type, array_type, dim_vector, type_name, type_name_space);
}

std::shared_ptr<ASTFunctionDefinitionNode> Parser::parse_function_definition(const std::string& identifier) {
	std::vector<TypeDefinition*> parameters;
	TypeDefinition type_def;
	std::shared_ptr<ASTBlockNode> block = nullptr;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	if (next_token.type != TOK_RIGHT_BRACKET) {
		do {
			consume_token();
			if (current_token.type == TOK_LEFT_BRACE) {
				TypeDefinition unpack_typedef;
				std::vector<VariableDefinition> unpack_parameters;
				do {
					consume_token();
					unpack_parameters.push_back(*parse_unpacked_formal_param());
					consume_token();
				} while (current_token.type == TOK_COMMA);
				unpack_typedef = parse_unpacked_type_definition();
				parameters.push_back(new UnpackedVariableDefinition(unpack_typedef, unpack_parameters));
			}
			else {
				parameters.push_back(parse_formal_param());
			}
			consume_token();
		} while (current_token.type == TOK_COMMA);
		check_current_token(TOK_RIGHT_BRACKET);
	}
	else {
		consume_token();
	}

	type_def = parse_function_type_definition();

	if (current_token.type == TOK_LEFT_CURLY) {
		block = parse_block();
	}
	else {
		if (current_token.type != TOK_SEMICOLON) {
			throw std::runtime_error(msg_header() + "expected { or ;");
		}
	}

	return std::make_shared<ASTFunctionDefinitionNode>(identifier, parameters, type_def.type,
		type_def.type_name, type_def.type_name_space, type_def.array_type, type_def.dim, block, row, col);
}

std::shared_ptr<ASTStructDefinitionNode> Parser::parse_struct_definition() {
	std::string identifier;
	std::map<std::string, VariableDefinition> variables;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_IDENTIFIER);
	identifier = current_token.value;
	consume_token(TOK_LEFT_CURLY);

	do {
		consume_token(TOK_VAR);
		consume_token(TOK_IDENTIFIER);
		auto struct_var_def = parse_struct_var_def();
		variables.emplace(struct_var_def->identifier, *struct_var_def);
		consume_token();

	} while (current_token.type == TOK_SEMICOLON && next_token.type != TOK_RIGHT_CURLY);

	consume_token(TOK_RIGHT_CURLY);

	check_consume_semicolon();

	return std::make_shared<ASTStructDefinitionNode>(identifier, variables, row, col);
}

std::shared_ptr<ASTExprNode> Parser::parse_expression() {
	return parse_ternary_expression();
}

std::shared_ptr<ASTExprNode> Parser::parse_ternary_expression() {
	auto expr = parse_in_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	if (next_token.type == TOK_QMARK) {
		std::shared_ptr<ASTExprNode> value_if_true;
		std::shared_ptr<ASTExprNode> value_if_false;
		consume_token();
		consume_token();
		value_if_true = parse_ternary_expression();
		consume_token(TOK_COLON);
		consume_token();
		value_if_false = parse_ternary_expression();
		return std::make_shared<ASTTernaryNode>(expr, value_if_true, value_if_false, row, col);
	}

	return expr;
}

std::shared_ptr<ASTExprNode> Parser::parse_in_expression() {
	auto expr = parse_logical_or_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	if (next_token.type == TOK_IN) {
		std::shared_ptr<ASTExprNode> collection;
		consume_token();
		consume_token();
		collection = parse_logical_or_expression();
		return std::make_shared<ASTInNode>(expr, collection, row, col);
	}

	return expr;
}

std::shared_ptr<ASTExprNode> Parser::parse_logical_or_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_logical_and_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_LOGICAL_OR_OP) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_logical_and_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_logical_and_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_bitwise_or_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_LOGICAL_AND_OP) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_bitwise_or_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_bitwise_or_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_bitwise_xor_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_BITWISE_OR) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_bitwise_xor_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_bitwise_xor_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_bitwise_and_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_BITWISE_XOR) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_bitwise_and_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_bitwise_and_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_equality_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_BITWISE_AND) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_equality_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_equality_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_relational_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_EQUALITY_OP) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_relational_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_relational_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_spaceship_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_RELATIONAL_OP) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_spaceship_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_spaceship_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_bitwise_shift_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_THREE_WAY_OP) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_bitwise_shift_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_bitwise_shift_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_simple_expression();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_BITWISE_SHIFT) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_simple_expression();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_simple_expression() {
	std::shared_ptr<ASTExprNode> lhs = parse_term();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_ADDITIVE_OP) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_term();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_term() {
	std::shared_ptr<ASTExprNode> lhs = parse_exponentiation();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_MULTIPLICATIVE_OP) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_exponentiation();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_exponentiation() {
	std::shared_ptr<ASTExprNode> lhs = parse_factor();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	while (next_token.type == TOK_EXPONENTIATION_OP) {
		consume_token();
		std::string current_token_value = current_token.value;
		consume_token();
		auto rhs = parse_factor();
		lhs = std::make_shared<ASTBinaryExprNode>(current_token_value, lhs, rhs, row, col);
	}

	return lhs;
}

std::shared_ptr<ASTExprNode> Parser::parse_factor() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	switch (current_token.type) {

		// literal cases
	case TOK_BOOL_LITERAL:
		return std::make_shared<ASTLiteralNode<flx_bool>>(parse_bool_literal(), row, col);
	case TOK_INT_LITERAL:
		return std::make_shared<ASTLiteralNode<flx_int>>(parse_int_literal(), row, col);
	case TOK_FLOAT_LITERAL:
		return std::make_shared<ASTLiteralNode<flx_float>>(parse_float_literal(), row, col);
	case TOK_CHAR_LITERAL:
		return std::make_shared<ASTLiteralNode<flx_char>>(parse_char_literal(), row, col);
	case TOK_STRING_LITERAL:
		return std::make_shared<ASTLiteralNode<flx_string>>(parse_string_literal(), row, col);

	case TOK_LEFT_CURLY:
		return parse_array_constructor_node();

		// type parsing cases
	case TOK_BOOL_TYPE:
	case TOK_INT_TYPE:
	case TOK_FLOAT_TYPE:
	case TOK_CHAR_TYPE:
	case TOK_STRING_TYPE:
		return parse_type_parse_node();

	case TOK_NULL:
		return std::make_shared<ASTNullNode>(row, col);

	case TOK_THIS:
		return parse_this_node();

	case TOK_TYPEOF:
	case TOK_TYPEID:
	case TOK_REFID:
	case TOK_IS_ANY:
	case TOK_IS_ARRAY:
	case TOK_IS_STRUCT:
		return parse_typing_node();

	case TOK_IDENTIFIER:
		return parse_identifier_expression();

	case TOK_FUN:
		return parse_function_expression();

		// subexpression case
	case TOK_LEFT_BRACKET: {
		consume_token();
		std::shared_ptr<ASTExprNode> sub_expr = parse_expression();
		consume_token(TOK_RIGHT_BRACKET);
		return sub_expr;
	}

						 // unary expression cases
	case TOK_REF:
	case TOK_UNREF: {
		std::string current_token_value = current_token.value;
		consume_token();
		return std::make_shared<ASTUnaryExprNode>(current_token_value, parse_factor(), row, col);
	}
	case TOK_ADDITIVE_OP:
	case TOK_NOT: {
		std::string current_token_value = current_token.value;
		consume_token();
		return std::make_shared<ASTUnaryExprNode>(current_token_value, parse_exponentiation(), row, col);
	}

	default:
		throw std::runtime_error(msg_header() + "expected expression");
	}
}

std::shared_ptr<ASTExprNode> Parser::parse_identifier_expression() {
	std::shared_ptr<ASTIdentifierNode> identifier = parse_identifier_node();

	switch (next_token.type) {
	case TOK_LEFT_BRACKET:
		return parse_function_call_node(identifier);

	case TOK_LEFT_CURLY:
		return parse_struct_constructor_node(identifier);

	case TOK_INCREMENT_OP: {
		consume_token();
		std::string op = current_token.value;
		return std::make_shared<ASTUnaryExprNode>(op, identifier, identifier->row, identifier->col);
	}
	default:
		return identifier;
	}
}

std::shared_ptr<ASTNode> Parser::parse_identifier_statement() {
	std::shared_ptr<ASTIdentifierNode> identifier = parse_identifier_node();

	switch (next_token.type) {
	case TOK_LEFT_BRACKET: {
		if (identifier->identifier_vector.size() > 1) {
			throw std::runtime_error(msg_header() + "unexpected token '" + next_token.value + "'");
		}
		std::shared_ptr<ASTFunctionCallNode> expr = parse_function_call_node(identifier);
		check_consume_semicolon();
		return expr;
	}
	case TOK_INCREMENT_OP:
		return parse_increment_expression(identifier);

	case TOK_ADDITIVE_OP:
	case TOK_MULTIPLICATIVE_OP:
	case TOK_EQUALS:
		return parse_assignment_statement(identifier);

	default:
		return parse_statement_expression();
	}
}

std::shared_ptr<ASTFunctionCallNode> Parser::parse_function_call_node(std::shared_ptr<ASTIdentifierNode> idnode) {
	std::string identifier = std::move(idnode->identifier_vector[0].identifier);
	std::string name_space = std::move(idnode->name_space);
	auto identifier_vector = std::vector<Identifier>();
	auto parameters = std::vector<std::shared_ptr<ASTExprNode>>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	consume_token(TOK_LEFT_BRACKET);

	if (next_token.type != TOK_RIGHT_BRACKET) {
		parameters = parse_actual_params();
	}
	else {
		consume_token();
	}

	check_current_token(TOK_RIGHT_BRACKET);

	Identifier id;
	if (next_token.type == TOK_LEFT_BRACE) {
		id = parse_identifier();
		id.identifier = identifier;
	}
	else {
		id = Identifier(identifier);
	}

	if (next_token.type == TOK_DOT) {
		consume_token();
		consume_token();
		identifier_vector = parse_identifier_vector();
	}

	identifier_vector.emplace(identifier_vector.begin(), id);

	return std::make_shared<ASTFunctionCallNode>(name_space, identifier_vector, parameters, row, col);
}

std::shared_ptr<ASTUnaryExprNode> Parser::parse_increment_expression(std::shared_ptr<ASTIdentifierNode> identifier) {
	consume_token();
	std::string op = current_token.value;

	check_consume_semicolon();

	return std::make_shared<ASTUnaryExprNode>(op, identifier, identifier->row, identifier->col);
}

std::shared_ptr<ASTIdentifierNode> Parser::parse_identifier_node() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	std::string name_space = "";
	auto identifier_vector = std::vector<Identifier>();

	if (next_token.type == TOK_LIB_ACESSOR_OP) {
		name_space = current_token.value;
		consume_token();
		consume_token();
	}

	identifier_vector = parse_identifier_vector();

	return std::make_shared<ASTIdentifierNode>(identifier_vector, name_space, row, col);
}

std::vector<std::shared_ptr<ASTExprNode>> Parser::parse_dimension_vector() {
	std::vector<std::shared_ptr<ASTExprNode>> access_vector = std::vector<std::shared_ptr<ASTExprNode>>();

	if (current_token.type == TOK_LEFT_BRACE) {
		do {
			std::shared_ptr<ASTExprNode> expr_size = nullptr;
			consume_token();
			if (current_token.type != TOK_RIGHT_BRACE) {
				expr_size = parse_expression();
				consume_token(TOK_RIGHT_BRACE);
			}
			access_vector.push_back(expr_size);

			if (next_token.type == TOK_LEFT_BRACE) {
				consume_token();
			}

		} while (current_token.type == TOK_LEFT_BRACE);
	}

	return access_vector;
}

Identifier Parser::parse_identifier() {
	std::string identifier = "";
	std::vector<std::shared_ptr<ASTExprNode>> access_vector = std::vector<std::shared_ptr<ASTExprNode>>();

	identifier = current_token.value;

	if (next_token.type == TOK_LEFT_BRACE) {
		consume_token();
		access_vector = parse_dimension_vector();
	}

	return Identifier(identifier, access_vector);
}

std::vector<Identifier> Parser::parse_identifier_vector() {
	auto identifier_vector = std::vector<Identifier>();

	while (current_token.type == TOK_IDENTIFIER) {
		identifier_vector.push_back(parse_identifier());

		if (next_token.type == TOK_DOT) {
			consume_token();
			consume_token(TOK_IDENTIFIER);
		}
		else {
			break;
		}
	}

	return identifier_vector;
}

std::shared_ptr<ASTAssignmentNode> Parser::parse_assignment_statement(std::shared_ptr<ASTIdentifierNode> identifier) {
	std::string op = std::string();
	std::shared_ptr<ASTExprNode> expr = nullptr;
	std::shared_ptr<ASTExprNode> expr_size = nullptr;
	auto access_vector = std::vector<std::shared_ptr<ASTExprNode>>();

	consume_token();
	if (current_token.type != TOK_ADDITIVE_OP && current_token.type != TOK_MULTIPLICATIVE_OP && current_token.type != TOK_EQUALS) {
		throw std::runtime_error(msg_header() + "invalid assignment operator '" + current_token.value + "'");
	}

	op = current_token.value;
	consume_token();
	expr = parse_expression();

	check_consume_semicolon();

	return std::make_shared<ASTAssignmentNode>(identifier->identifier_vector, identifier->name_space, op, expr, identifier->row, identifier->col);
}

std::shared_ptr<ASTDeclarationNode> Parser::parse_declaration_statement() {
	Type type = Type::T_UNDEFINED;
	TypeDefinition type_def;
	std::string identifier;
	std::shared_ptr<ASTExprNode> expr;
	auto dim_vector = std::vector<std::shared_ptr<ASTExprNode>>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	bool is_const = current_token.type == TOK_CONST;

	consume_token(TOK_IDENTIFIER);
	auto id = parse_identifier();
	identifier = id.identifier;
	dim_vector = id.access_vector;

	if (dim_vector.size() > 0) {
		type = Type::T_ARRAY;
	}

	type_def = parse_declaration_type_definition(type);

	if (next_token.type == TOK_EQUALS) {
		consume_token();
		consume_token();
		expr = parse_expression();
	}
	else {
		expr = nullptr;
	}

	check_consume_semicolon();

	return std::make_shared<ASTDeclarationNode>(identifier, type_def.type, type_def.array_type, dim_vector, type_def.type_name, type_def.type_name_space, expr, is_const, row, col);
}

std::shared_ptr<ASTStatementNode> Parser::parse_unpacked_declaration_statement() {
	if (current_token.type == TOK_CONST || next_token.type == TOK_IDENTIFIER) {
		return parse_declaration_statement();
	}
	else if (current_token.type == TOK_VAR && next_token.type == TOK_LEFT_BRACE) {
		TypeDefinition type_def;
		std::vector<std::shared_ptr<ASTDeclarationNode>> declarations;
		std::shared_ptr<ASTExprNode> expr;
		unsigned int row = current_token.row;
		unsigned int col = current_token.col;

		consume_token();

		while (next_token.type == TOK_IDENTIFIER) {
			consume_semicolon.push(false);
			declarations.push_back(parse_declaration_statement());
			consume_semicolon.pop();
			if (next_token.type == TOK_COMMA) {
				consume_token();
			}
		}

		consume_token(TOK_RIGHT_BRACE);

		type_def = parse_declaration_type_definition();

		if (next_token.type == TOK_EQUALS) {
			consume_token();
			consume_token();
			expr = parse_expression();
		}
		else {
			expr = nullptr;
		}

		check_consume_semicolon();

		return std::make_shared<ASTUnpackedDeclarationNode>(type_def.type, type_def.array_type, type_def.dim,
			type_def.type_name, type_def.type_name_space, declarations, expr, row, col);
	}
	else {
		throw std::runtime_error(msg_header() + "expected identifier or [");
	}
}

VariableDefinition* Parser::parse_formal_param() {
	bool is_rest = false;
	std::string identifier;
	std::string type_name;
	std::string type_name_space;
	Type type = Type::T_UNDEFINED;
	Type array_type = Type::T_UNDEFINED;
	std::shared_ptr<ASTExprNode> def_expr;
	auto dim = std::vector<std::shared_ptr<ASTExprNode>>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	if (current_token.type == TOK_ELLIPSIS) {
		is_rest = true;
		consume_token(TOK_IDENTIFIER);
	}
	else {
		check_current_token(TOK_IDENTIFIER);
	}

	auto id = parse_identifier();
	identifier = id.identifier;
	dim = id.access_vector;

	if (dim.size() > 0) {
		type = Type::T_ARRAY;
	}

	if (next_token.type == TOK_COLON) {
		consume_token();
		consume_token();

		if (next_token.type == TOK_LIB_ACESSOR_OP) {
			type_name_space = current_token.value;
			consume_token();
			consume_token();
		}

		if (type == Type::T_UNDEFINED) {
			type = parse_type();
		}
		else if (type == Type::T_ARRAY) {
			current_array_type = parse_type();

			if (current_array_type == parser::Type::T_UNDEFINED
				|| current_array_type == parser::Type::T_VOID
				|| current_array_type == parser::Type::T_ARRAY) {
				current_array_type = parser::Type::T_ANY;
			}
		}

		if (current_token.type == TOK_IDENTIFIER) {
			type_name = current_token.value;
		}
	}

	if (type == Type::T_UNDEFINED) {
		type = Type::T_ANY;
	}

	array_type = current_array_type;

	if (is_rest) {
		auto ndim = std::make_shared<ASTLiteralNode<flx_int>>(0, row, col);
		if (!is_array(type)) {
			array_type = type;
			type = Type::T_ARRAY;
		}
		dim.insert(dim.begin(), ndim);
	}

	if (next_token.type == TOK_EQUALS) {
		consume_token();
		consume_token();
		def_expr = parse_expression();
	}
	else {
		def_expr = nullptr;
	}

	return new VariableDefinition(identifier, type, type_name,
		type_name_space, array_type, dim, def_expr, is_rest, row, col);
}

VariableDefinition* Parser::parse_unpacked_formal_param() {
	std::string identifier;
	std::string type_name;
	std::string type_name_space;
	Type type = Type::T_UNDEFINED;
	Type array_type = Type::T_UNDEFINED;
	std::shared_ptr<ASTExprNode> def_expr;
	auto dim = std::vector<std::shared_ptr<ASTExprNode>>();
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	check_current_token(TOK_IDENTIFIER);

	auto id = parse_identifier();
	identifier = id.identifier;
	dim = id.access_vector;

	if (dim.size() > 0) {
		type = Type::T_ARRAY;
	}

	if (next_token.type == TOK_COLON) {
		consume_token();
		consume_token();

		if (next_token.type == TOK_LIB_ACESSOR_OP) {
			type_name_space = current_token.value;
			consume_token();
			consume_token();
		}

		if (type == Type::T_UNDEFINED) {
			type = parse_type();
		}
		else if (type == Type::T_ARRAY) {
			current_array_type = parse_type();

			if (current_array_type == parser::Type::T_UNDEFINED
				|| current_array_type == parser::Type::T_VOID
				|| current_array_type == parser::Type::T_ARRAY) {
				current_array_type = parser::Type::T_ANY;
			}
		}

		if (current_token.type == TOK_IDENTIFIER) {
			type_name = current_token.value;
		}
	}

	if (type == Type::T_UNDEFINED) {
		type = Type::T_ANY;
	}

	array_type = current_array_type;

	if (next_token.type == TOK_EQUALS) {
		consume_token();
		consume_token();
		def_expr = parse_expression();
	}
	else {
		def_expr = nullptr;
	}

	return new VariableDefinition(identifier, type, type_name,
		type_name_space, array_type, dim, def_expr, false, row, col);
}

std::vector<std::shared_ptr<ASTExprNode>> Parser::parse_actual_params() {
	auto parameters = std::vector<std::shared_ptr<ASTExprNode>>();

	do {
		consume_token();
		parameters.push_back(parse_expression());
		consume_token();
	} while (current_token.type == TOK_COMMA);

	return parameters;
}

std::shared_ptr<ASTTypingNode> Parser::parse_typing_node() {
	std::string image = current_token.value;
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	std::shared_ptr<ASTExprNode> expr = nullptr;

	consume_token(TOK_LEFT_BRACKET);

	consume_token();

	switch (current_token.type) {
	case TOK_BOOL_TYPE:
	case TOK_INT_TYPE:
	case TOK_FLOAT_TYPE:
	case TOK_CHAR_TYPE:
	case TOK_STRING_TYPE:
	case TOK_FUNCTION_TYPE: {
		auto id = parse_identifier();
		expr = std::make_shared<ASTIdentifierNode>(std::vector{ id }, std::string(), row, col);
		break;
	}
	default:
		expr = parse_expression();
	}

	consume_token(TOK_RIGHT_BRACKET);

	return std::make_shared<ASTTypingNode>(image, expr, row, col);
}

std::shared_ptr<ASTArrayConstructorNode> Parser::parse_array_constructor_node() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	std::vector<std::shared_ptr<ASTExprNode>> values = std::vector<std::shared_ptr<ASTExprNode>>();

	if (next_token.type != TOK_RIGHT_CURLY) {
		do {
			consume_token();
			values.push_back(parse_expression());

			consume_token();

		} while (current_token.type == TOK_COMMA);

		check_current_token(TOK_RIGHT_CURLY);
	}
	else {
		consume_token(TOK_RIGHT_CURLY);
	}

	return std::make_shared<ASTArrayConstructorNode>(values, row, col);
}

std::shared_ptr<ASTStructConstructorNode> Parser::parse_struct_constructor_node(std::shared_ptr<ASTIdentifierNode> idnode) {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	std::map<std::string, std::shared_ptr<ASTExprNode>> values = std::map<std::string, std::shared_ptr<ASTExprNode>>();
	std::string name_space = std::move(idnode->name_space);
	std::string type_name = std::move(idnode->identifier_vector[0].identifier);

	consume_token();
	consume_token();

	while (current_token.type == TOK_IDENTIFIER) {
		auto var_identifier = current_token.value;

		consume_token(TOK_EQUALS);

		consume_token();
		values[var_identifier] = parse_expression();

		consume_token();

		if (next_token.type == TOK_IDENTIFIER) {
			consume_token();
		}
	}

	check_current_token(TOK_RIGHT_CURLY);

	return std::make_shared<ASTStructConstructorNode>(type_name, name_space, values, row, col);
}

flx_bool Parser::parse_bool_literal() {
	return current_token.value == "true";
}

flx_int Parser::parse_int_literal() {
	try {
		if (current_token.value.starts_with("0b")) {
			return std::stoll(current_token.value.substr(2), 0, 2);
		}
		if (current_token.value.starts_with("0o")) {
			return std::stoll(current_token.value.substr(2), 0, 8);
		}
		if (current_token.value.starts_with("0d")) {
			return std::stoll(current_token.value.substr(2));
		}
		if (current_token.value.starts_with("0x")) {
			return std::stoll(current_token.value.substr(2), 0, 16);
		}
		return std::stoll(current_token.value);
	}
	catch (...) {
		throw std::runtime_error(msg_header() + "invalid literal: '" + current_token.value + "'");
	}
}

flx_float Parser::parse_float_literal() {
	try {
		return std::stold(current_token.value);
	}
	catch (...) {
		throw std::runtime_error(msg_header() + "invalid literal: '" + current_token.value + "'");
	}
}

flx_char Parser::parse_char_literal() {
	char chr = 0;
	if (current_token.value == "'\\\\'") {
		chr = '\\';
	}
	else if (current_token.value == "'\\n'") {
		chr = '\n';
	}
	else if (current_token.value == "'\\r'") {
		chr = '\r';
	}
	else if (current_token.value == "'\\''") {
		chr = '\'';
	}
	else if (current_token.value == "'\\t'") {
		chr = '\t';
	}
	else if (current_token.value == "'\\b'") {
		chr = '\b';
	}
	else if (current_token.value == "'\\0'") {
		chr = '\0';
	}
	else {
		chr = current_token.value.c_str()[1];
	}
	return chr;
}

flx_string Parser::parse_string_literal() {
	std::string str = current_token.value.substr(1, current_token.value.size() - 2);

	size_t pos = 0;
	while (pos < str.size()) {
		auto sc = str.substr(pos, 2);

		if (sc == "\\\"") {
			str.replace(pos, 2, "\"");
			++pos;
			continue;
		}

		if (sc == "\\n") {
			str.replace(pos, 2, "\n");
			++pos;
			continue;
		}

		if (sc == "\\r") {
			str.replace(pos, 2, "\r");
			++pos;
			continue;
		}

		if (sc == "\\t") {
			str.replace(pos, 2, "\t");
			++pos;
			continue;
		}

		if (sc == "\\b") {
			str.replace(pos, 2, "\b");
			++pos;
			continue;
		}

		if (sc == "\\0") {
			str.replace(pos, 2, "\0");
			++pos;
			continue;
		}

		if (sc == "\\\\") {
			str.replace(pos, 2, "\\");
			++pos;
			continue;
		}

		++pos;
	}

	return str;
}

std::shared_ptr<ASTTypeCastNode> Parser::parse_type_parse_node() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;
	Type type = parse_type();

	consume_token(TOK_LEFT_BRACKET);

	consume_token();
	std::shared_ptr<ASTExprNode> expr = parse_expression();

	consume_token(TOK_RIGHT_BRACKET);

	return std::make_shared<ASTTypeCastNode>(type, expr, row, col);
}

std::shared_ptr<ASTThisNode> Parser::parse_this_node() {
	unsigned int row = current_token.row;
	unsigned int col = current_token.col;

	return std::make_shared<ASTThisNode>(row, col);
}

void Parser::check_consume_semicolon() {
	if (!consume_semicolon.empty() && consume_semicolon.top()) {
		consume_token(TOK_SEMICOLON);
	}
}

std::string Parser::msg_header() const {
	return "(PERR) " + name + '[' + std::to_string(current_token.row) + ':' + std::to_string(current_token.col) + "]: ";
}

Type Parser::parse_type() {
	switch (current_token.type) {
	case TOK_VOID_TYPE:
	case TOK_NULL:
		return Type::T_VOID;

	case TOK_BOOL_TYPE:
	case TOK_BOOL_LITERAL:
		return Type::T_BOOL;

	case TOK_INT_TYPE:
	case TOK_INT_LITERAL:
		return Type::T_INT;

	case TOK_FLOAT_TYPE:
	case TOK_FLOAT_LITERAL:
		return Type::T_FLOAT;

	case TOK_CHAR_TYPE:
	case TOK_CHAR_LITERAL:
		return Type::T_CHAR;

	case TOK_STRING_TYPE:
	case TOK_STRING_LITERAL:
		return Type::T_STRING;

	case TOK_ANY_TYPE:
		return Type::T_ANY;

	case TOK_FUNCTION_TYPE:
		return Type::T_FUNCTION;

	case TOK_IDENTIFIER:
		return Type::T_STRUCT;

	case TOK_LEFT_CURLY:
		return Type::T_ARRAY;

	default:
		throw std::runtime_error(msg_header() + "invalid type");
	}
}

void Parser::consume_token() {
	current_token = next_token;
	next_token = lex->next_token();
}

void Parser::consume_token(LexTokenType type) {
	consume_token();
	check_current_token(type);
}

void Parser::check_current_token(LexTokenType type) const {
	if (current_token.type != type) {
		throw std::runtime_error(msg_header() + "expected '" + Token::token_image(type) + "'");
	}
}
