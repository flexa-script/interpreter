#ifndef TYPES_HPP
#define TYPES_HPP

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <functional>

#include "gcobject.hpp"
#include "utils.hpp"

namespace core {

	class ASTExprNode;
	class ASTBlockNode;

	class SemanticVariable;
	class RuntimeVariable;

	class RuntimeValue;

	enum class Type {
		T_UNDEFINED, T_VOID, T_BOOL, T_INT, T_FLOAT, T_CHAR, T_STRING, T_ARRAY, T_STRUCT, T_ANY, T_FUNCTION
	};

	class TypeUtils {
	public:
		static std::string type_str(Type);

		static bool match_type(Type, Type);
		static bool is_undefined(Type);
		static bool is_void(Type);
		static bool is_bool(Type);
		static bool is_int(Type);
		static bool is_float(Type);
		static bool is_char(Type);
		static bool is_string(Type);
		static bool is_any(Type);
		static bool is_array(Type);
		static bool is_struct(Type);
		static bool is_function(Type);
		static bool is_textual(Type);
		static bool is_numeric(Type);
		static bool is_collection(Type);
		static bool is_iterable(Type);

	};

	typedef std::function<std::vector<size_t>(const std::vector<std::shared_ptr<ASTExprNode>>&)> dim_eval_func_t;

	// boolean standardized type
	typedef bool flx_bool;

	// integer standardized type
	typedef int64_t flx_int;

	// floating point standardized type
	typedef long double flx_float;

	// character standardized type
	typedef char flx_char;

	// string standardized type
	typedef std::string flx_string;

	// array standardized type
	class flx_array {
	private:
		size_t _size;
		std::shared_ptr<RuntimeValue*[]> _data;

	public:
		flx_array();
		flx_array(size_t size);

		size_t size() const;

		RuntimeValue*& operator[](size_t index);
		const RuntimeValue* operator[](size_t index) const;
		
		void resize(size_t new_size) {
			if (new_size == _size) return;

			auto new_data = std::shared_ptr<RuntimeValue * []>(new RuntimeValue * [new_size]);
			size_t min_size = __min(_size, new_size);

			for (size_t i = 0; i < min_size; ++i) {
				new_data[i] = _data[i];
			}

			_size = new_size;
			_data = new_data;
		}

		void append(flx_array& other) {
			size_t old_size = _size;
			resize(_size + other.size());

			for (size_t i = 0; i < other.size(); ++i) {
				_data[old_size + i] = other[i];
			}
		}
	
	};

	// structure standardized type
	typedef std::unordered_map<std::string, RuntimeValue*> flx_struct;

	// function standardized type
	typedef std::pair<std::string, std::string> flx_function;

	class CodePosition {
	public:
		size_t row;
		size_t col;

		CodePosition(size_t row = 0, size_t col = 0) : row(row), col(col) {};
	};

	class TypeDefinition {
	public:
		Type type;
		std::string type_name;
		std::string type_name_space;
		Type array_type;
		std::vector<std::shared_ptr<ASTExprNode>> expr_dim;
		std::vector<size_t> dim;
		bool use_ref;

		TypeDefinition(Type type, Type array_type,
			const std::vector<std::shared_ptr<ASTExprNode>>& expr_dim,
			const std::string& type_name, const std::string& type_name_space);

		TypeDefinition(Type type, Type array_type, const std::vector<size_t>& dim,
			const std::string& type_name, const std::string& type_name_space);

		TypeDefinition(Type type);

		TypeDefinition();

		static TypeDefinition get_basic(Type type);
		static TypeDefinition get_array(Type array_type,
			const std::vector<size_t>& dim = std::vector<size_t>());
		static TypeDefinition get_struct(const std::string& type_name,
			const std::string& type_name_space);

		static bool is_any_or_match_type(TypeDefinition ltype, TypeDefinition rtype, bool strict = false, bool strict_array = false);
		static bool match_type(TypeDefinition ltype, TypeDefinition rtype, bool strict = false, bool strict_array = false);
		static bool match_type_bool(TypeDefinition ltype, TypeDefinition rtype);
		static bool match_type_int(TypeDefinition ltype, TypeDefinition rtype);
		static bool match_type_float(TypeDefinition ltype, TypeDefinition rtype, bool strict = false);
		static bool match_type_char(TypeDefinition ltype, TypeDefinition rtype);
		static bool match_type_string(TypeDefinition ltype, TypeDefinition rtype, bool strict = false);
		static bool match_type_array(TypeDefinition ltype, TypeDefinition rtype, bool strict = false, bool strict_array = false);
		static bool match_type_struct(TypeDefinition ltype, TypeDefinition rtype);
		static bool match_type_function(TypeDefinition ltype, TypeDefinition rtype);
		static bool match_array_dim(TypeDefinition ltype, TypeDefinition rtype);

		static std::string buid_type_str(const TypeDefinition& type);
		static std::string buid_struct_type_name(const std::string& type_name_space, const std::string& type_name);

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
			std::shared_ptr<ASTExprNode> default_value, bool is_rest, size_t row, size_t col);

		VariableDefinition(const std::string& identifier, Type type,
			const std::string& type_name, const std::string& type_name_space,
			Type array_type, const std::vector<size_t>& dim,
			std::shared_ptr<ASTExprNode> default_value, bool is_rest, size_t row, size_t col);

		VariableDefinition(const std::string& identifier, Type type,
			std::shared_ptr<ASTExprNode> default_value = nullptr, bool is_rest = false, size_t row = 0, size_t col = 0);

		VariableDefinition(const std::string& identifier,
			Type array_type, const std::vector<size_t>& dim,
			const std::string& type_name = "", const std::string& type_name_space = "",
			std::shared_ptr<ASTExprNode> default_value = nullptr, bool is_rest = false, size_t row = 0, size_t col = 0);

		VariableDefinition(const std::string& identifier, const std::string& type_name, const std::string& type_name_space,
			std::shared_ptr<ASTExprNode> default_value = nullptr, bool is_rest = false, size_t row = 0, size_t col = 0);
	};

	class UnpackedVariableDefinition : public TypeDefinition {
	public:
		std::vector<VariableDefinition> variables;
		std::shared_ptr<ASTExprNode> assign_value;

		UnpackedVariableDefinition(Type type, Type array_type, const std::vector<size_t>& dim, const std::string& type_name,
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
			const std::string& type_name_space, Type array_type, const std::vector<size_t>& dim,
			const std::vector<TypeDefinition*>& parameters,
			std::shared_ptr<ASTBlockNode> block, size_t row, size_t col);

		FunctionDefinition(const std::string& identifier, Type type,
			const std::vector<TypeDefinition*>& parameters, std::shared_ptr<ASTBlockNode> block);

		FunctionDefinition(const std::string& identifier, Type type, const std::string& type_name,
			const std::string& type_name_space, Type array_type, const std::vector<size_t>& dim);

		FunctionDefinition(const std::string& identifier, size_t row, size_t col);

		FunctionDefinition();

		void check_signature() const;
	};

	class StructureDefinition : public CodePosition {
	public:
		std::string identifier;
		std::map<std::string, VariableDefinition> variables;

		StructureDefinition(const std::string& identifier, const std::map<std::string, VariableDefinition>& variables,
			size_t row, size_t col);

		StructureDefinition(const std::string& identifier);

		StructureDefinition();
	};

	class Variable : public TypeDefinition {
	public:
		std::string identifier;

		Variable(const std::string& identifier, Type type, Type array_type, const std::vector<size_t>& dim,
			const std::string& type_name, const std::string& type_name_space);

		Variable(TypeDefinition value);

		Variable();

		virtual ~Variable() = default;
	};

	class Value : public TypeDefinition {
	public:
		std::weak_ptr<Variable> ref;

		Value(Type type, Type array_type, std::vector<size_t> dim,
			const std::string& type_name, const std::string& type_name_space);
		Value(TypeDefinition type);
		Value();
		virtual ~Value() = default;
	};

	class SemanticValue : public Value, public CodePosition {
	public:
		intmax_t hash;
		bool is_const;
		bool is_sub;

		// complete constructor
		SemanticValue(Type type, Type array_type, const std::vector<size_t>& dim,
			const std::string& type_name, const std::string& type_name_space, intmax_t hash,
			bool is_const, size_t row, size_t col);

		SemanticValue(Type type, intmax_t hash, bool is_const, size_t row, size_t col);

		SemanticValue(TypeDefinition type_definition, intmax_t hash,
			bool is_const, size_t row, size_t col);

		SemanticValue(VariableDefinition variable_definition, intmax_t hash,
			bool is_const, size_t row, size_t col);

		// simplified constructor
		SemanticValue(Type type, size_t row, size_t col);

		SemanticValue();

		void copy_from(const SemanticValue& value);
	};

	class SemanticVariable : public Variable, public CodePosition, public std::enable_shared_from_this<SemanticVariable> {
	public:
		std::shared_ptr<SemanticValue> value;
		bool is_const;

		SemanticVariable(const std::string& identifier, Type type, Type array_type, const std::vector<size_t>& dim,
			const std::string& type_name, const std::string& type_name_space,
			bool is_const, size_t row, size_t col);

		SemanticVariable(const std::string& identifier, Type type, bool is_const, size_t row, size_t col);

		SemanticVariable();

		void set_value(std::shared_ptr<SemanticValue> value);
		std::shared_ptr<SemanticValue> get_value();

		Type def_type(Type type);
		Type def_array_type(Type array_type, const std::vector<size_t>& dim);

		void reset_ref() override;
	};

	class RuntimeValue : public Value, public runtime::GCObject {
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
		size_t access_index = 0;
		flx_string access_identifier = "";

		RuntimeValue(Type type, Type array_type, std::vector<size_t> dim,
			const std::string& type_name, const std::string& type_name_space,
			size_t row, size_t col);
		RuntimeValue(flx_bool);
		RuntimeValue(flx_int);
		RuntimeValue(flx_float);
		RuntimeValue(flx_char);
		RuntimeValue(flx_string);
		RuntimeValue(flx_array);
		RuntimeValue(flx_array, Type array_type, std::vector<size_t> dim, std::string type_name = "", std::string type_name_space = "");
		RuntimeValue(flx_struct, std::string type_name, std::string type_name_space);
		RuntimeValue(flx_function);
		RuntimeValue(Type type);
		RuntimeValue(Type array_type, std::vector<size_t> dim, std::string type_name = "", std::string type_name_space = "");
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
		void set(flx_array, Type array_type, std::vector<size_t> dim, std::string type_name = "", std::string type_name_space = "");
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

		bool has_value();

		void set_type(Type type);
		void set_arr_type(Type arr_type);

		void copy_from(RuntimeValue* value);

		virtual std::vector<GCObject*> get_references() override;

	private:
		void unset();
	};

	class RuntimeVariable : public Variable, public runtime::GCObject, public std::enable_shared_from_this<RuntimeVariable> {
	public:
		RuntimeValue* value;

		RuntimeVariable(const std::string& identifier, Type type, Type array_type, std::vector<size_t> dim,
			const std::string& type_name, const std::string& type_name_space);
		RuntimeVariable(const std::string& identifier, TypeDefinition value);
		RuntimeVariable();
		~RuntimeVariable();

		void set_value(RuntimeValue* value);
		RuntimeValue* get_value();

		Type def_type(Type type);
		Type def_array_type(Type array_type, const std::vector<size_t>& dim);

		void reset_ref() override;

		virtual std::vector<runtime::GCObject*> get_references() override;
	};

	class RuntimeOperations {
	public:
		static flx_bool equals_value(const RuntimeValue* lval, const RuntimeValue* rval, std::vector<uintptr_t> compared = std::vector<uintptr_t>());
		static flx_bool equals_struct(const flx_struct& lstr, const flx_struct& rstr, std::vector<uintptr_t> compared);
		static flx_bool equals_array(const flx_array& larr, const flx_array& rarr, std::vector<uintptr_t> compared);

		static std::string parse_value_to_string(const RuntimeValue* value, std::vector<uintptr_t> printed = std::vector<uintptr_t>());
		static std::string parse_array_to_string(const flx_array& arr_value, std::vector<uintptr_t> printed);
		static std::string parse_struct_to_string(const RuntimeValue* value, std::vector<uintptr_t> printed);

		static RuntimeValue* do_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval, bool is_expr = false, flx_int str_pos = -1);
		static flx_bool do_relational_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval);
		static flx_int do_spaceship_operation(const std::string& op, RuntimeValue* lval, RuntimeValue* rval);
		static flx_int do_operation(flx_int lval, flx_int rval, const std::string& op);
		static flx_float do_operation(flx_float lval, flx_float rval, const std::string& op);
		static flx_string do_operation(flx_string lval, flx_string rval, const std::string& op);
		static flx_array do_operation(flx_array lval, flx_array rval, const std::string& op);

		static void normalize_type(TypeDefinition* owner, RuntimeValue* value);

	};

}

#endif // !TYPES_HPP
