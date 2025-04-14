#include "md_builtin.hpp"

#include <iostream>
#include <thread>
#if defined(_WIN32) || defined(WIN32)
#include <conio.h>
#endif // defined(_WIN32) || defined(WIN32)

#include "types.hpp"
#include "semantic_analysis.hpp"
#include "interpreter.hpp"
#include "constants.hpp"
#include "visitor.hpp"
#include "utils.hpp"

using namespace core;
using namespace core::modules;
using namespace core::runtime;
using namespace core::analysis;
using namespace utils;

ModuleBuiltin::ModuleBuiltin() {
	build_decls();
}

ModuleBuiltin::~ModuleBuiltin() = default;

void ModuleBuiltin::register_functions(SemanticAnalyser* visitor) {
	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT)] = nullptr;

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN)] = nullptr;

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::READ), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::READ)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::READ)] = nullptr;

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH)] = nullptr;

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN) + "A"]);
	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN) + "S"]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN)] = nullptr;

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP)] = nullptr;

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM)] = nullptr;

}

void ModuleBuiltin::register_functions(Interpreter* visitor) {
	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT)] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[Constants::DEFAULT_NAMESPACE].back();
		if (scope->already_declared_variable("args")) {
			auto var = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("args"));
			auto args = var->get_value()->get_arr();

			for (size_t i = 0; i < args.size(); ++i) {
				std::cout << RuntimeOperations::parse_value_to_string(args[i]);
			}
		}

		};

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN)] = [this, visitor]() {
		visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT)]();
		std::cout << std::endl;
		};

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::READ), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::READ)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::READ)] = [this, visitor]() {
		visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT)]();
		std::string line;
		std::getline(std::cin, line);
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_STRING));
		visitor->current_expression_value->set(flx_string(std::move(line)));
		};

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH)] = [this, visitor]() {
		while (!_kbhit());
		char ch = _getch();
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_CHAR));
		visitor->current_expression_value->set(flx_char(ch));
		};

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN) + "A"]);
	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN) + "S"]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN)] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::DEFAULT_NAMESPACE].back();
		auto var = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("it"));
		auto itval = var->get_value();

		auto val = visitor->allocate_value(new RuntimeValue(Type::T_INT));

		if (TypeUtils::is_array(itval->type)) {
			val->set(flx_int(itval->get_arr().size()));
		}
		else {
			val->set(flx_int(itval->get_s().size()));
		}

		visitor->current_expression_value = val;

		};

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP)] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[Constants::DEFAULT_NAMESPACE].back();
		auto var = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("ms"));
		auto ms = var->get_value()->get_i();

		std::this_thread::sleep_for(std::chrono::milliseconds(ms));

		};

	visitor->scopes[Constants::DEFAULT_NAMESPACE].back()->declare_function(Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM), func_decls[Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM)]);
	visitor->builtin_functions[Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM)] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[Constants::DEFAULT_NAMESPACE].back();
		auto var = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("cmd"));
		auto cmd = var->get_value()->get_s();

		system(cmd.c_str());

		};

}

void ModuleBuiltin::build_decls() {
	std::vector<TypeDefinition*> parameters;
	VariableDefinition* variable;

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("args", Type::T_ANY, std::make_shared<ASTNullNode>(0, 0), true);
	parameters.push_back(variable);
	func_decls.emplace(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT), FunctionDefinition(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT), Type::T_VOID, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINT), 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("args", Type::T_ANY, std::make_shared<ASTNullNode>(0, 0), true);
	parameters.push_back(variable);
	func_decls.emplace(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN), FunctionDefinition(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN), Type::T_VOID, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(Constants::BUILTIN_NAMES.at(BuintinFuncs::PRINTLN), 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("args", Type::T_ANY, std::make_shared<ASTNullNode>(0, 0), true);
	parameters.push_back(variable);
	func_decls.emplace(Constants::BUILTIN_NAMES.at(BuintinFuncs::READ), FunctionDefinition(Constants::BUILTIN_NAMES.at(BuintinFuncs::READ), Type::T_STRING, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(Constants::BUILTIN_NAMES.at(BuintinFuncs::READ), 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	func_decls.emplace(Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH), FunctionDefinition(Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH), Type::T_CHAR, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(Constants::BUILTIN_NAMES.at(BuintinFuncs::READCH), 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("it", Type::T_ANY, std::vector<size_t>());
	parameters.push_back(variable);
	func_decls.emplace(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN) + "A", FunctionDefinition(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN) + "A", Type::T_INT, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN), 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("it", Type::T_STRING);
	parameters.push_back(variable);
	func_decls.emplace(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN) + "S", FunctionDefinition(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN) + "S", Type::T_INT, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(Constants::BUILTIN_NAMES.at(BuintinFuncs::LEN), 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("ms", Type::T_INT);
	parameters.push_back(variable);
	func_decls.emplace(Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP), FunctionDefinition(Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP), Type::T_VOID, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(Constants::BUILTIN_NAMES.at(BuintinFuncs::SLEEP), 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("cmd", Type::T_STRING);
	parameters.push_back(variable);
	func_decls.emplace(Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM), FunctionDefinition(Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM), Type::T_VOID, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(Constants::BUILTIN_NAMES.at(BuintinFuncs::SYSTEM), 0, 0)}, 0, 0)));

}
