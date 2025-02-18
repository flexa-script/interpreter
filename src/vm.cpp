#include <iostream>
#include <algorithm> 
#include <cmath>
#include <compare>
#include <functional>

#include "vm.hpp"
#include "exception_handler.hpp"

#include "utils.hpp"
#include "watch.hpp"

using namespace vm;

VirtualMachine::VirtualMachine(std::shared_ptr<Scope> global_scope, std::vector<BytecodeInstruction> instructions)
	: instructions(instructions), gc(GarbageCollector()), set_default_value(nullptr) {
	cleanup_type_set();
	gc.add_root_container(value_stack);

	push_namespace(default_namespace);
	scopes[default_namespace].push_back(global_scope);

	built_in_libs["builtin"]->register_functions(this);
}

void VirtualMachine::run() {

	while (get_next()) {
		if (try_deep < 0) {
			break;
		}
		try {
			decode_operation();
		}
		catch (std::exception ex) {
			if (try_deep) {
				--try_deep;
			}
			throw std::runtime_error(ex.what());
		}
	}

	if (value_stack->empty()) {
		push_constant(new RuntimeValue(flx_int(0)));
	}

	gc.collect();

}

void VirtualMachine::push_empty(Type type) {
	auto val = gc.allocate(new RuntimeValue(type));
	value_stack->push_back(dynamic_cast<RuntimeValue*>(val));
}

RuntimeValue* VirtualMachine::alocate_value(RuntimeValue* value) {
	return dynamic_cast<RuntimeValue*>(gc.allocate(value));
}

void VirtualMachine::push_constant(RuntimeValue* value) {
	auto val = gc.allocate(value);
	value_stack->push_back(dynamic_cast<RuntimeValue*>(val));
}

void VirtualMachine::push_function_constant(const std::string& identifier) {
	auto fdef = utils::StringUtils::split(identifier, ":");
	push_constant(new RuntimeValue(flx_function(fdef[0], fdef[1])));
}

void VirtualMachine::binary_operation(const std::string& op) {
	RuntimeValue* rval = get_stack_top();
	RuntimeValue* lval = get_stack_top();

	auto res = RuntimeOperations::do_operation(op, lval, rval, evaluate_access_vector_ptr, true);

	if (res != lval && res != rval) {
		push_constant(res);
	}
}

void VirtualMachine::unary_operation(const std::string& op) {
	RuntimeValue* value = value_stack->back();

	if (op == "ref" || op == "unref") {
		if (op == "unref") {
			value->use_ref = false;
		}
		else if (op == "ref") {
			value->use_ref = true;
		}
	}
	else {
		if (!value->use_ref) {
			value_stack->pop_back();
			push_constant(new RuntimeValue(value));
		}

		switch (value->type) {
		case Type::T_INT:
			if (op == "-") {
				value->set(flx_int(-value->get_i()));
			}
			else if (op == "~") {
				value->set(flx_int(~value->get_i()));
			}
			break;
		case Type::T_FLOAT:
			if (op == "-") {
				value->set(flx_float(-value->get_f()));
			}
			break;
		case Type::T_BOOL:
			value->set(flx_bool(!value->get_b()));
			break;
		default:
			throw std::runtime_error("incompatible unary operator '" + op +
				"' in front of " + type_str(value->type) + " expression");
		}
	}
}

void VirtualMachine::handle_call() {
	return_stack.push(pc + 1);

	std::string name_space = get_namespace();
	std::string identifier = current_instruction.get_string_operand();
	bool strict = true;
	std::vector<TypeDefinition*> signature;
	std::vector<RuntimeValue*> function_arguments;

	//gc.add_root_container(&function_arguments);

	auto ss = value_stack->size();
	long long l = ss - param_count;
	for (long long i = ss - 1; i >= l; --i) {
		RuntimeValue* value = value_stack->at(i);

		RuntimeValue* pvalue = nullptr;
		if (value->use_ref) {
			pvalue = value;
		}
		else {
			pvalue = alocate_value(new RuntimeValue(value));
		}

		function_arguments.insert(function_arguments.begin(), pvalue);
		signature.insert(signature.begin(), pvalue);
	}

	std::shared_ptr<Scope> func_scope;
	try {
		func_scope = get_inner_most_function_scope(nullptr, name_space, identifier, &signature, evaluate_access_vector_ptr, strict);
	}
	catch (...) {
		try {
			strict = false;
			func_scope = get_inner_most_function_scope(nullptr, name_space, identifier, &signature, evaluate_access_vector_ptr, strict);
		}
		catch (...) {
			try {
				auto var_scope = get_inner_most_variable_scope(nullptr, name_space, identifier);
				auto var = std::dynamic_pointer_cast<RuntimeVariable>(var_scope->find_declared_variable(identifier));
				name_space = var->value->get_fun().first;
				identifier = var->value->get_fun().second;
				auto identifier_vector = std::vector<Identifier>{ Identifier(identifier) };
				func_scope = get_inner_most_function_scope(nullptr, name_space, identifier, &signature, evaluate_access_vector_ptr, strict);
			}
			catch (...) {
				std::string func_name = ExceptionHandler::buid_signature(identifier, signature, evaluate_access_vector_ptr);
				throw std::runtime_error("function '" + func_name + "' was never declared");
			}
		}
	}

	auto& declfun = func_scope->find_declared_function(identifier, &signature, evaluate_access_vector_ptr, strict);

	if (declfun.pointer) {
		pc = declfun.pointer;
	}
	else {
		return_stack.pop();
		builtin_functions[identifier]();
	}

	//gc.remove_root_container(&function_arguments);
}

void VirtualMachine::handle_throw() {
	auto value = get_stack_top();

	if (is_struct(value->type)
		&& value->type_name == "Exception") {
		try {
			get_inner_most_struct_definition_scope(nullptr, language_namespace, "Exception");
		}
		catch (...) {
			throw std::runtime_error("struct 'flx::Exception' not found");
		}

		throw std::exception(value->get_str()["error"]->get_s().c_str());
	}
	else if (is_string(value->type)) {
		throw std::runtime_error(value->get_s());
	}
	else {
		throw std::runtime_error("expected flx::Exception struct or string in throw");
	}

}

void VirtualMachine::handle_type_parse() {
	Type type = Type(current_instruction.get_uint8_operand());
	auto value = get_stack_top();
	RuntimeValue* new_value = new RuntimeValue();

	switch (type) {
	case Type::T_BOOL:
		switch (value->type) {
		case Type::T_BOOL:
			new_value->copy_from(value);
			break;
		case Type::T_INT:
			new_value->set(flx_bool(value->get_i() != 0));
			break;
		case Type::T_FLOAT:
			new_value->set(flx_bool(value->get_f() != .0));
			break;
		case Type::T_CHAR:
			new_value->set(flx_bool(value->get_c() != '\0'));
			break;
		case Type::T_STRING:
			new_value->set(flx_bool(!value->get_s().empty()));
			break;
		}
		break;

	case Type::T_INT:
		switch (type) {
		case Type::T_BOOL:
			new_value->set(flx_int(value->get_b()));
			break;
		case Type::T_INT:
			new_value->copy_from(value);
			break;
		case Type::T_FLOAT:
			new_value->set(flx_int(value->get_f()));
			break;
		case Type::T_CHAR:
			new_value->set(flx_int(value->get_c()));
			break;
		case Type::T_STRING:
			try {
				new_value->set(flx_int(std::stoll(value->get_s())));
			}
			catch (...) {
				throw std::runtime_error("'" + value->get_s() + "' is not a valid value to parse int");
			}
			break;
		}
		break;

	case Type::T_FLOAT:
		switch (value->type) {
		case Type::T_BOOL:
			new_value->set(flx_float(value->get_b()));
			break;
		case Type::T_INT:
			new_value->set(flx_float(value->get_i()));
			break;
		case Type::T_FLOAT:
			new_value->copy_from(value);
			break;
		case Type::T_CHAR:
			new_value->set(flx_float(value->get_c()));
			break;
		case Type::T_STRING:
			try {
				new_value->set(flx_float(std::stold(value->get_s())));
			}
			catch (...) {
				throw std::runtime_error("'" + value->get_s() + "' is not a valid value to parse float");
			}
			break;
		}
		break;

	case Type::T_CHAR:
		switch (value->type) {
		case Type::T_BOOL:
			new_value->set(flx_char(value->get_b()));
			break;
		case Type::T_INT:
			new_value->set(flx_char(value->get_i()));
			break;
		case Type::T_FLOAT:
			new_value->set(flx_char(value->get_f()));
			break;
		case Type::T_CHAR:
			new_value->copy_from(value);
			break;
		case Type::T_STRING:
			if (new_value->get_s().size() > 1) {
				throw std::runtime_error("'" + value->get_s() + "' is not a valid value to parse char");
			}
			else {
				new_value->set(flx_char(value->get_s()[0]));
			}
			break;
		}
		break;

	case Type::T_STRING:
		new_value->set(flx_string(RuntimeOperations::parse_value_to_string(value)));

	}

	new_value->set_type(type);

	push_constant(new_value);
}

void VirtualMachine::handle_store_var() {
	const auto& name_space = get_namespace();

	auto identifier = current_instruction.get_string_operand();

	RuntimeValue* new_value = get_stack_top();
	if (!new_value->use_ref) {
		new_value = alocate_value(new RuntimeValue(new_value));
	}

	auto new_var = std::make_shared<RuntimeVariable>(identifier, set_type,
		set_array_type, set_array_dim,
		set_type_name, set_type_name_space);
	gc.add_var_root(new_var);
	new_var->set_value(new_value);

	if ((!TypeDefinition::is_any_or_match_type(*new_var, *new_value, evaluate_access_vector_ptr) ||
		is_array(new_var->type) && !is_any(new_var->array_type)
		&& !TypeDefinition::match_type(*new_var, *new_value, evaluate_access_vector_ptr, false, true))) {
		ExceptionHandler::throw_declaration_type_err(identifier, *new_var, *new_value, evaluate_access_vector_ptr);
	}

	RuntimeOperations::normalize_type(new_var.get(), new_value);

	scopes[name_space].back()->declare_variable(identifier, new_var);

	cleanup_type_set();
}

void VirtualMachine::handle_load_var() {
	auto name_space = get_namespace();

	auto identifier = current_instruction.get_string_operand();

	std::shared_ptr<Scope> id_scope;
	try {
		id_scope = get_inner_most_variable_scope(nullptr, name_space, identifier);
	}
	catch (...) {
		//const auto& dim = astnode->identifier_vector[0].access_vector;
		//auto type = Type::T_UNDEFINED;
		//auto expression_value = alocate_value(new RuntimeValue(Type::T_UNDEFINED));

		//if (astnode->identifier == "bool") {
		//	type = Type::T_BOOL;
		//}
		//else if (astnode->identifier == "int") {
		//	type = Type::T_INT;
		//}
		//else if (astnode->identifier == "float") {
		//	type = Type::T_FLOAT;
		//}
		//else if (astnode->identifier == "char") {
		//	type = Type::T_CHAR;
		//}
		//else if (astnode->identifier == "string") {
		//	type = Type::T_STRING;
		//}
		//else if (astnode->identifier == "function") {
		//	type = Type::T_FUNCTION;
		//}

		//if (is_undefined(type)) {
		//	std::shared_ptr<Scope> curr_scope;
		//	try {
		//		curr_scope = get_inner_most_struct_definition_scope(nmspace, astnode->identifier);
		//	}
		//	catch (...) {
		//		try {
		//			curr_scope = get_inner_most_function_scope(nmspace, astnode->identifier, nullptr);
		//			auto fun = cp_function();
		//			fun.first = nmspace;
		//			fun.second = astnode->identifier;
		//			current_expression_value = alocate_value(new RuntimeValue(Type::T_FUNCTION));
		//			current_expression_value->set(fun);
		//			return;
		//		}
		//		catch (...) {
		//			throw std::runtime_error("identifier '" + astnode->identifier + "' was not declared");
		//		}
		//	}
		//	type = Type::T_STRUCT;
		//	auto str = cp_struct();
		//	expression_value->set(str, astnode->identifier, nmspace);
		//}

		//expression_value->set_type(type);

		//if (dim.size() > 0) {
		//	cp_array arr = build_array(dim, expression_value, dim.size() - 1);

		//	current_expression_value = alocate_value(new RuntimeValue(arr, type, dim));
		//}
		//else {
		//	current_expression_value = alocate_value(new RuntimeValue(expression_value));
		//}

		//return;
	}

	auto variable = std::dynamic_pointer_cast<RuntimeVariable>(id_scope->find_declared_variable(identifier));
	value_stack->push_back(variable->get_value());
}

void VirtualMachine::handle_include_namespace() {
	program_nmspaces[get_namespace()].push_back(current_instruction.get_string_operand());
}

void VirtualMachine::handle_exclude_namespace() {
	const auto& op_nmspace = current_instruction.get_string_operand();
	const auto& name_space = get_namespace();
	size_t pos = std::distance(program_nmspaces[name_space].begin(),
		std::find(program_nmspaces[name_space].begin(),
			program_nmspaces[name_space].end(), op_nmspace));
	program_nmspaces[name_space].erase(program_nmspaces[name_space].begin() + pos);
}

void VirtualMachine::handle_init_array() {
	auto size = current_instruction.get_size_operand();
	value_build_stack.push(new RuntimeValue(flx_array(size)));
}

void VirtualMachine::handle_set_element() {
	RuntimeValue* value = get_stack_top();
	value_build_stack.top()->set_sub(current_instruction.get_size_operand(), value);
}

void VirtualMachine::handle_push_array() {
	push_constant(value_build_stack.top());
	value_build_stack.pop();
}

void VirtualMachine::handle_init_struct() {
	auto type_name_space = get_namespace();
	auto identifier = current_instruction.get_string_operand();
	value_build_stack.push(new RuntimeValue(flx_struct(), identifier, type_name_space));
}

void VirtualMachine::handle_set_field() {
	RuntimeValue* value = get_stack_top();
	value_build_stack.top()->set_sub(current_instruction.get_string_operand(), value);
}

void VirtualMachine::handle_push_struct() {
	push_constant(value_build_stack.top());
	value_build_stack.pop();
}

void VirtualMachine::handle_struct_start() {
	struct_def_build_stack.push(StructureDefinition(current_instruction.get_string_operand()));
}

void VirtualMachine::handle_struct_set_var() {
	auto var_id = current_instruction.get_string_operand();

	auto var = VariableDefinition(var_id,
		set_type, set_type_name, set_type_name_space, set_array_type,
		set_array_dim, set_default_value, set_is_rest, 0, 0);

	struct_def_build_stack.top().variables[var_id] = var;

	cleanup_type_set();
}

void VirtualMachine::handle_struct_end() {
	auto& str = struct_def_build_stack.top();
	struct_def_build_stack.pop();
	scopes[get_namespace()].back()->declare_structure_definition(str);
}

void VirtualMachine::handle_load_sub_id() {
	auto id = current_instruction.get_string_operand();
	auto val = get_stack_top();
	if (!is_struct(val->type)) {
		throw std::runtime_error("Invalid " + type_str(val->type) + " access, this operation can only be performed on struct values");
	}
	value_stack->push_back(val->get_sub(id));
}

void VirtualMachine::handle_load_sub_ix() {
	auto i = get_stack_top();
	if (!is_int(i->type)) {
		throw std::runtime_error("Invalid type " + type_str(i->type) + " trying to access array");
	}
	auto val = get_stack_top();
	if (!is_array(val->type) && !is_string(val->type)) {
		throw std::runtime_error("Invalid " + type_str(val->type) + " index access, this operation can only be performed on array or string values");
	}
	value_stack->push_back(val->get_sub(i->get_i()));
}

void VirtualMachine::handle_assign_var() {
	auto val = get_stack_top();
	auto new_val = get_stack_top();
	if (new_val->use_ref) {
		if (auto val_ref = std::dynamic_pointer_cast<RuntimeVariable>(val->ref.lock())) {
			val_ref->set_value(val);
		}
	}
	else {
		val->copy_from(new_val);
	}
}

void VirtualMachine::handle_assign_sub_id() {
	auto id = current_instruction.get_string_operand();
	auto val = get_stack_top();
	auto new_val = get_stack_top();
	val->set_sub(id, new_val);
}

void VirtualMachine::handle_assign_sub_ix() {
	auto i = get_stack_top();
	auto val = get_stack_top();
	auto new_val = get_stack_top();
	val->set_sub(i->get_i(), new_val);
}

void VirtualMachine::handle_fun_start() {
	func_def_build_stack.push(FunctionDefinition(current_instruction.get_string_operand(), set_type, set_type_name,
		set_type_name_space, set_array_type, set_array_dim));
	cleanup_type_set();
}

void VirtualMachine::handle_fun_set_param() {
	auto var_id = current_instruction.get_string_operand();

	auto var = new VariableDefinition(var_id,
		set_type, set_type_name, set_type_name_space, set_array_type,
		set_array_dim, set_default_value, set_is_rest, 0, 0);

	func_def_build_stack.top().parameters.push_back(var);

	cleanup_type_set();
}

void VirtualMachine::handle_fun_end() {
	auto& fun = func_def_build_stack.top();
	func_def_build_stack.pop();
	fun.pointer = pc + 2;
	scopes[get_namespace()].back()->declare_function(fun.identifier, fun);
}

void VirtualMachine::handle_is_type() {
	Type type = static_cast<Type>(current_instruction.get_uint8_operand());
	auto value = get_stack_top();
	RuntimeValue* res_value = new RuntimeValue(Type::T_BOOL);
	if (is_any(type)) {
		res_value->set(flx_bool(
			(value->ref.lock() && (is_any(value->ref.lock()->type)) ||
				is_any(value->ref.lock()->array_type))));
		return;
	}
	else if (is_array(type)) {
		res_value->set(flx_bool(is_array(value->type) || value->dim.size() > 0));
		return;
	}
	else if (is_struct(type)) {
		res_value->set(flx_bool(is_struct(value->type) || is_struct(value->array_type)));
		return;
	}
}

void VirtualMachine::decode_operation() {
	switch (current_instruction.opcode) {
	case OP_RES:
		throw std::runtime_error("Reserved operation");
		break;

		// namespace operations
	case OP_POP_NAMESPACE:
		current_namespace.pop();
		break;
	case OP_PUSH_NAMESPACE:
		current_namespace.push(current_instruction.get_string_operand());
		break;
	case OP_PUSH_NAMESPACE_STACK:
		push_constant(new RuntimeValue(current_namespace.top()));
		break;
	case OP_INCLUDE_NAMESPACE:
		handle_include_namespace();
		break;

	case OP_EXCLUDE_NAMESPACE:
		handle_exclude_namespace();
		break;

		// constant operations
	case OP_POP_CONSTANT:
		value_stack->pop_back();
		break;
	case OP_PUSH_UNDEFINED:
		push_empty(Type::T_UNDEFINED);
		break;
	case OP_PUSH_VOID:
		push_empty(Type::T_VOID);
		break;
	case OP_PUSH_BOOL:
		push_constant(new RuntimeValue(current_instruction.get_bool_operand()));
		break;
	case OP_PUSH_INT:
		push_constant(new RuntimeValue(current_instruction.get_int_operand()));
		break;
	case OP_PUSH_FLOAT:
		push_constant(new RuntimeValue(current_instruction.get_float_operand()));
		break;
	case OP_PUSH_CHAR:
		push_constant(new RuntimeValue(current_instruction.get_char_operand()));
		break;
	case OP_PUSH_STRING:
		push_constant(new RuntimeValue(current_instruction.get_string_operand()));
		break;
	case OP_PUSH_FUNCTION:
		push_function_constant(current_instruction.get_string_operand());
		break;
	case OP_INIT_ARRAY:
		handle_init_array();
		break;
	case OP_SET_ELEMENT:
		handle_set_element();
		break;
	case OP_PUSH_ARRAY:
		handle_push_array();
		break;
	case OP_INIT_STRUCT:
		handle_init_struct();
		break;
	case OP_SET_FIELD:
		handle_set_field();
		break;
	case OP_PUSH_STRUCT:
		handle_push_struct();
		break;

		// struct definition operations
	case OP_STRUCT_START:
		handle_struct_start();
		break;
	case OP_STRUCT_SET_VAR:
		handle_struct_set_var();
		break;
	case OP_STRUCT_END:
		handle_struct_end();
		break;

		// typing operations
	case OP_SET_TYPE:
		set_type = (Type)current_instruction.get_uint8_operand();
		break;
	case OP_SET_ARRAY_TYPE:
		set_array_type = (Type)current_instruction.get_uint8_operand();
		break;
	case OP_SET_TYPE_NAME:
		set_type_name = current_instruction.get_string_operand();
		break;
	case OP_SET_TYPE_NAME_SPACE:
		set_type_name_space = current_instruction.get_string_operand();
		break;
	case OP_SET_ARRAY_SIZE:
		// todo: generating bug
	//set_array_dim.push_back(std::make_shared<ASTValueNode>(get_stack_top(), 0, 0));
		break;
	case OP_SET_DEFAULT_VALUE:
		set_default_value = std::make_shared<ASTValueNode>(get_stack_top(), 0, 0);
		break;
	case OP_SET_IS_REST:
		set_is_rest = current_instruction.get_bool_operand();
		break;

		// variable operations
	case OP_LOAD_VAR:
		handle_load_var();
		break;
	case OP_STORE_VAR:
		handle_store_var();
		break;
	case OP_LOAD_SUB_ID:
		handle_load_sub_id();
		break;
	case OP_LOAD_SUB_IX:
		handle_load_sub_ix();
		break;
	case OP_ASSIGN_VAR:
		handle_assign_var();
		break;
	case OP_ASSIGN_SUB_ID:
		handle_assign_sub_id();
		break;
	case OP_ASSIGN_SUB_IX:
		handle_assign_sub_ix();
		break;

		// function operations
	case OP_FUN_START:
		handle_fun_start();
		break;

	case OP_FUN_SET_PARAM:
		handle_fun_set_param();
		break;
	case OP_CALL_PARAM_COUNT:
		param_count = current_instruction.get_size_operand();
		break;
	case OP_FUN_END:
		handle_fun_end();
		break;
	case OP_CALL:
		handle_call();
		break;
	case OP_RETURN:
		pc = return_stack.top();
		return_stack.pop();
		break;

		// conditional operations
	case OP_CONTINUE:
		// todo OP_CONTINUE
		break;
	case OP_BREAK:
		// todo OP_BREAK
		break;
	case OP_TRY_START:
		try_deep++;
		break;
	case OP_TRY_END:
		try_deep--;
		break;
	case OP_THROW:
		handle_throw();
		break;
	case OP_GET_ITERATOR:
		// todo OP_GET_ITERATOR
		break;
	case OP_NEXT_ELEMENT:
		// todo OP_NEXT_ELEMENT
		break;
	case OP_JUMP:
		// todo OP_JUMP
		break;
	case OP_JUMP_IF_FALSE:
		// todo OP_JUMP_IF_FALSE
		break;
	case OP_JUMP_IF_FALSE_OR_NEXT:
		// todo OP_JUMP_IF_FALSE_OR_NEXT
		break;
	case OP_JUMP_IF_TRUE:
		// todo OP_JUMP_IF_TRUE
		break;
	case OP_JUMP_IF_TRUE_OR_NEXT:
		// todo OP_JUMP_IF_TRUE_OR_NEXT
		break;

		// expression operations
	case OP_IS_TYPE:
		handle_is_type();
		break;
	case OP_REFID:
		push_constant(new RuntimeValue(flx_int(get_stack_top())));
		break;
	case OP_TYPEID:
		push_constant(
			new RuntimeValue(
				flx_string(
					RuntimeOperations::build_str_type(get_stack_top(), evaluate_access_vector_ptr)
				)
			)
		);
		break;
	case OP_TYPEOF:
		push_constant(
			new RuntimeValue(
				flx_int(
					utils::StringUtils::hashcode(RuntimeOperations::build_str_type(get_stack_top(), evaluate_access_vector_ptr))
				)
			)
		);
		break;
	case OP_TYPE_PARSE:
		handle_type_parse();
		break;
	case OP_TERNARY:
		// todo OP_TERNARY
		break;
	case OP_IN:
		// todo OP_IN
		break;
	case OP_OR:
		binary_operation("or");
		break;
	case OP_AND:
		binary_operation("and");
		break;
	case OP_BIT_OR:
		binary_operation("|");
		break;
	case OP_BIT_XOR:
		binary_operation("^");
		break;
	case OP_BIT_AND:
		binary_operation("&");
		break;
	case OP_EQL:
		binary_operation("==");
		break;
	case OP_DIF:
		binary_operation("!=");
		break;
	case OP_LT:
		binary_operation("<");
		break;
	case OP_LTE:
		binary_operation("<=");
		break;
	case OP_GT:
		binary_operation(">");
		break;
	case OP_GTE:
		binary_operation("=>");
		break;
	case OP_SPACE_SHIP:
		binary_operation("<=>");
		break;
	case OP_LEFT_SHIFT:
		binary_operation("<<");
		break;
	case OP_RIGHT_SHIFT:
		binary_operation(">>");
		break;
	case OP_ADD:
		binary_operation("+");
		break;
	case OP_SUB:
		binary_operation("-");
		break;
	case OP_MUL:
		binary_operation("*");
		break;
	case OP_DIV:
		binary_operation("/");
		break;
	case OP_REMAINDER:
		binary_operation("%");
		break;
	case OP_FLOOR_DIV:
		binary_operation("/%");
		break;
	case OP_NOT:
		unary_operation("not");
		break;
	case OP_BIT_NOT:
		unary_operation("~");
		break;
	case OP_EXP:
		binary_operation("**");
		break;
	case OP_REF:
		unary_operation("ref");
		break;
	case OP_UNREF:
		unary_operation("unref");
		break;
	case OP_TRAP:
		break;
	case OP_HALT:
		pc = instructions.size();
		break;
	case OP_ERROR:
		std::cerr << "Operation error" << std::endl;
		break;

	default:
		std::cerr << "Unknow operation" << std::endl;
		break;
	}
}

RuntimeValue* VirtualMachine::get_stack_top() {
	auto value = value_stack->back();
	value_stack->pop_back();
	return value;
}

bool VirtualMachine::get_next() {
	if (pc >= instructions.size()) {
		return false;
	}
	current_instruction = instructions[pc++];
	return true;
}

void VirtualMachine::cleanup_type_set() {
	set_type = Type::T_UNDEFINED;
	set_type_name = "";
	set_type_name_space = "";
	set_array_type = Type::T_UNDEFINED;
	set_array_dim = std::vector<std::shared_ptr<ASTExprNode>>();
	set_default_value = nullptr;
	set_is_rest = false;
}

std::vector<unsigned int> VirtualMachine::evaluate_access_vector(const std::vector<std::shared_ptr<ASTExprNode>>& expr_access_vector) {
	auto access_vector = std::vector<unsigned int>();
	for (const auto& expr : expr_access_vector) {
		access_vector.push_back(std::dynamic_pointer_cast<RuntimeValue>(expr)->get_i());
	}
	return access_vector;
}
