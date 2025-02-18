#include <utility>
#include <iostream>

#include "scope.hpp"
#include "utils.hpp"

using namespace visitor;
using namespace parser;

Scope::Scope(std::shared_ptr<ASTProgramNode> owner, std::string name) : owner(owner), name(name) {};

Scope::Scope(std::shared_ptr<ASTProgramNode> owner) : owner(owner) {}

Scope::~Scope() = default;

StructureDefinition Scope::find_declared_structure_definition(const std::string& identifier) {
	return structure_symbol_table.at(identifier);
}

std::shared_ptr<Variable> Scope::find_declared_variable(const std::string& identifier) {
	auto& var = variable_symbol_table.at(identifier);
	var->reset_ref();
	return var;
}

FunctionDefinition& Scope::find_declared_function(const std::string& identifier, const std::vector<TypeDefinition*>* signature,
	dim_eval_func_t evaluate_access_vector, bool strict) {
	auto funcs = function_symbol_table.equal_range(identifier);

	if (std::distance(funcs.first, funcs.second) == 0) {
		throw std::runtime_error("definition of '" + identifier + "' function signature not found");
	}

	for (auto& it = funcs.first; it != funcs.second; ++it) {
		if (it->second.is_var || !signature) {
			return it->second;
		}

		auto& func_sig = it->second.parameters;
		bool rest = false;
		auto found = true;
		TypeDefinition* stype = nullptr;
		TypeDefinition* ftype = nullptr;
		size_t func_sig_size = func_sig.size();
		size_t call_sig_size = signature->size();

		// if signatures size match, handle normal cases
		if (func_sig_size == call_sig_size) {
			for (size_t i = 0; i < call_sig_size; ++i) {
				ftype = func_sig.at(i);
				stype = signature->at(i);

				if (!TypeDefinition::is_any_or_match_type(*ftype, *stype, evaluate_access_vector, strict || stype->use_ref)) {
					found = false;
					break;
				}
			}

			if (found) {
				return it->second;
			}
		}

		// if function signature is lesser than signature call, handle rest case
		found = true;
		if (func_sig_size >= 1 && func_sig_size < call_sig_size) {
			for (size_t i = 0; i < call_sig_size; ++i) {
				if (!rest) {
					ftype = func_sig.at(i);

					auto parameter = dynamic_cast<VariableDefinition*>(it->second.parameters[i]);

					if (parameter && parameter->is_rest) {
						rest = true;
						if (is_array(ftype->type)) {
							ftype = new TypeDefinition(ftype->array_type, Type::T_UNDEFINED, std::vector<std::shared_ptr<ASTExprNode>>(), ftype->type_name, ftype->type_name_space);
						}
					}

					if (parameter && !parameter->is_rest && i == func_sig.size() - 1) {
						found = false;
						break;
					}
				}
				stype = signature->at(i);

				if (!TypeDefinition::is_any_or_match_type(*ftype, *stype, evaluate_access_vector, strict || stype->use_ref)) {
					found = false;
					break;
				}
			}

			if (found) {
				return it->second;
			}
		}

		// if function signature is greater than signature call, handle default value cases
		found = true;
		if (func_sig_size > call_sig_size) {
			for (size_t i = 0; i < func_sig_size; ++i) {
				if (i < call_sig_size) {
					ftype = func_sig.at(i);
					stype = signature->at(i);

					if (!TypeDefinition::is_any_or_match_type(*ftype, *stype, evaluate_access_vector, strict || stype->use_ref)) {
						found = false;
						break;
					}
				}
				else {
					auto parameter = dynamic_cast<VariableDefinition*>(it->second.parameters[i]);

					if (parameter && !parameter->default_value) {
						found = false;
						break;
					}
				}
			}

			// if found and exactly signature size (not rest)
			if (found) {
				return it->second;
			}
		}
	}

	throw std::runtime_error("something went wrong when determining the type of '" + identifier + "' function");
}

std::pair<std::unordered_multimap<std::string, FunctionDefinition>::iterator,
	std::unordered_multimap<std::string, FunctionDefinition>::iterator>
	Scope::find_declared_functions(const std::string& identifier) {
	auto funcs = function_symbol_table.equal_range(identifier);
	if (std::distance(funcs.first, funcs.second) == 0) {
		throw std::runtime_error("definition of '" + identifier + "' function signature not found");
	}
	return funcs;
}

bool Scope::already_declared_structure_definition(const std::string& identifier) {
	return structure_symbol_table.find(identifier) != structure_symbol_table.end();
}

bool Scope::already_declared_variable(const std::string& identifier) {
	return variable_symbol_table.find(identifier) != variable_symbol_table.end();
}

bool Scope::already_declared_function(const std::string& identifier, const std::vector<TypeDefinition*>* signature,
	dim_eval_func_t evaluate_access_vector, bool strict) {
	try {
		find_declared_function(identifier, signature, evaluate_access_vector, strict);
		return true;
	}
	catch (...) {
		return false;
	}
}

bool Scope::already_declared_function_name(const std::string& identifier) {
	try {
		find_declared_functions(identifier);
		return true;
	}
	catch (...) {
		return false;
	}
}


size_t Scope::total_declared_variables() {
	return variable_symbol_table.size();
}

void Scope::declare_structure_definition(StructureDefinition structure) {
	structure_symbol_table[structure.identifier] = structure;
}

void Scope::declare_variable(const std::string& identifier, const std::shared_ptr<Variable>& variable) {
	variable_symbol_table[identifier] = variable;
}

void Scope::declare_function(const std::string& identifier, FunctionDefinition function) {
	function_symbol_table.insert(std::make_pair(identifier, function));
}
