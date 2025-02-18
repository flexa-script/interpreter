#ifndef MD_SOUND_HPP
#define MD_SOUND_HPP

#include "module.hpp"

namespace modules {
	class ModuleSound : public Module {
	public:
		ModuleSound();
		~ModuleSound();

		void register_functions(visitor::SemanticAnalyser* visitor) override;
		void register_functions(visitor::Interpreter* visitor) override;
		void register_functions(visitor::Compiler* visitor) override;
		void register_functions(vm::VirtualMachine* vm) override;
	};
}

#endif // !MD_SOUND_HPP
