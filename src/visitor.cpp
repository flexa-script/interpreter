#include "visitor.hpp"

using namespace core;

Visitor::Visitor(const std::map<std::string, std::shared_ptr<ASTProgramNode>>& programs, std::shared_ptr<ASTProgramNode> main_program, const std::string& current_this_name)
	: programs(programs), main_program(main_program), curr_row(0), curr_col(0) {
	current_program_stack.push(main_program);
};
