#ifndef MD_FILES_HPP
#define MD_FILES_HPP

#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>

#include "module.hpp"

namespace modules {
	class ModuleFiles : public Module {
	public:
		ModuleFiles();
		~ModuleFiles();

		void register_functions(visitor::SemanticAnalyser* visitor) override;
		void register_functions(visitor::Interpreter* visitor) override;
		void register_functions(visitor::Compiler* visitor) override;
		void register_functions(vm::VirtualMachine* vm) override;
	};
}

#endif // !MD_FILES_HPP
