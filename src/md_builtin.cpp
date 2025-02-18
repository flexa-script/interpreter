#include <iostream>
#include <functional>
#include <chrono>
#include <thread>
#include <conio.h>
#include <memory>

#include "md_builtin.hpp"

#include "types.hpp"
#include "semantic_analysis.hpp"
#include "interpreter.hpp"
#include "compiler.hpp"
#include "vm.hpp"

#include "visitor.hpp"

using namespace modules;
using namespace vm;

std::string modules::BUILTIN_NAMES[] = {
	"print",
	"println",
	"read",
	"readch",
	"len",
	"sleep",
	"system"
};

ModuleBuiltin::ModuleBuiltin() {
	build_decls();
}

ModuleBuiltin::~ModuleBuiltin() = default;

void ModuleBuiltin::register_functions(visitor::SemanticAnalyser* visitor) {
	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::PRINT], func_decls[BUILTIN_NAMES[BuintinFuncs::PRINT]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINT]] = nullptr;

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::PRINTLN], func_decls[BUILTIN_NAMES[BuintinFuncs::PRINTLN]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINTLN]] = nullptr;

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::READ], func_decls[BUILTIN_NAMES[BuintinFuncs::READ]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::READ]] = nullptr;

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::READCH], func_decls[BUILTIN_NAMES[BuintinFuncs::READCH]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::READCH]] = nullptr;

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::LEN], func_decls[BUILTIN_NAMES[BuintinFuncs::LEN] + "A"]);
	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::LEN], func_decls[BUILTIN_NAMES[BuintinFuncs::LEN] + "S"]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::LEN]] = nullptr;

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::SLEEP], func_decls[BUILTIN_NAMES[BuintinFuncs::SLEEP]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::SLEEP]] = nullptr;

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::SYSTEM], func_decls[BUILTIN_NAMES[BuintinFuncs::SYSTEM]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::SYSTEM]] = nullptr;

}

void ModuleBuiltin::register_functions(visitor::Interpreter* visitor) {
	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::PRINT], func_decls[BUILTIN_NAMES[BuintinFuncs::PRINT]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINT]] = [this, visitor]() {
		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[default_namespace].back();
		if (scope->already_declared_variable("args")) {
			auto var = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("args"));
			auto args = var->value->get_arr();

			for (size_t i = 0; i < args.size(); ++i) {
				std::cout << RuntimeOperations::parse_value_to_string(args[i]);
			}
		}

		};

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::PRINTLN], func_decls[BUILTIN_NAMES[BuintinFuncs::PRINTLN]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINTLN]] = [this, visitor]() {
		visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINT]]();
		std::cout << std::endl;
		};

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::READ], func_decls[BUILTIN_NAMES[BuintinFuncs::READ]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::READ]] = [this, visitor]() {
		visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINT]]();
		std::string line;
		std::getline(std::cin, line);
		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(Type::T_STRING));
		visitor->current_expression_value->set(flx_string(std::move(line)));
		};

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::READCH], func_decls[BUILTIN_NAMES[BuintinFuncs::READCH]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::READCH]] = [this, visitor]() {
		while (!_kbhit());
		char ch = _getch();
		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(Type::T_CHAR));
		visitor->current_expression_value->set(flx_char(ch));
		};

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::LEN], func_decls[BUILTIN_NAMES[BuintinFuncs::LEN] + "A"]);
	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::LEN], func_decls[BUILTIN_NAMES[BuintinFuncs::LEN] + "S"]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::LEN]] = [this, visitor]() {
		auto& scope = visitor->scopes[default_namespace].back();
		auto var = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("it"));
		auto itval = var->value;

		auto val = visitor->alocate_value(new RuntimeValue(Type::T_INT));

		if (is_array(itval->type)) {
			val->set(flx_int(itval->get_arr().size()));
		}
		else {
			val->set(flx_int(itval->get_s().size()));
		}

		visitor->current_expression_value = val;

		};

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::SLEEP], func_decls[BUILTIN_NAMES[BuintinFuncs::SLEEP]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::SLEEP]] = [this, visitor]() {
		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[default_namespace].back();
		auto var = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("ms"));
		auto ms = var->value->get_i();

		std::this_thread::sleep_for(std::chrono::milliseconds(ms));

		};

	visitor->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::SYSTEM], func_decls[BUILTIN_NAMES[BuintinFuncs::SYSTEM]]);
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::SYSTEM]] = [this, visitor]() {
		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[default_namespace].back();
		auto var = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("cmd"));
		auto cmd = var->value->get_s();

		system(cmd.c_str());

		};

}

void ModuleBuiltin::register_functions(visitor::Compiler* visitor) {
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINT]] = nullptr;
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINTLN]] = nullptr;
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::READ]] = nullptr;
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::READCH]] = nullptr;
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::LEN]] = nullptr;
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::SLEEP]] = nullptr;
	visitor->builtin_functions[BUILTIN_NAMES[BuintinFuncs::SYSTEM]] = nullptr;
}

void ModuleBuiltin::register_functions(VirtualMachine* vm) {
	vm->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::PRINT], func_decls[BUILTIN_NAMES[BuintinFuncs::PRINT]]);
	vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINT]] = [this, vm]() {
		try {
			flx_array args;
			while (vm->param_count-- > 0) {
				args.push_back(vm->get_stack_top());
			}

			for (size_t i = 0; i < args.size(); ++i) {
				std::cout << RuntimeOperations::parse_value_to_string(args[i]);
			}
		}
		catch (...) {}

		};

	vm->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::PRINTLN], func_decls[BUILTIN_NAMES[BuintinFuncs::PRINTLN]]);
	vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINTLN]] = [this, vm]() {
		vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINT]]();
		std::cout << std::endl;
		};

	vm->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::READ], func_decls[BUILTIN_NAMES[BuintinFuncs::READ]]);
	vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::READ]] = [this, vm]() {
		vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::PRINT]]();
		std::string line;
		std::getline(std::cin, line);
		vm->push_constant(new RuntimeValue(flx_string(std::move(line))));
		};

	vm->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::READCH], func_decls[BUILTIN_NAMES[BuintinFuncs::READCH]]);
	vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::READCH]] = [this, vm]() {
		while (!_kbhit());
		char ch = _getch();
		vm->push_constant(new RuntimeValue(flx_char(ch)));
		};

	vm->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::LEN], func_decls[BUILTIN_NAMES[BuintinFuncs::LEN] + "A"]);
	vm->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::LEN], func_decls[BUILTIN_NAMES[BuintinFuncs::LEN] + "S"]);
	vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::LEN]] = [this, vm]() {
		auto itval = vm->get_stack_top();

		auto val = new RuntimeValue(Type::T_INT);

		if (is_array(itval->type)) {
			val->set(flx_int(itval->get_arr().size()));
		}
		else {
			val->set(flx_int(itval->get_s().size()));
		}

		vm->push_constant(val);

		};

	vm->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::SLEEP], func_decls[BUILTIN_NAMES[BuintinFuncs::SLEEP]]);
	vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::SLEEP]] = [this, vm]() {
		auto ms = vm->get_stack_top()->get_i();

		std::this_thread::sleep_for(std::chrono::milliseconds(ms));

		};

	vm->scopes[default_namespace].back()->declare_function(BUILTIN_NAMES[BuintinFuncs::SYSTEM], func_decls[BUILTIN_NAMES[BuintinFuncs::SYSTEM]]);
	vm->builtin_functions[BUILTIN_NAMES[BuintinFuncs::SYSTEM]] = [this, vm]() {
		auto cmd = vm->get_stack_top()->get_s();

		system(cmd.c_str());

		};

}

void ModuleBuiltin::build_decls() {
	std::vector<TypeDefinition*> parameters;
	VariableDefinition* variable;

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("args", Type::T_ANY, std::make_shared<ASTNullNode>(0, 0), true);
	parameters.push_back(variable);
	func_decls.emplace(BUILTIN_NAMES[BuintinFuncs::PRINT], FunctionDefinition(BUILTIN_NAMES[BuintinFuncs::PRINT], Type::T_VOID, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(BUILTIN_NAMES[BuintinFuncs::PRINT], 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("args", Type::T_ANY, std::make_shared<ASTNullNode>(0, 0), true);
	parameters.push_back(variable);
	func_decls.emplace(BUILTIN_NAMES[BuintinFuncs::PRINTLN], FunctionDefinition(BUILTIN_NAMES[BuintinFuncs::PRINTLN], Type::T_VOID, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(BUILTIN_NAMES[BuintinFuncs::PRINTLN], 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("args", Type::T_ANY, std::make_shared<ASTNullNode>(0, 0), true);
	parameters.push_back(variable);
	func_decls.emplace(BUILTIN_NAMES[BuintinFuncs::READ], FunctionDefinition(BUILTIN_NAMES[BuintinFuncs::READ], Type::T_STRING, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(BUILTIN_NAMES[BuintinFuncs::READ], 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	func_decls.emplace(BUILTIN_NAMES[BuintinFuncs::READCH], FunctionDefinition(BUILTIN_NAMES[BuintinFuncs::READCH], Type::T_CHAR, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(BUILTIN_NAMES[BuintinFuncs::READCH], 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("it", Type::T_ANY, std::vector<std::shared_ptr<ASTExprNode>>());
	parameters.push_back(variable);
	func_decls.emplace(BUILTIN_NAMES[BuintinFuncs::LEN] + "A", FunctionDefinition(BUILTIN_NAMES[BuintinFuncs::LEN] + "A", Type::T_INT, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(BUILTIN_NAMES[BuintinFuncs::LEN], 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("it", Type::T_STRING);
	parameters.push_back(variable);
	func_decls.emplace(BUILTIN_NAMES[BuintinFuncs::LEN] + "S", FunctionDefinition(BUILTIN_NAMES[BuintinFuncs::LEN] + "S", Type::T_INT, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(BUILTIN_NAMES[BuintinFuncs::LEN], 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("ms", Type::T_INT);
	parameters.push_back(variable);
	func_decls.emplace(BUILTIN_NAMES[BuintinFuncs::SLEEP], FunctionDefinition(BUILTIN_NAMES[BuintinFuncs::SLEEP], Type::T_VOID, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(BUILTIN_NAMES[BuintinFuncs::SLEEP], 0, 0)}, 0, 0)));

	parameters = std::vector<TypeDefinition*>();
	variable = new VariableDefinition("cmd", Type::T_STRING);
	parameters.push_back(variable);
	func_decls.emplace(BUILTIN_NAMES[BuintinFuncs::SYSTEM], FunctionDefinition(BUILTIN_NAMES[BuintinFuncs::SYSTEM], Type::T_VOID, parameters,
		std::make_shared<ASTBlockNode>(std::vector<std::shared_ptr<ASTNode>>{
		std::make_shared<ASTBuiltinCallNode>(BUILTIN_NAMES[BuintinFuncs::SYSTEM], 0, 0)}, 0, 0)));
}
