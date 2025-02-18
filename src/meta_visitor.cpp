#include "meta_visitor.hpp"
#include "utils.hpp"

using namespace visitor;

const std::string& MetaVisitor::get_namespace() const {
	if (current_namespace.empty()) {
		return "";
	}
	return current_namespace.top();
}

bool MetaVisitor::push_namespace(const std::string name_space) {
	if (!name_space.empty() && name_space != get_namespace()) {
		current_namespace.push(name_space);
		return true;
	}
	return false;
}

void MetaVisitor::pop_namespace(bool pop) {
	if (pop) {
		current_namespace.pop();
	}
}

void MetaVisitor::validates_reference_type_assignment(TypeDefinition owner, Value* value) {
	if (is_string(owner.type) && is_char(value->type)
		&& value->use_ref && value->ref.lock() && !is_any(value->ref.lock()->type)) {
		throw std::runtime_error("cannot reference char to string variable");
	}
	else if (is_float(owner.type) && is_int(value->type)
		&& value->use_ref && value->ref.lock() && !is_any(value->ref.lock()->type)) {
		throw std::runtime_error("cannot reference int to float variable");
	}
}

StructureDefinition MetaVisitor::find_inner_most_struct(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier) {
	std::shared_ptr<Scope> scope = get_inner_most_struct_definition_scope(program, name_space, identifier);
	if (!scope) {
		throw std::runtime_error("struct '" + identifier + "' not found");
	}
	return scope->find_declared_structure_definition(identifier);
}

std::shared_ptr<Variable> MetaVisitor::find_inner_most_variable(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier) {
	std::shared_ptr<Scope> scope = get_inner_most_variable_scope(program, name_space, identifier);
	if (!scope) {
		throw std::runtime_error("variable '" + identifier + "' not found");
	}
	return scope->find_declared_variable(identifier);
}

std::shared_ptr<Scope> MetaVisitor::get_inner_most_variable_scope_aux(const std::string& name_space, const std::string& identifier, std::vector<std::string>& visited) {
	if (name_space.empty()) {
		return nullptr;
	}
	if (utils::CollectionUtils::contains(visited, name_space)) {
		return nullptr;
	}
	visited.push_back(name_space);

	long long i;
	for (i = scopes[name_space].size() - 1; i >= 0 && !scopes[name_space][i]->already_declared_variable(identifier); i--);
	if (i < 0) {
		return nullptr;
	}
	return scopes[name_space][i];
}

std::shared_ptr<Scope> MetaVisitor::get_inner_most_variable_scope(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier, std::vector<std::string> vp, std::vector<std::string> vf) {
	if (utils::CollectionUtils::contains(vp, program->name)) {
		return nullptr;
	}
	vp.push_back(program->name);

	std::shared_ptr<Scope> scope = nullptr;

	// try find at given namespace
	scope = get_inner_most_variable_scope_aux(name_space, identifier, vf);
	if (scope) {
		return scope;
	}

	// try find at program namespace
	if (!program->name_space.empty()) {
		scope = get_inner_most_variable_scope_aux(program->name_space, identifier, vf);
		if (scope) {
			return scope;
		}
	}

	// try find at program included namespace
	for (const auto& prgnmspace : program_nmspaces[program->name]) {
		scope = get_inner_most_variable_scope_aux(prgnmspace, identifier, vf);
		if (scope) {
			return scope;
		}
	}

	// try find at included libs namespaces
	for (auto& lib : program->libs) {
		for (const auto& prgnmspace : program_nmspaces[lib->name]) {
			scope = get_inner_most_variable_scope_aux(prgnmspace, identifier, vf);
			if (scope) {
				return scope;
			}
		}
	}

	return nullptr;
}

std::shared_ptr<Scope> MetaVisitor::get_inner_most_struct_definition_scope_aux(const std::string& name_space, const std::string& identifier, std::vector<std::string>& visited) {
	if (name_space.empty()) {
		return nullptr;
	}
	if (utils::CollectionUtils::contains(visited, name_space)) {
		return nullptr;
	}
	visited.push_back(name_space);

	long long i;
	for (i = scopes[name_space].size() - 1; i >= 0 && !scopes[name_space][i]->already_declared_structure_definition(identifier); i--);
	if (i < 0) {
		return nullptr;
	}
	return scopes[name_space][i];
}

std::shared_ptr<Scope> MetaVisitor::get_inner_most_struct_definition_scope(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier, std::vector<std::string> vp, std::vector<std::string> vf) {
	if (utils::CollectionUtils::contains(vp, program->name)) {
		return nullptr;
	}
	vp.push_back(program->name);

	std::shared_ptr<Scope> scope = nullptr;

	// try find at given namespace
	scope = get_inner_most_struct_definition_scope_aux(name_space, identifier, vf);
	if (scope) {
		return scope;
	}

	// try find at program namespace
	if (!program->name_space.empty()) {
		scope = get_inner_most_struct_definition_scope_aux(program->name_space, identifier, vf);
		if (scope) {
			return scope;
		}
	}

	// try find at program included namespace
	for (const auto& prgnmspace : program_nmspaces[program->name]) {
		scope = get_inner_most_struct_definition_scope_aux(prgnmspace, identifier, vf);
		if (scope) {
			return scope;
		}
	}

	// try find at included libs namespaces
	for (auto& lib : program->libs) {
		for (const auto& prgnmspace : program_nmspaces[lib->name]) {
			scope = get_inner_most_struct_definition_scope_aux(prgnmspace, identifier, vf);
			if (scope) {
				return scope;
			}
		}
	}

	return nullptr;
}

std::shared_ptr<Scope> MetaVisitor::get_inner_most_functions_scope_aux(const std::string& name_space, const std::string& identifier, std::vector<std::string>& visited) {
	if (name_space.empty()) {
		return nullptr;
	}
	if (utils::CollectionUtils::contains(visited, name_space)) {
		return nullptr;
	}
	visited.push_back(name_space);

	long long i;
	for (i = scopes[name_space].size() - 1; i >= 0 && !scopes[name_space][i]->already_declared_function_name(identifier); i--);
	if (i < 0) {
		return nullptr;
	}
	return scopes[name_space][i];
}

std::shared_ptr<Scope> MetaVisitor::get_inner_most_functions_scope(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier,
	std::vector<std::string> vp, std::vector<std::string> vf) {
	if (utils::CollectionUtils::contains(vp, program->name)) {
		return nullptr;
	}
	vp.push_back(program->name);

	std::shared_ptr<Scope> scope = nullptr;

	// try find at given namespace
	scope = get_inner_most_functions_scope_aux(name_space, identifier, vf);
	if (scope) {
		return scope;
	}

	// try find at program namespace
	if (!program->name_space.empty()) {
		scope = get_inner_most_functions_scope_aux(program->name_space, identifier, vf);
		if (scope) {
			return scope;
		}
	}

	// try find at program included namespace
	for (const auto& prgnmspace : program_nmspaces[program->name]) {
		scope = get_inner_most_functions_scope_aux(prgnmspace, identifier, vf);
		if (scope) {
			return scope;
		}
	}

	// try find at included libs namespaces
	for (auto& lib : program->libs) {
		for (const auto& prgnmspace : program_nmspaces[lib->name]) {
			scope = get_inner_most_functions_scope_aux(prgnmspace, identifier, vf);
			if (scope) {
				return scope;
			}
		}
	}

	return nullptr;
}

std::shared_ptr<Scope> MetaVisitor::get_inner_most_function_scope_aux(const std::string& name_space, const std::string& identifier,
	const std::vector<TypeDefinition*>* signature, dim_eval_func_t evaluate_access_vector_ptr, bool strict, std::vector<std::string>& visited) {
	if (name_space.empty()) {
		return nullptr;
	}
	if (utils::CollectionUtils::contains(visited, name_space)) {
		return nullptr;
	}
	visited.push_back(name_space);

	long long i;
	for (i = scopes[name_space].size() - 1; i >= 0 && !scopes[name_space][i]->already_declared_function(identifier, signature, evaluate_access_vector_ptr, strict); i--);
	if (i < 0) {
		return nullptr;
	}
	return scopes[name_space][i];
}

std::shared_ptr<Scope> MetaVisitor::get_inner_most_function_scope(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier,
	const std::vector<TypeDefinition*>* signature, dim_eval_func_t evaluate_access_vector_ptr, bool strict, std::vector<std::string> vp, std::vector<std::string> vf) {
	if (utils::CollectionUtils::contains(vp, program->name)) {
		return nullptr;
	}
	vp.push_back(program->name);

	std::shared_ptr<Scope> scope = nullptr;

	// try find at given namespace
	scope = get_inner_most_function_scope_aux(name_space, identifier, signature, evaluate_access_vector_ptr, strict, vf);
	if (scope) {
		return scope;
	}

	// try find at program namespace
	if (!program->name_space.empty()) {
		scope = get_inner_most_function_scope_aux(program->name_space, identifier, signature, evaluate_access_vector_ptr, strict, vf);
		if (scope) {
			return scope;
		}
	}

	// try find at program included namespace
	for (const auto& prgnmspace : program_nmspaces[program->name]) {
		scope = get_inner_most_function_scope_aux(prgnmspace, identifier, signature, evaluate_access_vector_ptr, strict, vf);
		if (scope) {
			return scope;
		}
	}

	// try find at included libs namespaces
	for (auto& lib : program->libs) {
		for (const auto& prgnmspace : program_nmspaces[lib->name]) {
			scope = get_inner_most_function_scope_aux(prgnmspace, identifier, signature, evaluate_access_vector_ptr, strict, vf);
			if (scope) {
				return scope;
			}
		}
	}

	return nullptr;
}
