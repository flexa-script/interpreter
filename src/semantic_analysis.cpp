#include "semantic_analysis.hpp"

#include "exception_handler.hpp"
#include "token.hpp"
#include "md_builtin.hpp"
#include "constants.hpp"
#include "utils.hpp"

using namespace core;
using namespace core::analysis;

SemanticAnalyser::SemanticAnalyser(std::shared_ptr<Scope> global_scope, std::shared_ptr<ASTProgramNode> main_program,
	std::map<std::string, std::shared_ptr<ASTProgramNode>> programs, const std::vector<std::string>& args)
	: Visitor(programs, main_program, main_program ? main_program->name : Constants::DEFAULT_NAMESPACE), is_max(false) {
	scopes[Constants::DEFAULT_NAMESPACE].push_back(std::make_shared<Scope>(std::make_shared<ASTProgramNode>("__builtin__", Constants::DEFAULT_NAMESPACE, std::vector<std::shared_ptr<ASTNode>>())));
	Constants::BUILT_IN_LIBS.at("builtin")->register_functions(this);

	scopes[main_program->name_space].push_back(global_scope);

	if (main_program->name_space != Constants::DEFAULT_NAMESPACE) {
		program_nmspaces[main_program->name].push_back(Constants::DEFAULT_NAMESPACE);
	}

	build_args(args);
};

void SemanticAnalyser::start() {
	visit(current_program_stack.top());
}

void SemanticAnalyser::visit(std::shared_ptr<ASTProgramNode> astnode) {
	for (const auto& statement : astnode->statements) {
		try {
			statement->accept(this);
		}
		catch (std::exception ex) {
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
}

void SemanticAnalyser::visit(std::shared_ptr<ASTUsingNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	std::string libname = utils::StringUtils::join(astnode->library, ".");

	if (Constants::BUILT_IN_LIBS.find(libname) != Constants::BUILT_IN_LIBS.end()) {
		Constants::BUILT_IN_LIBS.find(libname)->second->register_functions(this);
	}

	if (programs.find(libname) == programs.end()) {
		throw std::runtime_error("lib '" + libname + "' not found");
	}

	auto& program = programs[libname];

	// add lib to current program
	if (utils::CollectionUtils::contains(current_program_stack.top()->libs, program)) {
		throw std::runtime_error("lib '" + libname + "' already declared in " + current_program_stack.top()->name);
	}
	current_program_stack.top()->libs.push_back(program);

	// if not parsed yet
	if (!utils::CollectionUtils::contains(parsed_libs, libname)) {
		current_program_stack.push(program);

		if (!program->name_space.empty() && !utils::CollectionUtils::contains(nmspaces, program->name_space)) {
			nmspaces.push_back(program->name_space);
		}

		parsed_libs.push_back(libname);

		scopes[program->name_space].push_back(std::make_shared<Scope>(program));

		if (std::find(program_nmspaces[program->name].begin(), program_nmspaces[program->name].end(), Constants::DEFAULT_NAMESPACE) == program_nmspaces[program->name].end()) {
			program_nmspaces[program->name].push_back(Constants::DEFAULT_NAMESPACE);
		}

		start();

		current_program_stack.pop();
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTIncludeNamespaceNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& prg_name = current_program_stack.top()->name;

	validate_namespace(astnode->name_space);

	if (std::find(program_nmspaces[prg_name].begin(), program_nmspaces[prg_name].end(), astnode->name_space) == program_nmspaces[prg_name].end()) {
		program_nmspaces[prg_name].push_back(astnode->name_space);
	}
	else {
		throw std::runtime_error("namespace '" + astnode->name_space + "' is not included in '" + prg_name + "'");
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTExcludeNamespaceNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& prg_name = current_program_stack.top()->name;

	validate_namespace(astnode->name_space);

	size_t pos = std::distance(program_nmspaces[prg_name].begin(),
		std::find(program_nmspaces[prg_name].begin(),
			program_nmspaces[prg_name].end(), astnode->name_space));
	program_nmspaces[prg_name].erase(program_nmspaces[prg_name].begin() + pos);
}

void SemanticAnalyser::visit(std::shared_ptr<ASTEnumNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& name_space = current_program_stack.top()->name_space;

	for (size_t i = 0; i < astnode->identifiers.size(); ++i) {
		auto value = std::make_shared<SemanticValue>(Type::T_INT, i, true, astnode->row, astnode->col);
		auto variable = std::make_shared<SemanticVariable>(astnode->identifiers[i], Type::T_INT, true, astnode->row, astnode->col);
		variable->set_value(value);
		scopes[name_space].back()->declare_variable(astnode->identifiers[i], variable);
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTDeclarationNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	std::shared_ptr<Scope> current_scope = scopes[name_space].back();

	if (current_scope->already_declared_variable(astnode->identifier)) {
		throw std::runtime_error("variable '" + astnode->identifier + "' already declared");
	}

	if (TypeUtils::is_void(astnode->type)) {
		throw std::runtime_error("variables cannot be declared as void type: '" + astnode->identifier + "'");
	}

	if (TypeUtils::is_struct(astnode->type)) {
		auto type_name_space = astnode->type_name_space.empty() ? name_space : astnode->type_name_space;
		if (!get_inner_most_struct_definition_scope(current_program, type_name_space, astnode->type_name)) {
			throw std::runtime_error("struct '" + astnode->type_name + "' not found");
		}
	}

	if (astnode->expr) {
		astnode->expr->accept(this);
		if (TypeUtils::is_undefined(current_expression.type)) {
			throw std::runtime_error("'" + astnode->identifier + "' decaration expression is undefined");
		}
	}
	else {
		current_expression = SemanticValue(Type::T_UNDEFINED, astnode->row, astnode->col);
	}

	if (TypeUtils::is_function(current_expression.type)) {
		auto f = FunctionDefinition(astnode->identifier, astnode->row, astnode->row);
		scopes[name_space].back()->declare_function(astnode->identifier, f);
	}

	auto new_value = std::make_shared<SemanticValue>();
	new_value->copy_from(current_expression);
	new_value->hash = astnode->expr ? astnode->expr->hash(this) : 0;

	if (astnode->is_const && !new_value->is_const) {
		throw std::runtime_error("initializer of '" + astnode->identifier + "' is not a constant");
	}

	auto astnode_type_name = astnode->type_name.empty() ? new_value->type_name : astnode->type_name;

	auto new_var = std::make_shared<SemanticVariable>(
		astnode->identifier, astnode->type,
		astnode->array_type, evaluate_access_vector(astnode->expr_dim),
		astnode_type_name, astnode->type_name_space,
		astnode->is_const, astnode->row, astnode->col);
	new_var->set_value(new_value);

	if (!TypeDefinition::is_any_or_match_type(*new_var, *new_value)
		&& astnode->expr && !TypeUtils::is_undefined(new_value->type)) {
		ExceptionHandler::throw_declaration_type_err(astnode->identifier, *new_var, *new_value);
	}

	if (new_value->dim.size() < new_var->dim.size() && new_value->dim.size() == 1) {
		new_value->dim = new_var->dim;
	}

	if (TypeUtils::is_string(new_var->type) || TypeUtils::is_float(new_var->type)) {
		new_value->type = new_var->type;
	}

	current_scope->declare_variable(astnode->identifier, new_var);
}

void SemanticAnalyser::visit(std::shared_ptr<ASTUnpackedDeclarationNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	std::shared_ptr<ASTIdentifierNode> var = nullptr;
	if (astnode->expr) {
		var = std::dynamic_pointer_cast<ASTIdentifierNode>(astnode->expr);
		if (!var) {
			throw std::runtime_error("expected variable as value of unpacked declaration, but found value");
		}
	}

	if (var) {
		var->accept(this);
		if (!TypeDefinition::is_any_or_match_type(*astnode, current_expression)) {
			ExceptionHandler::throw_mismatched_type_err(*astnode, current_expression);
		}
	}

	for (const auto& declaration : astnode->declarations) {
		if (var) {
			auto ids = var->identifier_vector;
			ids.push_back(Identifier(declaration->identifier));
			auto access_expr = std::make_shared<ASTIdentifierNode>(ids, var->name_space, declaration->row, declaration->col);
			declaration->expr = access_expr;
		}

		declaration->accept(this);
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTAssignmentNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = normalize_name_space(astnode->name_space, current_program->name_space);
	const auto& identifier = astnode->identifier;

	if (!Token::is_assignment_op(astnode->op)) {
		throw std::runtime_error("expected assignment operator, but found '" + astnode->op + "'");
	}

	std::shared_ptr<Scope> curr_scope = get_inner_most_variable_scope(current_program, name_space, identifier);
	if (!curr_scope) {
		bool isfunc = false;
		curr_scope = get_inner_most_function_scope(current_program, name_space, identifier, nullptr);
		if (curr_scope) {
			isfunc = true;
			throw std::runtime_error("function '" + identifier + "' can't be assigned");
		}
		else {
			throw std::runtime_error("identifier '" + identifier + "' being reassigned was never declared");
		}
	}

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("'" + astnode->identifier + "' assign expression is undefined");
	}

	if (TypeUtils::is_function(current_expression.type)) {
		auto f = FunctionDefinition(astnode->identifier, astnode->row, astnode->row);
		scopes[name_space].back()->declare_function(astnode->identifier, f);
	}

	auto assignment_expr = current_expression;

	auto declared_variable = std::dynamic_pointer_cast<SemanticVariable>(curr_scope->find_declared_variable(identifier));

	if (TypeUtils::is_undefined(declared_variable->get_value()->type) && (astnode->identifier_vector.size() > 1 || astnode->identifier_vector[0].access_vector.size() > 0)) {
		throw std::runtime_error("base assigning variable '" + astnode->identifier + "' is undefined");
	}

	auto decl_var_expression = access_value(declared_variable->get_value(), astnode->identifier_vector);

	if (declared_variable->is_const) {
		throw std::runtime_error("'" + identifier + "' constant being reassigned");
	}

	if (decl_var_expression->dim.size() < declared_variable->dim.size() && decl_var_expression->dim.size() == 1) {
		decl_var_expression->dim = declared_variable->dim;
	}

	if (TypeUtils::is_string(declared_variable->type) || TypeUtils::is_float(declared_variable->type)) {
		decl_var_expression->type = declared_variable->type;
	}

	// gets compare expression
	TypeDefinition curr_expr = *decl_var_expression;
	if (Token::is_assignment_collection_op(astnode->op) && !TypeUtils::is_any(declared_variable->type) && declared_variable->get_value() == decl_var_expression) {
		curr_expr = *declared_variable;
	}

	assignment_expr = SemanticValue(do_operation(astnode->op, *declared_variable, curr_expr,
		assignment_expr, false), 0, false, astnode->row, astnode->col);

	if (declared_variable->get_value() == decl_var_expression) {
		declared_variable->get_value()->copy_from(assignment_expr);
	}

}

void SemanticAnalyser::visit(std::shared_ptr<ASTFunctionExpressionAssignmentNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	astnode->function->accept(this);

	std::shared_ptr<SemanticVariable> declared_variable;

	if (auto var = current_expression.ref.lock()) {
		declared_variable = std::dynamic_pointer_cast<SemanticVariable>(var);
	}
	else {
		auto decl_expr = current_expression;
		astnode->expr->accept(this);
		do_operation(astnode->op, decl_expr, decl_expr, current_expression, false);
		return;
	}

	auto decl_var_expression = declared_variable->get_value();

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("'" + declared_variable->identifier + "' assign expression is undefined");
	}

	if (TypeUtils::is_function(current_expression.type)) {
		auto f = FunctionDefinition(declared_variable->identifier, astnode->row, astnode->row);
		scopes[name_space].back()->declare_function(declared_variable->identifier, f);
	}

	auto assignment_expr = current_expression;

	if (declared_variable->is_const) {
		throw std::runtime_error("'" + declared_variable->identifier + "' constant being reassigned");
	}

	if (decl_var_expression->dim.size() < declared_variable->dim.size() && decl_var_expression->dim.size() == 1) {
		decl_var_expression->dim = declared_variable->dim;
	}

	if (TypeUtils::is_string(declared_variable->type) || TypeUtils::is_float(declared_variable->type)) {
		decl_var_expression->type = declared_variable->type;
	}

	// gets compare expression
	TypeDefinition curr_expr = *decl_var_expression;
	if (Token::is_assignment_collection_op(astnode->op) && !TypeUtils::is_any(declared_variable->type) && declared_variable->get_value() == decl_var_expression) {
		curr_expr = *declared_variable;
	}

	assignment_expr = SemanticValue(do_operation(astnode->op, *declared_variable, curr_expr,
		assignment_expr, false), 0, false, astnode->row, astnode->col);

	if (declared_variable->get_value() == decl_var_expression) {
		declared_variable->get_value()->copy_from(assignment_expr);
	}

}

void SemanticAnalyser::visit(std::shared_ptr<ASTReturnNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	auto return_expr = SemanticValue();

	if (astnode->expr) {
		astnode->expr->accept(this);
		return_expr = current_expression;
		if (TypeUtils::is_undefined(return_expr.type)) {
			if (!current_function.empty()) {
				throw std::runtime_error("'" + current_function.top().identifier + "' return expression is undefined");
			}
			throw std::runtime_error("return expression is undefined");
		}
	}

	if (!current_function.empty()) {
		auto& currfun = current_function.top();
		if (!TypeDefinition::is_any_or_match_type(currfun, return_expr)) {
			ExceptionHandler::throw_return_type_err(currfun.identifier, currfun, return_expr);
		}
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTFunctionCallNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = normalize_name_space(astnode->name_space, current_program->name_space);
	bool strict = true;
	auto returned_expression = current_expression;

	std::vector<TypeDefinition*> signature = std::vector<TypeDefinition*>();

	for (const auto& param : astnode->parameters) {
		param->accept(this);
		if (TypeUtils::is_undefined(current_expression.type)) {
			throw std::runtime_error("'" + astnode->identifier + "' parameter in position " + std::to_string(signature.size()) + " is undefined");
		}

		signature.push_back(new TypeDefinition(current_expression));
	}

	// handle function return
	if (astnode->identifier.empty()) {
		if (!TypeUtils::is_function(returned_expression.type) && !TypeUtils::is_any(returned_expression.type)) {
			throw std::runtime_error(ExceptionHandler::buid_signature(astnode->identifier_vector, signature));
		}

		current_expression = SemanticValue(Type::T_ANY, 0, 0);

	}
	// handle subvalue call
	else if (astnode->identifier_vector.size() > 1) {
		auto idnode = std::make_shared<ASTIdentifierNode>(astnode->identifier_vector, name_space, astnode->row, astnode->col);
		idnode->accept(this);

		if (!TypeUtils::is_function(current_expression.type)) {
			throw std::runtime_error(ExceptionHandler::buid_signature(astnode->identifier_vector, signature));
		}

	}
	// handle regular call
	else {
		std::shared_ptr<Scope> curr_scope = get_inner_most_function_scope(current_program, name_space, astnode->identifier, &signature, strict);
		if (!curr_scope) {
			curr_scope = get_inner_most_function_scope(current_program, name_space, astnode->identifier, &signature, strict);
			if (!curr_scope) {
				std::string func_name = ExceptionHandler::buid_signature(astnode->identifier, signature);
				throw std::runtime_error("function '" + func_name + "' was never declared");
			}
		}

		auto& curr_function = curr_scope->find_declared_function(astnode->identifier, &signature, strict);

		if (TypeUtils::is_void(curr_function.type)) {
			current_expression = SemanticValue(Type::T_UNDEFINED, 0, 0);
		}
		else {
			auto typedeg = std::make_shared<SemanticValue>(static_cast<TypeDefinition>(curr_function), 0, false, 0, 0);
			typedeg->type_name_space = astnode->name_space;
			if (astnode->expression_identifier_vector.size() > 0) {
				current_expression = *access_value(typedeg, astnode->expression_identifier_vector);
			}
			else if (astnode->expression_call) {
				current_expression = SemanticValue(Type::T_FUNCTION, 0, 0);
			}
			else {
				current_expression = *typedeg;
			}
		}

		if (astnode->expression_call) {
			astnode->expression_call->accept(this);
		}

	}

}

void SemanticAnalyser::visit(std::shared_ptr<ASTBuiltinCallNode> astnode) {}

void SemanticAnalyser::visit(std::shared_ptr<ASTFunctionDefinitionNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = normalize_name_space(astnode->type_name_space, current_program->name_space);

	for (const auto& scope : scopes[name_space]) {
		if (scope->already_declared_function(astnode->identifier, &astnode->parameters)) {
			const auto& decl_function = scope->find_declared_function(astnode->identifier, &astnode->parameters);

			if (!decl_function.block && astnode->block) {
				break;
			}

			std::string signature = ExceptionHandler::buid_signature(astnode->identifier, astnode->parameters);
			throw std::runtime_error("function " + signature + " already defined");
		}
	}

	if (astnode->block) {
		auto has_return = returns(astnode->block);
		auto type = TypeUtils::is_void(astnode->type) && has_return ? Type::T_ANY : astnode->type;

		if (astnode->identifier != "") {
			try {
				std::shared_ptr<Scope> func_scope = scopes[name_space].back();
				auto& declfun = func_scope->find_declared_function(astnode->identifier, &astnode->parameters, true);
				declfun.type = type;
				declfun.block = astnode->block;
			}
			catch (...) {
				auto f = FunctionDefinition(astnode->identifier, type, astnode->type_name, astnode->type_name_space,
					astnode->array_type, astnode->dim, astnode->parameters, astnode->block, astnode->row, astnode->row);
				scopes[name_space].back()->declare_function(astnode->identifier, f);
			}

			auto& curr_function = scopes[name_space].back()->find_declared_function(astnode->identifier, &astnode->parameters);

			current_function.push(curr_function);
		}

		astnode->block->accept(this);

		if (!TypeUtils::is_void(type)) {
			if (!has_return) {
				throw std::runtime_error("defined function '" + astnode->identifier + "' is not guaranteed to return a value");
			}
		}

		current_function.pop();
	}
	else {
		if (astnode->identifier != "") {
			auto f = FunctionDefinition(astnode->identifier, astnode->type, astnode->type_name, astnode->type_name_space,
				astnode->array_type, astnode->dim, astnode->parameters, astnode->block, astnode->row, astnode->row);
			scopes[name_space].back()->declare_function(astnode->identifier, f);
		}
	}

}

void SemanticAnalyser::visit(std::shared_ptr<ASTLambdaFunction> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	auto fun = std::dynamic_pointer_cast<ASTFunctionDefinitionNode>(astnode->fun);

	FunctionDefinition tempfun = FunctionDefinition("", fun->type, fun->type_name, fun->type_name_space,
		fun->array_type, fun->dim, fun->parameters, fun->block, fun->row, fun->col);

	current_function.push(tempfun);

	fun->accept(this);

	current_expression = SemanticValue();
	current_expression.type = Type::T_FUNCTION;
	current_expression.array_type = fun->array_type;
	current_expression.dim = fun->dim;
	current_expression.type_name = fun->type_name;
	current_expression.type_name_space = fun->type_name_space;
	current_expression.row = fun->row;
	current_expression.col = fun->col;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTBlockNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	scopes[name_space].push_back(std::make_shared<Scope>(current_program));

	auto& curr_scope = scopes[name_space].back();

	if (!current_function.empty()) {
		for (auto param : current_function.top().parameters) {
			if (const auto decl = dynamic_cast<VariableDefinition*>(param)) {
				declare_function_parameter(curr_scope, *decl);
			}
			else if (const auto decls = dynamic_cast<UnpackedVariableDefinition*>(param)) {
				for (auto& decl : decls->variables) {
					declare_function_parameter(curr_scope, decl);
				}
			}
		}
	}

	for (const auto& stmt : astnode->statements) {
		stmt->accept(this);
	}

	scopes[name_space].pop_back();
}

void SemanticAnalyser::visit(std::shared_ptr<ASTExitNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->exit_code->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("exit expression is undefined");
	}

	if (!TypeUtils::is_int(current_expression.type)) {
		throw std::runtime_error("expected int value");
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTContinueNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	if (!is_loop) {
		throw std::runtime_error("continue must be inside a loop");
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTBreakNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	if (!is_loop && !is_switch) {
		throw std::runtime_error("break must be inside a loop or switch");
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTSwitchNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	is_switch = true;

	scopes[name_space].push_back(std::make_shared<Scope>(current_program));

	astnode->parsed_case_blocks = std::map<size_t, size_t>();

	astnode->condition->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("switch expression is undefined");
	}

	auto cond_type = static_cast<TypeDefinition>(current_expression);
	auto case_type = TypeDefinition::get_basic(Type::T_UNDEFINED);

	for (const auto& expr : astnode->case_blocks) {
		expr.first->accept(this);

		if (TypeUtils::is_undefined(current_expression.type)) {
			throw std::runtime_error("case expression is undefined");
		}

		if (!current_expression.is_const) {
			throw std::runtime_error("case expression is not an constant");
		}

		if (TypeUtils::is_undefined(case_type.type)) {
			if (TypeUtils::is_undefined(current_expression.type)
				|| TypeUtils::is_void(current_expression.type)
				|| TypeUtils::is_any(current_expression.type)) {
				throw std::runtime_error("case values cannot be undefined");
			}
			case_type = current_expression;
		}

		if (!TypeDefinition::match_type(case_type, current_expression)) {
			ExceptionHandler::throw_mismatched_type_err(case_type, current_expression);
		}

		auto hash = expr.first->hash(this);
		if (astnode->parsed_case_blocks.contains(hash)) {
			throw std::runtime_error("duplicated case value: '" + std::to_string(hash) + "'");
		}

		astnode->parsed_case_blocks.emplace(hash, expr.second);
	}

	if (!TypeDefinition::is_any_or_match_type(cond_type, case_type)) {
		ExceptionHandler::throw_mismatched_type_err(cond_type, case_type);
	}

	for (auto& stmt : astnode->statements) {
		stmt->accept(this);
	}

	scopes[name_space].pop_back();
	is_switch = false;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTElseIfNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->condition->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("else if expression is undefined");
	}

	if (!TypeUtils::is_bool(current_expression.type)
		&& !TypeUtils::is_any(current_expression.type)) {
		ExceptionHandler::throw_condition_type_err();
	}

	astnode->block->accept(this);
}

void SemanticAnalyser::visit(std::shared_ptr<ASTIfNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->condition->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("if expression is undefined");
	}

	if (!TypeUtils::is_bool(current_expression.type)
		&& !TypeUtils::is_any(current_expression.type)) {
		ExceptionHandler::throw_condition_type_err();
	}

	astnode->if_block->accept(this);

	for (const auto& elif : astnode->else_ifs) {
		elif->accept(this);
	}

	if (astnode->else_block) {
		astnode->else_block->accept(this);
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTForNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	is_loop = true;

	scopes[name_space].push_back(std::make_shared<Scope>(current_program));

	if (astnode->expressions[0]) {
		astnode->expressions[0]->accept(this);
	}
	if (astnode->expressions[1]) {
		astnode->expressions[1]->accept(this);

		if (TypeUtils::is_undefined(current_expression.type)) {
			throw std::runtime_error("for expression is undefined");
		}

		if (!TypeUtils::is_bool(current_expression.type)
			&& !TypeUtils::is_any(current_expression.type)) {
			ExceptionHandler::throw_condition_type_err();
		}
	}
	if (astnode->expressions[2]) {
		astnode->expressions[2]->accept(this);
	}
	astnode->block->accept(this);

	scopes[name_space].pop_back();
	is_loop = false;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTForEachNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	is_loop = true;
	SemanticValue col_value;

	scopes[name_space].push_back(std::make_shared<Scope>(current_program));
	std::shared_ptr<Scope> back_scope = scopes[name_space].back();

	astnode->collection->accept(this);
	col_value = current_expression;

	if (const auto idnode = std::dynamic_pointer_cast<ASTUnpackedDeclarationNode>(astnode->itdecl)) {
		if (!TypeUtils::is_struct(col_value.type) && !TypeUtils::is_any(col_value.type)) {
			throw std::runtime_error("[key, value] can only be used with struct");
		}

		if (idnode->declarations.size() != 2) {
			throw std::runtime_error("invalid number of values");
		}

		auto key_node = std::make_shared<ASTLiteralNode<flx_string>>("", astnode->row, astnode->col);
		idnode->declarations[0]->expr = key_node;
		auto val_node = std::make_shared<ASTValueNode>(new SemanticValue(Type::T_ANY, astnode->row, astnode->col), astnode->row, astnode->col);
		idnode->declarations[1]->expr = val_node;
		idnode->accept(this);
		idnode->declarations[0]->expr = nullptr;
		idnode->declarations[1]->expr = nullptr;
	}
	else if (const auto idnode = std::dynamic_pointer_cast<ASTDeclarationNode>(astnode->itdecl)) {
		if (!TypeUtils::is_array(col_value.type)
			&& !TypeUtils::is_string(col_value.type)
			&& !TypeUtils::is_struct(col_value.type)
			&& !TypeUtils::is_any(col_value.type)) {
			throw std::runtime_error("expected iterable in foreach");
		}

		SemanticValue* value;

		if (TypeUtils::is_struct(col_value.type)) {
			value = new SemanticValue(Type::T_STRUCT, Type::T_UNDEFINED, std::vector<size_t>(), "Pair", Constants::STD_NAMESPACE, 0, false, astnode->row, astnode->col);
		}
		else if (TypeUtils::is_string(col_value.type)) {
			value = new SemanticValue(Type::T_CHAR, Type::T_UNDEFINED, std::vector<size_t>(), "", "", 0, false, astnode->row, astnode->col);
		}
		else if (TypeUtils::is_any(col_value.type)) {
			value = new SemanticValue(Type::T_ANY, Type::T_UNDEFINED, std::vector<size_t>(), "", "", 0, false, astnode->row, astnode->col);
		}
		else if (col_value.dim.size() > 1) {
			auto dim = col_value.dim;
			if (dim.size() > 0) {
				dim.erase(dim.begin());
			}
			if (!TypeUtils::is_any(idnode->type)) {
				value = new SemanticValue(idnode->type, idnode->array_type, dim,
					idnode->type_name, idnode->type_name_space, 0, false, astnode->row, astnode->col);
			}
			else {
				value = new SemanticValue(idnode->type, col_value.array_type, dim,
					"", "", 0, false, astnode->row, astnode->col);
				if (!col_value.type_name.empty()) {
					value->type_name = col_value.type_name;
					value->type_name_space = col_value.type_name_space;
				}
			}
		}
		else {
			value = new SemanticValue(col_value.array_type, Type::T_UNDEFINED, std::vector<size_t>(), "", "", 0, false, astnode->row, astnode->col);
			if (!current_expression.type_name.empty()) {
				value->type_name = current_expression.type_name;
				value->type_name_space = current_expression.type_name_space;
			}
		}

		std::shared_ptr<ASTValueNode> exnode = std::make_shared<ASTValueNode>(value, astnode->row, astnode->col);
		idnode->expr = exnode;
		idnode->accept(this);
		idnode->expr = nullptr;

	}
	else if (const auto idnode = std::dynamic_pointer_cast<ASTIdentifierNode>(astnode->itdecl)) {
		if (!TypeUtils::is_array(col_value.type)
			&& !TypeUtils::is_string(col_value.type)
			&& !TypeUtils::is_struct(col_value.type)
			&& !TypeUtils::is_any(col_value.type)) {
			throw std::runtime_error("expected iterable in foreach");
		}

		SemanticValue* value;

		if (TypeUtils::is_struct(col_value.type)) {
			value = new SemanticValue(Type::T_STRUCT, Type::T_UNDEFINED, std::vector<size_t>(), "Pair", Constants::STD_NAMESPACE, 0, false, astnode->row, astnode->col);
		}
		else if (TypeUtils::is_string(col_value.type)) {
			value = new SemanticValue(Type::T_CHAR, Type::T_UNDEFINED, std::vector<size_t>(), "", "", 0, false, astnode->row, astnode->col);
		}
		else if (TypeUtils::is_any(col_value.type)) {
			value = new SemanticValue(Type::T_ANY, Type::T_UNDEFINED, std::vector<size_t>(), "", "", 0, false, astnode->row, astnode->col);
		}
		else {
			value = new SemanticValue(col_value.array_type, Type::T_UNDEFINED, std::vector<size_t>(), "", "", 0, false, astnode->row, astnode->col);
			if (!current_expression.type_name.empty()) {
				value->type_name = current_expression.type_name;
				value->type_name_space = current_expression.type_name_space;
			}
		}

		auto exnode = std::make_shared<ASTValueNode>(value, astnode->row, astnode->col);

		// creates an assignment node
		auto assign_node = std::make_shared<ASTAssignmentNode>(idnode->identifier_vector, idnode->name_space, "=", exnode, idnode->row, idnode->col);
		assign_node->accept(this);

	}
	else {
		throw std::runtime_error("expected declaration or identifier");
	}

	astnode->block->accept(this);

	scopes[name_space].pop_back();
	is_loop = false;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTTryCatchNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	astnode->try_block->accept(this);

	scopes[name_space].push_back(std::make_shared<Scope>(current_program));

	auto error_node = std::make_shared<ASTLiteralNode<flx_string>>("", astnode->row, astnode->col);
	auto code_node = std::make_shared<ASTLiteralNode<flx_int>>(0, astnode->row, astnode->col);

	if (const auto idnode = std::dynamic_pointer_cast<ASTUnpackedDeclarationNode>(astnode->decl)) {
		if (idnode->declarations.size() != 2) {
			throw std::runtime_error("invalid number of values");
		}
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
}

void SemanticAnalyser::visit(std::shared_ptr<ASTThrowNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	const auto& prg = current_program_stack.top();

	astnode->error->accept(this);

	if (TypeUtils::is_struct(current_expression.type) && current_expression.type_name == "Exception") {
		if (!get_inner_most_struct_definition_scope(prg, Constants::STD_NAMESPACE, "Exception")) {
			throw std::runtime_error("struct 'flx::Exception' not found");
		}
	}
	else if (!TypeUtils::is_string(current_expression.type)) {
		throw std::runtime_error("expected Exception or string in throw");
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTEllipsisNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
}

void SemanticAnalyser::visit(std::shared_ptr<ASTWhileNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	is_loop = true;
	astnode->condition->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("while expression is undefined");
	}

	if (!TypeUtils::is_bool(current_expression.type)
		&& !TypeUtils::is_any(current_expression.type)) {
		ExceptionHandler::throw_condition_type_err();
	}

	astnode->block->accept(this);
	is_loop = false;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTDoWhileNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	is_loop = true;
	astnode->condition->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("do-while expression is undefined");
	}

	if (!TypeUtils::is_bool(current_expression.type)
		&& !TypeUtils::is_any(current_expression.type)) {
		ExceptionHandler::throw_condition_type_err();
	}

	astnode->block->accept(this);
	is_loop = false;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTStructDefinitionNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = current_program->name_space;

	if (get_inner_most_struct_definition_scope(current_program, name_space, astnode->identifier)) {
		throw std::runtime_error("struct '" + astnode->identifier + "' already defined");
	}

	for (auto& var : astnode->variables) {
		var.second.dim = evaluate_access_vector(var.second.expr_dim);
	}

	auto str = StructureDefinition(astnode->identifier, astnode->variables, astnode->row, astnode->col);

	scopes[name_space].back()->declare_structure_definition(str);
}

void SemanticAnalyser::visit(std::shared_ptr<ASTValueNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	current_expression = *dynamic_cast<SemanticValue*>(astnode->value);
}

void SemanticAnalyser::visit(std::shared_ptr<ASTLiteralNode<flx_bool>> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression = SemanticValue();
	current_expression.type = Type::T_BOOL;
	current_expression.is_const = true;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTLiteralNode<flx_int>> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression = SemanticValue();
	current_expression.type = Type::T_INT;
	current_expression.is_const = true;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTLiteralNode<flx_float>> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression = SemanticValue();
	current_expression.type = Type::T_FLOAT;
	current_expression.is_const = true;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTLiteralNode<flx_char>> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression = SemanticValue();
	current_expression.type = Type::T_CHAR;
	current_expression.is_const = true;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTLiteralNode<flx_string>> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression = SemanticValue();
	current_expression.type = Type::T_STRING;
	current_expression.is_const = true;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTArrayConstructorNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	auto is_const = true;
	flx_int arr_size = 0;

	if (current_expression_array_dim.size() == 0) {
		current_expression_array_type = TypeDefinition();
		current_expression_array_dim_max = 0;
		is_max = false;
	}

	++current_expression_array_dim_max;
	if (!is_max) {
		current_expression_array_dim.push_back(-1);
	}

	for (size_t i = 0; i < astnode->values.size(); ++i) {
		astnode->values.at(i)->accept(this);
		if (!current_expression.is_const) {
			is_const = false;
		}

		if (TypeUtils::is_undefined(current_expression.type)) {
			throw std::runtime_error("array value is undefined");
		}

		if (TypeUtils::is_undefined(current_expression_array_type.type) || TypeUtils::is_array(current_expression_array_type.type)) {
			current_expression_array_type = current_expression;
		}
		else {
			if (!TypeUtils::match_type(current_expression_array_type.type, current_expression.type)
				&& !TypeUtils::is_any(current_expression.type) && !TypeUtils::is_void(current_expression.type)
				&& !TypeUtils::is_array(current_expression.type)) {
				current_expression_array_type = TypeDefinition::get_basic(Type::T_ANY);
			}
		}

		++arr_size;
	}

	auto current_sim_index = current_expression_array_dim_max - 1;
	if (current_expression_array_dim[current_sim_index] == -1) {
		current_expression_array_dim[current_sim_index] = arr_size;
	}

	is_max = true;

	current_expression = SemanticValue();
	current_expression.type = Type::T_ARRAY;
	current_expression.array_type = current_expression_array_type.type;
	current_expression.type_name = current_expression_array_type.type_name;
	current_expression.type_name_space = current_expression_array_type.type_name_space;
	current_expression.is_const = is_const;
	--current_expression_array_dim_max;
	size_t stay = current_expression_array_dim.size() - current_expression_array_dim_max;
	std::vector<size_t> current_expression_array_dim_aux;
	size_t curr_dim_i = current_expression_array_dim.size() - 1;
	for (size_t i = 0; i < stay; ++i) {
		current_expression_array_dim_aux.emplace(current_expression_array_dim_aux.begin(), current_expression_array_dim.at(curr_dim_i));
		--curr_dim_i;
	}
	current_expression.dim = current_expression_array_dim_aux;

	if (current_expression_array_dim_max == 0) {
		if (TypeUtils::is_undefined(current_expression.array_type)) {
			current_expression.array_type = Type::T_ANY;
		}
		current_expression_array_dim.clear();
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTStructConstructorNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = normalize_name_space(astnode->name_space, current_program->name_space);
	auto is_const = true;

	std::shared_ptr<Scope> curr_scope = get_inner_most_struct_definition_scope(current_program, name_space, astnode->type_name);
	if (!curr_scope) {
		throw std::runtime_error("struct '" + astnode->type_name + "' was not declared");
	}
	auto type_struct = curr_scope->find_declared_structure_definition(astnode->type_name);

	for (const auto& expr : astnode->values) {
		if (type_struct.variables.find(expr.first) == type_struct.variables.end()) {
			ExceptionHandler::throw_struct_member_err(name_space, astnode->type_name, expr.first);
		}
		const auto& var_type_struct = type_struct.variables[expr.first];
		expr.second->accept(this);
		if (!current_expression.is_const) {
			is_const = false;
		}

		if (!TypeDefinition::is_any_or_match_type(var_type_struct,
			current_expression)) {
			ExceptionHandler::throw_struct_type_err(name_space, astnode->type_name, var_type_struct);
		}
	}

	current_expression = SemanticValue();
	current_expression.type = Type::T_STRUCT;
	current_expression.type_name = astnode->type_name;
	current_expression.type_name_space = name_space;
	current_expression.is_const = is_const;

}

void SemanticAnalyser::visit(std::shared_ptr<ASTIdentifierNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);
	const auto& current_program = current_program_stack.top();
	const auto& name_space = normalize_name_space(astnode->name_space, current_program->name_space);

	std::shared_ptr<Scope> curr_scope = get_inner_most_variable_scope(current_program, name_space, astnode->identifier);

	if (!curr_scope) {
		current_expression = SemanticValue();

		curr_scope = get_inner_most_struct_definition_scope(current_program, name_space, astnode->identifier);

		if (curr_scope) {
			current_expression.type = Type::T_STRUCT;
			return;
		}
		else {
			curr_scope = get_inner_most_function_scope(current_program, name_space, astnode->identifier, nullptr);
			if (curr_scope) {
				current_expression.type = Type::T_FUNCTION;
				return;
			}
			else {
				throw std::runtime_error("identifier '" + astnode->identifier + "' was not declared");
			}
		}
	}

	const auto& declared_variable = std::dynamic_pointer_cast<SemanticVariable>(curr_scope->find_declared_variable(astnode->identifier));

	if (TypeUtils::is_undefined(declared_variable->get_value()->type)) {
		throw std::runtime_error("variable '" + astnode->identifier + "' is undefined");
	}

	auto variable_expr = access_value(declared_variable->get_value(), astnode->identifier_vector);
	variable_expr->reset_ref();

	if (TypeUtils::is_undefined(variable_expr->type)) {
		throw std::runtime_error("base variable '" + astnode->identifier + "' is undefined");
	}

	current_expression = *variable_expr;
	current_expression.reset_ref();
	current_expression.is_sub = declared_variable->get_value() != variable_expr;

}

void SemanticAnalyser::visit(std::shared_ptr<ASTBinaryExprNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->left->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("left expression is undefined");
	}

	auto lexpr = current_expression;

	astnode->right->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("right expression is undefined");
	}

	auto rexpr = current_expression;

	current_expression = SemanticValue(do_operation(astnode->op, lexpr, lexpr, rexpr, true), 0, false, 0, 0);
	current_expression.is_const = lexpr.is_const && rexpr.is_const;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTUnaryExprNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("unary expression is undefined");
	}

	if (astnode->unary_op == "ref" || astnode->unary_op == "unref") {
		if (astnode->unary_op == "ref") {
			current_expression.use_ref = true;
		}
		if (astnode->unary_op == "unref") {
			current_expression.use_ref = false;
		}
	}
	else {
		switch (current_expression.type) {
		case Type::T_INT:
			if (astnode->unary_op != "+" && astnode->unary_op != "-"
				&& astnode->unary_op != "--" && astnode->unary_op != "++"
				&& astnode->unary_op != "~") {
				throw std::runtime_error("operator '" + astnode->unary_op + "' in front of int expression");
			}
			break;
		case Type::T_FLOAT:
			if (astnode->unary_op != "+" && astnode->unary_op != "-"
				&& astnode->unary_op != "--" && astnode->unary_op != "++") {
				throw std::runtime_error("operator '" + astnode->unary_op + "' in front of float expression");
			}
			break;
		case Type::T_BOOL:
			if (astnode->unary_op != "not") {
				throw std::runtime_error("operator '" + astnode->unary_op + "' in front of boolean expression");
			}
			break;
		case Type::T_ANY:
			if (astnode->unary_op != "not" && astnode->unary_op != "~"
				&& astnode->unary_op != "+" && astnode->unary_op != "-"
				&& astnode->unary_op != "--" && astnode->unary_op != "++") {
				throw std::runtime_error("operator '" + astnode->unary_op + "' in front of boolean expression");
			}
			break;
		default:
			throw std::runtime_error("incompatible unary operator '" + astnode->unary_op +
				"' in front of " + TypeUtils::type_str(current_expression.type) + " expression");
		}
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTTernaryNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->condition->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("ternary condition is undefined");
	}

	astnode->value_if_true->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("left ternary expression is undefined");
	}

	astnode->value_if_false->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("right ternary expression is undefined");
	}
}

void SemanticAnalyser::visit(std::shared_ptr<ASTInNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->value->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("in value is undefined");
	}

	auto valtype = current_expression.type;

	astnode->collection->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("in collection is undefined");
	}

	auto coltype = current_expression.type;
	auto colarrtype = current_expression.type;

	current_expression = SemanticValue();
	current_expression.type = Type::T_BOOL;

	if (TypeUtils::is_any(valtype)
		&& (TypeUtils::is_any(coltype)
			|| TypeUtils::is_array(coltype)
			&& TypeUtils::is_any(colarrtype))) {
		return;
	}

	if (!TypeUtils::match_type(valtype, colarrtype)
		&& !TypeUtils::is_any(valtype) && !TypeUtils::is_any(coltype) && !TypeUtils::is_any(colarrtype)
		&& TypeUtils::is_string(valtype) && !TypeUtils::is_string(coltype)
		&& TypeUtils::is_char(valtype) && !TypeUtils::is_string(coltype)) {
		throw std::runtime_error("types don't match '" + TypeUtils::type_str(valtype) + "' and '" + TypeUtils::type_str(coltype) + "'");
	}

	if (!TypeUtils::is_collection(coltype) && !TypeUtils::is_any(coltype)) {
		throw std::runtime_error("invalid type '" + TypeUtils::type_str(coltype) + "', value must be a array or string");
	}

}

void SemanticAnalyser::visit(std::shared_ptr<ASTTypeCastNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("cast expression is undefined");
	}

	if ((TypeUtils::is_array(current_expression.type) || TypeUtils::is_struct(current_expression.type))
		&& !TypeUtils::is_string(astnode->type)) {
		throw std::runtime_error("invalid type conversion from "
			+ TypeUtils::type_str(current_expression.type) + " to " + TypeUtils::type_str(astnode->type));
	}

	current_expression = SemanticValue();
	current_expression.type = astnode->type;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTTypeNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression = SemanticValue(astnode->type, 0, true, astnode->row, astnode->col);

}

void SemanticAnalyser::visit(std::shared_ptr<ASTNullNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression = SemanticValue();
	current_expression.type = Type::T_VOID;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTThisNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	current_expression = SemanticValue();
	current_expression.type = Type::T_STRING;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTTypeOfNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("typeof expression is undefined");
	}

	current_expression = SemanticValue();
	current_expression.type = Type::T_STRING;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTTypeIdNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("typeid expression is undefined");
	}

	current_expression = SemanticValue();
	current_expression.type = Type::T_INT;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTRefIdNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("refid expression is undefined");
	}

	current_expression = SemanticValue();
	current_expression.type = Type::T_INT;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTIsStructNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("is_struct expression is undefined");
	}

	current_expression = SemanticValue();
	current_expression.type = Type::T_BOOL;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTIsArrayNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("is_array expression is undefined");
	}

	current_expression = SemanticValue();
	current_expression.type = Type::T_BOOL;
}

void SemanticAnalyser::visit(std::shared_ptr<ASTIsAnyNode> astnode) {
	set_curr_pos(astnode->row, astnode->col);

	astnode->expr->accept(this);

	if (TypeUtils::is_undefined(current_expression.type)) {
		throw std::runtime_error("is_any expression is undefined");
	}

	current_expression = SemanticValue();
	current_expression.type = Type::T_BOOL;
}

void SemanticAnalyser::declare_function_parameter(std::shared_ptr<Scope> scope, const VariableDefinition& param) {
	if (TypeUtils::is_function(param.type) || TypeUtils::is_any(param.type)) {
		auto f = FunctionDefinition(param.identifier, param.row, param.row);
		scope->declare_function(param.identifier, f);
	}

	if (!TypeUtils::is_function(param.type)) {
		auto var_expr = std::make_shared<SemanticValue>();
		var_expr->type = param.type;
		var_expr->array_type = param.array_type;
		var_expr->type_name = param.type_name;
		var_expr->dim = evaluate_access_vector(param.expr_dim);
		var_expr->row = param.row;
		var_expr->col = param.col;

		auto v = std::make_shared<SemanticVariable>(param.identifier, param.type, param.array_type,
			param.dim, param.type_name, param.type_name_space, false, param.row, param.col);

		v->set_value(var_expr);

		scope->declare_variable(param.identifier, v);
	}
}

bool SemanticAnalyser::namespace_exists(const std::string& name_space) {
	return scopes.find(name_space) != scopes.end();
}


void SemanticAnalyser::validate_namespace(const std::string& name_space) const {
	if (!utils::CollectionUtils::contains(nmspaces, name_space)) {
		throw std::runtime_error("namespace '" + name_space + "' not found");
	}
	if (name_space == Constants::DEFAULT_NAMESPACE) {
		throw std::runtime_error("namespace '" + name_space + "' is default included");
	}
}
TypeDefinition SemanticAnalyser::do_operation(const std::string& op, TypeDefinition lvar, TypeDefinition lvalue, TypeDefinition rvalue, bool is_expr) {
	Type l_var_type = lvar.type;
	lvalue = TypeUtils::is_undefined(lvalue.type) ? lvar : lvalue;
	Type l_type = lvalue.type;
	Type r_type = rvalue.type;

	if ((TypeUtils::is_any(l_var_type)
		|| TypeUtils::is_any(l_type) || TypeUtils::is_any(r_type)
		|| TypeUtils::is_void(l_type) || TypeUtils::is_void(r_type)) && op == "=") {
		return rvalue;
	}

	if ((TypeUtils::is_void(l_type) || TypeUtils::is_void(r_type))
		&& Token::is_equality_op(op)) {
		return TypeDefinition::get_basic(Type::T_BOOL);
	}

	if (TypeUtils::is_any(l_type) || TypeUtils::is_any(r_type)) {
		if (Token::is_relational_op(op) || Token::is_equality_op(op)) {
			return TypeDefinition::get_basic(Type::T_BOOL);
		}
		return TypeUtils::is_any(r_type) ? TypeUtils::is_any(l_type) ? lvar : lvalue : rvalue;
	}

	switch (r_type) {
	case Type::T_BOOL: {
		if (!TypeUtils::is_bool(l_type)
			|| op != "="
			&& op != "and"
			&& op != "or"
			&& !Token::is_equality_op(op)) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}

		break;
	}
	case Type::T_INT: {
		if (TypeUtils::is_float(l_type) && TypeUtils::is_any(l_var_type)
			|| TypeUtils::is_int(l_type) && TypeUtils::is_any(l_var_type)
			&& !Token::is_int_ex_op(op)) {
			if (!Token::is_float_op(op)
				&& !Token::is_relational_op(op)
				&& !Token::is_equality_op(op)) {
				ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
			}
		}
		else if (TypeUtils::is_int(l_type)
			&& !Token::is_int_op(op)
			&& !Token::is_relational_op(op)
			&& !Token::is_equality_op(op)) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}
		else if (!TypeUtils::is_numeric(l_type) && !TypeUtils::is_any(l_type) && !TypeUtils::is_any(l_var_type)) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}

		break;
	}
	case Type::T_FLOAT: {
		if ((TypeUtils::is_float(l_type) || TypeUtils::is_int(l_type))
			&& !Token::is_float_op(op)
			&& !Token::is_relational_op(op)
			&& !Token::is_equality_op(op)) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}
		else if (!TypeUtils::is_numeric(l_type) && !TypeUtils::is_any(l_type) && !TypeUtils::is_any(l_var_type)) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}

		break;
	}
	case Type::T_CHAR: {
		if (TypeUtils::is_string(l_type) && !Token::is_collection_op(op)) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}
		else if (TypeUtils::is_char(l_type)) {
			if (op != "=" && !Token::is_equality_op(op) && !is_expr) {
				ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
			}
		}
		else if (!TypeUtils::is_textual(l_type) && !TypeUtils::is_any(l_type) && !TypeUtils::is_any(l_var_type)) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}

		break;
	}
	case Type::T_STRING: {
		if ((!TypeUtils::is_string(l_type)
			|| (!Token::is_collection_op(op)
				&& !Token::is_equality_op(op)))
			&& (is_expr && (!TypeUtils::is_char(l_type)
				|| !Token::is_expression_collection_op(op)))) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}
		else if (!TypeUtils::is_textual(l_type) && !TypeUtils::is_any(l_type) && !TypeUtils::is_any(l_var_type)) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}

		break;
	}
	case Type::T_ARRAY: {
		if (!TypeDefinition::match_type_array(lvalue, rvalue)
			|| (!Token::is_collection_op(op) && !Token::is_equality_op(op))) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}

		break;
	}
	case Type::T_STRUCT: {
		if (!TypeUtils::is_struct(l_type) || (!Token::is_equality_op(op) && op != "=")) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}

		break;
	}
	case Type::T_FUNCTION: {
		if (!TypeUtils::is_function(l_type) || (!Token::is_equality_op(op) && op != "=")) {
			ExceptionHandler::throw_operation_err(op, lvalue, rvalue);
		}

		break;
	}
	default:
		throw std::runtime_error("cannot determine type of operation");

	}

	if (Token::is_equality_op(op) || Token::is_relational_op(op)) {
		return TypeDefinition::get_basic(Type::T_BOOL);
	}

	return TypeUtils::is_float(lvalue.type) || TypeUtils::is_string(lvalue.type) ? lvalue : rvalue;
}

void SemanticAnalyser::equals_value(const SemanticValue& lval, const SemanticValue& rval) {
	if (lval.use_ref && !rval.use_ref) {
		throw std::runtime_error("both values must be references");
	}
	if (!lval.use_ref && rval.use_ref) {
		throw std::runtime_error("both values must be unreferenced");
	}
}

const std::string& SemanticAnalyser::normalize_name_space(std::string& astnode_name_space, const std::string& name_space) const {
	if (astnode_name_space.empty()) {
		astnode_name_space = name_space;
	}
	return astnode_name_space;
}

std::shared_ptr<SemanticValue> SemanticAnalyser::access_value(std::shared_ptr<SemanticValue> value, const std::vector<Identifier>& identifier_vector, size_t i) {
	const auto& current_program = current_program_stack.top();
	const auto& name_space = value->type_name_space.empty() ? current_program->name_space : value->type_name_space;
	std::shared_ptr<SemanticValue> next_value = value;

	auto access_vector = evaluate_access_vector(identifier_vector[i].access_vector);

	if (access_vector.size() > 0) {
		if (access_vector.size() == value->dim.size()) {
			next_value = std::make_shared<SemanticValue>(next_value->array_type, Type::T_UNDEFINED,
				std::vector<size_t>(), next_value->type_name, next_value->type_name_space,
				0, false, next_value->row, next_value->col);
		}
		else if (access_vector.size() - 1 == value->dim.size()
			&& TypeUtils::is_string(next_value->type)) {
			next_value = std::make_shared<SemanticValue>(Type::T_CHAR, Type::T_UNDEFINED,
				std::vector<size_t>(), "", "",
				0, false, next_value->row, next_value->col);
		}
		else  if (access_vector.size() < value->dim.size()) {
			std::vector<size_t> calc_dim;
			// remove the first dimensions from the array
			for (size_t i = access_vector.size(); i < value->dim.size(); ++i) {
				calc_dim.push_back(value->dim[i]);
			}
			next_value = std::make_shared<SemanticValue>(*value, value->hash, value->is_const, value->row, value->col);
			next_value->dim = calc_dim;
		}
	}

	++i;

	if (i < identifier_vector.size()) {
		if (next_value->type_name.empty()) {
			next_value = std::make_shared<SemanticValue>(Type::T_ANY, next_value->row, next_value->col);
		}
		else {
			std::shared_ptr<Scope> curr_scope = get_inner_most_struct_definition_scope(current_program, name_space, next_value->type_name);
			if (!curr_scope) {
				throw std::runtime_error("cannot find '" + TypeDefinition::buid_struct_type_name(name_space, next_value->type_name) + "' struct");
			}
			auto type_struct = curr_scope->find_declared_structure_definition(next_value->type_name);

			if (type_struct.variables.find(identifier_vector[i].identifier) == type_struct.variables.end()) {
				ExceptionHandler::throw_struct_member_err(next_value->type_name_space, next_value->type_name, identifier_vector[i].identifier);
			}

			next_value = std::make_shared<SemanticValue>(type_struct.variables[identifier_vector[i].identifier], 0, false, next_value->row, next_value->col);
		}

		if (identifier_vector[i].access_vector.size() > 0 || i < identifier_vector.size()) {
			return access_value(next_value, identifier_vector, i);
		}
	}

	return next_value;
}

void SemanticAnalyser::check_is_struct_exists(Type type, const std::string& name_space, const std::string& type_name) {
	if (TypeUtils::is_struct(type)) {
		if (!get_inner_most_struct_definition_scope(current_program_stack.top(), name_space, type_name)) {
			throw std::runtime_error("struct '" + TypeDefinition::buid_struct_type_name(name_space, type_name) + "' was not defined");
		}
	}
}

std::vector<size_t> SemanticAnalyser::evaluate_access_vector(const std::vector<std::shared_ptr<ASTExprNode>>& expr_access_vector) {
	auto access_vector = std::vector<size_t>();
	for (const auto& expr_ptr : expr_access_vector) {
		auto expr = std::dynamic_pointer_cast<ASTExprNode>(expr_ptr);
		size_t val = 0;
		if (expr) {
			expr->accept(this);
			val = expr->hash(this);
			if (!TypeUtils::is_int(current_expression.type) && !TypeUtils::is_any(current_expression.type)) {
				throw std::runtime_error("array index access must be a integer value");
			}
		}
		access_vector.push_back(val);
	}
	return access_vector;
}

bool SemanticAnalyser::returns(std::shared_ptr<ASTNode> astnode) {
	if (is_return_node(astnode)) {
		return true;
	}

	if (const auto& block_node = std::dynamic_pointer_cast<ASTBlockNode>(astnode)) {
		bool block_return = false;
		bool sub_return = block_node->statements.size() > 0;
		for (const auto& block_stmt : block_node->statements) {
			if (is_return_node(block_stmt)) {
				block_return = true;
				break;
			}

			if (sub_return) {
				if (!returns(block_stmt)) {
					sub_return = false;
				}

				if (std::dynamic_pointer_cast<ASTBreakNode>(block_stmt) || std::dynamic_pointer_cast<ASTContinueNode>(block_stmt)) {
					sub_return = false;
				}
			}
		}
		return block_return || sub_return;
	}

	if (const auto& if_node = std::dynamic_pointer_cast<ASTIfNode>(astnode)) {
		bool if_return = returns(if_node->if_block);
		bool elif_return = true;
		bool else_return = true;
		for (const auto& elif_stmt : if_node->else_ifs) {
			if (!returns(elif_stmt->block)) {
				elif_return = false;
				break;
			}
		}
		if (if_node->else_block) {
			else_return = returns(if_node->else_block);
		}
		return if_return && elif_return && else_return;
	}

	if (const auto& trycatch_node = std::dynamic_pointer_cast<ASTTryCatchNode>(astnode)) {
		return returns(trycatch_node->try_block) && returns(trycatch_node->catch_block);
	}

	if (const auto& switch_node = std::dynamic_pointer_cast<ASTSwitchNode>(astnode)) {
		std::vector<size_t> positions;
		for (const auto& [key, value] : switch_node->case_blocks) {
			positions.push_back(value);
		}
		std::sort(positions.begin(), positions.end());

		for (size_t pi = 0; pi < positions.size() + 2; ++pi) {
			size_t current_block_start = 0;
			size_t current_block_end = 0;

			if (pi < positions.size()) {
				current_block_start = positions[pi];
				current_block_end = pi < positions.size() - 1 ? positions[pi + 1] : switch_node->default_block;
			}
			else {
				current_block_start = switch_node->default_block;
				current_block_end = switch_node->statements.size();
			}


			bool block_return = false;
			for (size_t i = current_block_start; i < current_block_end; ++i) {
				const auto& switch_stmt = switch_node->statements[i];

				if (returns(switch_stmt)) {
					block_return = true;
					break;
				}

				if (const auto& break_node = std::dynamic_pointer_cast<ASTBreakNode>(switch_stmt)) {
					break;
				}
			}

			if (!block_return) {
				return false;
			}
		}

		return true;
	}

	if (const auto& for_node = std::dynamic_pointer_cast<ASTForNode>(astnode)) {
		return returns(for_node->block);
	}

	if (const auto& foreach_node = std::dynamic_pointer_cast<ASTForEachNode>(astnode)) {
		return returns(foreach_node->block);
	}

	if (const auto& while_node = std::dynamic_pointer_cast<ASTWhileNode>(astnode)) {
		return returns(while_node->block);
	}

	return false;
}

void SemanticAnalyser::build_args(const std::vector<std::string>& args) {
	// program args
	auto dim = std::vector<size_t>{ (size_t)args.size() };
	auto var = std::make_shared<SemanticVariable>("args", Type::T_ARRAY, Type::T_STRING, dim, "", "", true, 0, 0);
	var->set_value(std::make_shared<SemanticValue>(Type::T_ARRAY, Type::T_STRING, dim, "", "", 0, true, 0, 0));
	scopes[Constants::DEFAULT_NAMESPACE].back()->declare_variable("args", var);

	// cwd
	auto cwd_var = std::make_shared<SemanticVariable>("cwd", Type::T_STRING, true, 0, 0);
	cwd_var->set_value(std::make_shared<SemanticValue>(Type::T_STRING, 0, true, 0, 0));
	scopes[Constants::DEFAULT_NAMESPACE].back()->declare_variable("cwd", cwd_var);
}

bool SemanticAnalyser::is_return_node(std::shared_ptr<ASTNode> astnode) {
	return std::dynamic_pointer_cast<ASTReturnNode>(astnode)
		|| std::dynamic_pointer_cast<ASTThrowNode>(astnode);
}

intmax_t SemanticAnalyser::hash(std::shared_ptr<ASTExprNode> astnode) {
	astnode->accept(this);
	return 0;
}

intmax_t SemanticAnalyser::hash(std::shared_ptr<ASTLiteralNode<flx_bool>> astnode) {
	return static_cast<intmax_t>(astnode->value);
}

intmax_t SemanticAnalyser::hash(std::shared_ptr<ASTLiteralNode<flx_int>> astnode) {
	return static_cast<intmax_t>(astnode->value);
}

intmax_t SemanticAnalyser::hash(std::shared_ptr<ASTLiteralNode<flx_float>> astnode) {
	return static_cast<intmax_t>(astnode->value);
}

intmax_t SemanticAnalyser::hash(std::shared_ptr<ASTLiteralNode<flx_char>> astnode) {
	return static_cast<intmax_t>(astnode->value);
}

intmax_t SemanticAnalyser::hash(std::shared_ptr<ASTLiteralNode<flx_string>> astnode) {
	return utils::StringUtils::hashcode(astnode->value);
}

intmax_t SemanticAnalyser::hash(std::shared_ptr<ASTIdentifierNode> astnode) {
	astnode->accept(this);
	return current_expression.hash;
}

intmax_t SemanticAnalyser::hash(std::shared_ptr<ASTValueNode> astnode) {
	astnode->accept(this);
	return current_expression.hash;
}

void SemanticAnalyser::set_curr_pos(size_t row, size_t col) {
	curr_row = row;
	curr_col = col;
}

std::string SemanticAnalyser::msg_header() {
	return "(SERR) " + current_program_stack.top()->name + '[' + std::to_string(curr_row) + ':' + std::to_string(curr_col) + "]: ";
}
