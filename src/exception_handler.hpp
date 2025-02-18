#ifndef EXCEPTION_HANDLER_HPP
#define EXCEPTION_HANDLER_HPP

#include <string>

#include "visitor.hpp"

using namespace parser;

class ExceptionHandler {
public:
	static void throw_operation_err(const std::string op, const TypeDefinition& ltype, const TypeDefinition& rtype, dim_eval_func_t evaluate_access_vector);
	static void throw_unary_operation_err(const std::string op, const TypeDefinition& type, dim_eval_func_t evaluate_access_vector);
	static void throw_declaration_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype, dim_eval_func_t evaluate_access_vector);
	static void throw_return_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype, dim_eval_func_t evaluate_access_vector);
	static void throw_mismatched_type_err(const TypeDefinition& ltype, const TypeDefinition& rtype, dim_eval_func_t evaluate_access_vector);
	static void throw_condition_type_err();
	static void throw_struct_type_err(const std::string& type_name_space, const std::string& type_name, const TypeDefinition& type, dim_eval_func_t evaluate_access_vector);
	static void throw_struct_member_err(const std::string& type_name_space, const std::string& type_name, const std::string& variable);

	static std::string buid_signature(const std::string& identifier, const std::vector<TypeDefinition*> signature, dim_eval_func_t evaluate_access_vector);
	static std::string buid_type_str(const TypeDefinition& type, dim_eval_func_t evaluate_access_vector);
	static std::string buid_struct_type_name(const std::string& type_name_space, const std::string& type_name);
};

#endif // !EXCEPTION_HANDLER_HPP
