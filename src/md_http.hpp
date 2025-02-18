#ifndef MD_HTTP_HPP
#define MD_HTTP_HPP

#include "module.hpp"

namespace modules {
	class ModuleHTTP : public Module {
	public:
		ModuleHTTP();
		~ModuleHTTP();

		void register_functions(visitor::SemanticAnalyser* visitor) override;
		void register_functions(visitor::Interpreter* visitor) override;
		void register_functions(visitor::Compiler* visitor) override;
		void register_functions(vm::VirtualMachine* vm) override;
	};
}

#endif // !MD_HTTP_HPP
