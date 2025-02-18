#include <iostream>
#include <utility>

#include "compiler.hpp"
#include "token.hpp"
#include "md_builtin.hpp"

#include "utils.hpp"

using namespace visitor;
using namespace parser;
using namespace lexer;

Compiler::Compiler(std::shared_ptr<ASTProgramNode> main_program, std::map<std::string, std::shared_ptr<ASTProgramNode>> programs, const std::vector<std::string>& args)
	: Visitor(programs, main_program, default_namespace) {

	built_in_libs["builtin"]->register_functions(this);

	build_args(args);
};

void Compiler::start() {
	auto pop = push_namespace(flx_string(default_namespace));
	visit(current_program.top());
	pop_namespace(pop);
	add_instruction(OpCode::OP_HALT, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTProgramNode> astnode) {
	for (const auto& statement : astnode->statements) {
		try {
			statement->accept(this);
		}
		catch (std::exception ex) {
			throw std::runtime_error(ex.what());
		}
	}
}

void Compiler::visit(std::shared_ptr<ASTUsingNode> astnode) {
	std::string libname = utils::StringUtils::join(astnode->library, ".");

	if (built_in_libs.find(libname) != built_in_libs.end()) {
		//built_in_libs.find(libname)->second->register_functions(this);
	}

	auto& program = programs[libname];

	// add lib to current program
	current_program.top()->libs.push_back(program);

	// if can't parsed yet
	if (!utils::CollectionUtils::contains(parsed_libs, libname)) {
		current_program.push(program);
		parsed_libs.push_back(libname);
		auto pop = push_namespace(flx_string(program->name_space));
		visit(program);
		current_program.pop();
		pop_namespace(pop);
	}
}

void Compiler::visit(std::shared_ptr<ASTNamespaceManagerNode> astnode) {
	if (astnode->image == "include") {
		add_instruction(OpCode::OP_INCLUDE_NAMESPACE, flx_string(astnode->name_space));
	}
	else {
		add_instruction(OpCode::OP_EXCLUDE_NAMESPACE, flx_string(astnode->name_space));
	}
}

void Compiler::visit(std::shared_ptr<ASTEnumNode> astnode) {
	for (size_t i = 0; i < astnode->identifiers.size(); ++i) {
		add_instruction(OpCode::OP_PUSH_INT, flx_int(i));
		add_instruction(OpCode::OP_SET_TYPE, uint8_t(Type::T_INT));
		add_instruction(OpCode::OP_STORE_VAR, flx_string(astnode->identifiers[i]));
	}
}

void Compiler::visit(std::shared_ptr<ASTDeclarationNode> astnode) {
	auto pop = push_namespace(flx_string(astnode->type_name_space));

	type_definition_operations(*astnode);

	if (astnode->expr) {
		astnode->expr->accept(this);
	}
	else {
		add_instruction(OpCode::OP_PUSH_UNDEFINED, nullptr);
	}

	add_instruction(OpCode::OP_STORE_VAR, flx_string(astnode->identifier));

	pop_namespace(pop);
}

void Compiler::visit(std::shared_ptr<ASTUnpackedDeclarationNode> astnode) {
	for (const auto& declaration : astnode->declarations) {
		declaration->accept(this);
	}
}

void Compiler::visit(std::shared_ptr<ASTAssignmentNode> astnode) {
	auto pop = push_namespace(flx_string(astnode->name_space));

	astnode->expr->accept(this);

	if (has_sub_value(astnode->identifier_vector)) {
		add_instruction(OpCode::OP_LOAD_VAR, flx_string(astnode->identifier));

		for (size_t i = 0; i < astnode->identifier_vector.size(); ++i) {
			const auto& id = astnode->identifier_vector[i];

			if (i > 0) {
				if (i == astnode->identifier_vector.size() - 1 && id.access_vector.size() == 0) {
					add_instruction(OpCode::OP_ASSIGN_SUB_ID, flx_string(id.identifier));
				}
				else {
					add_instruction(OpCode::OP_LOAD_SUB_ID, flx_string(id.identifier));
				}
			}

			for (size_t j = 0; j < id.access_vector.size(); ++j) {
				id.access_vector[j]->accept(this);

				if (i == astnode->identifier_vector.size() - 1 && j == id.access_vector.size() - 1) {
					add_instruction(OpCode::OP_ASSIGN_SUB_IX, nullptr);
				}
				else {
					add_instruction(OpCode::OP_LOAD_SUB_IX, nullptr);
				}
			}
		}
	}
	else {
		add_instruction(OpCode::OP_LOAD_VAR, flx_string(astnode->identifier));
		add_instruction(OpCode::OP_ASSIGN_VAR, nullptr);
	}

	pop_namespace(pop);
}

void Compiler::visit(std::shared_ptr<ASTReturnNode> astnode) {
	if (astnode->expr) {
		astnode->expr->accept(this);
	}
	else {
		add_instruction(OpCode::OP_PUSH_UNDEFINED, nullptr);
	}

	add_instruction(OpCode::OP_RETURN, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTFunctionCallNode> astnode) {
	auto pop = push_namespace(flx_string(astnode->name_space));

	for (const auto& param : astnode->parameters) {
		param->accept(this);
	}

	add_instruction(OpCode::OP_CALL_PARAM_COUNT, astnode->parameters.size());

	add_instruction(OpCode::OP_CALL, flx_string(astnode->identifier));

	access_sub_value_operations(astnode->identifier_vector);

	pop_namespace(pop);
}

void Compiler::visit(std::shared_ptr<ASTBuiltinCallNode> astnode) {}

void Compiler::visit(std::shared_ptr<ASTFunctionDefinitionNode> astnode) {
	auto pop = push_namespace(flx_string(astnode->type_name_space));

	if (astnode->block) {
		// function will be defined here
		type_definition_operations(*astnode);
		add_instruction(OpCode::OP_FUN_START, flx_string(astnode->identifier));

		for (auto& param : astnode->parameters) {
			const auto& var = *dynamic_cast<VariableDefinition*>(param);
			type_definition_operations(var);
			//add_instruction(OpCode::OP_SET_DEFAULT_VALUE, var.default_value);
			add_instruction(OpCode::OP_SET_IS_REST, var.is_rest);
			add_instruction(OpCode::OP_FUN_SET_PARAM, flx_string(var.identifier));
		}

		// call will start here, function metadata will be stored with at starting pointer
		add_instruction(OpCode::OP_FUN_END, nullptr);

		// at this point, vm will jump to OP_FUN_END
		auto id = add_instruction(OpCode::OP_JUMP, nullptr);

		for (auto& param : astnode->parameters) {
			const auto& var = *dynamic_cast<VariableDefinition*>(param);
			if (var.default_value) {
				auto param_dcl = std::make_unique<ASTDeclarationNode>(var.identifier, var.type, var.array_type,
					var.dim, var.type_name, var.type_name_space, static_cast<std::shared_ptr<ASTExprNode>>(var.default_value), false,
					astnode->row, astnode->col);
				param_dcl->accept(this);
			}
		}

		astnode->block->accept(this);

		// it will return to prev
		add_instruction(OpCode::OP_RETURN, nullptr);

		replace_last_operand(id, pointer);
	}

	pop_namespace(pop);
}

void Compiler::visit(std::shared_ptr<ASTLambdaFunction> astnode) {
	astnode->fun->identifier = utils::UUID::generate();
	astnode->fun->accept(this);
	add_instruction(OpCode::OP_PUSH_FUNCTION, flx_string(astnode->fun->identifier));
}

void Compiler::visit(std::shared_ptr<ASTBlockNode> astnode) {
	for (const auto& stmt : astnode->statements) {
		stmt->accept(this);
	}
}

void Compiler::visit(std::shared_ptr<ASTExitNode> astnode) {
	astnode->exit_code->accept(this);
	add_instruction(OpCode::OP_HALT, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTContinueNode> astnode) {
	add_instruction(OpCode::OP_JUMP, size_t(deviation_stack.top()));
}

void Compiler::visit(std::shared_ptr<ASTBreakNode> astnode) {
	add_instruction(OpCode::OP_BREAK, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTSwitchNode> astnode) {
	astnode->condition->accept(this);
	add_instruction(OpCode::OP_STORE_COMP, nullptr);

	for (size_t i = 0; i < astnode->statements.size(); ++i) {
		size_t ip = 0;

		for (const auto& [key, value] : astnode->parsed_case_blocks) {
			if (i == value) {
				add_instruction(OpCode::OP_PUSH_INT, size_t(key));
				ip = add_instruction(OpCode::OP_JUMP_IF_FALSE_OR_NEXT, nullptr);
			}
		}

		add_instruction(OpCode::OP_POP_CONSTANT, nullptr);

		astnode->statements[i]->accept(this);

		replace_last_operand(ip, size_t(pointer));
	}

	add_instruction(OpCode::OP_RELEASE_COMP, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTElseIfNode> astnode) {
	astnode->condition->accept(this);

	auto ip = add_instruction(OpCode::OP_JUMP_IF_FALSE, nullptr);

	astnode->block->accept(this);

	replace_last_operand(ip, size_t(pointer));
}

void Compiler::visit(std::shared_ptr<ASTIfNode> astnode) {
	astnode->condition->accept(this);

	auto ip = add_instruction(OpCode::OP_JUMP_IF_FALSE, nullptr);

	astnode->if_block->accept(this);

	replace_last_operand(ip, size_t(pointer));

	for (const auto& elif : astnode->else_ifs) {
		elif->accept(this);
	}

	if (astnode->else_block) {
		auto eip = add_instruction(OpCode::OP_JUMP_IF_FALSE, nullptr);

		astnode->else_block->accept(this);

		replace_last_operand(eip, size_t(pointer));
	}
}

void Compiler::visit(std::shared_ptr<ASTForNode> astnode) {
	if (astnode->dci[0]) {
		astnode->dci[0]->accept(this);
	}
	auto start = pointer;
	if (astnode->dci[1]) {
		astnode->dci[1]->accept(this);
	}
	astnode->block->accept(this);
	if (astnode->dci[2]) {
		astnode->dci[2]->accept(this);
	}
	add_instruction(OpCode::OP_JUMP_IF_TRUE, size_t(start));
}

void Compiler::visit(std::shared_ptr<ASTForEachNode> astnode) {
	astnode->collection->accept(this);

	add_instruction(OpCode::OP_GET_ITERATOR, nullptr);

	astnode->itdecl->accept(this);

	add_instruction(OpCode::OP_NEXT_ELEMENT, nullptr);

	auto start = pointer;

	if (const auto idnode = std::dynamic_pointer_cast<ASTUnpackedDeclarationNode>(astnode->itdecl)) {
		for (auto decl : idnode->declarations) {
			add_instruction(OpCode::OP_STORE_VAR, flx_string(decl->identifier));
		}
	}
	else if (const auto idnode = std::dynamic_pointer_cast<ASTDeclarationNode>(astnode->itdecl)) {
		add_instruction(OpCode::OP_STORE_VAR, flx_string(idnode->identifier));
	}

	astnode->block->accept(this);

	add_instruction(OpCode::OP_NEXT_ELEMENT, nullptr);

	add_instruction(OpCode::OP_JUMP_IF_TRUE, size_t(start));
}

void Compiler::visit(std::shared_ptr<ASTTryCatchNode> astnode) {
	add_instruction(OpCode::OP_TRY_START, nullptr);

	astnode->try_block->accept(this);

	add_instruction(OpCode::OP_TRY_END, nullptr);

	auto ip = add_instruction(OpCode::OP_JUMP_IF_TRUE, nullptr);

	astnode->decl->accept(this);

	if (const auto idnode = std::dynamic_pointer_cast<ASTUnpackedDeclarationNode>(astnode->decl)) {
		for (auto decl : idnode->declarations) {
			add_instruction(OpCode::OP_STORE_VAR, flx_string(decl->identifier));
		}
	}
	else if (const auto idnode = std::dynamic_pointer_cast<ASTDeclarationNode>(astnode->decl)) {
		add_instruction(OpCode::OP_STORE_VAR, flx_string(idnode->identifier));
	}

	astnode->catch_block->accept(this);

	replace_last_operand(ip, size_t(pointer));
}

void Compiler::visit(std::shared_ptr<ASTThrowNode> astnode) {
	astnode->error->accept(this);
	add_instruction(OpCode::OP_THROW, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTEllipsisNode> astnode) {}

void Compiler::visit(std::shared_ptr<ASTWhileNode> astnode) {
	astnode->condition->accept(this);

	auto ip = add_instruction(OpCode::OP_JUMP_IF_FALSE, nullptr);

	astnode->block->accept(this);

	replace_last_operand(ip, size_t(pointer));
}

void Compiler::visit(std::shared_ptr<ASTDoWhileNode> astnode) {
	auto start = pointer;

	astnode->block->accept(this);

	astnode->condition->accept(this);

	add_instruction(OpCode::OP_JUMP_IF_TRUE, size_t(start));
}

void Compiler::visit(std::shared_ptr<ASTStructDefinitionNode> astnode) {
	add_instruction(OpCode::OP_STRUCT_START, flx_string(astnode->identifier));

	for (const auto& var : astnode->variables) {
		type_definition_operations(var.second);
		add_instruction(OpCode::OP_STRUCT_SET_VAR, flx_string(var.first));
	}

	add_instruction(OpCode::OP_STRUCT_END, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTLiteralNode<flx_bool>> astnode) {
	add_instruction(OpCode::OP_PUSH_BOOL, flx_bool(astnode->val));
}

void Compiler::visit(std::shared_ptr<ASTLiteralNode<flx_int>> astnode) {
	add_instruction(OpCode::OP_PUSH_INT, flx_int(astnode->val));
}

void Compiler::visit(std::shared_ptr<ASTLiteralNode<flx_float>> astnode) {
	add_instruction(OpCode::OP_PUSH_FLOAT, flx_float(astnode->val));
}

void Compiler::visit(std::shared_ptr<ASTLiteralNode<flx_char>> astnode) {
	add_instruction(OpCode::OP_PUSH_CHAR, flx_char(astnode->val));
}

void Compiler::visit(std::shared_ptr<ASTLiteralNode<flx_string>> astnode) {
	add_instruction(OpCode::OP_PUSH_STRING, flx_string(astnode->val));
}

void Compiler::visit(std::shared_ptr<ASTArrayConstructorNode> astnode) {
	auto size = astnode->values.size();

	add_instruction(OpCode::OP_INIT_ARRAY, size_t(size));

	for (size_t i = 0; i < size; ++i) {
		astnode->values[i]->accept(this);
		add_instruction(OpCode::OP_SET_ELEMENT, size_t(i));
	}

	add_instruction(OpCode::OP_PUSH_ARRAY, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTStructConstructorNode> astnode) {
	auto pop = push_namespace(flx_string(astnode->name_space));

	add_instruction(OpCode::OP_INIT_STRUCT, flx_string(astnode->type_name));

	for (const auto& expr : astnode->values) {
		expr.second->accept(this);
		add_instruction(OpCode::OP_SET_FIELD, flx_string(expr.first));
	}

	add_instruction(OpCode::OP_PUSH_STRUCT, nullptr);

	pop_namespace(pop);
}

void Compiler::visit(std::shared_ptr<ASTIdentifierNode> astnode) {
	if (has_sub_value(astnode->identifier_vector)) {
		access_sub_value_operations(astnode->identifier_vector);
	}
	else {
		add_instruction(OpCode::OP_LOAD_VAR, flx_string(astnode->identifier));
	}
}

void Compiler::visit(std::shared_ptr<ASTBinaryExprNode> astnode) {
	astnode->left->accept(this);
	astnode->right->accept(this);

	auto op = OpCode::OP_RES;

	if (astnode->op == "or") {
		op = OpCode::OP_OR;
	}
	else if (astnode->op == "and") {
		op = OpCode::OP_AND;
	}
	else if (astnode->op == "|") {
		op = OpCode::OP_BIT_OR;
	}
	else if (astnode->op == "^") {
		op = OpCode::OP_BIT_XOR;
	}
	else if (astnode->op == "&") {
		op = OpCode::OP_BIT_AND;
	}
	else if (astnode->op == "==") {
		op = OpCode::OP_EQL;
	}
	else if (astnode->op == "!=") {
		op = OpCode::OP_DIF;
	}
	else if (astnode->op == "<") {
		op = OpCode::OP_LT;
	}
	else if (astnode->op == "<=") {
		op = OpCode::OP_LTE;
	}
	else if (astnode->op == ">") {
		op = OpCode::OP_GT;
	}
	else if (astnode->op == ">=") {
		op = OpCode::OP_GTE;
	}
	else if (astnode->op == "<=>") {
		op = OpCode::OP_SPACE_SHIP;
	}
	else if (astnode->op == "<<") {
		op = OpCode::OP_LEFT_SHIFT;
	}
	else if (astnode->op == ">>") {
		op = OpCode::OP_RIGHT_SHIFT;
	}
	else if (astnode->op == "+") {
		op = OpCode::OP_ADD;
	}
	else if (astnode->op == "-") {
		op = OpCode::OP_SUB;
	}
	else if (astnode->op == "*") {
		op = OpCode::OP_MUL;
	}
	else if (astnode->op == "/") {
		op = OpCode::OP_DIV;
	}
	else if (astnode->op == "%") {
		op = OpCode::OP_REMAINDER;
	}
	else if (astnode->op == "/%") {
		op = OpCode::OP_FLOOR_DIV;
	}
	else if (astnode->op == "**") {
		op = OpCode::OP_EXP;
	}

	add_instruction(op, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTUnaryExprNode> astnode) {
	auto op = OpCode::OP_RES;

	if (astnode->unary_op == "ref") {
		op = OpCode::OP_REF;
	}
	else if (astnode->unary_op == "unref") {
		op = OpCode::OP_UNREF;
	}
	else if (astnode->unary_op == "-") {
		add_instruction(OpCode::OP_PUSH_INT, flx_int(0));
		op = OpCode::OP_SUB;
	}
	else if (astnode->unary_op == "++") {
		add_instruction(OpCode::OP_PUSH_INT, flx_int(1));
		op = OpCode::OP_ADD;
	}
	else if (astnode->unary_op == "--") {
		add_instruction(OpCode::OP_PUSH_INT, flx_int(1));
		op = OpCode::OP_SUB;
	}
	else if (astnode->unary_op == "not") {
		op = OpCode::OP_NOT;
	}
	else if (astnode->unary_op == "~") {
		op = OpCode::OP_BIT_NOT;
	}

	astnode->expr->accept(this);

	add_instruction(op, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTTernaryNode> astnode) {
	astnode->condition->accept(this);
	astnode->value_if_true->accept(this);
	astnode->value_if_false->accept(this);
	add_instruction(OpCode::OP_TERNARY, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTInNode> astnode) {
	// TODO: binary expression?
	astnode->value->accept(this);
	astnode->collection->accept(this);
	add_instruction(OpCode::OP_IN, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTTypeCastNode> astnode) {
	astnode->expr->accept(this);
	add_instruction(OpCode::OP_TYPE_PARSE, uint8_t(astnode->type));
}

void Compiler::visit(std::shared_ptr<ASTNullNode> astnode) {
	add_instruction(OpCode::OP_PUSH_VOID, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTThisNode> astnode) {
	add_instruction(OpCode::OP_PUSH_NAMESPACE_STACK, nullptr);
}

void Compiler::visit(std::shared_ptr<ASTTypingNode> astnode) {
	astnode->expr->accept(this);

	if (astnode->image == "typeid") {
		add_instruction(OpCode::OP_TYPEID, nullptr);
	}
	else if (astnode->image == "refid") {
		add_instruction(OpCode::OP_REFID, nullptr);
		}
	else if (astnode->image == "typeof") {
		add_instruction(OpCode::OP_TYPEOF, nullptr);
	}
	else {
		auto type = Type::T_UNDEFINED;
		if (astnode->image == "is_any") {
			type = Type::T_ANY;
		}
		else if (astnode->image == "is_array") {
			type = Type::T_ARRAY;
		}
		else if (astnode->image == "is_struct") {
			type = Type::T_STRUCT;
		}
		add_instruction(OpCode::OP_IS_TYPE, uint8_t(type));
	}
}

void Compiler::visit(std::shared_ptr<ASTValueNode> astnode) {}

bool Compiler::has_sub_value(std::vector<Identifier> identifier_vector) {
	return identifier_vector.size() > 1 || identifier_vector[0].access_vector.size() > 0;
}

void Compiler::type_definition_operations(TypeDefinition type) {
	if (type.dim.size() > 0) {
		add_instruction(OpCode::OP_SET_TYPE, uint8_t(Type::T_ARRAY));

		for (const auto& s : type.dim) {
			if (s) {
				s->accept(this);
			}
			else {
				add_instruction(OpCode::OP_PUSH_INT, flx_int(0));
			}
			add_instruction(OpCode::OP_SET_ARRAY_SIZE, nullptr);
		}

		//add_instruction(OpCode::OP_SET_ARRAY_DIM, size_t(dim));
	}
	else {
		add_instruction(OpCode::OP_SET_TYPE, uint8_t(type.type));
	}

	if (is_array(type.type) || type.dim.size() > 0) {
		add_instruction(OpCode::OP_SET_ARRAY_TYPE, uint8_t(
			is_undefined(type.array_type) ? Type::T_ANY : type.array_type
		));
	}

	if (!type.type_name.empty()) {
		if (!type.type_name_space.empty()) {
			add_instruction(OpCode::OP_SET_TYPE_NAME_SPACE, flx_string(type.type_name_space));
		}
		add_instruction(OpCode::OP_SET_TYPE_NAME, flx_string(type.type_name));
	}
}

void Compiler::access_sub_value_operations(std::vector<Identifier> identifier_vector) {
	if (has_sub_value(identifier_vector)) {
		add_instruction(OpCode::OP_LOAD_VAR, flx_string(identifier_vector[0].identifier));

		for (size_t i = 0; i < identifier_vector.size(); ++i) {
			const auto& id = identifier_vector[i];

			if (i > 0) {
				add_instruction(OpCode::OP_LOAD_SUB_ID, flx_string(id.identifier));
			}

			for (auto& av : id.access_vector) {
				av->accept(this);
				add_instruction(OpCode::OP_LOAD_SUB_IX, size_t(0));
			}
		}
	}
}

template <typename T>
size_t Compiler::add_instruction(OpCode opcode, T operand) {
	auto ins_pointer = pointer;
	bytecode_program.push_back(BytecodeInstruction(opcode, operand));
	++pointer;
	return ins_pointer;
}

template <typename T>
void Compiler::replace_last_operand(size_t pos, T operand) {
	bytecode_program[pos].operand = BytecodeInstruction::to_byteopnd(operand);
}

void Compiler::build_args(const std::vector<std::string>& args) {
	//auto dim = std::vector<std::shared_ptr<ASTExprNode>>{ std::make_shared<ASTLiteralNode<cp_int>>(cp_int(args.size()), 0, 0) };

	//auto var = std::make_shared<RuntimeVariable>("args", Type::T_ARRAY, Type::T_STRING, dim, "", "");
	//gc.add_var_root(var);

	//auto arr = cp_array();
	//for (size_t i = 0; i < args.size(); ++i) {
	//	arr.push_back(alocate_value(new RuntimeValue(args[i])));
	//}

	//var->set_value(alocate_value(new RuntimeValue(arr, Type::T_STRING, dim)));

	//scopes[default_namespace].back()->declare_variable("args", var);
}

bool Compiler::push_namespace(const std::string name_space) {
	if (!name_space.empty()) {
		add_instruction(OpCode::OP_PUSH_NAMESPACE, name_space);
		return true;
	}
	return false;
}

void Compiler::pop_namespace(bool pop) {
	if (pop) {
		add_instruction(OpCode::OP_POP_NAMESPACE, nullptr);
	}
}

long long Compiler::hash(std::shared_ptr<ASTExprNode>) { return 0; }
long long Compiler::hash(std::shared_ptr<ASTValueNode>) { return 0; }
long long Compiler::hash(std::shared_ptr<ASTLiteralNode<flx_bool>>) { return 0; }
long long Compiler::hash(std::shared_ptr<ASTLiteralNode<flx_int>>) { return 0; }
long long Compiler::hash(std::shared_ptr<ASTLiteralNode<flx_float>>) { return 0; }
long long Compiler::hash(std::shared_ptr<ASTLiteralNode<flx_char>>) { return 0; }
long long Compiler::hash(std::shared_ptr<ASTLiteralNode<flx_string>>) { return 0; }
long long Compiler::hash(std::shared_ptr<ASTIdentifierNode>) { return 0; }

void Compiler::set_curr_pos(unsigned int row, unsigned int col) {
	curr_row = row;
	curr_col = col;
}

std::string Compiler::msg_header() {
	return "(CMP) " + current_program.top()->name + '[' + std::to_string(curr_row) + ':' + std::to_string(curr_col) + "]: ";
}
