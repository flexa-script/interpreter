#include "exception_handler.hpp"

#include "constants.hpp"

using namespace core;

void ExceptionHandler::throw_operation_err(const std::string op, const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("invalid '" + op + "' operation for types '" + TypeDefinition::buid_type_str(ltype)
		+ "' and '" + TypeDefinition::buid_type_str(rtype) + "'");
}

void ExceptionHandler::throw_unary_operation_err(const std::string op, const TypeDefinition& type) {
	throw std::runtime_error("incompatible unary operator '" + op +
		"' in front of " + TypeDefinition::buid_type_str(type) + " expression");
}

void ExceptionHandler::throw_declaration_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("found " + TypeDefinition::buid_type_str(rtype)
		+ " in definition of '" + identifier
		+ "', expected " + TypeDefinition::buid_type_str(ltype) + " type");
}

void ExceptionHandler::throw_return_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("invalid " + TypeDefinition::buid_type_str(ltype)
		+ " return type for '" + identifier
		+ "' function with " + TypeDefinition::buid_type_str(rtype)
		+ " return type");
}

void ExceptionHandler::throw_mismatched_type_err(const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("mismatched types " + TypeDefinition::buid_type_str(ltype)
		+ " and " + TypeDefinition::buid_type_str(rtype));
}

void ExceptionHandler::throw_condition_type_err() {
	throw std::runtime_error("conditions must be boolean expression");
}

void ExceptionHandler::throw_struct_type_err(const std::string& type_name_space, const std::string& type_name, const TypeDefinition& type) {
	throw std::runtime_error("invalid type " + TypeDefinition::buid_type_str(type) +
		" trying to assign '" + TypeDefinition::buid_struct_type_name(type_name_space, type_name) + "' struct");
}

void ExceptionHandler::throw_struct_value_assign_type_err(const std::string& type_name_space, const std::string& type_name,
	const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("invalid type " + TypeDefinition::buid_type_str(rtype) +
		" trying to assign '" + identifier + "' member of '" + TypeDefinition::buid_struct_type_name(type_name_space, type_name) + "' struct, "
		"expected " + TypeDefinition::buid_type_str(rtype));
}

void ExceptionHandler::throw_struct_member_err(const std::string& type_name_space, const std::string& type_name, const std::string& variable) {
	throw std::runtime_error("'" + variable + "' is not a member of '" + TypeDefinition::buid_struct_type_name(type_name_space, type_name) + "'");
}

void ExceptionHandler::undeclared_function(const std::string& identifier, const std::vector<TypeDefinition*> signature) {
	std::string func_name = ExceptionHandler::buid_signature(identifier, signature);
	throw std::runtime_error("function '" + func_name + "' was never declared");
}

std::string ExceptionHandler::buid_member_name(const std::vector<Identifier>& identifier_vector) {
	std::string ss;
	
	for (auto& id : identifier_vector) {
		ss += id.identifier;
		if (id.access_vector.size() > 0) {
			for (size_t i = 0; i < id.access_vector.size(); ++i) {
				ss += "[]";
			}
		}
		ss += ".";
	}

	if (ss.ends_with(".")) {
		ss.erase(ss.end());
	}

	return ss;
}

std::string ExceptionHandler::buid_signature(const std::vector<Identifier>& identifier_vector, const std::vector<TypeDefinition*> signature) {
	std::string ss = buid_member_name(identifier_vector) + "(";
	for (const auto& param : signature) {
		ss += TypeDefinition::buid_type_str(*param) + ", ";
	}
	if (signature.size() > 0) {
		ss.pop_back();
		ss.pop_back();
	}
	ss += ")";

	return ss;
}

std::string ExceptionHandler::buid_signature(const std::string& identifier, const std::vector<TypeDefinition*> signature) {
	std::string ss= identifier + "(";
	for (const auto& param : signature) {
		ss += TypeDefinition::buid_type_str(*param) + ", ";
	}
	if (signature.size() > 0) {
		ss.pop_back();
		ss.pop_back();
	}
	ss += ")";

	return ss;
}
