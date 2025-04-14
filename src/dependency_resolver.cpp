#include "dependency_resolver.hpp"

#include <filesystem>

#include "utils.hpp"

using namespace core::analysis;

DependencyResolver::DependencyResolver(std::shared_ptr<ASTProgramNode> main_program, const std::map<std::string, std::shared_ptr<ASTProgramNode>>& programs)
	: Visitor(programs, main_program, main_program->name),
	libs(std::vector<std::string>()), lib_names(std::vector<std::string>()) {};

void DependencyResolver::start() {
	visit(current_program_stack.top());
}

void DependencyResolver::visit(std::shared_ptr<ASTProgramNode> astnode) {
	for (auto& statement : astnode->statements) {
		if (std::dynamic_pointer_cast<ASTUsingNode>(statement)) {
			statement->accept(this);
		}
	}
}

void DependencyResolver::visit(std::shared_ptr<ASTUsingNode> astnode) {
	std::string libname = utils::StringUtils::join(astnode->library, ".");

	if (programs.find(libname) == programs.end()) {
		std::string path = utils::StringUtils::replace(libname, ".", std::string{ std::filesystem::path::preferred_separator }) + ".flx";
		if (std::find(lib_names.begin(), lib_names.end(), path) == lib_names.end()) {
			lib_names.push_back(path);
		}
		return;
	}

	auto program = programs[libname];

	// if wasn't parsed yet
	if (!utils::CollectionUtils::contains(libs, libname)) {
		libs.push_back(libname);
		current_program_stack.push(program);
		start();
		current_program_stack.pop();
	}
}

void DependencyResolver::visit(std::shared_ptr<ASTIncludeNamespaceNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTExcludeNamespaceNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTDeclarationNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTUnpackedDeclarationNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTAssignmentNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTFunctionExpressionAssignmentNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTBuiltinCallNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTFunctionCallNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTFunctionDefinitionNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTBlockNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTContinueNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTBreakNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTReturnNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTExitNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTEnumNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTTryCatchNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTThrowNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTEllipsisNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTSwitchNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTElseIfNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTIfNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTForNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTForEachNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTWhileNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTDoWhileNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTBinaryExprNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTUnaryExprNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTTernaryNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTLiteralNode<flx_bool>>) {}
void DependencyResolver::visit(std::shared_ptr<ASTLiteralNode<flx_int>>) {}
void DependencyResolver::visit(std::shared_ptr<ASTLiteralNode<flx_float>>) {}
void DependencyResolver::visit(std::shared_ptr<ASTLiteralNode<flx_char>>) {}
void DependencyResolver::visit(std::shared_ptr<ASTLiteralNode<flx_string>>) {}
void DependencyResolver::visit(std::shared_ptr<ASTIdentifierNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTInNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTStructDefinitionNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTLambdaFunction>) {}
void DependencyResolver::visit(std::shared_ptr<ASTArrayConstructorNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTStructConstructorNode>) {}

void DependencyResolver::visit(std::shared_ptr<ASTTypeCastNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTTypeNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTTypeOfNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTTypeIdNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTRefIdNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTIsStructNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTIsArrayNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTIsAnyNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTNullNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTThisNode>) {}
void DependencyResolver::visit(std::shared_ptr<ASTValueNode>) {}

intmax_t DependencyResolver::hash(std::shared_ptr<ASTExprNode>) { return 0; }
intmax_t DependencyResolver::hash(std::shared_ptr<ASTValueNode>) { return 0; }
intmax_t DependencyResolver::hash(std::shared_ptr<ASTLiteralNode<flx_bool>>) { return 0; }
intmax_t DependencyResolver::hash(std::shared_ptr<ASTLiteralNode<flx_int>>) { return 0; }
intmax_t DependencyResolver::hash(std::shared_ptr<ASTLiteralNode<flx_float>>) { return 0; }
intmax_t DependencyResolver::hash(std::shared_ptr<ASTLiteralNode<flx_char>>) { return 0; }
intmax_t DependencyResolver::hash(std::shared_ptr<ASTLiteralNode<flx_string>>) { return 0; }
intmax_t DependencyResolver::hash(std::shared_ptr<ASTIdentifierNode>) { return 0; }

void DependencyResolver::set_curr_pos(size_t, size_t) {}
std::string DependencyResolver::msg_header() { return ""; }
