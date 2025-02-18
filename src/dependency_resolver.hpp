#ifndef LINKER_HPP
#define LINKER_HPP

#include "ast.hpp"
#include "flx_utils.hpp"

namespace visitor {

	class DependencyResolver : Visitor {
	public:
		std::vector<std::string> lib_names;

	private:
		std::vector<std::string> libs;

	public:
		DependencyResolver(std::shared_ptr<ASTProgramNode> main_program, const std::map<std::string, std::shared_ptr<ASTProgramNode>>& programs);

		void start();

	private:
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

		void set_curr_pos(unsigned int row, unsigned int col) override;
		std::string msg_header() override;
	};

}

#endif // !LINKER_HPP
