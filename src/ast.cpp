#include <utility>
#include <numeric>

#include "ast.hpp"
#include "utils.hpp"

using namespace visitor;
using namespace parser;

Identifier::Identifier(const std::string& identifier, const std::vector<std::shared_ptr<ASTExprNode>>& access_vector)
	: identifier(identifier), access_vector(access_vector) {}

Identifier::Identifier(const std::string& identifier)
	: identifier(identifier) {}

Identifier::Identifier()
	: identifier(""), access_vector(std::vector<std::shared_ptr<ASTExprNode>>()) {}

ASTProgramNode::ASTProgramNode(const std::string& name, const std::string& name_space, const std::vector<std::shared_ptr<ASTNode>>& statements)
	: ASTNode(row, col), name(name), name_space(name_space), statements(statements), libs(std::vector<std::shared_ptr<ASTProgramNode>>()) {}

ASTBuiltinCallNode::ASTBuiltinCallNode(std::string identifier, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), identifier(identifier) {}

ASTUsingNode::ASTUsingNode(const std::vector<std::string>& library, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), library(library) {}

ASTNamespaceManagerNode::ASTNamespaceManagerNode(const std::string& image, const std::string& name_space, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), image(image), name_space(name_space) {}

ASTDeclarationNode::ASTDeclarationNode(const std::string& identifier, Type type, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
	const std::string& type_name, const std::string& type_name_space, std::shared_ptr<ASTExprNode> expr, bool is_const, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), TypeDefinition(type, array_type, dim, type_name, type_name_space),
	identifier(identifier), expr(expr), is_const(is_const) {}

ASTUnpackedDeclarationNode::ASTUnpackedDeclarationNode(Type type, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
	const std::string& type_name, const std::string& type_name_space, const std::vector<std::shared_ptr<ASTDeclarationNode>>& declarations,
	std::shared_ptr<ASTExprNode> expr, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), TypeDefinition(type, array_type, dim, type_name, type_name_space),
	declarations(declarations), expr(expr) {}

ASTAssignmentNode::ASTAssignmentNode(const std::vector<Identifier>& identifier_vector, const std::string& name_space,
	const std::string& op, std::shared_ptr<ASTExprNode> expr, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), identifier(identifier_vector[0].identifier),
	identifier_vector(identifier_vector), name_space(name_space), expr(expr), op(op) {}

ASTReturnNode::ASTReturnNode(std::shared_ptr<ASTExprNode> expr, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), expr(expr) {}

ASTBlockNode::ASTBlockNode(const std::vector<std::shared_ptr<ASTNode>>& statements, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), statements(statements) {}

ASTContinueNode::ASTContinueNode(unsigned int row, unsigned int col)
	: ASTStatementNode(row, col) {}

ASTBreakNode::ASTBreakNode(unsigned int row, unsigned int col)
	: ASTStatementNode(row, col) {}

ASTExitNode::ASTExitNode(std::shared_ptr<ASTExprNode> exit_code, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), exit_code(exit_code) {}

ASTSwitchNode::ASTSwitchNode(std::shared_ptr<ASTExprNode> condition, const std::vector<std::shared_ptr<ASTNode>>& statements,
	const std::map<std::shared_ptr<ASTExprNode>, unsigned int>& case_blocks,
	unsigned int default_block, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), condition(condition), statements(statements), case_blocks(case_blocks),
	default_block(default_block), parsed_case_blocks(std::map<unsigned int, unsigned int>()) {}

ASTIfNode::ASTIfNode(std::shared_ptr<ASTExprNode> condition, std::shared_ptr<ASTBlockNode> if_block, const std::vector<std::shared_ptr<ASTElseIfNode>>& else_ifs,
	std::shared_ptr<ASTBlockNode> else_block, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), condition(condition), if_block(if_block), else_ifs(else_ifs), else_block(else_block) {}

ASTElseIfNode::ASTElseIfNode(std::shared_ptr<ASTExprNode> condition, std::shared_ptr<ASTBlockNode> block, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), condition(condition), block(block) {}

ASTEnumNode::ASTEnumNode(const std::vector<std::string>& identifiers, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), identifiers(identifiers) {}

ASTTryCatchNode::ASTTryCatchNode(std::shared_ptr<ASTStatementNode> decl, std::shared_ptr<ASTBlockNode> try_block, std::shared_ptr<ASTBlockNode> catch_block, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), decl(decl), try_block(try_block), catch_block(catch_block) {}

ASTThrowNode::ASTThrowNode(std::shared_ptr<ASTExprNode> error, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), error(error) {}

ASTEllipsisNode::ASTEllipsisNode(unsigned int row, unsigned int col)
	: ASTStatementNode(row, col) {}

ASTForNode::ASTForNode(const std::array<std::shared_ptr<ASTNode>, 3>& dci, std::shared_ptr<ASTBlockNode> block, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), dci(dci), block(block) {}

ASTForEachNode::ASTForEachNode(std::shared_ptr<ASTStatementNode> itdecl, std::shared_ptr<ASTNode> collection, std::shared_ptr<ASTBlockNode> block, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), itdecl(itdecl), collection(collection), block(block) {}

ASTWhileNode::ASTWhileNode(std::shared_ptr<ASTExprNode> condition, std::shared_ptr<ASTBlockNode> block, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), condition(condition), block(block) {}

ASTDoWhileNode::ASTDoWhileNode(std::shared_ptr<ASTExprNode> condition, std::shared_ptr<ASTBlockNode> block, unsigned int row, unsigned int col)
	: ASTWhileNode(condition, block, row, col) {}

ASTStructDefinitionNode::ASTStructDefinitionNode(const std::string& identifier, const std::map<std::string, VariableDefinition>& variables,
	unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), identifier(identifier), variables(variables) {}

ASTFunctionDefinitionNode::ASTFunctionDefinitionNode(const std::string& identifier, const std::vector<TypeDefinition*>& parameters,
	Type type, const std::string& type_name, const std::string& type_name_space, Type array_type, const std::vector<std::shared_ptr<ASTExprNode>>& dim,
	std::shared_ptr<ASTBlockNode> block, unsigned int row, unsigned int col)
	: ASTStatementNode(row, col), TypeDefinition(type, array_type, dim, type_name, type_name_space),
	identifier(identifier), parameters(parameters), block(block) {}

ASTArrayConstructorNode::ASTArrayConstructorNode(const std::vector<std::shared_ptr<ASTExprNode>>& values, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), values(values) {}

ASTStructConstructorNode::ASTStructConstructorNode(const std::string& type_name, const std::string& name_space,
	const std::map<std::string, std::shared_ptr<ASTExprNode>>& values, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), type_name(type_name), name_space(name_space), values(values) {}

ASTNullNode::ASTNullNode(unsigned int row, unsigned int col)
	: ASTExprNode(row, col) {}

ASTThisNode::ASTThisNode(unsigned int row, unsigned int col)
	: ASTExprNode(row, col) {}

ASTBinaryExprNode::ASTBinaryExprNode(const std::string& op, std::shared_ptr<ASTExprNode> left, std::shared_ptr<ASTExprNode> right, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), op(op), left(left), right(right) {}

ASTUnaryExprNode::ASTUnaryExprNode(const std::string& unary_op, std::shared_ptr<ASTExprNode> expr, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), unary_op(unary_op), expr(expr) {}

ASTIdentifierNode::ASTIdentifierNode(const std::vector<Identifier>& identifier_vector, std::string name_space, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), identifier(identifier_vector[0].identifier),
	identifier_vector(identifier_vector), name_space(name_space) {}

ASTTernaryNode::ASTTernaryNode(std::shared_ptr<ASTExprNode> condition, std::shared_ptr<ASTExprNode> value_if_true, std::shared_ptr<ASTExprNode> value_if_false, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), condition(condition), value_if_true(value_if_true), value_if_false(value_if_false) {}

ASTInNode::ASTInNode(std::shared_ptr<ASTExprNode> value, std::shared_ptr<ASTExprNode> collection, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), value(value), collection(collection) {}

ASTFunctionCallNode::ASTFunctionCallNode(const std::string& name_space,
	const std::vector<Identifier>& identifier_vector,
	const std::vector<std::shared_ptr<ASTExprNode>>& parameters, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), identifier(identifier_vector[0].identifier),
	name_space(name_space), identifier_vector(identifier_vector), parameters(parameters) {}

ASTTypeCastNode::ASTTypeCastNode(Type type, std::shared_ptr<ASTExprNode> expr, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), type(type), expr(expr) {}

ASTTypingNode::ASTTypingNode(const std::string& image, std::shared_ptr<ASTExprNode> expr, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), image(image), expr(expr) {}

ASTLambdaFunction::ASTLambdaFunction(std::shared_ptr<ASTFunctionDefinitionNode> fun, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), fun(fun) {}

ASTValueNode::ASTValueNode(Value* value, unsigned int row, unsigned int col)
	: ASTExprNode(row, col), value(value) {}

void ASTLiteralNode<flx_bool>::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTLiteralNode<flx_bool>>(shared_from_this()));
}

long long ASTLiteralNode<flx_bool>::hash(Visitor* v) {
	return v->hash(std::dynamic_pointer_cast<ASTLiteralNode<flx_bool>>(shared_from_this()));
}

void ASTLiteralNode<flx_int>::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTLiteralNode<flx_int>>(shared_from_this()));
}

long long ASTLiteralNode<flx_int>::hash(Visitor* v) {
	return v->hash(std::dynamic_pointer_cast<ASTLiteralNode<flx_int>>(shared_from_this()));
}

void ASTLiteralNode<flx_float>::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTLiteralNode<flx_float>>(shared_from_this()));
}

long long ASTLiteralNode<flx_float>::hash(Visitor* v) {
	return v->hash(std::dynamic_pointer_cast<ASTLiteralNode<flx_float>>(shared_from_this()));
}

void ASTLiteralNode<flx_char>::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTLiteralNode<flx_char>>(shared_from_this()));
}

long long ASTLiteralNode<flx_char>::hash(Visitor* v) {
	return v->hash(std::dynamic_pointer_cast<ASTLiteralNode<flx_char>>(shared_from_this()));
}

void ASTLiteralNode<flx_string>::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTLiteralNode<flx_string>>(shared_from_this()));
}

long long ASTLiteralNode<flx_string>::hash(Visitor* v) {
	return v->hash(std::dynamic_pointer_cast<ASTLiteralNode<flx_string>>(shared_from_this()));
}

void ASTArrayConstructorNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTArrayConstructorNode>(shared_from_this()));
}

long long ASTArrayConstructorNode::hash(Visitor* v) { return 0; }

void ASTStructConstructorNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTStructConstructorNode>(shared_from_this()));
}

long long ASTStructConstructorNode::hash(Visitor* v) { return 0; }

void ASTBinaryExprNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTBinaryExprNode>(shared_from_this()));
}

long long ASTBinaryExprNode::hash(Visitor* v) { return 0; }

void ASTInNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTInNode>(shared_from_this()));
}

long long ASTInNode::hash(Visitor* v) { return 0; }

void ASTFunctionCallNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTFunctionCallNode>(shared_from_this()));
}

long long ASTFunctionCallNode::hash(Visitor* v) { return 0; }

void ASTIdentifierNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTIdentifierNode>(shared_from_this()));
}

long long ASTIdentifierNode::hash(Visitor* v) {
	return v->hash(std::dynamic_pointer_cast<ASTIdentifierNode>(shared_from_this()));
}

void ASTTypingNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTTypingNode>(shared_from_this()));
}

long long ASTTypingNode::hash(Visitor* v) { return 0; }

void ASTUnaryExprNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTUnaryExprNode>(shared_from_this()));
}

long long ASTUnaryExprNode::hash(Visitor* v) { return 0; }

void ASTTernaryNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTTernaryNode>(shared_from_this()));
}

long long ASTTernaryNode::hash(Visitor* v) { return 0; }

void ASTTypeCastNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTTypeCastNode>(shared_from_this()));
}

long long ASTTypeCastNode::hash(Visitor* v) { return 0; }

void ASTNullNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTNullNode>(shared_from_this()));
}

long long ASTNullNode::hash(Visitor* v) { return 0; }

void ASTThisNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTThisNode>(shared_from_this()));
}

long long ASTThisNode::hash(Visitor* v) { return 0; }

void ASTLambdaFunction::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTLambdaFunction>(shared_from_this()));
}

long long ASTLambdaFunction::hash(Visitor* v) { return 0; }

void ASTValueNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTValueNode>(shared_from_this()));
}

long long ASTValueNode::hash(Visitor* v) {
	return v->hash(std::dynamic_pointer_cast<ASTValueNode>(shared_from_this()));
}

void ASTDeclarationNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTDeclarationNode>(shared_from_this()));
}

void ASTUnpackedDeclarationNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTUnpackedDeclarationNode>(shared_from_this()));
}

void ASTNamespaceManagerNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTNamespaceManagerNode>(shared_from_this()));
}

void ASTAssignmentNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTAssignmentNode>(shared_from_this()));
}

void ASTReturnNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTReturnNode>(shared_from_this()));
}

void ASTExitNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTExitNode>(shared_from_this()));
}

void ASTBlockNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTBlockNode>(shared_from_this()));
}

void ASTContinueNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTContinueNode>(shared_from_this()));
}

void ASTBreakNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTBreakNode>(shared_from_this()));
}

void ASTSwitchNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTSwitchNode>(shared_from_this()));
}

void ASTEnumNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTEnumNode>(shared_from_this()));
}

void ASTTryCatchNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTTryCatchNode>(shared_from_this()));
}

void ASTThrowNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTThrowNode>(shared_from_this()));
}

void ASTEllipsisNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTEllipsisNode>(shared_from_this()));
}

void ASTElseIfNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTElseIfNode>(shared_from_this()));
}

void ASTIfNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTIfNode>(shared_from_this()));
}

void ASTForNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTForNode>(shared_from_this()));
}

void ASTForEachNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTForEachNode>(shared_from_this()));
}

void ASTWhileNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTWhileNode>(shared_from_this()));
}

void ASTDoWhileNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTDoWhileNode>(shared_from_this()));
}

void ASTFunctionDefinitionNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTFunctionDefinitionNode>(shared_from_this()));
}

void ASTStructDefinitionNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTStructDefinitionNode>(shared_from_this()));
}

void ASTUsingNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTUsingNode>(shared_from_this()));
}

void ASTProgramNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTProgramNode>(shared_from_this()));
}

void ASTBuiltinCallNode::accept(Visitor* v) {
	v->visit(std::dynamic_pointer_cast<ASTBuiltinCallNode>(shared_from_this()));
}
