#ifndef MODULE_HPP
#define MODULE_HPP

#include <string>

namespace core {

	namespace analysis {
		class SemanticAnalyser;
	}

	namespace runtime {
		class Interpreter;
	}

	namespace modules {

		class Module {
		public:
			static const std::string INSTANCE_ID_NAME;

		public:
			virtual ~Module() = default;

			virtual void register_functions(analysis::SemanticAnalyser* visitor) = 0;
			virtual void register_functions(runtime::Interpreter* visitor) = 0;
		};

	}

}

#endif // !MODULE_HPP
