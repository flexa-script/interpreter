#ifndef MODULE_HPP
#define MODULE_HPP

#include <string>

namespace visitor {
	class SemanticAnalyser;
	class Interpreter;
	class Compiler;
}

namespace vm {
	class VirtualMachine;
}

namespace modules {

	class Module {
	public:
		static const std::string INSTANCE_ID_NAME;

	public:
		virtual ~Module() = default;

		virtual void register_functions(visitor::SemanticAnalyser* visitor) = 0;
		virtual void register_functions(visitor::Interpreter* visitor) = 0;
		virtual void register_functions(visitor::Compiler* visitor) = 0;
		virtual void register_functions(vm::VirtualMachine* vm) = 0;
	};

}

#endif // !MODULE_HPP
