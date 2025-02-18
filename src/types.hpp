#ifndef TYPES_HPP
#define TYPES_HPP

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <functional>

#include "gcobject.hpp"

using namespace gc;

namespace parser {
	enum class Type {
		T_UNDEFINED, T_VOID, T_BOOL, T_INT, T_FLOAT, T_CHAR, T_STRING, T_ARRAY, T_STRUCT, T_ANY, T_FUNCTION
	};

	std::string type_str(Type);

	bool match_type(Type, Type);
	bool is_undefined(Type);
	bool is_void(Type);
	bool is_bool(Type);
	bool is_int(Type);
	bool is_float(Type);
	bool is_char(Type);
	bool is_string(Type);
	bool is_any(Type);
	bool is_array(Type);
	bool is_struct(Type);
	bool is_function(Type);
	bool is_text(Type);
	bool is_numeric(Type);
	bool is_collection(Type);
	bool is_iterable(Type);

	class ASTExprNode;
	class ASTBlockNode;
}

using namespace parser;

namespace visitor {
	typedef std::function<std::vector<unsigned int>(const std::vector<std::shared_ptr<ASTExprNode>>&)> dim_eval_func_t;
};

using namespace visitor;

class RuntimeValue;

typedef bool flx_bool;
typedef int64_t flx_int;
typedef long double flx_float;
typedef char flx_char;
typedef std::string flx_string;
typedef std::vector<RuntimeValue*> flx_array;
typedef std::unordered_map<std::string, RuntimeValue*> flx_struct;
typedef std::pair<std::string, std::string> flx_function;

extern std::string language_namespace;

class SemanticVariable;
class RuntimeVariable;

class CodePosition {
public:
	unsigned int row;
	unsigned int col;

	CodePosition(unsigned int row = 0, unsigned int col = 0) : row(row), col(col) {};
};

class TypeDefinition {
public:
	Type type;
	std::string type_name;
	std::string type_name_space;
	Type array_type;
	std::vector<std::shared_ptr<ASTExprNode>> dim;
	bool use_ref;

	TypeDefinition(Type type, Type array_type,
		const std::vector<std::shared_ptr<ASTExprNode>>& dim,
		const std::string& type_name, const std::string& type_name_space);

	TypeDefinition(Type type);

	TypeDefinition();

	static TypeDefinition get_basic(Type type);
	static TypeDefinition get_array(Type array_type,
		const std::vector<std::shared_ptr<ASTExprNode>>& dim = std::vector<std::shared_ptr<ASTExprNode>>());
	static TypeDefinition get_struct(const std::string& type_name,
		const std::string& type_name_space);

	static bool is_any_or_match_type(TypeDefinition ltype, TypeDefinition rtype,
		dim_eval_func_t evaluate_access_vector, bool strict = false, bool strict_array = false);
	static bool match_type(TypeDefinition ltype, TypeDefinition rtype, dim_eval_func_t evaluate_access_vector, bool strict = false, bool strict_array = false);
	static bool match_type_bool(TypeDefinition ltype, TypeDefinition rtype);
	static bool match_type_int(TypeDefinition ltype, TypeDefinition rtype);
	static bool match_type_float(TypeDefinition ltype, TypeDefinition rtype, bool strict = false);
	static bool match_type_char(TypeDefinition ltype, TypeDefinition rtype);
	static bool match_type_string(TypeDefinition ltype, TypeDefinition rtype, bool strict = false);
	static bool match_type_array(TypeDefinition ltype, TypeDefinition rtype, dim_eval_func_t evaluate_access_vector, bool strict = false, bool strict_array = false);
	static bool match_type_struct(TypeDefinition ltype, TypeDefinition rtype);
	static bool match_type_function(TypeDefinition ltype, TypeDefinition rtype);
	static bool match_array_dim(TypeDefinition ltype, TypeDefinition rtype, dim_eval_func_t evaluate_access_vector);

	virtual void reset_ref();
};

class VariableDefinition : public TypeDefinition, public CodePosition {
public:
	std::string identifier;
	std::shared_ptr<ASTExprNode> default_value;
	bool is_rest;

	VariableDefinition();

	VariableDefinition(const std::string& identifier, Type type,
		const std::string& type_name, const std::string& type_name_space,
		Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
		std::shared_ptr<ASTExprNode> default_value, bool is_rest, unsigned int row, unsigned int col);

	VariableDefinition(const std::string& identifier, Type type,
		std::shared_ptr<ASTExprNode> default_value = nullptr, bool is_rest = false, unsigned int row = 0, unsigned int col = 0);

	VariableDefinition(const std::string& identifier,
		Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
		const std::string& type_name = "", const std::string& type_name_space = "",
		std::shared_ptr<ASTExprNode> default_value = nullptr, bool is_rest = false, unsigned int row = 0, unsigned int col = 0);

	VariableDefinition(const std::string& identifier, const std::string& type_name, const std::string& type_name_space,
		std::shared_ptr<ASTExprNode> default_value = nullptr, bool is_rest = false, unsigned int row = 0, unsigned int col = 0);
};

class UnpackedVariableDefinition : public TypeDefinition {
public:
	std::vector<VariableDefinition> variables;
	std::shared_ptr<ASTExprNode> assign_value;

	UnpackedVariableDefinition(Type type, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim, const std::string& type_name,
		const std::string& type_name_space, const std::vector<VariableDefinition>& variables);

	UnpackedVariableDefinition(TypeDefinition type_definition, const std::vector<VariableDefinition>& variables);
};

class FunctionDefinition : public TypeDefinition, public CodePosition {
public:
	std::string identifier;
	std::vector<TypeDefinition*> parameters;
	size_t pointer = 0;
	std::shared_ptr<ASTBlockNode> block;
	bool is_var = false;

	FunctionDefinition(const std::string& identifier, Type type, const std::string& type_name,
		const std::string& type_name_space, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
		const std::vector<TypeDefinition*>& parameters,
		std::shared_ptr<ASTBlockNode> block, unsigned int row, unsigned int col);

	FunctionDefinition(const std::string& identifier, Type type,
		const std::vector<TypeDefinition*>& parameters, std::shared_ptr<ASTBlockNode> block);

	FunctionDefinition(const std::string& identifier, Type type, const std::string& type_name,
		const std::string& type_name_space, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim);

	FunctionDefinition(const std::string& identifier, unsigned int row, unsigned int col);

	FunctionDefinition();

	void check_signature() const;
};

class StructureDefinition : public CodePosition {
public:
	std::string identifier;
	std::map<std::string, VariableDefinition> variables;

	StructureDefinition(const std::string& identifier, const std::map<std::string, VariableDefinition>& variables,
		unsigned int row, unsigned int col);

	StructureDefinition(const std::string& identifier);

	StructureDefinition();
};

class Variable : public TypeDefinition {
public:
	std::string identifier;

	Variable(const std::string& identifier, Type type, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
		const std::string& type_name, const std::string& type_name_space);

	Variable(TypeDefinition value);

	Variable();

	virtual ~Variable() = default;
};

class Value : public TypeDefinition {
public:
	std::weak_ptr<Variable> ref;

	Value(Type type, Type array_type, std::vector<std::shared_ptr<ASTExprNode>> dim,
		const std::string& type_name, const std::string& type_name_space);
	Value(TypeDefinition type);
	Value();
	virtual ~Value() = default;
};

class SemanticValue : public Value, public CodePosition {
public:
	long long hash;
	bool is_const;
	bool is_sub;

	// complete constructor
	SemanticValue(Type type, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
		const std::string& type_name, const std::string& type_name_space, long long hash,
		bool is_const, unsigned int row, unsigned int col);

	SemanticValue(Type type, long long hash, bool is_const, unsigned int row, unsigned int col);

	SemanticValue(TypeDefinition type_definition, long long hash,
		bool is_const, unsigned int row, unsigned int col);

	SemanticValue(VariableDefinition variable_definition, long long hash,
		bool is_const, unsigned int row, unsigned int col);

	// simplified constructor
	SemanticValue(Type type, unsigned int row, unsigned int col);

	SemanticValue();

	void copy_from(const SemanticValue& value);
};

class SemanticVariable : public Variable, public CodePosition, public std::enable_shared_from_this<SemanticVariable> {
public:
	std::shared_ptr<SemanticValue> value;
	bool is_const;

	SemanticVariable(const std::string& identifier, Type type, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
		const std::string& type_name, const std::string& type_name_space, bool is_const, unsigned int row, unsigned int col);

	SemanticVariable(const std::string& identifier, Type type, bool is_const, unsigned int row, unsigned int col);

	SemanticVariable();

	void set_value(std::shared_ptr<SemanticValue> value);
	std::shared_ptr<SemanticValue> get_value();

	Type def_type(Type type);
	Type def_array_type(Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim);

	void reset_ref() override;
};

class RuntimeValue : public Value, public GCObject {
private:
	flx_bool* b = nullptr;
	flx_int* i = nullptr;
	flx_float* f = nullptr;
	flx_char* c = nullptr;
	flx_string* s = nullptr;
	flx_array* arr = nullptr;
	flx_struct* str = nullptr;
	flx_function* fun = nullptr;

public:
	RuntimeValue* value_ref = nullptr;

	RuntimeValue(Type type, Type array_type, std::vector<std::shared_ptr<ASTExprNode>> dim,
		const std::string& type_name, const std::string& type_name_space,
		unsigned int row, unsigned int col);
	RuntimeValue(flx_bool);
	RuntimeValue(flx_int);
	RuntimeValue(flx_float);
	RuntimeValue(flx_char);
	RuntimeValue(flx_string);
	RuntimeValue(flx_array);
	RuntimeValue(flx_array, Type array_type, std::vector<std::shared_ptr<ASTExprNode>> dim, std::string type_name = "", std::string type_name_space = "");
	RuntimeValue(flx_struct, std::string type_name, std::string type_name_space);
	RuntimeValue(flx_function);
	RuntimeValue(Type type);
	RuntimeValue(Type array_type, std::vector<std::shared_ptr<ASTExprNode>> dim, std::string type_name = "", std::string type_name_space = "");
	RuntimeValue(std::string type_name, std::string type_name_space);
	RuntimeValue(RuntimeValue*);
	RuntimeValue(TypeDefinition type);
	RuntimeValue();
	~RuntimeValue();

	void set(flx_bool);
	void set(flx_int);
	void set(flx_float);
	void set(flx_char);
	void set(flx_string);
	void set(flx_array);
	void set(flx_array, Type array_type, std::vector<std::shared_ptr<ASTExprNode>> dim, std::string type_name = "", std::string type_name_space = "");
	void set(flx_struct, std::string type_name, std::string type_name_space);
	void set(flx_function);
	void set_sub(std::string identifier, RuntimeValue* sub_value);
	void set_sub(size_t index, RuntimeValue* sub_value);

	flx_bool get_b() const;
	flx_int get_i() const;
	flx_float get_f() const;
	flx_char get_c() const;
	flx_string get_s() const;
	flx_array get_arr() const;
	flx_struct get_str() const;
	flx_function get_fun() const;
	RuntimeValue* get_sub(std::string identifier);
	RuntimeValue* get_sub(size_t index);

	flx_bool* get_raw_b();
	flx_int* get_raw_i();
	flx_float* get_raw_f();
	flx_char* get_raw_c();
	flx_string* get_raw_s();
	flx_array* get_raw_arr();
	flx_struct* get_raw_str();
	flx_function* get_raw_fun();

	void set_null();
	void set_undefined();

	void set_type(Type type);
	void set_arr_type(Type arr_type);

	bool has_value();

	long double value_hash() const;

	void copy_array(flx_array arr);
	void copy_from(RuntimeValue* value);

	virtual std::vector<GCObject*> get_references() override;

private:
	void unset();
};

class RuntimeVariable : public Variable, public GCObject, public std::enable_shared_from_this<RuntimeVariable> {
public:
	RuntimeValue* value;

	RuntimeVariable(const std::string& identifier, Type type, Type array_type, std::vector<std::shared_ptr<ASTExprNode>> dim,
		const std::string& type_name, const std::string& type_name_space);
	RuntimeVariable(const std::string& identifier, TypeDefinition value);
	RuntimeVariable();
	~RuntimeVariable();

	void set_value(RuntimeValue* value);
	RuntimeValue* get_value();

	Type def_type(Type type);
	Type def_array_type(Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim);

	void reset_ref() override;

	virtual std::vector<GCObject*> get_references() override;
};

class RuntimeOperations {
public:
	static flx_bool equals_value(const RuntimeValue* lval, const RuntimeValue* rval, std::vector<uintptr_t> compared = std::vector<uintptr_t>());
	static flx_bool equals_struct(const flx_struct& lstr, const flx_struct& rstr, std::vector<uintptr_t> compared);
	static flx_bool equals_array(const flx_array& larr, const flx_array& rarr, std::vector<uintptr_t> compared);

	static std::string parse_value_to_string(const RuntimeValue* value, std::vector<uintptr_t> printed = std::vector<uintptr_t>());
	static std::string parse_array_to_string(const flx_array& arr_value, std::vector<uintptr_t> printed);
	static std::string parse_struct_to_string(const RuntimeValue* value, std::vector<uintptr_t> printed);

	static RuntimeValue* do_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval, dim_eval_func_t evaluate_access_vector_ptr, bool is_expr = false, flx_int str_pos = -1);
	static flx_bool do_relational_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval, dim_eval_func_t evaluate_access_vector_ptr);
	static flx_int do_spaceship_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval);
	static flx_int do_operation(flx_int lval, flx_int rval, const std::string& op);
	static flx_float do_operation(flx_float lval, flx_float rval, const std::string& op);
	static flx_string do_operation(flx_string lval, flx_string rval, const std::string& op);
	static flx_array do_operation(flx_array lval, flx_array rval, const std::string& op);

	static void normalize_type(TypeDefinition* owner, RuntimeValue* value);
	
	static std::string build_str_type(RuntimeValue* curr_value, dim_eval_func_t evaluate_access_vector_ptr);
};

#endif // !TYPES_HPP
