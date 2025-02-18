#include "exception_handler.hpp"

void ExceptionHandler::throw_operation_err(const std::string op, const TypeDefinition& ltype, const TypeDefinition& rtype, dim_eval_func_t evaluate_access_vector) {
	throw std::runtime_error("invalid '" + op + "' operation for types '" + buid_type_str(ltype, evaluate_access_vector)
		+ "' and '" + buid_type_str(rtype, evaluate_access_vector) + "'");
}

void ExceptionHandler::throw_unary_operation_err(const std::string op, const TypeDefinition& type, dim_eval_func_t evaluate_access_vector) {
	throw std::runtime_error("incompatible unary operator '" + op +
		"' in front of " + buid_type_str(type, evaluate_access_vector) + " expression");
}

void ExceptionHandler::throw_declaration_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype, dim_eval_func_t evaluate_access_vector) {
	throw std::runtime_error("found " + buid_type_str(rtype, evaluate_access_vector)
		+ " in definition of '" + identifier
		+ "', expected " + buid_type_str(ltype, evaluate_access_vector) + " type");
}

void ExceptionHandler::throw_return_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype, dim_eval_func_t evaluate_access_vector) {
	throw std::runtime_error("invalid " + buid_type_str(ltype, evaluate_access_vector)
		+ " return type for '" + identifier
		+ "' function with " + buid_type_str(rtype, evaluate_access_vector)
		+ " return type");
}

void ExceptionHandler::throw_mismatched_type_err(const TypeDefinition& ltype, const TypeDefinition& rtype, dim_eval_func_t evaluate_access_vector) {
	throw std::runtime_error("mismatched types " + buid_type_str(ltype, evaluate_access_vector)
		+ " and " + buid_type_str(rtype, evaluate_access_vector));
}

void ExceptionHandler::throw_condition_type_err() {
	throw std::runtime_error("conditions must be boolean expression");
}

void ExceptionHandler::throw_struct_type_err(const std::string& type_name_space, const std::string& type_name, const TypeDefinition& type, dim_eval_func_t evaluate_access_vector) {
	throw std::runtime_error("invalid type " + buid_type_str(type, evaluate_access_vector) +
		" trying to assign '" + (type_name_space.empty() ? "" : type_name_space + "::") + type_name + "' struct");
}

void ExceptionHandler::throw_struct_member_err(const std::string& type_name_space, const std::string& type_name, const std::string& variable) {
	throw std::runtime_error("'" + variable + "' is not a member of '" + (type_name_space.empty() ? "" : type_name_space + "::") + type_name + "'");
}

std::string ExceptionHandler::buid_signature(const std::string& identifier, const std::vector<TypeDefinition*> signature, dim_eval_func_t evaluate_access_vector) {
	std::string ss= identifier + "(";
	for (const auto& param : signature) {
		ss += buid_type_str(*param, evaluate_access_vector) + ", ";
	}
	if (signature.size() > 0) {
		ss.pop_back();
		ss.pop_back();
	}
	ss += ")";

	return ss;
}

std::string ExceptionHandler::buid_type_str(const TypeDefinition& type_def, dim_eval_func_t evaluate_access_vector) {
	std::string ss;

	auto type = type_def.type;

	if (is_array(type)) {
		type = type_def.array_type;
	}

	if (is_struct(type)) {
		ss = type_def.type_name;
	}
	else {
		ss = type_str(type);
	}

	if (type_def.dim.size() > 0) {
		auto dim = evaluate_access_vector(type_def.dim);
		for (size_t i = 0; i < dim.size(); ++i) {
			ss += "[" + std::to_string(dim[i]) + "]";
		}
	}

	if (is_struct(type) && !type_def.type_name_space.empty()) {
		ss = type_def.type_name_space + "::" + ss;
	}

	return ss;
}

std::string ExceptionHandler::buid_struct_type_name(const std::string& type_name_space, const std::string& type_name) {
	return (type_name_space == default_namespace ? "" : type_name_space + "::") + type_name;
}
