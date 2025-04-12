#include "visitor.hpp"

#include "ast.hpp"

using namespace core;

Visitor::Visitor(const std::map<std::string, std::shared_ptr<ASTProgramNode>>& programs, std::shared_ptr<ASTProgramNode> main_program, const std::string& current_this_name)
	: programs(programs), main_program(main_program), curr_row(0), curr_col(0) {
	current_program_stack.push(main_program);
};

std::shared_ptr<ASTStructDefinitionNode> Visitor::get_flexa_struct() {
	std::map<std::string, VariableDefinition> variables;
	variables.emplace("args", VariableDefinition("args", Type::T_ARRAY, "", "",
		Type::T_STRING, std::vector<std::shared_ptr<ASTExprNode>>(), nullptr, false, 0, 0));
	variables.emplace("cwd", VariableDefinition("cwd", Type::T_STRING, "", "",
		Type::T_UNDEFINED, std::vector<std::shared_ptr<ASTExprNode>>(), nullptr, false, 0, 0));
	variables.emplace("so", VariableDefinition("so", Type::T_STRING, "", "",
		Type::T_UNDEFINED, std::vector<std::shared_ptr<ASTExprNode>>(), nullptr, false, 0, 0));
	return std::make_shared<ASTStructDefinitionNode>("Flexa", variables, 0, 0);
}
