#ifndef SEMANTIC_ANALYSIS_HPP
#define SEMANTIC_ANALYSIS_HPP

#include <memory>
#include <map>
#include <vector>
#include <stack>
#include <xutility>
#include <functional>

#include "ast.hpp"
#include "scope.hpp"
#include "meta_visitor.hpp"

using namespace visitor;
using namespace parser;

namespace visitor {

	class SemanticAnalyser : public Visitor, public MetaVisitor {
	public:
		std::map<std::string, std::shared_ptr<ASTExprNode>> builtin_functions;

	private:
		dim_eval_func_t evaluate_access_vector_ptr = std::bind(&SemanticAnalyser::evaluate_access_vector, this, std::placeholders::_1);
		std::vector<std::string> nmspaces;
		SemanticValue current_expression;
		std::stack<FunctionDefinition> current_function;
		bool exception = false;
		bool is_switch = false;
		bool is_loop = false;

		std::vector<std::shared_ptr<ASTExprNode>> current_expression_array_dim;
		int current_expression_array_dim_max;
		TypeDefinition current_expression_array_type;
		bool is_max;

	private:
		bool returns(std::shared_ptr<ASTNode> astnode);

		void declare_function_parameter(std::shared_ptr<Scope> scope, const VariableDefinition& param);

		void equals_value(const SemanticValue& lval, const SemanticValue& rval);

		std::vector<unsigned int> evaluate_access_vector(const std::vector<std::shared_ptr<ASTExprNode>>& expr_access_vector);

		TypeDefinition do_operation(const std::string& op, TypeDefinition lvtype, TypeDefinition ltype, TypeDefinition* rvtype, TypeDefinition rtype, bool is_expr = true);
		std::shared_ptr<SemanticValue> access_value(std::shared_ptr<SemanticValue> value, const std::vector<Identifier>& identifier_vector, size_t i = 0);
		void build_args(const std::vector<std::string>& args);

		void check_is_struct_exists(Type type, const std::string& name_space, const std::string& identifier);

		bool namespace_exists(const std::string& name_space);

		void set_curr_pos(unsigned int row, unsigned int col) override;
		std::string msg_header() override;

	public:
		SemanticAnalyser(std::shared_ptr<Scope> global_scope, std::shared_ptr<ASTProgramNode> main_program,
			std::map<std::string, std::shared_ptr<ASTProgramNode>> programs, const std::vector<std::string>& args);
		~SemanticAnalyser() = default;

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
		void visit(std::shared_ptr<ASTSwitchNode>) override;
		void visit(std::shared_ptr<ASTEnumNode>) override;
		void visit(std::shared_ptr<ASTTryCatchNode>) override;
		void visit(std::shared_ptr<ASTThrowNode>) override;
		void visit(std::shared_ptr<ASTEllipsisNode>) override;
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

#endif // !SEMANTIC_ANALYSIS_HPP
