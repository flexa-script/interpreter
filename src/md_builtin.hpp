#ifndef MD_BUILTIN_HPP
#define MD_BUILTIN_HPP

#include <string>
#include <unordered_map>

#include "module.hpp"

namespace core {

	class FunctionDefinition;

	namespace modules {

		class ModuleBuiltin : public Module {
		private:
			std::unordered_map<std::string, FunctionDefinition> func_decls;

		public:
			ModuleBuiltin();
			~ModuleBuiltin();

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;

		private:
			void build_decls();
		};

	}

}

#endif // !MD_BUILTIN_HPP
