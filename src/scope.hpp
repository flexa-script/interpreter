#ifndef SEMANTIC_SCOPE_HPP
#define SEMANTIC_SCOPE_HPP

#include <map>
#include <unordered_map>
#include <vector>
#include <stack>
#include <xutility>

#include "types.hpp"
#include "ast.hpp"

using namespace parser;

namespace visitor {

	class Scope {
	private:
		std::unordered_map<std::string, StructureDefinition> structure_symbol_table;
		std::unordered_multimap<std::string, FunctionDefinition> function_symbol_table;
		std::unordered_map<std::string, std::shared_ptr<Variable>> variable_symbol_table;

	public:
		std::string name;
		std::shared_ptr<ASTProgramNode> owner;

		Scope(std::shared_ptr<ASTProgramNode> owner, std::string name);
		Scope(std::shared_ptr<ASTProgramNode> owner);
		~Scope();

		bool already_declared_structure_definition(const std::string& identifier);
		bool already_declared_variable(const std::string& identifier);
		bool already_declared_function(const std::string& identifier, const std::vector<TypeDefinition*>* signature,
			dim_eval_func_t evaluate_access_vector, bool strict = true);
		bool already_declared_function_name(const std::string& identifier);

		size_t total_declared_variables();

		void declare_structure_definition(StructureDefinition structure);
		void declare_function(const std::string& identifier, FunctionDefinition function);
		void declare_variable(const std::string& identifier, const std::shared_ptr<Variable>& variable);

		StructureDefinition find_declared_structure_definition(const std::string& identifier);
		FunctionDefinition& find_declared_function(const std::string& identifier, const std::vector<TypeDefinition*>* signature,
			dim_eval_func_t evaluate_access_vector, bool strict = true);
		std::pair<std::unordered_multimap<std::string, FunctionDefinition>::iterator,
			std::unordered_multimap<std::string, FunctionDefinition>::iterator> find_declared_functions(const std::string& identifier);
		std::shared_ptr<Variable> find_declared_variable(const std::string& identifier);

	};
}

#endif // !SEMANTIC_SCOPE_HPP
