#ifndef MODULE_HPP
#define MODULE_HPP

#include <string>

namespace core {

	namespace analysis {
		class SemanticAnalyser;
		class Compiler;
	}

	namespace runtime {
		class Interpreter;
		class VirtualMachine;
	}

	namespace modules {

		class Module {
		public:
			static const std::string INSTANCE_ID_NAME;

		public:
			virtual ~Module() = default;

			virtual void register_functions(analysis::SemanticAnalyser* visitor) = 0;
			virtual void register_functions(runtime::Interpreter* visitor) = 0;
			virtual void register_functions(analysis::Compiler* visitor) = 0;
			virtual void register_functions(runtime::VirtualMachine* vm) = 0;
		};

	}

}

#endif // !MODULE_HPP
