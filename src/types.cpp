#include "types.hpp"

#include <cmath>

#include "utils.hpp"
#include "exception_handler.hpp"
#include "module.hpp";
#include "md_datetime.hpp"
#include "md_graphics.hpp"
#include "md_files.hpp"
#include "md_console.hpp"
#include "visitor.hpp"
#include "token.hpp"
#include "constants.hpp"

using namespace core;

std::string TypeUtils::type_str(Type t) {
	switch (t) {
	case Type::T_UNDEFINED:
		return "undefined";
	case Type::T_VOID:
		return "void";
	case Type::T_BOOL:
		return "bool";
	case Type::T_INT:
		return "int";
	case Type::T_FLOAT:
		return "float";
	case Type::T_CHAR:
		return "char";
	case Type::T_STRING:
		return "string";
	case Type::T_ANY:
		return "any";
	case Type::T_ARRAY:
		return "array";
	case Type::T_STRUCT:
		return "struct";
	case Type::T_FUNCTION:
		return "function";
	default:
		throw std::runtime_error("invalid type encountered");
	}
}

bool TypeUtils::match_type(Type type1, Type type2) {
	return type1 == type2;
}

bool TypeUtils::is_undefined(Type type) {
	return TypeUtils::match_type(type, Type::T_UNDEFINED);
}

bool TypeUtils::is_void(Type type) {
	return TypeUtils::match_type(type, Type::T_VOID);
}

bool TypeUtils::is_bool(Type type) {
	return TypeUtils::match_type(type, Type::T_BOOL);
}

bool TypeUtils::is_int(Type type) {
	return TypeUtils::match_type(type, Type::T_INT);
}

bool TypeUtils::is_float(Type type) {
	return TypeUtils::match_type(type, Type::T_FLOAT);
}

bool TypeUtils::is_char(Type type) {
	return TypeUtils::match_type(type, Type::T_CHAR);
}

bool TypeUtils::is_string(Type type) {
	return TypeUtils::match_type(type, Type::T_STRING);
}

bool TypeUtils::is_any(Type type) {
	return TypeUtils::match_type(type, Type::T_ANY);
}

bool TypeUtils::is_array(Type type) {
	return TypeUtils::match_type(type, Type::T_ARRAY);
}

bool TypeUtils::is_struct(Type type) {
	return TypeUtils::match_type(type, Type::T_STRUCT);
}

bool TypeUtils::is_function(Type type) {
	return TypeUtils::match_type(type, Type::T_FUNCTION);
}

bool TypeUtils::is_textual(Type type) {
	return TypeUtils::is_string(type) || TypeUtils::is_char(type);
}

bool TypeUtils::is_numeric(Type type) {
	return TypeUtils::is_int(type) || TypeUtils::is_float(type);
}

bool TypeUtils::is_collection(Type type) {
	return TypeUtils::is_string(type) || TypeUtils::is_array(type);
}

bool TypeUtils::is_iterable(Type type) {
	return TypeUtils::is_collection(type) || TypeUtils::is_struct(type);
}

flx_array::flx_array()
	: _size(0), _data(nullptr) {
}

flx_array::flx_array(size_t size)
	: _size(size), _data(std::make_unique<RuntimeValue* []>(size)){
}

size_t flx_array::size() const {
	return _size;
}

RuntimeValue*& flx_array::operator[](size_t index) {
	return _data[index];
}

const RuntimeValue* flx_array::operator[](size_t index) const {
	return _data[index];
}

TypeDefinition::TypeDefinition(Type type, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& expr_dim,
	const std::string& type_name, const std::string& type_name_space)
	: type(type), array_type(array_type), expr_dim(expr_dim), type_name(type_name), type_name_space(type_name_space) {
	reset_ref();
}

TypeDefinition::TypeDefinition(Type type, Type array_type, const std::vector<size_t>& dim,
	const std::string& type_name, const std::string& type_name_space)
	: type(type), array_type(array_type), dim(dim), type_name(type_name), type_name_space(type_name_space) {
	reset_ref();
}

TypeDefinition::TypeDefinition(Type type)
	: type(type), array_type(Type::T_UNDEFINED) {
	reset_ref();
}

TypeDefinition::TypeDefinition()
	: type(Type::T_UNDEFINED), array_type(Type::T_UNDEFINED) {
	reset_ref();
}

TypeDefinition TypeDefinition::get_basic(Type type) {
	return TypeDefinition(type, Type::T_UNDEFINED, std::vector<size_t>(), "", "");
}

TypeDefinition TypeDefinition::get_array(Type array_type, const std::vector<size_t>& dim) {
	return TypeDefinition(Type::T_ARRAY, array_type, dim, "", "");
}

TypeDefinition TypeDefinition::get_struct(const std::string& type_name, const std::string& type_name_space) {
	return TypeDefinition(Type::T_STRUCT, Type::T_UNDEFINED, std::vector<size_t>(), type_name, type_name_space);
}

bool TypeDefinition::is_any_or_match_type(TypeDefinition ltype, TypeDefinition rtype, bool strict, bool strict_array) {
	if (TypeUtils::is_any(ltype.type)
		|| TypeUtils::is_any(rtype.type)
		|| TypeUtils::is_void(ltype.type)
		|| TypeUtils::is_void(rtype.type)) return true;
	return match_type(ltype, rtype, strict, strict_array);
}

bool TypeDefinition::match_type(TypeDefinition ltype, TypeDefinition rtype, bool strict, bool strict_array) {
	if (match_type_bool(ltype, rtype)) return true;
	if (match_type_int(ltype, rtype)) return true;
	if (match_type_float(ltype, rtype, strict)) return true;
	if (match_type_char(ltype, rtype)) return true;
	if (match_type_string(ltype, rtype, strict)) return true;
	if (match_type_array(ltype, rtype, strict, strict_array)) return true;
	if (match_type_struct(ltype, rtype)) return true;
	if (match_type_function(ltype, rtype)) return true;
	return false;
}

bool TypeDefinition::match_type_bool(TypeDefinition ltype, TypeDefinition rtype) {
	return TypeUtils::is_bool(ltype.type) && TypeUtils::is_bool(rtype.type);
}

bool TypeDefinition::match_type_int(TypeDefinition ltype, TypeDefinition rtype) {
	return TypeUtils::is_int(ltype.type) && TypeUtils::is_int(rtype.type);
}

bool TypeDefinition::match_type_float(TypeDefinition ltype, TypeDefinition rtype, bool strict) {
	return TypeUtils::is_float(ltype.type)
		&& (strict && TypeUtils::is_float(rtype.type) ||
			!strict && TypeUtils::is_numeric(rtype.type));
}

bool TypeDefinition::match_type_char(TypeDefinition ltype, TypeDefinition rtype) {
	return TypeUtils::is_char(ltype.type) && TypeUtils::is_char(rtype.type);
}

bool TypeDefinition::match_type_string(TypeDefinition ltype, TypeDefinition rtype, bool strict) {
	return TypeUtils::is_string(ltype.type)
		&& (strict && TypeUtils::is_string(rtype.type) ||
			!strict && TypeUtils::is_textual(rtype.type));
}

bool TypeDefinition::match_type_array(TypeDefinition ltype, TypeDefinition rtype, bool strict, bool strict_array) {
	if (TypeUtils::is_any(ltype.type)
		|| TypeUtils::is_any(rtype.type)
		|| TypeUtils::is_void(ltype.type)
		|| TypeUtils::is_void(rtype.type)) return true;

	TypeDefinition latype = TypeDefinition(TypeUtils::is_undefined(ltype.array_type) ? Type::T_ANY : ltype.array_type,
		Type::T_UNDEFINED, std::vector<size_t>(), ltype.type_name, ltype.type_name_space);
	TypeDefinition ratype = TypeDefinition(TypeUtils::is_undefined(rtype.array_type) ? Type::T_ANY : rtype.array_type,
		Type::T_UNDEFINED, std::vector<size_t>(), rtype.type_name, rtype.type_name_space);

	return TypeUtils::is_array(ltype.type) && TypeUtils::is_array(rtype.type)
		&& (!strict_array && is_any_or_match_type(latype, ratype, strict, strict_array) ||
			match_type(latype, ratype, strict, strict_array))
		&& match_array_dim(ltype, rtype);
}

bool TypeDefinition::match_type_struct(TypeDefinition ltype, TypeDefinition rtype) {
	return TypeUtils::is_struct(ltype.type) && TypeUtils::is_struct(rtype.type)
		&& ltype.type_name == rtype.type_name;
}

bool TypeDefinition::match_type_function(TypeDefinition ltype, TypeDefinition rtype) {
	return TypeUtils::is_function(ltype.type) && TypeUtils::is_function(rtype.type);
}

bool TypeDefinition::match_array_dim(TypeDefinition ltype, TypeDefinition rtype) {
	std::vector<size_t> var_dim = ltype.dim;
	std::vector<size_t> expr_dim = rtype.dim;

	if ((var_dim.size() == 1 && var_dim[0] >= 0 && var_dim[0] <= 1) || (expr_dim.size() == 1 && expr_dim[0] >= 0 && expr_dim[0] <= 1)
		|| var_dim.size() == 0 || expr_dim.size() == 0) {
		return true;
	}

	if (var_dim.size() != expr_dim.size()) {
		return false;
	}

	for (size_t dc = 0; dc < var_dim.size(); ++dc) {
		if (ltype.dim.at(dc) && var_dim.at(dc) != expr_dim.at(dc)) {
			return false;
		}
	}

	return true;
}

std::string TypeDefinition::buid_type_str(const TypeDefinition& type_def) {
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

	if (TypeUtils::is_struct(type)) {
		ss = TypeDefinition::buid_struct_type_name(type_def.type_name_space, ss);
	}

	return ss;
}

std::string TypeDefinition::buid_struct_type_name(const std::string& type_name_space, const std::string& type_name) {
	return (type_name_space == Constants::DEFAULT_NAMESPACE || type_name_space.empty() ? "" : type_name_space + "::") + type_name;
}

void TypeDefinition::reset_ref() {
	use_ref = TypeUtils::is_struct(type);
}

VariableDefinition::VariableDefinition()
	: TypeDefinition(Type::T_UNDEFINED, Type::T_UNDEFINED, std::vector<size_t>(), "", ""), CodePosition(),
	identifier(""), default_value(nullptr), is_rest(false) {
}

VariableDefinition::VariableDefinition(const std::string& identifier, Type type, const std::string& type_name,
	const std::string& type_name_space, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
	std::shared_ptr<ASTExprNode> default_value, bool is_rest, size_t row, size_t col)
	: TypeDefinition(type, array_type, dim, type_name, type_name_space), CodePosition(row, col),
	identifier(identifier), default_value(default_value), is_rest(is_rest) {
}

VariableDefinition::VariableDefinition(const std::string& identifier, Type type, const std::string& type_name,
	const std::string& type_name_space, Type array_type, const std::vector<size_t>& dim,
	std::shared_ptr<ASTExprNode> default_value, bool is_rest, size_t row, size_t col)
	: TypeDefinition(type, array_type, dim, type_name, type_name_space), CodePosition(row, col),
	identifier(identifier), default_value(default_value), is_rest(is_rest) {
}

VariableDefinition::VariableDefinition(const std::string& identifier, Type type,
	std::shared_ptr<ASTExprNode> default_value, bool is_rest, size_t row, size_t col)
	: TypeDefinition(type, Type::T_UNDEFINED, std::vector<size_t>(), "", ""), CodePosition(row, col),
	identifier(identifier), default_value(default_value), is_rest(is_rest) {
}

VariableDefinition::VariableDefinition(const std::string& identifier, Type array_type, const std::vector<size_t>& dim,
	const std::string& type_name, const std::string& type_name_space,
	std::shared_ptr<ASTExprNode> default_value, bool is_rest, size_t row, size_t col)
	: TypeDefinition(Type::T_ARRAY, array_type, dim, "", ""), CodePosition(row, col),
	identifier(identifier), default_value(default_value), is_rest(is_rest) {
}

VariableDefinition::VariableDefinition(const std::string& identifier,
	const std::string& type_name, const std::string& type_name_space,
	std::shared_ptr<ASTExprNode> default_value, bool is_rest, size_t row, size_t col)
	: TypeDefinition(Type::T_STRUCT, Type::T_UNDEFINED, std::vector<size_t>(), type_name, type_name_space), CodePosition(row, col),
	identifier(identifier), default_value(default_value), is_rest(is_rest) {
}

UnpackedVariableDefinition::UnpackedVariableDefinition(Type type, Type array_type, const std::vector<size_t>& dim, const std::string& type_name,
	const std::string& type_name_space, const std::vector<VariableDefinition>& variables)
	: TypeDefinition(type, array_type, dim, type_name, type_name_space), variables(variables) {
}

UnpackedVariableDefinition::UnpackedVariableDefinition(TypeDefinition type_definition, const std::vector<VariableDefinition>& variables)
	: TypeDefinition(type_definition), variables(variables) {
}

FunctionDefinition::FunctionDefinition(const std::string& identifier, Type type, const std::string& type_name,
	const std::string& type_name_space, Type array_type, const std::vector<size_t>& dim,
	const std::vector<TypeDefinition*>& parameters, std::shared_ptr<ASTBlockNode> block, size_t row, size_t col)
	: CodePosition(row, col), TypeDefinition(type, array_type, dim, type_name, type_name_space),
	identifier(identifier), parameters(parameters), block(block) {
	check_signature();
}

FunctionDefinition::FunctionDefinition(const std::string& identifier, Type type,
	const std::vector<TypeDefinition*>& parameters, std::shared_ptr<ASTBlockNode> block)
	: CodePosition(), TypeDefinition(type),
	identifier(identifier), parameters(parameters), block(block) {
	check_signature();
}

FunctionDefinition::FunctionDefinition(const std::string& identifier, Type type, const std::string& type_name,
	const std::string& type_name_space, Type array_type, const std::vector<size_t>& dim)
	: CodePosition(), TypeDefinition(type, array_type, dim, type_name, type_name_space),
	identifier(identifier) {
}

FunctionDefinition::FunctionDefinition(const std::string& identifier, size_t row, size_t col)
	: CodePosition(row, col), TypeDefinition(Type::T_ANY, Type::T_UNDEFINED, std::vector<size_t>(), "", ""),
	identifier(identifier), parameters(std::vector<TypeDefinition*>()), block(nullptr), is_var(true) {
}

FunctionDefinition::FunctionDefinition()
	: TypeDefinition(), CodePosition(),
	identifier(""), parameters(std::vector<TypeDefinition*>()), block(nullptr) {
}

void FunctionDefinition::check_signature() const {
	bool has_default = false;
	for (size_t i = 0; i < parameters.size(); ++i) {
		if (auto parameter = dynamic_cast<VariableDefinition*>(parameters[i])) {
			if (parameter->is_rest && parameters.size() - 1 != i) {
				throw std::runtime_error("rest '" + identifier + "' parameter must be the last parameter");
			}
			if (parameter->default_value) {
				has_default = true;
			}
			if (!parameter->default_value && has_default) {
				throw std::runtime_error("default values as '" + identifier + "' must be at end");
			}
		}
	}
}

StructureDefinition::StructureDefinition(const std::string& identifier, const std::map<std::string, VariableDefinition>& variables,
	size_t row, size_t col)
	: CodePosition(row, col), identifier(identifier), variables(variables) {
}

StructureDefinition::StructureDefinition(const std::string& identifier)
	: CodePosition(), identifier(identifier) {
}

StructureDefinition::StructureDefinition()
	: CodePosition(row, col), identifier(""), variables(std::map<std::string, VariableDefinition>()) {
}

Variable::Variable(const std::string& identifier, Type type, Type array_type, const std::vector<size_t>& dim,
	const std::string& type_name, const std::string& type_name_space)
	: TypeDefinition(type, array_type, dim, type_name, type_name_space),
	identifier(identifier) {
}

Variable::Variable(TypeDefinition value)
	: TypeDefinition(value) {
}

Variable::Variable()
	: TypeDefinition(Type::T_UNDEFINED, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
}

Value::Value(Type type, Type array_type, std::vector<size_t> dim,
	const std::string& type_name, const std::string& type_name_space)
	: TypeDefinition(type, array_type, dim, type_name, type_name_space) {
}

Value::Value(TypeDefinition type)
	: TypeDefinition(type) {
}

Value::Value()
	: TypeDefinition() {
}

SemanticValue::SemanticValue(Type type, Type array_type, const std::vector<size_t>& dim,
	const std::string& type_name, const std::string& type_name_space, intmax_t hash,
	bool is_const, size_t row, size_t col)
	: CodePosition(row, col), Value(type, array_type, dim, type_name, type_name_space),
	hash(hash), is_const(is_const), is_sub(false) {
}

SemanticValue::SemanticValue(Type type, intmax_t hash,
	bool is_const, size_t row, size_t col)
	: CodePosition(row, col), Value(type),
	hash(hash), is_const(is_const), is_sub(false) {
}

SemanticValue::SemanticValue(TypeDefinition type_definition, intmax_t hash,
	bool is_const, size_t row, size_t col)
	: CodePosition(row, col),
	Value(type_definition.type, type_definition.array_type,
		type_definition.dim, type_definition.type_name, type_definition.type_name_space),
	hash(hash), is_const(is_const), is_sub(false) {
}

SemanticValue::SemanticValue(VariableDefinition variable_definition, intmax_t hash,
	bool is_const, size_t row, size_t col)
	: CodePosition(row, col),
	Value(variable_definition.type, variable_definition.array_type,
		variable_definition.dim, variable_definition.type_name, variable_definition.type_name_space),
	hash(hash), is_const(is_const), is_sub(false) {
}

SemanticValue::SemanticValue(Type type, size_t row, size_t col)
	: CodePosition(row, col), Value(type),
	hash(0), is_const(false), is_sub(false) {
}

SemanticValue::SemanticValue()
	: CodePosition(), Value(),
	hash(0), is_const(false), is_sub(false) {
}

void SemanticValue::copy_from(const SemanticValue& value) {
	type = value.type;
	array_type = value.array_type;
	dim = value.dim;
	type_name = value.type_name;
	type_name_space = value.type_name_space;
	hash = value.hash;
	is_const = value.is_const;
	is_sub = value.is_sub;
	row = value.row;
	col = value.col;
}

SemanticVariable::SemanticVariable(const std::string& identifier, Type type, Type array_type, const std::vector<size_t>& dim,
	const std::string& type_name, const std::string& type_name_space, bool is_const, size_t row, size_t col)
	: CodePosition(row, col), Variable(identifier, def_type(type), def_array_type(array_type, dim), dim, type_name, type_name_space),
	value(nullptr), is_const(is_const) {
}

SemanticVariable::SemanticVariable(const std::string& identifier, Type type, bool is_const, size_t row, size_t col)
	: CodePosition(row, col), Variable(identifier, def_type(type), Type::T_UNDEFINED, std::vector<size_t>(), "", ""),
	value(nullptr), is_const(is_const) {
}

SemanticVariable::SemanticVariable()
	: CodePosition(0, 0), Variable("", Type::T_UNDEFINED, Type::T_UNDEFINED, std::vector<size_t>(), "", ""),
	value(nullptr), is_const(false) {
}

void SemanticVariable::set_value(std::shared_ptr<SemanticValue> value) {
	this->value = value;
	this->value->ref = shared_from_this();
}

std::shared_ptr<SemanticValue> SemanticVariable::get_value() {
	value->ref = shared_from_this();
	return value;
}

Type SemanticVariable::def_type(Type type) {
	return TypeUtils::is_void(type) || TypeUtils::is_undefined(type) ? Type::T_ANY : type;
}

Type SemanticVariable::def_array_type(Type array_type, const std::vector<size_t>& dim) {
	return TypeUtils::is_void(array_type) || TypeUtils::is_undefined(array_type) && dim.size() > 0 ? Type::T_ANY : array_type;
}

void SemanticVariable::reset_ref() {
	TypeDefinition::reset_ref();
	use_ref = value && (use_ref || TypeUtils::is_struct(value->type));
}


RuntimeValue::RuntimeValue(Type type, Type array_type, std::vector<size_t> dim,
	const std::string& type_name, const std::string& type_name_space,
	size_t row, size_t col)
	: Value(type, array_type, dim, type_name, type_name_space) {
}

RuntimeValue::RuntimeValue()
	: Value(Type::T_UNDEFINED, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
}

RuntimeValue::RuntimeValue(flx_bool rawv)
	: Value(Type::T_BOOL, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
	set(rawv);
}

RuntimeValue::RuntimeValue(flx_int rawv)
	: Value(Type::T_INT, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
	set(rawv);
}

RuntimeValue::RuntimeValue(flx_float rawv)
	: Value(Type::T_FLOAT, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
	set(rawv);
}

RuntimeValue::RuntimeValue(flx_char rawv)
	: Value(Type::T_CHAR, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
	set(rawv);
}

RuntimeValue::RuntimeValue(flx_string rawv)
	: Value(Type::T_STRING, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
	set(rawv);
}

RuntimeValue::RuntimeValue(flx_array rawv)
	: Value(Type::T_ARRAY, Type::T_ANY, std::vector<size_t>(), "", "") {
	set(rawv);
}

RuntimeValue::RuntimeValue(flx_array rawv, Type array_type, std::vector<size_t> dim, std::string type_name, std::string type_name_space)
	: Value(Type::T_ARRAY, array_type, dim, type_name, type_name_space) {
	set(rawv, array_type, dim, type_name, type_name_space);
}

RuntimeValue::RuntimeValue(flx_struct rawv, std::string type_name, std::string type_name_space)
	: Value(Type::T_STRUCT, Type::T_UNDEFINED, std::vector<size_t>(), type_name, type_name_space) {
	set(rawv, type_name, type_name_space);
}

RuntimeValue::RuntimeValue(flx_function rawv)
	: Value(Type::T_FUNCTION, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
	set(rawv);
}

RuntimeValue::RuntimeValue(Type type)
	: Value(type, Type::T_UNDEFINED, std::vector<size_t>(), "", "") {
}

RuntimeValue::RuntimeValue(Type array_type, std::vector<size_t> dim, std::string type_name, std::string type_name_space)
	: Value(Type::T_ARRAY, array_type, dim, type_name, type_name_space) {
}

RuntimeValue::RuntimeValue(std::string type_name, std::string type_name_space)
	: Value(Type::T_STRUCT, Type::T_UNDEFINED, std::vector<size_t>(), type_name, type_name_space) {
}

RuntimeValue::RuntimeValue(RuntimeValue* v) {
	copy_from(v);
}

RuntimeValue::RuntimeValue(TypeDefinition v)
	: Value(v) {
}

RuntimeValue::~RuntimeValue() {
	if (str) {
		for (auto& var : *str) {
			if (var.first == modules::Module::INSTANCE_ID_NAME) {
				delete reinterpret_cast<void*>(*var.second->i);
			}
		}
	}
	unset();
};

void RuntimeValue::set(flx_bool b) {
	unset();
	this->b = new flx_bool(b);
	type = Type::T_BOOL;
	array_type = Type::T_UNDEFINED;
}

void RuntimeValue::set(flx_int i) {
	unset();
	this->i = new flx_int(i);
	type = Type::T_INT;
	array_type = Type::T_UNDEFINED;
}

void RuntimeValue::set(flx_float f) {
	unset();
	this->f = new flx_float(f);
	type = Type::T_FLOAT;
	array_type = Type::T_UNDEFINED;
}

void RuntimeValue::set(flx_char c) {
	unset();
	this->c = new flx_char(c);
	type = Type::T_CHAR;
	array_type = Type::T_UNDEFINED;
}

void RuntimeValue::set(flx_string s) {
	unset();
	this->s = new flx_string(s);
	type = Type::T_STRING;
	array_type = Type::T_UNDEFINED;
}

void RuntimeValue::set(flx_array arr) {
	unset();
	this->arr = new flx_array(arr);
	type = Type::T_ARRAY;
}

void RuntimeValue::set(flx_array arr, Type array_type, std::vector<size_t> dim, std::string type_name, std::string type_name_space) {
	unset();
	this->arr = new flx_array(arr);
	type = Type::T_ARRAY;
	this->dim = dim;
	this->array_type = array_type;
	this->type_name = type_name;
	this->type_name_space = type_name_space;
}

void RuntimeValue::set(flx_struct str, std::string type_name, std::string type_name_space) {
	unset();
	this->str = new flx_struct(str);
	type = Type::T_STRUCT;
	array_type = Type::T_UNDEFINED;
	this->type_name = type_name;
	this->type_name_space = type_name_space;
}

void RuntimeValue::set(flx_function fun) {
	unset();
	this->fun = new flx_function(fun);
	type = Type::T_FUNCTION;
	array_type = Type::T_UNDEFINED;
}

void RuntimeValue::set_sub(std::string identifier, RuntimeValue* sub_value) {
	if (!str) return;
	sub_value->value_ref = this;
	sub_value->access_identifier = identifier;
	(*str)[identifier] = sub_value;
}

void RuntimeValue::set_sub(size_t index, RuntimeValue* sub_value) {
	if (!arr) return;
	sub_value->value_ref = this;
	sub_value->access_index = index;
	(*arr)[index] = sub_value;
}

flx_bool RuntimeValue::get_b() const {
	if (!b) return flx_bool();
	return *b;
}

flx_int RuntimeValue::get_i() const {
	if (!i) return flx_int();
	return *i;
}

flx_float RuntimeValue::get_f() const {
	if (!f) return flx_float();
	return *f;
}

flx_char RuntimeValue::get_c() const {
	if (!c) return flx_char();
	return *c;
}

flx_string RuntimeValue::get_s() const {
	if (!s) return flx_string();
	return *s;
}

flx_array RuntimeValue::get_arr() const {
	if (!arr) return flx_array();
	return *arr;
}

flx_struct RuntimeValue::get_str() const {
	if (!str) return flx_struct();
	return *str;
}

flx_function RuntimeValue::get_fun() const {
	if (!fun) return flx_function();
	return *fun;
}

RuntimeValue* RuntimeValue::get_sub(std::string identifier) {
	if (!str) return nullptr;
	auto sub_value = (*str)[identifier];
	if (sub_value) {
		sub_value->value_ref = this;
		sub_value->access_identifier = identifier;
	}
	return sub_value;
}

RuntimeValue* RuntimeValue::get_sub(size_t index) {
	if (!arr) return nullptr;
	if (index >= (*arr).size()) {
		throw std::runtime_error("invalid array access position " + std::to_string(index) + " in a array with size " + std::to_string((*arr).size()));
	}
	auto sub_value = (*arr)[index];
	if (sub_value) {
		sub_value->value_ref = this;
		sub_value->access_index = index;
	}
	return sub_value;
}

flx_bool* RuntimeValue::get_raw_b() {
	return b;
}

flx_int* RuntimeValue::get_raw_i() {
	return i;
}

flx_float* RuntimeValue::get_raw_f() {
	return f;
}

flx_char* RuntimeValue::get_raw_c() {
	return c;
}

flx_string* RuntimeValue::get_raw_s() {
	return s;
}

flx_array* RuntimeValue::get_raw_arr() {
	return arr;
}

flx_struct* RuntimeValue::get_raw_str() {
	return str;
}

flx_function* RuntimeValue::get_raw_fun() {
	return fun;
}

void RuntimeValue::set_type(Type type) {
	this->type = type;
}

void RuntimeValue::set_arr_type(Type arr_type) {
	this->array_type = arr_type;
}

void RuntimeValue::unset() {
	access_identifier = "";
	access_index = 0;

	if (this->b) {
		delete this->b;
		this->b = nullptr;
	}
	if (this->i) {
		delete this->i;
		this->i = nullptr;
	}
	if (this->f) {
		delete this->f;
		this->f = nullptr;
	}
	if (this->c) {
		delete this->c;
		this->c = nullptr;
	}
	if (this->s) {
		delete this->s;
		this->s = nullptr;
	}
	if (this->arr) {
		delete this->arr;
		this->arr = nullptr;
	}
	if (this->str) {
		delete this->str;
		this->str = nullptr;
	}
	if (this->fun) {
		delete this->fun;
		this->fun = nullptr;
	}
}

void RuntimeValue::set_null() {
	auto v = RuntimeValue(Type::T_VOID);
	copy_from(&v);
}

bool RuntimeValue::has_value() {
	return type != Type::T_UNDEFINED
		&& type != Type::T_VOID;
}

void RuntimeValue::copy_from(RuntimeValue* value) {
	type = value->type;
	type_name = value->type_name;
	type_name_space = value->type_name_space;
	array_type = value->array_type;
	dim = value->dim;
	unset();
	switch (type)
	{
	case Type::T_BOOL:
		b = new flx_bool(value->get_b());
		break;
	case Type::T_INT:
		i = new flx_int(value->get_i());
		break;
	case Type::T_FLOAT:
		f = new flx_float(value->get_f());
		break;
	case Type::T_CHAR:
		c = new flx_char(value->get_c());
		break;
	case Type::T_STRING:
		s = new flx_string(value->get_s());
		break;
	case Type::T_ARRAY:
		arr = new flx_array(value->get_arr());
		break;
	case Type::T_STRUCT:
		str = new flx_struct(value->get_str());
		break;
	case Type::T_FUNCTION:
		fun = new flx_function(value->get_fun());
		break;
	default:
		break;
	}
	ref = value->ref;
	use_ref = value->use_ref;
}

std::vector<runtime::GCObject*> RuntimeValue::get_references() {
	std::vector<GCObject*> references;

	if (TypeUtils::is_array(type)) {
		auto arr = get_arr();
		for (size_t i = 0; i < arr.size(); ++i) {
			const auto& v = arr[i];
			references.push_back(v);
		}
	}
	if (TypeUtils::is_struct(type)) {
		for (const auto& sub : get_str()) {
			references.push_back(sub.second);
		}
	}

	return references;
}

RuntimeVariable::RuntimeVariable(const std::string& identifier, Type type, Type array_type, std::vector<size_t> dim,
	const std::string& type_name, const std::string& type_name_space)
	: Variable(identifier, def_type(type), def_array_type(array_type, dim),
		std::move(dim), type_name, type_name_space),
	value(nullptr) {
}

RuntimeVariable::RuntimeVariable(const std::string& identifier, TypeDefinition v)
	: Variable(identifier, def_type(v.type), def_array_type(v.array_type, dim),
		v.dim, v.type_name, v.type_name_space),
	value(nullptr) {
}

RuntimeVariable::RuntimeVariable()
	: Variable("", Type::T_UNDEFINED, Type::T_UNDEFINED, std::vector<size_t>(), "", ""),
	value(nullptr) {
}

RuntimeVariable::~RuntimeVariable() = default;

void RuntimeVariable::set_value(RuntimeValue* val) {
	value = val;
	value->ref = shared_from_this();
}

RuntimeValue* RuntimeVariable::get_value() {
	value->ref = shared_from_this();
	return value;
}

Type RuntimeVariable::def_type(Type type) {
	return TypeUtils::is_void(type) || TypeUtils::is_undefined(type) ? Type::T_ANY : type;
}

Type RuntimeVariable::def_array_type(Type array_type, const std::vector<size_t>& dim) {
	return TypeUtils::is_void(array_type) || TypeUtils::is_undefined(array_type) && dim.size() > 0 ? Type::T_ANY : array_type;
}

void RuntimeVariable::reset_ref() {
	TypeDefinition::reset_ref();
	use_ref = value && (use_ref || TypeUtils::is_struct(value->type));
}

std::vector<runtime::GCObject*> RuntimeVariable::get_references() {
	std::vector<GCObject*> references;

	references.push_back(value);

	return references;
}

flx_bool RuntimeOperations::equals_value(const RuntimeValue* lval, const RuntimeValue* rval, std::vector<uintptr_t> compared) {
	if (lval->use_ref) {
		return lval == rval;
	}

	// to prevent cyclic reference, we add complex structures to compared after the first compare and not compare again
	switch (lval->type) {
	case Type::T_ARRAY:
	case Type::T_STRUCT:
		if (std::find(compared.begin(), compared.end(), reinterpret_cast<uintptr_t>(lval)) != compared.end()
			&& std::find(compared.begin(), compared.end(), reinterpret_cast<uintptr_t>(rval)) != compared.end()) {
			return lval == rval;
		}
		else {
			if (std::find(compared.begin(), compared.end(), reinterpret_cast<uintptr_t>(lval)) == compared.end()) {
				compared.push_back(reinterpret_cast<uintptr_t>(lval));
			}
			if (std::find(compared.begin(), compared.end(), reinterpret_cast<uintptr_t>(rval)) == compared.end()) {
				compared.push_back(reinterpret_cast<uintptr_t>(rval));
			}
		}
	}

	switch (lval->type) {
	case Type::T_VOID:
		return TypeUtils::is_void(rval->type);
	case Type::T_BOOL:
		return lval->get_b() == rval->get_b();
	case Type::T_INT:
		return lval->get_i() == rval->get_i();
	case Type::T_FLOAT:
		return lval->get_f() == rval->get_f();
	case Type::T_CHAR:
		return lval->get_c() == rval->get_c();
	case Type::T_STRING:
		return lval->get_s() == rval->get_s();
	case Type::T_ARRAY:
		return RuntimeOperations::equals_array(lval->get_arr(), rval->get_arr(), compared);
	case Type::T_STRUCT:
		return RuntimeOperations::equals_struct(lval->get_str(), rval->get_str(), compared);
	}

	return false;
}

flx_bool RuntimeOperations::equals_struct(const flx_struct& lstr, const flx_struct& rstr, std::vector<uintptr_t> compared) {
	if (lstr.size() != rstr.size()) {
		return false;
	}

	for (auto& lval : lstr) {
		if (rstr.find(lval.first) == rstr.end()) {
			return false;
		}
		if (!equals_value(lval.second, rstr.at(lval.first), compared)) {
			return false;
		}
	}

	return true;
}

flx_bool RuntimeOperations::equals_array(const flx_array& larr, const flx_array& rarr, std::vector<uintptr_t> compared) {
	if (larr.size() != rarr.size()) {
		return false;
	}

	for (size_t i = 0; i < larr.size(); ++i) {
		if (!equals_value(larr[i], rarr[i], compared)) {
			return false;
		}
	}

	return true;
}

std::string RuntimeOperations::parse_value_to_string(const RuntimeValue* value, std::vector<uintptr_t> printed) {
	if (!value) {
		return "null";
	}

	std::string str = "";
	switch (value->type) {
	case Type::T_VOID:
		str = "null";
		break;
	case Type::T_BOOL:
		str = ((value->get_b()) ? "true" : "false");
		break;
	case Type::T_INT:
		str = std::to_string(value->get_i());
		break;
	case Type::T_FLOAT:
		str = std::to_string(value->get_f());
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);
		if (!str.empty() && str.back() == '.') {
			str += "0";
		}
		break;
	case Type::T_CHAR:
		str = flx_string(std::string{ value->get_c() });
		break;
	case Type::T_STRING:
		str = value->get_s();
		break;
	case Type::T_STRUCT: {
		if (std::find(printed.begin(), printed.end(), reinterpret_cast<uintptr_t>(value)) != printed.end()) {
			std::stringstream s = std::stringstream();
			if (!value->type_name_space.empty()) {
				s << value->type_name_space << "::";
			}
			s << value->type_name << "@" << reinterpret_cast<uintptr_t>(value) << "{...}";
			str = s.str();
		}
		else {
			printed.push_back(reinterpret_cast<uintptr_t>(value));
			str = RuntimeOperations::parse_struct_to_string(value, printed);
		}
		break;
	}
	case Type::T_ARRAY: {
		if (std::find(printed.begin(), printed.end(), reinterpret_cast<uintptr_t>(value)) != printed.end()) {
			str = "array@" + std::to_string(reinterpret_cast<uintptr_t>(value)) + "[...]";
		}
		else {
			printed.push_back(reinterpret_cast<uintptr_t>(value));
			str = RuntimeOperations::parse_array_to_string(value->get_arr(), printed);
		}
		break;
	}
	case Type::T_FUNCTION: {
		str = value->get_fun().first + (value->get_fun().first.empty() ? "" : "::") + value->get_fun().second + "(...)";
		break;
	}
	case Type::T_UNDEFINED:
		throw std::runtime_error("undefined expression");
	default:
		throw std::runtime_error("can't determine value type on parsing");
	}
	return str;
}

std::string RuntimeOperations::parse_array_to_string(const flx_array& arr_value, std::vector<uintptr_t> printed) {
	std::stringstream s = std::stringstream();
	s << "[";
	for (auto i = 0; i < arr_value.size(); ++i) {
		bool isc = arr_value[i] && TypeUtils::is_char(arr_value[i]->type);
		bool iss = arr_value[i] && TypeUtils::is_string(arr_value[i]->type);

		if (isc) s << "'";
		else if (iss) s << '"';

		s << parse_value_to_string(arr_value[i], printed);

		if (isc) s << "'";
		else if (iss) s << '"';

		if (i < arr_value.size() - 1) {
			s << ",";
		}
	}
	s << "]";
	return s.str();
}

std::string RuntimeOperations::parse_struct_to_string(const RuntimeValue* value, std::vector<uintptr_t> printed) {
	auto str_value = value->get_str();
	std::stringstream s = std::stringstream();
	if (!value->type_name_space.empty() && value->type_name_space != Constants::DEFAULT_NAMESPACE) {
		s << value->type_name_space << "::";
	}
	s << value->type_name << "{";
	for (auto const& [key, val] : str_value) {
		s << key << ":";
		s << parse_value_to_string(val, printed);
		s << ",";
	}
	auto rs = s.str();
	if (rs[rs.size() - 1] != '{') {
		s.seekp(-1, std::ios_base::end);
	}
	s << "}";
	return s.str();
}

RuntimeValue* RuntimeOperations::do_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval, bool is_expr, flx_int str_pos) {
	Type l_var_type = lval->ref.lock() ? lval->ref.lock()->type : lval->type;
	Type l_var_array_type = lval->ref.lock() ? lval->ref.lock()->array_type : lval->array_type;
	Type l_type = TypeUtils::is_undefined(lval->type) ? l_var_type : lval->type;
	Type r_type = rval->type;
	bool has_string_access = str_pos >= 0;
	RuntimeValue* res_value = nullptr;

	if (TypeUtils::is_void(r_type) && op == "=") {
		lval->set_null();
		return lval;
	}

	if (TypeUtils::is_void(l_type) && op == "=") {
		lval->copy_from(rval);
		return lval;
	}

	if ((TypeUtils::is_void(l_type) || TypeUtils::is_void(r_type))
		&& Token::is_equality_op(op)) {
		return new RuntimeValue((flx_bool)((op == "==") ?
			TypeUtils::match_type(l_type, r_type)
			: !TypeUtils::match_type(l_type, r_type)));
	}

	if (lval->use_ref
		&& Token::is_equality_op(op)) {
		return new RuntimeValue((flx_bool)((op == "==") ?
			lval == rval
			: lval != rval));
	}

	switch (r_type) {
	case Type::T_BOOL: {
		if (TypeUtils::is_any(l_var_type) && op == "=") {
			lval->set(rval->get_b());
			break;
		}

		if (!TypeUtils::is_bool(l_type)) {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		if (op == "=") {
			lval->set(rval->get_b());
		}
		else if (op == "and") {
			res_value = new RuntimeValue((flx_bool)(lval->get_b() && rval->get_b()));
		}
		else if (op == "or") {
			res_value = new RuntimeValue((flx_bool)(lval->get_b() || rval->get_b()));
		}
		else if (op == "==") {
			res_value = new RuntimeValue((flx_bool)(lval->get_b() == rval->get_b()));
		}
		else if (op == "!=") {
			res_value = new RuntimeValue((flx_bool)(lval->get_b() != rval->get_b()));
		}
		else {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		break;
	}
	case Type::T_INT: {
		if (TypeUtils::is_any(l_var_type) && op == "=") {
			lval->set(rval->get_i());
			break;
		}

		if (is_expr
			&& TypeUtils::is_numeric(l_type)
			&& op == "<=>") {
			res_value = new RuntimeValue((flx_int)(do_spaceship_operation(op, lval, rval)));

			break;
		}

		if (is_expr
			&& TypeUtils::is_numeric(l_type)
			&& Token::is_relational_op(op)) {
			res_value = new RuntimeValue(do_relational_operation(op, lval, rval));

			break;
		}

		if (is_expr
			&& TypeUtils::is_numeric(l_type)
			&& Token::is_equality_op(op)) {
			flx_float l = TypeUtils::is_float(lval->type) ? lval->get_f() : lval->get_i();
			flx_float r = TypeUtils::is_float(rval->type) ? rval->get_f() : rval->get_i();

			res_value = new RuntimeValue((flx_bool)(op == "==" ?
				l == r : l != r));

			break;
		}

		if (TypeUtils::is_float(l_type) && (TypeUtils::is_any(l_var_type) || is_expr)) {
			lval->set(do_operation(lval->get_f(), flx_float(rval->get_i()), op));
		}
		else if (TypeUtils::is_int(l_type) && TypeUtils::is_any(l_var_type)
			&& (op == "/=" || op == "/%=" || op == "/" || op == "/%")) {
			lval->set(do_operation(flx_float(lval->get_i()), flx_float(rval->get_i()), op));
		}
		else if (TypeUtils::is_int(l_type)) {
			lval->set(do_operation(lval->get_i(), rval->get_i(), op));
		}
		else {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		break;
	}
	case Type::T_FLOAT: {
		if (TypeUtils::is_any(l_var_type) && op == "=") {
			lval->set(rval->get_f());
			break;
		}

		if (is_expr
			&& TypeUtils::is_numeric(l_type)
			&& op == "<=>") {
			res_value = new RuntimeValue(do_spaceship_operation(op, lval, rval));

			break;
		}

		if (is_expr
			&& TypeUtils::is_numeric(l_type)
			&& Token::is_relational_op(op)) {
			res_value = new RuntimeValue(do_relational_operation(op, lval, rval));

			break;
		}

		if (is_expr
			&& TypeUtils::is_numeric(l_type)
			&& Token::is_equality_op(op)) {
			flx_float l = TypeUtils::is_float(lval->type) ? lval->get_f() : lval->get_i();
			flx_float r = TypeUtils::is_float(rval->type) ? rval->get_f() : rval->get_i();

			res_value = new RuntimeValue((flx_bool)(op == "==" ?
				l == r : l != r));

			break;
		}

		if (TypeUtils::is_float(l_type)) {
			lval->set(do_operation(lval->get_f(), rval->get_f(), op));
		}
		else if (TypeUtils::is_int(l_type)) {
			lval->set(do_operation(flx_float(lval->get_i()), rval->get_f(), op));
		}
		else {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		break;
	}
	case Type::T_CHAR: {
		if (TypeUtils::is_any(l_var_type) && op == "=" && !has_string_access) {
			lval->set(rval->get_c());
			break;
		}

		if (is_expr
			&& TypeUtils::is_char(l_type)
			&& Token::is_equality_op(op)) {
			res_value = new RuntimeValue((flx_bool)(op == "==" ?
				lval->get_c() == rval->get_c()
				: lval->get_c() != rval->get_c()));

			break;
		}

		if (TypeUtils::is_string(l_type)) {
			if (has_string_access) {
				if (op != "=") {
					ExceptionHandler::throw_operation_err(op, *lval, *rval);
				}
				has_string_access = false;
				lval->get_s()[str_pos] = rval->get_c();
				lval->set(lval->get_s());
			}
			else {
				lval->set(do_operation(lval->get_s(), std::string{ rval->get_c() }, op));
			}
		}
		else if (TypeUtils::is_char(l_type)) {
			if (op != "=") {
				ExceptionHandler::throw_operation_err(op, *lval, *rval);
			}

			lval->set(rval->get_c());
		}
		else if (TypeUtils::is_any(l_var_type)) {
			if (op != "=") {
				ExceptionHandler::throw_operation_err(op, *lval, *rval);
			}

			lval->set(rval->get_c());
		}
		else {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		break;
	}
	case Type::T_STRING: {
		if (TypeUtils::is_any(l_var_type) && op == "=") {
			lval->set(rval->get_s());
			break;
		}

		if (is_expr
			&& TypeUtils::is_string(l_type)
			&& Token::is_equality_op(op)) {

			if (lval->get_s().size() > 30) {
				int x = 0;
			}

			res_value = new RuntimeValue((flx_bool)(op == "==" ?
				lval->get_s() == rval->get_s()
				: lval->get_s() != rval->get_s()));

			break;
		}

		if (TypeUtils::is_string(l_type)) {
			lval->set(do_operation(lval->get_s(), rval->get_s(), op));
		}
		else if (is_expr && TypeUtils::is_char(l_type)) {
			lval->set(do_operation(flx_string{ lval->get_c() }, rval->get_s(), op));
		}
		else {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		break;
	}
	case Type::T_ARRAY: {
		if (TypeUtils::is_any(l_var_type) && op == "=") {
			lval->set(rval->get_arr(), lval->array_type, lval->dim, lval->type_name, lval->type_name_space);
			break;
		}

		if (is_expr
			&& TypeUtils::is_array(l_type)
			&& Token::is_equality_op(op)) {
			res_value = new RuntimeValue((flx_bool)(op == "==" ?
				equals_value(lval, rval)
				: !equals_value(lval, rval)));

			break;
		}

		if (!TypeDefinition::match_type_array(*lval, *rval) && op == "=") {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		bool match_arr_t = lval->array_type == rval->array_type || TypeUtils::is_any(lval->array_type);
		if (!match_arr_t && !TypeUtils::is_any(l_var_array_type) && !is_expr) {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		lval->set(do_operation(lval->get_arr(), rval->get_arr(), op),
			match_arr_t ? lval->array_type : Type::T_ANY, lval->dim,
			lval->type_name, lval->type_name_space);

		break;
	}
	case Type::T_STRUCT: {
		if (TypeUtils::is_any(l_var_type) && op == "=") {
			lval->set(rval->get_str(), rval->type_name, lval->type_name_space);
			break;
		}

		if (is_expr
			&& TypeUtils::is_struct(l_type)
			&& Token::is_equality_op(op)) {
			res_value = new RuntimeValue((flx_bool)(op == "==" ?
				equals_value(lval, rval)
				: !equals_value(lval, rval)));

			break;
		}

		if (!TypeUtils::is_struct(l_type) || op != "=") {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		lval->set(rval->get_str(), rval->type_name, rval->type_name_space);

		break;
	}
	case Type::T_FUNCTION: {
		if (TypeUtils::is_any(l_var_type) && op == "=") {
			lval->set(rval->get_str(), rval->type_name, rval->type_name_space);
			break;
		}

		if (!TypeUtils::is_function(l_type) || op != "=") {
			ExceptionHandler::throw_operation_err(op, *lval, *rval);
		}

		lval->set(rval->get_fun());

		break;
	}
	default:
		throw std::runtime_error("cannot determine type of operation");

	}

	if (!res_value) {
		res_value = lval;
	}

	return res_value;
}

flx_bool RuntimeOperations::do_relational_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval) {
	flx_float l = TypeUtils::is_float(lval->type) ? lval->get_f() : lval->get_i();
	flx_float r = TypeUtils::is_float(rval->type) ? rval->get_f() : rval->get_i();

	if (op == "<") {
		return l < r;
	}
	else if (op == ">") {
		return l > r;
	}
	else if (op == "<=") {
		return l <= r;
	}
	else if (op == ">=") {
		return l >= r;
	}
	ExceptionHandler::throw_operation_err(op, *lval, *rval);
}

flx_int RuntimeOperations::do_spaceship_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval) {
	flx_float l = TypeUtils::is_float(lval->type) ? lval->get_f() : lval->get_i();
	flx_float r = TypeUtils::is_float(rval->type) ? rval->get_f() : rval->get_i();

	auto res = l <=> r;
	if (res == std::strong_ordering::less) {
		return flx_int(-1);
	}
	else if (res == std::strong_ordering::equal) {
		return flx_int(0);
	}
	else if (res == std::strong_ordering::greater) {
		return flx_int(1);
	}
}

flx_int RuntimeOperations::do_operation(flx_int lval, flx_int rval, const std::string& op) {
	if (op == "=") {
		return rval;
	}
	else if (op == "+=" || op == "+") {
		return lval + rval;
	}
	else if (op == "-=" || op == "-") {
		return lval - rval;
	}
	else if (op == "*=" || op == "*") {
		return lval * rval;
	}
	else if (op == "/=" || op == "/") {
		if (rval == 0) {
			throw std::runtime_error("division by zero encountered");
		}
		return lval / rval;
	}
	else if (op == "%=" || op == "%") {
		if (rval == 0) {
			throw std::runtime_error("remainder by zero is undefined");
		}
		return lval % rval;
	}
	else if (op == "/%=" || op == "/%") {
		if (rval == 0) {
			throw std::runtime_error("floor division by zero encountered");
		}
		return flx_int(std::floor(lval / rval));
	}
	else if (op == "**=" || op == "**") {
		return flx_int(std::pow(lval, rval));
	}
	else if (op == ">>=" || op == ">>") {
		return lval >> rval;
	}
	else if (op == "<<=" || op == "<<") {
		return lval << rval;
	}
	else if (op == "|=" || op == "|") {
		return lval | rval;
	}
	else if (op == "&=" || op == "&") {
		return lval & rval;
	}
	else if (op == "^=" || op == "^") {
		return lval ^ rval;
	}
	throw std::runtime_error("invalid '" + op + "' operator for types 'int' and 'int'");
}

flx_float RuntimeOperations::do_operation(flx_float lval, flx_float rval, const std::string& op) {
	if (op == "=") {
		return rval;
	}
	else if (op == "+=" || op == "+") {
		return lval + rval;
	}
	else if (op == "-=" || op == "-") {
		return lval - rval;
	}
	else if (op == "*=" || op == "*") {
		return lval * rval;
	}
	else if (op == "/=" || op == "/") {
		if (int(rval) == 0) {
			throw std::runtime_error("division by zero encountered");
		}
		return lval / rval;
	}
	else if (op == "%=" || op == "%") {
		if (int(rval) == 0) {
			throw std::runtime_error("remainder by zero is undefined");
		}
		return std::fmod(lval, rval);
	}
	else if (op == "/%=" || op == "/%") {
		if (int(rval) == 0) {
			throw std::runtime_error("floor division by zero encountered");
		}
		return std::floor(lval / rval);
	}
	else if (op == "**=" || op == "**") {
		return flx_int(std::pow(lval, rval));
	}
	throw std::runtime_error("invalid '" + op + "' operator");
}

flx_string RuntimeOperations::do_operation(flx_string lval, flx_string rval, const std::string& op) {
	if (op == "=") {
		return rval;
	}
	else if (op == "+=" || op == "+") {
		return lval + rval;
	}
	throw std::runtime_error("invalid '" + op + "' operator for types 'string' and 'string'");
}

flx_array RuntimeOperations::do_operation(flx_array lval, flx_array rval, const std::string& op) {
	if (op == "=") {
		return rval;
	}
	else if (op == "+=" || op == "+") {
		lval.append(rval);

		return lval;
	}

	throw std::runtime_error("invalid '" + op + "' operator for types 'array' and 'array'");
}

void RuntimeOperations::normalize_type(TypeDefinition* owner, RuntimeValue* value) {
	if (TypeUtils::is_string(owner->type) && TypeUtils::is_char(value->type)) {
		value->type = owner->type;
		value->set(flx_string{ value->get_c() });
	}
	else if (TypeUtils::is_float(owner->type) && TypeUtils::is_int(value->type)) {
		value->type = owner->type;
		value->set(flx_float(value->get_i()));
	}
}
