#ifndef META_VISITOR_HPP
#define META_VISITOR_HPP

#include <string>
#include <vector>
#include <stack>
#include <map>

#include "scope.hpp"
#include "types.hpp"

namespace parser {
	class ASTProgramNode;
	template <typename T> class ASTLiteralNode;
	class ASTLambdaFunction;
	class ASTIdentifierNode;
}

using namespace parser;

namespace visitor {

	class ScopeManager {
	public:
		std::unordered_map<std::string, std::vector<std::shared_ptr<visitor::Scope>>> scopes;
		std::map<std::string, std::vector<std::string>> program_nmspaces;

		ScopeManager() = default;
		virtual ~ScopeManager() = default;

		void validates_reference_type_assignment(TypeDefinition owner, Value* value);

		StructureDefinition find_inner_most_struct(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier);
		std::shared_ptr<Variable> find_inner_most_variable(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier);

		std::shared_ptr<Scope> get_inner_most_struct_definition_scope_aux(const std::string& name_space, const std::string& identifier, std::vector<std::string>& visited);
		std::shared_ptr<Scope> get_inner_most_struct_definition_scope(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier,
			std::vector<std::string> vp = std::vector<std::string>(), std::vector<std::string> vf = std::vector<std::string>());

		std::shared_ptr<Scope> get_inner_most_functions_scope_aux(const std::string& name_space, const std::string& identifier, std::vector<std::string>& visited);
		std::shared_ptr<Scope> get_inner_most_functions_scope(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier,
			std::vector<std::string> vp = std::vector<std::string>(), std::vector<std::string> vf = std::vector<std::string>());

		std::shared_ptr<Scope> get_inner_most_variable_scope_aux(const std::string& name_space, const std::string& identifier, std::vector<std::string>& visited);
		std::shared_ptr<Scope> get_inner_most_variable_scope(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier,
			std::vector<std::string> vp = std::vector<std::string>(), std::vector<std::string> vf = std::vector<std::string>());

		std::shared_ptr<Scope> get_inner_most_function_scope_aux(const std::string& name_space, const std::string& identifier,
			const std::vector<TypeDefinition*>* signature, dim_eval_func_t evaluate_access_vector_ptr, bool strict, std::vector<std::string>& visited);
		std::shared_ptr<Scope> get_inner_most_function_scope(std::shared_ptr<ASTProgramNode> program, const std::string& name_space, const std::string& identifier,
			const std::vector<TypeDefinition*>* signature, dim_eval_func_t evaluate_access_vector_ptr, bool strict = true,
			std::vector<std::string> vp = std::vector<std::string>(), std::vector<std::string> vf = std::vector<std::string>());

	};
}

#endif // !META_VISITOR_HPP
