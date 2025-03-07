#ifndef EXCEPTION_HANDLER_HPP
#define EXCEPTION_HANDLER_HPP

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "visitor.hpp"
#include "ast.hpp"

namespace core {

	class ExceptionHandler {
	public:
		static void throw_operation_err(const std::string op, const TypeDefinition& ltype, const TypeDefinition& rtype);
		static void throw_unary_operation_err(const std::string op, const TypeDefinition& type);
		static void throw_declaration_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype);
		static void throw_return_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype);
		static void throw_mismatched_type_err(const TypeDefinition& ltype, const TypeDefinition& rtype);
		static void throw_condition_type_err();
		static void throw_struct_type_err(const std::string& type_name_space, const std::string& type_name, const TypeDefinition& type);
		static void throw_struct_member_err(const std::string& type_name_space, const std::string& type_name, const std::string& variable);
		static void throw_struct_value_assign_type_err(const std::string& type_name_space, const std::string& type_name,
			const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype);
		static void undeclared_function(const std::string& identifier, const std::vector<TypeDefinition*> signature);

		static std::string buid_signature(const std::vector<Identifier>& identifier_vector, const std::vector<TypeDefinition*> signature);
		static std::string buid_member_name(const std::vector<Identifier>& identifier_vector);
		static std::string buid_signature(const std::string& identifier, const std::vector<TypeDefinition*> signature);

	};

}

#endif // !EXCEPTION_HANDLER_HPP
