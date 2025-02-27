#include "exception_handler.hpp"

#include "constants.hpp"

using namespace core;

void ExceptionHandler::throw_operation_err(const std::string op, const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("invalid '" + op + "' operation for types '" + buid_type_str(ltype)
		+ "' and '" + buid_type_str(rtype) + "'");
}

void ExceptionHandler::throw_unary_operation_err(const std::string op, const TypeDefinition& type) {
	throw std::runtime_error("incompatible unary operator '" + op +
		"' in front of " + buid_type_str(type) + " expression");
}

void ExceptionHandler::throw_declaration_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("found " + buid_type_str(rtype)
		+ " in definition of '" + identifier
		+ "', expected " + buid_type_str(ltype) + " type");
}

void ExceptionHandler::throw_return_type_err(const std::string& identifier, const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("invalid " + buid_type_str(ltype)
		+ " return type for '" + identifier
		+ "' function with " + buid_type_str(rtype)
		+ " return type");
}

void ExceptionHandler::throw_mismatched_type_err(const TypeDefinition& ltype, const TypeDefinition& rtype) {
	throw std::runtime_error("mismatched types " + buid_type_str(ltype)
		+ " and " + buid_type_str(rtype));
}

void ExceptionHandler::throw_condition_type_err() {
	throw std::runtime_error("conditions must be boolean expression");
}

void ExceptionHandler::throw_struct_type_err(const std::string& type_name_space, const std::string& type_name, const TypeDefinition& type) {
	throw std::runtime_error("invalid type " + buid_type_str(type) +
		" trying to assign '" + (type_name_space.empty() ? "" : type_name_space + "::") + type_name + "' struct");
}

void ExceptionHandler::throw_struct_member_err(const std::string& type_name_space, const std::string& type_name, const std::string& variable) {
	throw std::runtime_error("'" + variable + "' is not a member of '" + (type_name_space.empty() ? "" : type_name_space + "::") + type_name + "'");
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
		ss += buid_type_str(*param) + ", ";
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
		ss += buid_type_str(*param) + ", ";
	}
	if (signature.size() > 0) {
		ss.pop_back();
		ss.pop_back();
	}
	ss += ")";

	return ss;
}

std::string ExceptionHandler::buid_type_str(const TypeDefinition& type_def) {
	std::string ss;

	auto type = type_def.type;

	if (TypeUtils::is_array(type)) {
		type = type_def.array_type;
	}

	if (TypeUtils::is_struct(type)) {
		ss = type_def.type_name;
	}
	else {
		ss = TypeUtils::type_str(type);
	}

	if (type_def.dim.size() > 0) {
		auto& dim = type_def.dim;
		for (size_t i = 0; i < dim.size(); ++i) {
			ss += "[";
			if (dim[i] > 0) {
				ss += std::to_string(dim[i]);
			}
			ss += "]";
		}
	}

	if (TypeUtils::is_struct(type) && !type_def.type_name_space.empty()) {
		ss = type_def.type_name_space + "::" + ss;
	}

	return ss;
}

std::string ExceptionHandler::buid_struct_type_name(const std::string& type_name_space, const std::string& type_name) {
	return (type_name_space == Constants::DEFAULT_NAMESPACE ? "" : type_name_space + "::") + type_name;
}
