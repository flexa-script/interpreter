#include "interpreter.hpp"

#include <filesystem>

#include "exception_handler.hpp"
#include "token.hpp"
#include "md_builtin.hpp"
#include "gc.hpp"
#include "utils.hpp"
#include "watch.hpp"
#include "constants.hpp"

using namespace core;
using namespace core::runtime;

Interpreter::Interpreter(std::shared_ptr<Scope> global_scope, std::shared_ptr<ASTProgramNode> main_program,
	const std::map<std::string, std::shared_ptr<ASTProgramNode>>& programs, const std::vector<std::string>& args)
	: Visitor(programs, main_program, main_program ? main_program->name : Constants::DEFAULT_NAMESPACE) {
	current_expression_value = allocate_value(new RuntimeValue(Type::T_UNDEFINED));
	gc.add_ptr_root(&current_expression_value);

	current_this_name.push(main_program->name);

	scopes[Constants::DEFAULT_NAMESPACE].push_back(std::make_shared<Scope>(std::make_shared<ASTProgramNode>("__builtin__", Constants::DEFAULT_NAMESPACE, std::vector<std::shared_ptr<ASTNode>>())));
	Constants::BUILT_IN_LIBS.at("builtin")->register_functions(this);

	if (main_program->name_space != Constants::DEFAULT_NAMESPACE) {
		program_nmspaces[main_program->name].push_back(Constants::DEFAULT_NAMESPACE);
	}

	scopes[main_program->name_space].push_back(global_scope);

	global_scope->declare_flexa_struct(this);

	build_args(args);
}

void Interpreter::start() {
	current_this_name.push(current_program_stack.top()->name);
	visit(current_program_stack.top());
	current_this_name.pop();
}

void Interpreter::visit(std::shared_ptr<ASTProgramNode> astnode) {
	for (const auto& statement : astnode->statements) {
		try {
			statement->accept(this);

			if (exit_from_program) {
				break;
			}
		}
		catch (std::runtime_error ex) {
			if (curr_row == 0 || curr_col == 0) {
				set_curr_pos(statement->row, statement->col);
			}
			if (exception) {
				throw std::runtime_error(ex.what());
			}
			exception = true;
			throw std::runtime_error(msg_header() + ex.what());
		}
	}

	if (astnode->statements.size() > 1
		|| astnode->statements.size() > 0
		&& !std::dynamic_pointer_cast<ASTExprNode>(astnode->statements[0])) {
		current_expression_value = allocate_value(new RuntimeValue(Type::T_UNDEFINED));
	}
}

void Interpreter::visit(std::shared_ptr<ASTUsingNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	std::string libname = utils::StringUtils::join(astnode->library, ".");

	if (Constants::BUILT_IN_LIBS.find(libname) != Constants::BUILT_IN_LIBS.end()) {
		Constants::BUILT_IN_LIBS.find(libname)->second->register_functions(this);
	}

	const auto& program = programs[libname];

	// if not parsed yet
	if (!utils::CollectionUtils::contains(parsed_libs, libname)) {
		parsed_libs.push_back(libname);

		current_program_stack.push(program);

		scopes[program->name_space].push_back(std::make_shared<Scope>(program));

		if (std::find(program_nmspaces[program->name].begin(), program_nmspaces[program->name].end(), Constants::DEFAULT_NAMESPACE) == program_nmspaces[program->name].end()) {
			program_nmspaces[program->name].push_back(Constants::DEFAULT_NAMESPACE);
		}

		start();

		current_program_stack.pop();
	}
}

void Interpreter::visit(std::shared_ptr<ASTIncludeNamespaceNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	const auto& current_program_name = current_program_stack.top()->name;

	if (std::find(
		program_nmspaces[current_program_name].begin(),
		program_nmspaces[current_program_name].end(),
		astnode->name_space
	) == program_nmspaces[current_program_name].end()) {

		program_nmspaces[current_program_name].push_back(astnode->name_space);
	}
}

void Interpreter::visit(std::shared_ptr<ASTExcludeNamespaceNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program_name = current_program_stack.top()->name;

	size_t pos = std::distance(
		program_nmspaces[current_program_name].begin(),
		std::find(
			program_nmspaces[current_program_name].begin(),
			program_nmspaces[current_program_name].end(),
			astnode->name_space
		)
	);
	program_nmspaces[current_program_name].erase(program_nmspaces[current_program_name].begin() + pos);
}

void Interpreter::visit(std::shared_ptr<ASTEnumNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& name_space = current_program_stack.top()->name_space;

	for (size_t i = 0; i < astnode->identifiers.size(); ++i) {
		auto var = std::make_shared<RuntimeVariable>(astnode->identifiers[i], Type::T_INT, Type::T_UNDEFINED, std::vector<size_t>(), "", "");
		gc.add_var_root(var);
		var->set_value(allocate_value(new RuntimeValue(flx_int(i))));
		scopes[name_space].back()->declare_variable(astnode->identifiers[i], var);
	}
}

void Interpreter::visit(std::shared_ptr<ASTDeclarationNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& name_space = current_program_stack.top()->name_space;

	// evaluate assignment expression
	if (astnode->expr) {
		astnode->expr->accept(this);
	}
	else {
		clear_current_expression();
	}

	// check if it's reference
	RuntimeValue* new_value = current_expression_value;
	clear_current_expression();

	if (!new_value->use_ref) {
		new_value = allocate_value(new RuntimeValue(new_value));
	}

	auto ev_dim = evaluate_access_vector(astnode->expr_dim);

	// check and fill array if necessary
	check_build_array(new_value, ev_dim);

	// creates variable
	auto new_var = std::make_shared<RuntimeVariable>(astnode->identifier, astnode->type,
		astnode->array_type, ev_dim,
		astnode->type_name, astnode->type_name_space);
	gc.add_var_root(new_var);
	new_var->set_value(new_value);

	// validate assignment type
	if ((!TypeDefinition::is_any_or_match_type(*new_var, *new_value) ||
		TypeUtils::is_array(new_var->type) && !TypeUtils::is_any(new_var->array_type)
		&& !TypeDefinition::match_type(*new_var, *new_value, false, true))
		&& astnode->expr && !TypeUtils::is_undefined(new_value->type) && !TypeUtils::is_array(new_value->type)) {
		ExceptionHandler::throw_declaration_type_err(astnode->identifier, *new_var, *new_value);
	}

	validates_reference_type_assignment(*new_var, new_value);

	// normalize string and number types
	RuntimeOperations::normalize_type(new_var.get(), new_value);

	scopes[name_space].back()->declare_variable(astnode->identifier, new_var);
}

void Interpreter::visit(std::shared_ptr<ASTUnpackedDeclarationNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	// check if it's an identifier (variable)
	std::shared_ptr<ASTIdentifierNode> var = nullptr;
	if (astnode->expr) {
		var = std::dynamic_pointer_cast<ASTIdentifierNode>(astnode->expr);
	}

	if (var) {
		var->accept(this);
		if (!TypeDefinition::is_any_or_match_type(*astnode, *current_expression_value)) {
			ExceptionHandler::throw_mismatched_type_err(*astnode, *current_expression_value);
		}
		clear_current_expression();
	}

	for (auto& declaration : astnode->declarations) {
		// if it's a variable, it'll "unpack" struct into declarations
		if (var) {
			// get the id vector
			auto ids = var->identifier_vector;
			// adds sub value identifier to id vector
			// the id vector represents the struct value at the end and sub value identifier the struct subvalue
			ids.push_back(Identifier(declaration->identifier));
			// creates an identifier node as sub declaration assignment expression
			auto access_expr = std::make_shared<ASTIdentifierNode>(ids, var->name_space, declaration->row, declaration->col);
			declaration->expr = access_expr;
		}

		declaration->accept(this);

		if (var) {
			declaration->expr = nullptr;
		}
	}
}

void Interpreter::visit(std::shared_ptr<ASTAssignmentNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();

	// finds assignment variable
	std::shared_ptr<RuntimeVariable> variable = std::dynamic_pointer_cast<RuntimeVariable>(find_inner_most_variable(current_program, astnode->name_space, astnode->identifier));
	RuntimeValue* value = access_value(variable->get_value(), astnode->identifier_vector);

	// evaluate assignment expression
	astnode->expr->accept(this);

	auto ptr_value = current_expression_value; // saves the ptr into a variable to use after

	// check if it's reference
	RuntimeValue* new_value = ptr_value;

	if (!ptr_value->use_ref) {
		new_value = allocate_value(new RuntimeValue(ptr_value));
	}

	validates_reference_type_assignment(*variable, new_value);

	RuntimeOperations::normalize_type(variable.get(), new_value);

	// handle direct assignment
	if (astnode->op == "="
		&& astnode->identifier_vector.size() == 1
		&& astnode->identifier_vector[0].access_vector.size() == 0
		&& !has_string_access) {
		if (!TypeDefinition::is_any_or_match_type(*variable, *ptr_value) ||
			TypeUtils::is_array(variable->type) && !TypeUtils::is_any(variable->array_type)
			&& !TypeDefinition::match_type(*variable, *ptr_value, false, true)) {
			ExceptionHandler::throw_mismatched_type_err(*variable, *ptr_value);
		}

		// direct variable assignment
		variable->set_value(new_value);
	}
	// handle sub value assignment
	else {
		if (astnode->op == "=") {
			// variable sub value assignment
			set_value(variable, astnode->identifier_vector, new_value);
		}
		else {
			// gets string access position
			flx_int pos = -1;
			if (has_string_access) {
				astnode->identifier_vector.back().access_vector[astnode->identifier_vector.back().access_vector.size() - 1]->accept(this);
				pos = current_expression_value->get_i();
				clear_current_expression();
			}

			// do value/sub value operation
			RuntimeOperations::do_operation(astnode->op, value, new_value, false, pos);
		}
	}

}

void Interpreter::visit(std::shared_ptr<ASTFunctionExpressionAssignmentNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();

	// evaluate assigned expression
	astnode->function->accept(this);

	auto variable = current_expression_value;

	// evaluate assignment expression
	astnode->expr->accept(this);

	auto ptr_value = current_expression_value;

	// check if it's reference
	RuntimeValue* new_value = ptr_value;

	if (!ptr_value->use_ref) {
		new_value = allocate_value(new RuntimeValue(ptr_value));
	}

	validates_reference_type_assignment(*variable, new_value);

	RuntimeOperations::normalize_type(variable, new_value);

	if (variable->value_ref && TypeUtils::is_string(variable->value_ref->type) && astnode->op == "=" && TypeUtils::is_char(variable->type)) {
		(*variable->value_ref->get_raw_s())[variable->access_index] = new_value->get_c();
	}
	else {
		RuntimeOperations::do_operation(astnode->op, variable, new_value, false);
	}

}

void Interpreter::visit(std::shared_ptr<ASTReturnNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& name_space = current_program_stack.top()->name_space;

	if (astnode->expr) {
		// gets the current function return
		TypeDefinition curr_func_ret_type = current_function.top();
		//auto curr_func_call_id_vector = current_function_call_identifier_vector.top();
		const auto& curr_func_call_expr_id_vector = current_function_call_expression_identifier_vector.top();
		const auto& curr_func_call_expr_call = current_function_call_expression_call.top();
		// evaluates return expression
		astnode->expr->accept(this);
		// keeps return value
		RuntimeValue* returned_value = current_expression_value;
		clear_current_expression();

		// check types match
		if (!TypeDefinition::is_any_or_match_type(curr_func_ret_type, *returned_value)) {
			ExceptionHandler::throw_return_type_err(current_function.top().identifier,
				curr_func_ret_type, *returned_value);
		}

		// evaluates access vector
		RuntimeValue* value = returned_value;
		if (curr_func_call_expr_id_vector.size() > 0) {
			value = access_value(value, curr_func_call_expr_id_vector);
			// handle string char access
			if (TypeUtils::is_string(value->type) && curr_func_call_expr_id_vector.back().access_vector.size() > 0 && has_string_access) {
				has_string_access = false;
				std::string str = value->get_s();
				curr_func_call_expr_id_vector.back().access_vector.back()->accept(this);
				auto pos = current_expression_value->get_i();

				value = allocate_value(new RuntimeValue(flx_char(str[pos])));
				value->value_ref = returned_value;
				value->access_index = pos;
			}
		}

		current_expression_value = value;

		if (curr_func_call_expr_call) {
			curr_func_call_expr_call->accept(this);
		}
	}
	else {
		current_expression_value = allocate_value(new RuntimeValue(Type::T_UNDEFINED));
	}

	// return node activates return flow
	for (intmax_t i = scopes[name_space].size() - 1; i >= 0; --i) {
		if (!scopes[name_space][i]->name.empty()) {
			return_from_function_name = scopes[name_space][i]->name;
			return_from_function = true;
			break;
		}
	}
}

void Interpreter::visit(std::shared_ptr<ASTFunctionCallNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	auto name_space = astnode->name_space;
	std::string identifier = astnode->identifier;
	std::vector<Identifier> identifier_vector = astnode->identifier_vector;
	bool strict = true;
	std::vector<TypeDefinition*> signature;
	std::shared_ptr<std::vector<RuntimeValue*>> function_arguments = std::make_shared<std::vector<RuntimeValue*>>();
	bool pop_program = false;
	auto returned_expression_value = current_expression_value;
	clear_current_expression();

	// adds function args container to root, to prevent values sweep while evaluating each one
	gc.add_root_container(function_arguments);

	for (auto& param : astnode->parameters) {
		param->accept(this);

		// check if it's reference
		RuntimeValue* pvalue = current_expression_value;
		clear_current_expression();
		if (!pvalue->use_ref) {
			pvalue = allocate_value(new RuntimeValue(pvalue));
		}

		function_arguments->push_back(pvalue);
		signature.push_back(pvalue);
	}

	std::shared_ptr<Scope> func_scope;

	// handle function return
	if (astnode->identifier.empty()) {
		name_space = returned_expression_value->get_fun().first;
		identifier = returned_expression_value->get_fun().second;

		func_scope = find_declared_function_strict(current_program, name_space, identifier, signature, strict, pop_program);

		if (!func_scope) {
			ExceptionHandler::undeclared_function(identifier, signature);
		}

	}
	// handle subvalue call
	else if (astnode->identifier_vector.size() > 1) {
		auto idnode = std::make_shared<ASTIdentifierNode>(astnode->identifier_vector, name_space, astnode->row, astnode->col);
		idnode->accept(this);

		name_space = current_expression_value->get_fun().first;
		identifier = current_expression_value->get_fun().second;
		clear_current_expression();

		func_scope = find_declared_function_strict(current_program, name_space, identifier, signature, strict, pop_program);

		if (!func_scope) {
			ExceptionHandler::undeclared_function(identifier, signature);
		}

	}
	// handle regular call
	else {
		// check if is a common declared function
		func_scope = find_declared_function_strict(current_program, name_space, identifier, signature, strict, pop_program);

		if (!func_scope) {
			// set strict to true again to try find from variable
			strict = true;

			auto var_scope = get_inner_most_variable_scope(current_program, name_space, identifier);

			// if there's no variable
			if (!var_scope) {
				ExceptionHandler::undeclared_function(identifier, signature);
			}

			// gets variable value
			auto var = std::dynamic_pointer_cast<RuntimeVariable>(var_scope->find_declared_variable(identifier));
			auto var_value = var->get_value();

			// if variable is not a function type, throw error
			if (!TypeUtils::is_function(var_value->type)) {
				ExceptionHandler::undeclared_function(identifier, signature);
			}

			// get function namespace and name
			name_space = var_value->get_fun().first;
			identifier = var_value->get_fun().second;

			func_scope = find_declared_function_strict(current_program, name_space, identifier, signature, strict, pop_program);

			if (!func_scope) {
				ExceptionHandler::undeclared_function(identifier, signature);
			}

		}

	}

	auto& declfun = func_scope->find_declared_function(identifier, &signature, strict);

	current_function.push(declfun);
	current_function_defined_parameters.push(declfun.parameters);
	current_this_name.push(identifier);
	current_function_signature.push(signature);
	current_function_call_expression_identifier_vector.push(astnode->expression_identifier_vector);
	current_function_call_expression_call.push(astnode->expression_call);
	current_function_calling_arguments.push(*function_arguments);

	// it's not a stack cause it's one shot use, right it reachs block it's cleaned
	function_call_name = identifier;
	declfun.block->accept(this);

	current_function.pop();
	current_function_call_expression_identifier_vector.pop();
	current_function_call_expression_call.pop();
	current_function_signature.pop();
	current_this_name.pop();
	gc.remove_root_container(function_arguments);

	if (pop_program) {
		current_program_stack.pop();
	}

}

void Interpreter::visit(std::shared_ptr<ASTBuiltinCallNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	builtin_functions[astnode->identifier]();

	const auto& expression_identifier_vector = current_function_call_expression_identifier_vector.top();

	if (expression_identifier_vector.size() > 0) {
		current_expression_value = access_value(current_expression_value, expression_identifier_vector);
	}

	gc.collect();

}

void Interpreter::visit(std::shared_ptr<ASTFunctionDefinitionNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = astnode->type_name_space;

	try {
		// if its already declared, it's a block definition
		auto& declfun = scopes[name_space].back()->find_declared_function(astnode->identifier, &astnode->parameters, true);
		declfun.block = astnode->block;
	}
	catch (...) {
		auto& block = astnode->block;

		// if node not has block and it's a builtin, it's create a builtin executor
		if (!block && builtin_functions.find(astnode->identifier) != builtin_functions.end()) {
			block = std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
				std::make_shared<ASTBuiltinCallNode>(astnode->identifier, astnode->row, astnode->col)
			}, astnode->row, astnode->col);
		}

		scopes[name_space].back()->declare_function(astnode->identifier,
			FunctionDefinition(astnode->identifier, astnode->type, astnode->type_name, name_space,
				astnode->array_type, astnode->dim, astnode->parameters, block, astnode->row, astnode->row));
	}

}

void Interpreter::visit(std::shared_ptr<ASTLambdaFunction> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& name_space = current_program_stack.top()->name_space;

	// generate an random identifier and evaluates
	auto& fun = astnode->fun;

	auto fun_name = "lambda@" + utils::FlexaUUID::generate();
	while (scopes[name_space].back()->already_declared_function_name(fun_name)) {
		fun_name = "lambda@" + utils::FlexaUUID::generate();
	}

	fun->identifier = fun_name;
	fun->accept(this);

	current_expression_value = allocate_value(new RuntimeValue(flx_function(name_space, fun->identifier)));
}

void Interpreter::visit(std::shared_ptr<ASTBlockNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;
	auto current_function_call_name = function_call_name;
	function_call_name = "";

	scopes[name_space].push_back(std::make_shared<Scope>(current_program, current_function_call_name));

	// declare all parameters in block if its a function
	declare_function_block_parameters(name_space);

	// executes block 
	for (auto& stmt : astnode->statements) {
		stmt->accept(this);

		if (exit_from_program) {
			return;
		}

		if (continue_block && (is_loop)) {
			break;
		}

		if (break_block && (is_loop || is_switch)) {
			break;
		}

		if (return_from_function) {
			if (!return_from_function_name.empty() && return_from_function_name == scopes[name_space].back()->name) {
				return_from_function_name = "";
				return_from_function = false;
			}
			break;
		}
	}

	scopes[name_space].pop_back();
	gc.collect();
}

void Interpreter::visit(std::shared_ptr<ASTExitNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->exit_code->accept(this);
	if (!TypeUtils::is_int(current_expression_value->type)) {
		throw std::runtime_error("expected int value");
	}
	exit_from_program = true;
}

void Interpreter::visit(std::shared_ptr<ASTContinueNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	continue_block = true;
}

void Interpreter::visit(std::shared_ptr<ASTBreakNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	break_block = true;
}

void Interpreter::visit(std::shared_ptr<ASTSwitchNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	++is_switch; // use increment due to nested cases

	scopes[name_space].push_back(std::make_shared<Scope>(current_program));

	// if it has case blocks we evaluate condition,
	// if it's not, we don't need do nothing and save cpu
	if (astnode->case_blocks.size() > 0) {
		// here it'll validates if matches type
		astnode->condition->accept(this);
		TypeDefinition cond_type = *current_expression_value;
		clear_current_expression();

		// as all cases have same type (guaranted by constant values in semantic analysis),
		// we just need to evaluate first case to get type
		for (const auto& expr : astnode->case_blocks) {
			expr.first->accept(this);
			break;
		}
		TypeDefinition case_type = *current_expression_value;
		clear_current_expression();

		if (!TypeDefinition::match_type(cond_type, case_type)) {
			ExceptionHandler::throw_mismatched_type_err(cond_type, case_type);
		}
	}

	intmax_t pos;
	try {
		// if has and match condition
		auto hash = astnode->condition->hash(this);
		pos = astnode->parsed_case_blocks.at(hash);
	}
	catch (...) {
		// else we go to default block
		pos = astnode->default_block;
	}

	// executes block
	for (intmax_t i = pos; i < astnode->statements.size(); ++i) {
		astnode->statements.at(i)->accept(this);

		if (exit_from_program) {
			return;
		}

		if (continue_block) {
			continue_block = false;
			continue;
		}

		if (break_block) {
			break_block = false;
			break;
		}

		if (return_from_function) {
			if (!return_from_function_name.empty() && return_from_function_name == scopes[name_space].back()->name) {
				return_from_function_name = "";
				return_from_function = false;
			}
			break;
		}
	}

	scopes[name_space].pop_back();
	gc.collect();
	--is_switch;
}

void Interpreter::visit(std::shared_ptr<ASTElseIfNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	executed_elif = false;

	astnode->condition->accept(this);

	if (!TypeUtils::is_bool(current_expression_value->type)) {
		ExceptionHandler::throw_condition_type_err();
	}

	bool result = current_expression_value->get_b();
	clear_current_expression();

	if (result) {
		astnode->block->accept(this);
		executed_elif = true;
	}
}

void Interpreter::visit(std::shared_ptr<ASTIfNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->condition->accept(this);

	if (!TypeUtils::is_bool(current_expression_value->type)) {
		ExceptionHandler::throw_condition_type_err();
	}

	bool result = current_expression_value->get_b();
	clear_current_expression();

	if (result) {
		// execute if block
		astnode->if_block->accept(this);
	}
	else {
		// check each else if block
		for (auto& elif : astnode->else_ifs) {
			elif->accept(this);
			// if entered some, we stops
			if (executed_elif) {
				break;
			}
		}
		// if has else block and not executed else if
		if (astnode->else_block && !executed_elif) {
			astnode->else_block->accept(this);
		}
	}

	executed_elif = false;
}

void Interpreter::visit(std::shared_ptr<ASTForNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	++is_loop;
	scopes[name_space].push_back(std::make_shared<Scope>(current_program));

	// the first statement executes once at start
	if (astnode->expressions[0]) {
		astnode->expressions[0]->accept(this);
	}

	for (;;) {
		// the second statement executes after each block execution
		// and defines the condition for executing the block
		if (astnode->expressions[1]) {
			astnode->expressions[1]->accept(this);

			if (!TypeUtils::is_bool(current_expression_value->type)) {
				ExceptionHandler::throw_condition_type_err();
			}
		}
		else {
			// if empty, execute with no condition
			current_expression_value = allocate_value(new RuntimeValue(flx_bool(true)));
		}

		auto result = current_expression_value->get_b();
		clear_current_expression();

		// if result is false
		if (!result) {
			break;
		}

		astnode->block->accept(this);

		if (exit_from_program) {
			return;
		}

		if (return_from_function) {
			break;
		}

		// always execute after the block
		if (astnode->expressions[2]) {
			astnode->expressions[2]->accept(this);
		}

		if (continue_block) {
			continue_block = false;
		}

		if (break_block) {
			break_block = false;
			break;
		}
	}

	scopes[name_space].pop_back();
	gc.collect();
	--is_loop;
}

void Interpreter::visit(std::shared_ptr<ASTForEachNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	++is_loop;

	// stores colection at current_expression_value 
	astnode->collection->accept(this);

	// adds a meta scope, to store current collection value
	scopes[name_space].push_back(std::make_shared<Scope>(current_program));

	switch (current_expression_value->type) {
	case Type::T_ARRAY: { // if the collection is an array
		auto colletion = current_expression_value->get_arr();
		clear_current_expression();
		auto root_colletion = std::make_shared<flx_array>(colletion);
		gc.add_array_root(root_colletion);

		for (size_t i = 0; i < colletion.size(); ++i) {
			auto exnode = std::make_shared<ASTValueNode>(colletion[i], astnode->row, astnode->col);

			// declare each value at meta block
			if (auto itdecl = std::dynamic_pointer_cast<ASTDeclarationNode>(astnode->itdecl)) {
				itdecl->expr = exnode;
				itdecl->accept(this);
				itdecl->expr = nullptr;

			}
			else if (auto itdecl = std::dynamic_pointer_cast<ASTIdentifierNode>(astnode->itdecl)) {
				auto assign_node = std::make_shared<ASTAssignmentNode>(itdecl->identifier_vector, itdecl->name_space, "=", exnode, itdecl->row, itdecl->col);
				assign_node->accept(this);

			}

			astnode->block->accept(this);

			if (exit_from_program) {
				return;
			}

			if (continue_block) {
				continue_block = false;
				continue;
			}

			if (break_block) {
				break_block = false;
				break;
			}

			if (return_from_function) {
				break;
			}
		}

		gc.add_array_root(root_colletion);

		break;
	}
	case Type::T_STRING: { // if the collection is a string
		auto colletion = current_expression_value->get_s();
		clear_current_expression();

		for (auto val : colletion) {
			auto exnode = std::make_shared<ASTValueNode>(allocate_value(new RuntimeValue(flx_char(val))), astnode->row, astnode->col);

			// declare each value at meta block
			if (auto itdecl = std::dynamic_pointer_cast<ASTDeclarationNode>(astnode->itdecl)) {
				itdecl->expr = exnode;
				itdecl->accept(this);
				itdecl->expr = nullptr;

			}
			else if (auto itdecl = std::dynamic_pointer_cast<ASTIdentifierNode>(astnode->itdecl)) {
				auto assign_node = std::make_shared<ASTAssignmentNode>(itdecl->identifier_vector, itdecl->name_space, "=", exnode, itdecl->row, itdecl->col);
				assign_node->accept(this);

			}

			astnode->block->accept(this);

			if (exit_from_program) {
				return;
			}

			if (continue_block) {
				continue_block = false;
				continue;
			}

			if (break_block) {
				break_block = false;
				break;
			}

			if (return_from_function) {
				break;
			}
		}
		break;
	}
	case Type::T_STRUCT: { // if the collection is a struct
		auto coll_expr = current_expression_value;
		clear_current_expression();
		gc.add_ptr_root(&coll_expr);

		const auto& colletion = coll_expr->get_str();

		for (const auto& val : colletion) {
			auto key = std::make_shared<ASTLiteralNode<flx_string>>(flx_string(val.first), astnode->row, astnode->col);
			auto value = std::make_shared<ASTValueNode>(val.second, astnode->row, astnode->col);

			// when handling structs, we have a third type of declaration: unpacked declaration
			if (auto itdecl = std::dynamic_pointer_cast<ASTDeclarationNode>(astnode->itdecl)) {
				std::map<std::string, std::shared_ptr<ASTExprNode>> values = { { "key", key }, { "value", value } };
				auto exnode = std::make_shared<ASTStructConstructorNode>("Pair", Constants::STD_NAMESPACE, values, astnode->row, astnode->col);
				itdecl->expr = exnode;
				itdecl->accept(this);
				itdecl->expr = nullptr;

			}
			else if (auto itdecl = std::dynamic_pointer_cast<ASTIdentifierNode>(astnode->itdecl)) {
				std::map<std::string, std::shared_ptr<ASTExprNode>> values = { { "key", key }, { "value", value } };
				auto exnode = std::make_shared<ASTStructConstructorNode>("Pair", Constants::STD_NAMESPACE, values, astnode->row, astnode->col);
				auto assign_node = std::make_shared<ASTAssignmentNode>(itdecl->identifier_vector, itdecl->name_space, "=", exnode, itdecl->row, itdecl->col);
				assign_node->accept(this);

			}
			else if (const auto idnode = std::dynamic_pointer_cast<ASTUnpackedDeclarationNode>(astnode->itdecl)) {
				// expect a 2 sized unpacked declaration
				if (idnode->declarations.size() != 2) {
					throw std::runtime_error("invalid number of values");
				}

				idnode->declarations[0]->expr = key;
				idnode->declarations[1]->expr = value;
				idnode->accept(this);
				idnode->declarations[0]->expr = nullptr;
				idnode->declarations[1]->expr = nullptr;
			}

			astnode->block->accept(this);

			if (exit_from_program) {
				return;
			}

			if (continue_block) {
				continue_block = false;
				continue;
			}

			if (break_block) {
				break_block = false;
				break;
			}

			if (return_from_function) {
				break;
			}
		}

		gc.remove_ptr_root(&coll_expr);

		break;
	}
	default:
		throw std::runtime_error("invalid foreach iterable type");
	}

	scopes[name_space].pop_back();
	gc.collect();
	--is_loop;
}

void Interpreter::visit(std::shared_ptr<ASTTryCatchNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	try {
		scopes[name_space].push_back(std::make_shared<Scope>(current_program));

		astnode->try_block->accept(this);

		scopes[name_space].pop_back();
		gc.collect();
	}
	catch (std::runtime_error ex) {
		scopes[name_space].pop_back();
		gc.collect();

		scopes[name_space].push_back(std::make_shared<Scope>(current_program));

		auto error_node = std::make_shared<ASTLiteralNode<flx_string>>(ex.what(), astnode->row, astnode->col);
		auto code_node = std::make_shared<ASTLiteralNode<flx_int>>(0, astnode->row, astnode->col);

		// another place we can use unpacked declaration
		if (const auto idnode = std::dynamic_pointer_cast<ASTUnpackedDeclarationNode>(astnode->decl)) {
			idnode->declarations[0]->expr = error_node;
			idnode->declarations[1]->expr = code_node;
			idnode->accept(this);
			idnode->declarations[0]->expr = nullptr;
			idnode->declarations[1]->expr = nullptr;
		}
		else if (const auto idnode = std::dynamic_pointer_cast<ASTDeclarationNode>(astnode->decl)) {
			std::map<std::string, std::shared_ptr<ASTExprNode>> values = { { "error", error_node }, { "code", code_node } };
			auto exnode = std::make_shared<ASTStructConstructorNode>("Exception", Constants::STD_NAMESPACE, values, astnode->row, astnode->col);
			idnode->expr = exnode;
			idnode->accept(this);
			idnode->expr = nullptr;
		}
		else if (!std::dynamic_pointer_cast<ASTEllipsisNode>(astnode->decl)) {
			throw std::runtime_error("expected declaration");
		}

		astnode->catch_block->accept(this);
		scopes[name_space].pop_back();
		gc.collect();

	}

}

void Interpreter::visit(std::shared_ptr<ASTThrowNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->error->accept(this);

	auto throw_expression = current_expression_value;
	clear_current_expression();

	// handle Exception struct
	if (TypeUtils::is_struct(throw_expression->type)) {
		// check struct type
		if (throw_expression->type_name != "Exception"
			|| throw_expression->type_name_space != Constants::STD_NAMESPACE) {
			throw std::runtime_error("expected flx::Exception not " + TypeDefinition::buid_type_str(*throw_expression));
		}

		throw std::runtime_error(throw_expression->get_str()["error"]->get_s().c_str());
	}
	// handle bare string
	else if (TypeUtils::is_string(throw_expression->type)) {
		throw std::runtime_error(throw_expression->get_s());
	}
	else {
		throw std::runtime_error("expected flx::Exception struct or string in throw");
	}

}

void Interpreter::visit(std::shared_ptr<ASTEllipsisNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	current_expression_value = allocate_value(new RuntimeValue(Type::T_UNDEFINED));
}

void Interpreter::visit(std::shared_ptr<ASTWhileNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	++is_loop;

	for (;;) {
		// evaluates while condition after each block execution
		astnode->condition->accept(this);

		if (!TypeUtils::is_bool(current_expression_value->type)) {
			ExceptionHandler::throw_condition_type_err();
		}

		auto result = current_expression_value->get_b();
		clear_current_expression();

		if (!result) {
			break;
		}

		astnode->block->accept(this);

		if (exit_from_program) {
			return;
		}

		if (continue_block) {
			continue_block = false;
		}

		if (break_block) {
			break_block = false;
			break;
		}

		if (return_from_function) {
			break;
		}
	}

	--is_loop;
}

void Interpreter::visit(std::shared_ptr<ASTDoWhileNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	++is_loop;

	bool result = false;

	do {
		astnode->block->accept(this);

		if (exit_from_program) {
			return;
		}

		if (continue_block) {
			continue_block = false;
		}

		if (break_block) {
			break_block = false;
			break;
		}

		if (return_from_function) {
			break;
		}

		// executes condition at the end of block
		astnode->condition->accept(this);

		auto result_expr = current_expression_value;
		clear_current_expression();

		if (!TypeUtils::is_bool(result_expr->type)) {
			ExceptionHandler::throw_condition_type_err();
		}

		result = result_expr->get_b();

	} while (result);

	--is_loop;
}

void Interpreter::visit(std::shared_ptr<ASTStructDefinitionNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& name_space = current_program_stack.top()->name_space;

	for (auto& var : astnode->variables) {
		var.second.dim = evaluate_access_vector(var.second.expr_dim);
	}

	scopes[name_space].back()->declare_structure_definition(
		StructureDefinition(astnode->identifier, astnode->variables, astnode->row, astnode->col)
	);
}

void Interpreter::visit(std::shared_ptr<ASTValueNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	current_expression_value = dynamic_cast<RuntimeValue*>(astnode->value);
}

void Interpreter::visit(std::shared_ptr<ASTLiteralNode<flx_bool>> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	current_expression_value = allocate_value(new RuntimeValue(astnode->value));
}

void Interpreter::visit(std::shared_ptr<ASTLiteralNode<flx_int>> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	current_expression_value = allocate_value(new RuntimeValue(astnode->value));
}

void Interpreter::visit(std::shared_ptr<ASTLiteralNode<flx_float>> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	current_expression_value = allocate_value(new RuntimeValue(astnode->value));
}

void Interpreter::visit(std::shared_ptr<ASTLiteralNode<flx_char>> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	current_expression_value = allocate_value(new RuntimeValue(astnode->value));
}

void Interpreter::visit(std::shared_ptr<ASTLiteralNode<flx_string>> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	current_expression_value = allocate_value(new RuntimeValue(astnode->value));
}

void Interpreter::visit(std::shared_ptr<ASTArrayConstructorNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	// initialize raw array
	flx_array arr = flx_array(astnode->values.size());

	// clean array type on start
	if (current_expression_array_dim.size() == 0) {
		current_expression_array_type = TypeDefinition();
		// used to control nested arrays
		current_expression_array_dim_max = 0;
		// end of array dimension calculation, reached max
		is_max = false;
	}

	// increments array dimension
	++current_expression_array_dim_max;
	// if isn't reached max yet, we adds more one expr_dim
	if (!is_max) {
		current_expression_array_dim.push_back(arr.size());
	}

	for (size_t i = 0; i < astnode->values.size(); ++i) {
		const auto& expr = astnode->values[i];

		expr->accept(this);

		// check if it's a reference
		RuntimeValue* arr_value = current_expression_value;
		clear_current_expression();

		if (!arr_value->use_ref) {
			arr_value = allocate_value(new RuntimeValue(arr_value));
		}

		// if it's undefined or array (nested), it's accepts the first type encountered
		if (TypeUtils::is_undefined(current_expression_array_type.type) || TypeUtils::is_array(current_expression_array_type.type)) {
			current_expression_array_type = *arr_value;
		}
		else {
			// else check if is any array
			if (!TypeUtils::match_type(current_expression_array_type.type, arr_value->type)
				&& !TypeUtils::is_any(arr_value->type) && !TypeUtils::is_void(arr_value->type)
				&& !TypeUtils::is_array(arr_value->type)) {
				current_expression_array_type = TypeDefinition::get_basic(Type::T_ANY);
			}
		}

		arr[i] = arr_value;
	}

	// as size by dimension is fixed, it's not necessary to check after max (max nested deep)
	is_max = true;

	current_expression_value = allocate_value(
		new RuntimeValue(arr, current_expression_array_type.type, current_expression_array_dim,
			current_expression_array_type.type_name, current_expression_array_type.type_name_space)
	);

	// here it's calculate de current dimension of array
	--current_expression_array_dim_max;
	size_t stay = current_expression_array_dim.size() - current_expression_array_dim_max;
	std::vector<size_t> current_expression_array_dim_aux;
	size_t curr_dim_i = current_expression_array_dim.size() - 1;
	for (size_t i = 0; i < stay; ++i) {
		current_expression_array_dim_aux.emplace(current_expression_array_dim_aux.begin(), current_expression_array_dim.at(curr_dim_i));
		--curr_dim_i;
	}
	current_expression_value->dim = current_expression_array_dim_aux;

	// final type check
	if (current_expression_array_dim_max == 0) {
		if (TypeUtils::is_undefined(current_expression_value->array_type)) {
			current_expression_value->array_type = Type::T_ANY;
		}
		current_expression_array_dim.clear();
	}
}

void Interpreter::visit(std::shared_ptr<ASTStructConstructorNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = astnode->name_space;

	auto type_struct = find_inner_most_struct(current_program, name_space, astnode->type_name);

	auto str = flx_struct();

	for (auto& expr : astnode->values) {
		// check it is a member
		if (type_struct.variables.find(expr.first) == type_struct.variables.end()) {
			ExceptionHandler::throw_struct_member_err(name_space, astnode->type_name, expr.first);
		}
		VariableDefinition var_type_struct = type_struct.variables[expr.first];

		expr.second->accept(this);

		RuntimeValue* str_value = current_expression_value;
		clear_current_expression();

		if (!TypeDefinition::is_any_or_match_type(var_type_struct, *str_value)) {
			ExceptionHandler::throw_struct_value_assign_type_err(name_space, astnode->type_name,
				expr.first, var_type_struct, *str_value);
		}

		// check if it's a reference
		if (!str_value->use_ref) {
			str_value = allocate_value(new RuntimeValue(str_value));
		}

		check_build_array(str_value, evaluate_access_vector(var_type_struct.expr_dim));
		validates_reference_type_assignment(var_type_struct, str_value);
		RuntimeOperations::normalize_type(&var_type_struct, str_value);

		if (!TypeUtils::is_any(var_type_struct.type) && !TypeUtils::is_void(str_value->type)) {
			str_value->type = var_type_struct.type;
			str_value->array_type = var_type_struct.array_type;
			str_value->type_name = var_type_struct.type_name;
			str_value->type_name_space = var_type_struct.type_name_space;
		}

		str[expr.first] = str_value;
	}

	// declare rest values as null
	for (auto& struct_var_def : type_struct.variables) {
		if (str.find(struct_var_def.first) == str.end()) {
			RuntimeValue* str_value = allocate_value(new RuntimeValue(struct_var_def.second.type));
			str_value->set_null();
			str[struct_var_def.first] = str_value;
		}
	}

	current_expression_value = allocate_value(new RuntimeValue(str, astnode->type_name, name_space));

}

void Interpreter::visit(std::shared_ptr<ASTIdentifierNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = astnode->name_space;

	// handle regular identifier
	if (const auto& id_scope = get_inner_most_variable_scope(current_program, name_space, astnode->identifier)) {
		auto variable = std::dynamic_pointer_cast<RuntimeVariable>(id_scope->find_declared_variable(astnode->identifier));
		auto sub_val = access_value(variable->get_value(), astnode->identifier_vector);
		sub_val->reset_ref();

		current_expression_value = sub_val;

		if (current_expression_value->type == Type::T_STRING && astnode->identifier_vector.back().access_vector.size() > 0 && has_string_access) {
			has_string_access = false;
			auto str = current_expression_value->get_s();
			std::dynamic_pointer_cast<ASTExprNode>(astnode->identifier_vector.back().access_vector[astnode->identifier_vector.back().access_vector.size() - 1])->accept(this);
			auto pos = current_expression_value->get_i();

			auto char_value = allocate_value(new RuntimeValue(flx_char(str[pos])));
			char_value->value_ref = sub_val;
			char_value->access_index = pos;
			current_expression_value = char_value;

		}

	}
	// handle struct type names for type checks
	else if (get_inner_most_struct_definition_scope(current_program, name_space, astnode->identifier)) {
		const auto& expr_dim = astnode->identifier_vector[0].access_vector;
		auto expression_value = allocate_value(new RuntimeValue(astnode->identifier, name_space));

		if (expr_dim.size() > 0) {
			auto dim = evaluate_access_vector(expr_dim);
			flx_array arr = build_array(dim, expression_value, expr_dim.size() - 1);
			current_expression_value = allocate_value(new RuntimeValue(arr, Type::T_STRUCT, dim, astnode->identifier, name_space));

		}
		else {
			current_expression_value = expression_value;

		}

	}
	// handle expression function calls
	else if (get_inner_most_function_scope(current_program, name_space, astnode->identifier, nullptr)) {
		auto fun = flx_function{ name_space, astnode->identifier };
		current_expression_value = allocate_value(new RuntimeValue(fun));

	}
	else {
		throw std::runtime_error("identifier '" + astnode->identifier + "' was not declared");

	}

}

void Interpreter::visit(std::shared_ptr<ASTBinaryExprNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->left->accept(this);

	RuntimeValue* l_value = current_expression_value;
	clear_current_expression();

	if (!l_value->use_ref) {
		l_value = allocate_value(new RuntimeValue(l_value));
	}
	gc.add_root(l_value);

	if (TypeUtils::is_bool(l_value->type) && astnode->op == "and" && !l_value->get_b()) {
		current_expression_value = l_value;
		gc.remove_root(l_value);
		return;
	}

	astnode->right->accept(this);
	RuntimeValue* r_value = current_expression_value;
	clear_current_expression();

	if (!r_value->use_ref) {
		r_value = allocate_value(new RuntimeValue(r_value));
	}
	gc.add_root(r_value);

	current_expression_value = RuntimeOperations::do_operation(astnode->op, l_value, r_value, true);

	if (current_expression_value != l_value && current_expression_value != r_value) {
		allocate_value(current_expression_value);
	}

	gc.remove_root(l_value);
	gc.remove_root(r_value);
}

void Interpreter::visit(std::shared_ptr<ASTTernaryNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->condition->accept(this);

	RuntimeValue* condition_expr = current_expression_value;
	clear_current_expression();

	if (!TypeUtils::is_bool(condition_expr->type)) {
		ExceptionHandler::throw_condition_type_err();
	}

	if (condition_expr->get_b()) {
		astnode->value_if_true->accept(this);
	}
	else {
		astnode->value_if_false->accept(this);
	}
}

void Interpreter::visit(std::shared_ptr<ASTInNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->value->accept(this);

	RuntimeValue expr_val = RuntimeValue(current_expression_value);
	clear_current_expression();

	astnode->collection->accept(this);

	RuntimeValue coll_val = RuntimeValue(current_expression_value);
	clear_current_expression();

	bool res = false;

	if (TypeUtils::is_array(coll_val.type)) {
		flx_array expr_col = coll_val.get_arr();

		for (size_t i = 0; i < expr_col.size(); ++i) {
			res = RuntimeOperations::equals_value(&expr_val, expr_col[i]);
			if (res) {
				break;
			}
		}
	}
	else {
		const auto& expr_col = coll_val.get_s();

		if (TypeUtils::is_char(expr_val.type)) {
			res = coll_val.get_s().find(expr_val.get_c()) != std::string::npos;
		}
		else {
			res = coll_val.get_s().find(expr_val.get_s()) != std::string::npos;
		}
	}

	current_expression_value = allocate_value(new RuntimeValue(flx_bool(res)));
}

void Interpreter::visit(std::shared_ptr<ASTUnaryExprNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	if (astnode->unary_op == "--" || astnode->unary_op == "++") {
		auto expr = std::make_shared<ASTLiteralNode<flx_int>>(1, astnode->row, astnode->col);
		if (const auto id = std::dynamic_pointer_cast<ASTIdentifierNode>(astnode->expr)) {
			auto assign_node = std::make_shared<ASTAssignmentNode>(id->identifier_vector, id->name_space, std::string{ astnode->unary_op[0] } + "=", expr, astnode->row, astnode->col);
			assign_node->accept(this);
		}
		else {
			auto binex_node = std::make_shared<ASTBinaryExprNode>(std::string{ astnode->unary_op[0] }, astnode->expr, expr, astnode->row, astnode->col);
			binex_node->accept(this);
		}
	}
	else {
		astnode->expr->accept(this);

		if (astnode->unary_op == "ref" || astnode->unary_op == "unref") {
			if (astnode->unary_op == "unref") {
				current_expression_value->use_ref = false;
			}
			else if (astnode->unary_op == "ref") {
				current_expression_value->use_ref = true;
			}
		}
		else {
			if (!current_expression_value->use_ref) {
				current_expression_value = allocate_value(new RuntimeValue(current_expression_value));
			}

			switch (current_expression_value->type) {
			case Type::T_INT:
				if (astnode->unary_op == "-") {
					current_expression_value->set(flx_int(-current_expression_value->get_i()));
				}
				else if (astnode->unary_op == "~") {
					current_expression_value->set(flx_int(~current_expression_value->get_i()));
				}
				else {
					ExceptionHandler::throw_unary_operation_err(astnode->unary_op, *current_expression_value);
				}
				break;
			case Type::T_FLOAT:
				if (astnode->unary_op == "-") {
					current_expression_value->set(flx_float(-current_expression_value->get_f()));
				}
				else {
					ExceptionHandler::throw_unary_operation_err(astnode->unary_op, *current_expression_value);
				}
				break;
			case Type::T_BOOL:
				if (astnode->unary_op == "not") {
					current_expression_value->set(flx_bool(!current_expression_value->get_b()));
				}
				else {
					ExceptionHandler::throw_unary_operation_err(astnode->unary_op, *current_expression_value);
				}
				break;
			default:
				ExceptionHandler::throw_unary_operation_err(astnode->unary_op, *current_expression_value);
			}
		}
	}

}

void Interpreter::visit(std::shared_ptr<ASTTypeCastNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	RuntimeValue* new_value = allocate_value(new RuntimeValue());

	switch (astnode->type) {
	case Type::T_BOOL:
		switch (current_expression_value->type) {
		case Type::T_BOOL:
			new_value->copy_from(current_expression_value);
			break;
		case Type::T_INT:
			new_value->set(flx_bool(current_expression_value->get_i() != 0));
			break;
		case Type::T_FLOAT:
			new_value->set(flx_bool(current_expression_value->get_f() != .0));
			break;
		case Type::T_CHAR:
			new_value->set(flx_bool(current_expression_value->get_c() != '\0'));
			break;
		case Type::T_STRING:
			new_value->set(flx_bool(!current_expression_value->get_s().empty()));
			break;
		}
		break;

	case Type::T_INT:
		switch (current_expression_value->type) {
		case Type::T_BOOL:
			new_value->set(flx_int(current_expression_value->get_b()));
			break;
		case Type::T_INT:
			new_value->copy_from(current_expression_value);
			break;
		case Type::T_FLOAT:
			new_value->set(flx_int(current_expression_value->get_f()));
			break;
		case Type::T_CHAR:
			new_value->set(flx_int(current_expression_value->get_c()));
			break;
		case Type::T_STRING:
			try {
				new_value->set(flx_int(std::stoll(current_expression_value->get_s())));
			}
			catch (...) {
				throw std::runtime_error("'" + current_expression_value->get_s() + "' is not a valid value to parse int");
			}
			break;
		}
		break;

	case Type::T_FLOAT:
		switch (current_expression_value->type) {
		case Type::T_BOOL:
			new_value->set(flx_float(current_expression_value->get_b()));
			break;
		case Type::T_INT:
			new_value->set(flx_float(current_expression_value->get_i()));
			break;
		case Type::T_FLOAT:
			new_value->copy_from(current_expression_value);
			break;
		case Type::T_CHAR:
			new_value->set(flx_float(current_expression_value->get_c()));
			break;
		case Type::T_STRING:
			try {
				new_value->set(flx_float(std::stold(current_expression_value->get_s())));
			}
			catch (...) {
				throw std::runtime_error("'" + current_expression_value->get_s() + "' is not a valid value to parse float");
			}
			break;
		}
		break;

	case Type::T_CHAR:
		switch (current_expression_value->type) {
		case Type::T_BOOL:
			new_value->set(flx_char(current_expression_value->get_b()));
			break;
		case Type::T_INT:
			new_value->set(flx_char(current_expression_value->get_i()));
			break;
		case Type::T_FLOAT:
			new_value->set(flx_char(current_expression_value->get_f()));
			break;
		case Type::T_CHAR:
			new_value->copy_from(current_expression_value);
			break;
		case Type::T_STRING:
			if (new_value->get_s().size() > 1) {
				throw std::runtime_error("'" + current_expression_value->get_s() + "' is not a valid value to parse char");
			}
			else {
				new_value->set(flx_char(current_expression_value->get_s()[0]));
			}
			break;
		}
		break;

	case Type::T_STRING:
		new_value->set(flx_string(RuntimeOperations::parse_value_to_string(current_expression_value)));

	}

	new_value->set_type(astnode->type);

	current_expression_value = new_value;
}

void Interpreter::visit(std::shared_ptr<ASTTypeNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	auto& type = astnode->type;
	type.dim = evaluate_access_vector(type.expr_dim);
	current_expression_value = allocate_value(new RuntimeValue(type));
}

void Interpreter::visit(std::shared_ptr<ASTNullNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression_value = allocate_value(new RuntimeValue(Type::T_VOID));
}

void Interpreter::visit(std::shared_ptr<ASTThisNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression_value = allocate_value(new RuntimeValue(flx_string(current_this_name.top())));
}

void Interpreter::visit(std::shared_ptr<ASTTypeOfNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	auto str_type = TypeDefinition::buid_type_str(*current_expression_value);

	current_expression_value = allocate_value(new RuntimeValue(str_type));
}

void Interpreter::visit(std::shared_ptr<ASTTypeIdNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	auto str_type = TypeDefinition::buid_type_str(*current_expression_value);

	current_expression_value = allocate_value(new RuntimeValue(flx_int(utils::StringUtils::hashcode(str_type))));
}

void Interpreter::visit(std::shared_ptr<ASTRefIdNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	current_expression_value = allocate_value(new RuntimeValue(flx_int(current_expression_value)));
}

void Interpreter::visit(std::shared_ptr<ASTIsStructNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	auto check_expr = current_expression_value;

	current_expression_value = allocate_value(
		new RuntimeValue(
			flx_bool(
				TypeUtils::is_struct(check_expr->type)
			)
		)
	);
}

void Interpreter::visit(std::shared_ptr<ASTIsArrayNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	auto check_expr = current_expression_value;

	current_expression_value = allocate_value(
		new RuntimeValue(
			flx_bool(
				TypeUtils::is_array(check_expr->type) || check_expr->dim.size() > 0
			)
		)
	);
}

void Interpreter::visit(std::shared_ptr<ASTIsAnyNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	auto check_expr = current_expression_value;

	current_expression_value = allocate_value(
		new RuntimeValue(
			flx_bool(
				check_expr->ref.lock()
				&& (
					TypeUtils::is_any(check_expr->ref.lock()->type) || TypeUtils::is_any(check_expr->ref.lock()->array_type)
					)
			)
		)
	);
}

RuntimeValue* Interpreter::set_value(std::shared_ptr<RuntimeVariable> var, const std::vector<Identifier>& identifier_vector, RuntimeValue* new_value) {
	if (identifier_vector.size() == 1 && identifier_vector[0].access_vector.size() == 0) {
		var->set_value(new_value);
		return var->get_value();
	}

	RuntimeValue* value = var->get_value();
	size_t i = 0;

	while (i < identifier_vector.size()) {
		auto access_vector = evaluate_access_vector(identifier_vector[i].access_vector);

		if (access_vector.size() > 0) {
			auto current_val = value->get_raw_arr();
			size_t s = 0;
			size_t access_pos = 0;

			for (s = 0; s < access_vector.size() - 1; ++s) {
				access_pos = access_vector.at(s);

				// break if it is a string, and the string access will be handled in identifier node evaluation
				if (TypeUtils::is_string((*current_val)[access_pos]->type)) {
					(*current_val)[access_pos]->get_s()[access_vector.at(s + 1)] = new_value->get_c();
					return (*current_val)[access_pos];
				}
				if (access_pos >= current_val->size()) {
					throw std::runtime_error("invalid array position access");
				}
				current_val = (*current_val)[access_pos]->get_raw_arr();
			}
			if (TypeUtils::is_string(value->type)) {
				value->get_s()[access_vector.at(s)] = new_value->get_c();
				return value;
			}
			access_pos = access_vector.at(s);
			if (i == identifier_vector.size() - 1) {
				(*current_val)[access_pos] = new_value;
			}
			else {
				value = (*current_val)[access_pos];
			}
		}

		++i;

		if (i < identifier_vector.size()) {
			if (TypeUtils::is_void(value->type)) {
				std::stringstream ss;
				for (size_t si = 0; si <= i; ++si) {
					ss << identifier_vector[si].identifier;
					if (si < i) {
						ss << '.';
					}
				}
				throw std::runtime_error("cannot reach '" + ss.str() + "', previous '" + identifier_vector[i - 1].identifier + "' is null");
			}

			if (i == identifier_vector.size() - 1 && identifier_vector[i].access_vector.size() == 0) {
				value->set_sub(identifier_vector[i].identifier, new_value);
			}
			else {
				value = value->get_str()[identifier_vector[i].identifier];
			}
		}
	}

	return value;
}

RuntimeValue* Interpreter::access_value(RuntimeValue* value, const std::vector<Identifier>& identifier_vector, size_t i) {
	RuntimeValue* next_value = value;

	auto access_vector = evaluate_access_vector(identifier_vector[i].access_vector);

	if (access_vector.size() > 0) {
		size_t s = 0;
		size_t access_pos = 0;

		for (s = 0; s < access_vector.size() - 1; ++s) {
			access_pos = access_vector.at(s);
			next_value = next_value->get_sub(access_pos);
			// break if it is a string, and the string access will be handled in identifier node evaluation
			if (TypeUtils::is_string(next_value->type)) {
				has_string_access = true;
				return next_value;
			}
		}
		// break if it is a string, and the string access will be handled in identifier node evaluation
		if (TypeUtils::is_string(next_value->type)) {
			has_string_access = true;
			return next_value;
		}
		access_pos = access_vector.at(s);
		next_value = next_value->get_sub(access_pos);

		if (!next_value) {
			next_value = allocate_value(new RuntimeValue(Type::T_VOID));
		}
	}

	++i;

	if (i < identifier_vector.size()) {
		if (TypeUtils::is_void(next_value->type)) {
			std::stringstream ss;
			for (size_t si = 0; si <= i; ++si) {
				ss << identifier_vector[si].identifier;
				if (si < i) {
					ss << '.';
				}
			}
			throw std::runtime_error("cannot reach '" + ss.str() + "', previous '" + identifier_vector[i - 1].identifier + "' value is null");
		}

		if (!next_value->get_str().contains(identifier_vector[i].identifier)) {
			ExceptionHandler::throw_struct_member_err(next_value->type_name_space, next_value->type_name, identifier_vector[i].identifier);
		}

		next_value = next_value->get_sub(identifier_vector[i].identifier);

		if (identifier_vector[i].access_vector.size() > 0 || i < identifier_vector.size()) {
			return access_value(next_value, identifier_vector, i);
		}
	}

	return next_value;
}

std::shared_ptr<Scope> Interpreter::find_declared_function(const std::shared_ptr<ASTProgramNode>& current_program,
	const std::string& name_space, const std::string& identifier, const std::vector<TypeDefinition*>& signature,
	bool& strict, bool& pop_program) {

	std::shared_ptr<Scope> func_scope = get_inner_most_function_scope(current_program, name_space, identifier, &signature, strict);

	if (func_scope) {
		current_program_stack.push(func_scope->owner);
		pop_program = true;
	}

	return func_scope;

}

std::shared_ptr<Scope> Interpreter::find_declared_function_strict(const std::shared_ptr<ASTProgramNode>& current_program,
	const std::string& name_space, const std::string& identifier, const std::vector<TypeDefinition*>& signature,
	bool& strict, bool& pop_program) {

	std::shared_ptr<Scope> func_scope = find_declared_function(current_program, name_space, identifier, signature, strict, pop_program);

	if (!func_scope) {
		strict = false;

		func_scope = find_declared_function(current_program, name_space, identifier, signature, strict, pop_program);

	}

	return func_scope;

}

intmax_t Interpreter::hash(RuntimeValue* value) {
	switch (value->type) {
	case Type::T_BOOL:
		return static_cast<intmax_t>(value->get_b());
	case Type::T_INT:
		return static_cast<intmax_t>(value->get_i());
	case Type::T_FLOAT:
		return static_cast<intmax_t>(value->get_f());
	case Type::T_CHAR:
		return static_cast<intmax_t>(value->get_c());
	case Type::T_STRING:
		return utils::StringUtils::hashcode(value->get_s());
	default:
		throw std::runtime_error("cannot determine type");
	}
}

intmax_t Interpreter::hash(std::shared_ptr<ASTExprNode> astnode) {
	astnode->accept(this);
	return hash(current_expression_value);
}

intmax_t Interpreter::hash(std::shared_ptr<ASTValueNode> astnode) {
	return hash(dynamic_cast<RuntimeValue*>(astnode->value));
}

intmax_t Interpreter::hash(std::shared_ptr<ASTLiteralNode<flx_bool>> astnode) {
	return static_cast<intmax_t>(astnode->value);
}

intmax_t Interpreter::hash(std::shared_ptr<ASTLiteralNode<flx_int>> astnode) {
	return static_cast<intmax_t>(astnode->value);
}

intmax_t Interpreter::hash(std::shared_ptr<ASTLiteralNode<flx_float>> astnode) {
	return static_cast<intmax_t>(astnode->value);
}

intmax_t Interpreter::hash(std::shared_ptr<ASTLiteralNode<flx_char>> astnode) {
	return static_cast<intmax_t>(astnode->value);
}

intmax_t Interpreter::hash(std::shared_ptr<ASTLiteralNode<flx_string>> astnode) {
	return utils::StringUtils::hashcode(astnode->value);
}

intmax_t Interpreter::hash(std::shared_ptr<ASTIdentifierNode> astnode) {
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;
	auto variable = std::dynamic_pointer_cast<RuntimeVariable>(find_inner_most_variable(current_program, name_space, astnode->identifier_vector[0].identifier));
	auto value = access_value(variable->get_value(), astnode->identifier_vector);

	return hash(value);
}

void Interpreter::check_build_array(RuntimeValue* new_value, std::vector<size_t> dim) {
	auto arr = new_value->get_arr();
	auto arrsize = arr.size();

	if (TypeUtils::is_array(new_value->type) && dim.size() > 1 && dim[0] > 0) {
		auto val = arrsize == 1 ? arr[0] : allocate_value(new RuntimeValue(Type::T_VOID));

		flx_array rarr = build_array(dim, val, dim.size() - 1);

		new_value->set(rarr,
			TypeUtils::is_void(current_expression_array_type.type) ?
			Type::T_ANY : current_expression_array_type.type,
			dim, current_expression_array_type.type_name,
			current_expression_array_type.type_name_space);

	}
	else if (TypeUtils::is_array(new_value->type) && dim.size() == 1 && dim[0] > 0) {

		switch (arrsize)
		{
		case 0: {
			flx_array rarr = flx_array(dim[0]);

			new_value->set(rarr, TypeUtils::is_void(new_value->array_type) ? Type::T_ANY : new_value->array_type, dim);

			break;
		}
		case 1: {
			auto val = arr[0];

			flx_array rarr = build_array(dim, val, 0);

			new_value->set(rarr,
				TypeUtils::is_void(current_expression_array_type.type) ?
				Type::T_ANY : current_expression_array_type.type,
				dim, current_expression_array_type.type_name,
				current_expression_array_type.type_name_space);

			break;
		}
		}

	}

}

flx_array Interpreter::build_array(const std::vector<size_t>& dim, RuntimeValue* init_value, intmax_t i) {
	flx_array raw_arr;

	if (dim.size() - 1 == i) {
		current_expression_array_type = TypeDefinition();
	}

	size_t size = 0;
	if (dim.size() == 0) {
		size = 1;
	}
	else {
		size = dim[i];
	}

	raw_arr = flx_array(size);

	for (size_t j = 0; j < size; ++j) {
		RuntimeValue* val;

		if (i > 0) {
			// for superior dimentions, its creates a new array for each element
			std::vector<size_t> sub_dims(dim.begin(), dim.begin() + i);
			val = allocate_value(new RuntimeValue(init_value)); // copy the initial value
			raw_arr[j] = allocate_value(new RuntimeValue(build_array(sub_dims, val, i - 1), 
				val->type, val->dim, val->type_name, val->type_name_space));
		}
		else {
			// for the last dimension, we create independent values
			val = allocate_value(new RuntimeValue(init_value)); // create a independent copy
			raw_arr[j] = val;
		}

		if (TypeUtils::is_undefined(current_expression_array_type.type) || TypeUtils::is_array(current_expression_array_type.type)) {
			current_expression_array_type = *val;
		}
	}

	return raw_arr;
}

std::vector<size_t> Interpreter::calculate_array_dim_size(const flx_array& arr) {
	auto dim = std::vector<size_t>();

	dim.push_back(arr.size());

	if (TypeUtils::is_array(arr[0]->type)) {
		auto dim2 = calculate_array_dim_size(arr[0]->get_arr());
		dim.insert(dim.end(), dim2.begin(), dim2.end());
	}

	return dim;
}

std::vector<size_t> Interpreter::evaluate_access_vector(const std::vector<std::shared_ptr<ASTExprNode>>& expr_access_vector) {
	auto access_vector = std::vector<size_t>();
	for (const auto& expr : expr_access_vector) {
		size_t val = 0;
		if (expr) {
			std::dynamic_pointer_cast<ASTExprNode>(expr)->accept(this);
			auto size_expr = current_expression_value;
			clear_current_expression();
			if (!TypeUtils::is_int(size_expr->type)) {
				throw std::runtime_error("array index access must be a integer value");
			}
			val = size_expr->get_i();
		}
		access_vector.push_back(val);
	}
	return access_vector;
}

RuntimeValue* Interpreter::allocate_value(RuntimeValue* value) {
	return dynamic_cast<RuntimeValue*>(gc.allocate(value));
}

void Interpreter::clear_current_expression() {
	if (current_expression_value->has_value()) {
		current_expression_value = allocate_value(new RuntimeValue(Type::T_UNDEFINED));
	}
}

void Interpreter::declare_function_parameter(std::shared_ptr<Scope> scope, const std::string& identifier, TypeDefinition variable, RuntimeValue* value) {
	if (TypeUtils::is_function(value->type)) {
		const auto& prg = current_program_stack.top();
		const auto& name_space = value->get_fun().first;
		auto funcs = get_inner_most_functions_scope(prg, name_space, value->get_fun().second)->find_declared_functions(value->get_fun().second);
		for (auto& it = funcs.first; it != funcs.second; ++it) {
			scope->declare_function(identifier, it->second);
		}
	}
	else {
		if (value->use_ref && value->ref.lock()) {
			scope->declare_variable(identifier, value->ref.lock());
		}
		else {
			auto var = std::make_shared<RuntimeVariable>(identifier, variable);
			gc.add_var_root(var);
			var->set_value(value);
			scope->declare_variable(identifier, var);
		}
	}
}

bool Interpreter::is_builtin_execution_block(std::vector<std::shared_ptr<ASTNode>> statements) {
	return !function_call_name.empty() && statements.size() == 1 && std::dynamic_pointer_cast<ASTBuiltinCallNode>(statements[0]);
}

void Interpreter::declare_function_block_parameters(const std::string& name_space) {
	auto& curr_scope = scopes[name_space].back();
	auto rest_name = std::string();
	auto vec = std::vector<RuntimeValue*>();
	size_t i = 0;

	if (current_function_calling_arguments.size() == 0 || current_function_defined_parameters.size() == 0) {
		return;
	}

	// adds function arguments
	for (i = 0; i < current_function_calling_arguments.top().size(); ++i) {
		auto current_function_calling_argument = current_function_calling_arguments.top()[i];

		if (current_function_defined_parameters.top().size() > i) {
			validates_reference_type_assignment(*current_function_defined_parameters.top()[i], current_function_calling_argument);
			RuntimeOperations::normalize_type(current_function_defined_parameters.top()[i], current_function_calling_argument);
		}

		// is reference : not reference
		RuntimeValue* current_value = current_function_calling_argument;
		if (!current_function_calling_argument->use_ref) {
			current_value = allocate_value(new RuntimeValue(current_function_calling_argument));
			current_value->ref.reset();
		}

		if (i >= current_function_defined_parameters.top().size()) {
			vec.push_back(current_value);
		}
		else {
			if (const auto decl = dynamic_cast<VariableDefinition*>(current_function_defined_parameters.top()[i])) {
				declare_function_parameter(curr_scope, decl->identifier, *decl, current_value);

				// is rest
				if (decl->is_rest) {
					rest_name = decl->identifier;
					// if is last parameter and is array and is not a single parameter
					if (current_function_defined_parameters.top().size() - 1 == i
						&& TypeUtils::is_array(current_value->type)) {
						for (size_t i = 0; i < vec.size(); ++i) {
							vec.push_back(current_value->get_arr()[i]);
						}
					}
					else {
						vec.push_back(current_value);
					}
				}
			}
			else if (const auto decls = dynamic_cast<UnpackedVariableDefinition*>(current_function_defined_parameters.top()[i])) {
				for (auto& decl : decls->variables) {
					auto sub_value = allocate_value(new RuntimeValue(current_value->get_str()[decl.identifier]));
					declare_function_parameter(curr_scope, decl.identifier, decl, sub_value);
				}
			}
		}
	}

	// adds default values
	for (; i < current_function_defined_parameters.top().size(); ++i) {
		if (const auto decl = dynamic_cast<VariableDefinition*>(current_function_defined_parameters.top()[i])) {
			if (decl->is_rest) {
				break;
			}

			std::dynamic_pointer_cast<ASTExprNode>(decl->default_value)->accept(this);
			auto current_value = allocate_value(new RuntimeValue(current_expression_value));
			clear_current_expression();

			declare_function_parameter(curr_scope, decl->identifier, *decl, current_value);
		}
	}

	if (vec.size() > 0) {
		auto arr = flx_array(vec.size());
		for (size_t i = 0; i < vec.size(); ++i) {
			arr[i] = vec[i];
		}
		auto rest = allocate_value(new RuntimeValue(arr, Type::T_ANY, std::vector<size_t>{(size_t)arr.size()}));
		auto var = std::make_shared<RuntimeVariable>(rest_name, *current_function_defined_parameters.top().back());
		gc.add_var_root(var);
		var->set_value(rest);
		curr_scope->declare_variable(rest_name, var);
	}

	current_function_defined_parameters.pop();
	current_function_calling_arguments.pop();
}

void Interpreter::build_args(const std::vector<std::string>& args) {
	auto flx = std::make_shared<RuntimeVariable>("flx", Type::T_STRUCT, Type::T_UNDEFINED, std::vector<size_t>(), "Flexa", Constants::DEFAULT_NAMESPACE);
	auto str_value = flx_struct();

	// program args
	auto dim = std::vector<size_t>{ (size_t)args.size() };
	auto arr = flx_array(args.size());
	for (size_t i = 0; i < args.size(); ++i) {
		arr[i] = allocate_value(new RuntimeValue(args[i]));
	}
	str_value["args"] = allocate_value(new RuntimeValue(arr, Type::T_STRING, dim));

	// cwd
	str_value["cwd"] = allocate_value(new RuntimeValue(std::filesystem::current_path().string()));

	// so
#ifdef linux

	str_value["so"] = allocate_value(new RuntimeValue(flx_string("linux")));

#elif defined(_WIN32) || defined(WIN32)

	str_value["so"] = allocate_value(new RuntimeValue(flx_string("windows")));

#endif // linux

	flx->set_value(allocate_value(new RuntimeValue(str_value, "Flexa", Constants::DEFAULT_NAMESPACE)));
	gc.add_var_root(flx);

	scopes[Constants::DEFAULT_NAMESPACE].back()->declare_variable("flx", flx);

}

void Interpreter::set_curr_pos(size_t row, size_t col) {
	curr_row = row;
	curr_col = col;
}

std::string Interpreter::msg_header() {
	return "(IERR) " + current_program_stack.top()->name + '[' + std::to_string(curr_row) + ':' + std::to_string(curr_col) + "]: ";
}
