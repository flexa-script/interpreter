#ifndef MD_CONSOLE_HPP
#define MD_CONSOLE_HPP

#include "module.hpp"

namespace core {

	namespace modules {

		class ModuleConsole : public Module {
		public:
			ModuleConsole();
			~ModuleConsole();

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;
		};

	}

}

#endif // !MD_CONSOLE_HPP
