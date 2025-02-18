#ifndef MD_GRAPHICS_HPP
#define MD_GRAPHICS_HPP

#include <functional>

#include "module.hpp"
#include "graphics_utils.hpp"

namespace modules {
	class ModuleGraphics : public Module {
	public:
		ModuleGraphics();
		~ModuleGraphics();

		void register_functions(visitor::SemanticAnalyser* visitor) override;
		void register_functions(visitor::Interpreter* visitor) override;
		void register_functions(visitor::Compiler* visitor) override;
		void register_functions(vm::VirtualMachine* vm) override;
	};
}

#endif // !MD_GRAPHICS_HPP
