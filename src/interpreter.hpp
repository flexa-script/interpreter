#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <memory>
#include <map>
#include <stack>
#include <functional>

#include "types.hpp"
#include "visitor.hpp"
#include "ast.hpp"
#include "scope.hpp"
#include "meta_visitor.hpp"
#include "gc.hpp"

using namespace visitor;
using namespace parser;
using namespace gc;

namespace visitor {
	class Interpreter : public Visitor, public MetaVisitor {
	public:
		std::map<std::string, std::function<void()>> builtin_functions;
		RuntimeValue* current_expression_value;
		GarbageCollector gc;

		RuntimeValue* alocate_value(RuntimeValue* value);

	private:
		dim_eval_func_t evaluate_access_vector_ptr = std::bind(&Interpreter::evaluate_access_vector, this, std::placeholders::_1);
		std::string function_call_name;
		std::string return_from_function_name;
		std::stack<std::vector<TypeDefinition*>> current_function_signature;
		std::stack<std::vector<Identifier>> current_function_call_identifier_vector;
		std::stack<FunctionDefinition> current_function;
		std::stack<std::vector<TypeDefinition*>> current_function_defined_parameters;
		std::stack<std::vector<RuntimeValue*>> current_function_calling_arguments;
		std::stack<std::string> current_this_name;
		size_t is_switch = 0;
		size_t is_loop = 0;
		bool continue_block = false;
		bool break_block = false;
		bool return_from_function = false;
		bool exit_from_program = false;
		bool executed_elif = false;
		bool has_string_access = false;
		bool exception = false;

		std::vector<std::shared_ptr<ASTExprNode>> current_expression_array_dim;
		int current_expression_array_dim_max = 0;
		TypeDefinition current_expression_array_type;
		bool is_max = false;

		size_t print_level = 0;
		std::vector<uintptr_t> printed;

	private:
		void declare_function_parameter(std::shared_ptr<Scope> scope, const std::string& identifier, RuntimeValue* value);

		std::vector<unsigned int> evaluate_access_vector(const std::vector<std::shared_ptr<ASTExprNode>>& expr_access_vector);
		std::vector<unsigned int> calculate_array_dim_size(const flx_array& arr);

		void check_build_array(RuntimeValue* new_value, std::vector<std::shared_ptr<ASTExprNode>> dim);
		flx_array build_array(const std::vector<std::shared_ptr<ASTExprNode>>& dim, RuntimeValue* init_value, long long i);
		flx_array build_undefined_array(const std::vector<std::shared_ptr<ASTExprNode>>& dim, long long i);

		RuntimeValue* set_value(std::shared_ptr<RuntimeVariable> var, const std::vector<Identifier>& identifier_vector, RuntimeValue* new_value);
		RuntimeValue* access_value(RuntimeValue* value, const std::vector<Identifier>& identifier_vector, size_t i = 0);

		long long hash(RuntimeValue* value);

		void declare_function_block_parameters(const std::string& name_space);
		void build_args(const std::vector<std::string>& args);

		void set_curr_pos(unsigned int row, unsigned int col) override;
		std::string msg_header() override;

	public:
		Interpreter(std::shared_ptr<Scope> global_scope, std::shared_ptr<ASTProgramNode> main_program,
			const std::map<std::string, std::shared_ptr<ASTProgramNode>>& programs, const std::vector<std::string>& args);
		Interpreter() = default;
		~Interpreter() = default;

		void start();

		void visit(std::shared_ptr<ASTProgramNode>) override;
		void visit(std::shared_ptr<ASTUsingNode>) override;
		void visit(std::shared_ptr<ASTNamespaceManagerNode>) override;
		void visit(std::shared_ptr<ASTDeclarationNode>) override;
		void visit(std::shared_ptr<ASTUnpackedDeclarationNode>) override;
		void visit(std::shared_ptr<ASTAssignmentNode>) override;
		void visit(std::shared_ptr<ASTReturnNode>) override;
		void visit(std::shared_ptr<ASTExitNode>) override;
		void visit(std::shared_ptr<ASTBlockNode>) override;
		void visit(std::shared_ptr<ASTContinueNode>) override;
		void visit(std::shared_ptr<ASTBreakNode>) override;
		void visit(std::shared_ptr<ASTEnumNode>) override;
		void visit(std::shared_ptr<ASTTryCatchNode>) override;
		void visit(std::shared_ptr<ASTThrowNode>) override;
		void visit(std::shared_ptr<ASTEllipsisNode>) override;
		void visit(std::shared_ptr<ASTSwitchNode>) override;
		void visit(std::shared_ptr<ASTElseIfNode>) override;
		void visit(std::shared_ptr<ASTIfNode>) override;
		void visit(std::shared_ptr<ASTForNode>) override;
		void visit(std::shared_ptr<ASTForEachNode>) override;
		void visit(std::shared_ptr<ASTWhileNode>) override;
		void visit(std::shared_ptr<ASTDoWhileNode>) override;
		void visit(std::shared_ptr<ASTFunctionDefinitionNode>) override;
		void visit(std::shared_ptr<ASTStructDefinitionNode>) override;
		void visit(std::shared_ptr<ASTLiteralNode<flx_bool>>) override;
		void visit(std::shared_ptr<ASTLiteralNode<flx_int>>) override;
		void visit(std::shared_ptr<ASTLiteralNode<flx_float>>) override;
		void visit(std::shared_ptr<ASTLiteralNode<flx_char>>) override;
		void visit(std::shared_ptr<ASTLiteralNode<flx_string>>) override;
		void visit(std::shared_ptr<ASTLambdaFunction>) override;
		void visit(std::shared_ptr<ASTArrayConstructorNode>) override;
		void visit(std::shared_ptr<ASTStructConstructorNode>) override;
		void visit(std::shared_ptr<ASTBinaryExprNode>) override;
		void visit(std::shared_ptr<ASTUnaryExprNode>) override;
		void visit(std::shared_ptr<ASTIdentifierNode>) override;
		void visit(std::shared_ptr<ASTTernaryNode>) override;
		void visit(std::shared_ptr<ASTInNode>) override;
		void visit(std::shared_ptr<ASTFunctionCallNode>) override;
		void visit(std::shared_ptr<ASTTypeCastNode>) override;
		void visit(std::shared_ptr<ASTNullNode>) override;
		void visit(std::shared_ptr<ASTThisNode>) override;
		void visit(std::shared_ptr<ASTTypingNode>) override;
		void visit(std::shared_ptr<ASTValueNode>) override;
		void visit(std::shared_ptr<ASTBuiltinCallNode>) override;

		long long hash(std::shared_ptr<ASTExprNode>) override;
		long long hash(std::shared_ptr<ASTValueNode>) override;
		long long hash(std::shared_ptr<ASTIdentifierNode>) override;
		long long hash(std::shared_ptr<ASTLiteralNode<flx_bool>>) override;
		long long hash(std::shared_ptr<ASTLiteralNode<flx_int>>) override;
		long long hash(std::shared_ptr<ASTLiteralNode<flx_float>>) override;
		long long hash(std::shared_ptr<ASTLiteralNode<flx_char>>) override;
		long long hash(std::shared_ptr<ASTLiteralNode<flx_string>>) override;
	};
}

#endif // !INTERPRETER_HPP
