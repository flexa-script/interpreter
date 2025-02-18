#ifndef MD_CONSOLE_HPP
#define MD_CONSOLE_HPP

#include <functional>

#include "module.hpp"

namespace modules {
	class ModuleConsole : public Module {
	public:
		ModuleConsole();
		~ModuleConsole();

		void register_functions(visitor::SemanticAnalyser* visitor) override;
		void register_functions(visitor::Interpreter* visitor) override;
		void register_functions(visitor::Compiler* visitor) override;
		void register_functions(vm::VirtualMachine* vm) override;
	};
}

#endif // !MD_CONSOLE_HPP
