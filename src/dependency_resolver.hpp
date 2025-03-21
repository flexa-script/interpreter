#ifndef DEPENDENCY_RESOLVER_HPP
#define DEPENDENCY_RESOLVER_HPP

#include <string>
#include <vector>
#include <memory>

#include "ast.hpp"
#include "flx_utils.hpp"

namespace core {

	namespace analysis {

		class DependencyResolver : public Visitor {
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
			void visit(std::shared_ptr<ASTIncludeNamespaceNode>) override;
			void visit(std::shared_ptr<ASTExcludeNamespaceNode>) override;
			void visit(std::shared_ptr<ASTDeclarationNode>) override;
			void visit(std::shared_ptr<ASTUnpackedDeclarationNode>) override;
			void visit(std::shared_ptr<ASTAssignmentNode>) override;
			void visit(std::shared_ptr<ASTFunctionExpressionAssignmentNode>) override;
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
			void visit(std::shared_ptr<ASTTypeNode>) override;
			void visit(std::shared_ptr<ASTNullNode>) override;
			void visit(std::shared_ptr<ASTThisNode>) override;
			void visit(std::shared_ptr<ASTTypeOfNode>) override;
			void visit(std::shared_ptr<ASTTypeIdNode>) override;
			void visit(std::shared_ptr<ASTRefIdNode>) override;
			void visit(std::shared_ptr<ASTIsStructNode>) override;
			void visit(std::shared_ptr<ASTIsArrayNode>) override;
			void visit(std::shared_ptr<ASTIsAnyNode>) override;
			void visit(std::shared_ptr<ASTValueNode>) override;
			void visit(std::shared_ptr<ASTBuiltinCallNode>) override;

			intmax_t hash(std::shared_ptr<ASTExprNode>) override;
			intmax_t hash(std::shared_ptr<ASTValueNode>) override;
			intmax_t hash(std::shared_ptr<ASTIdentifierNode>) override;
			intmax_t hash(std::shared_ptr<ASTLiteralNode<flx_bool>>) override;
			intmax_t hash(std::shared_ptr<ASTLiteralNode<flx_int>>) override;
			intmax_t hash(std::shared_ptr<ASTLiteralNode<flx_float>>) override;
			intmax_t hash(std::shared_ptr<ASTLiteralNode<flx_char>>) override;
			intmax_t hash(std::shared_ptr<ASTLiteralNode<flx_string>>) override;

			void set_curr_pos(size_t row, size_t col) override;
			std::string msg_header() override;
		};

	}

}

#endif // !DEPENDENCY_RESOLVER_HPP
