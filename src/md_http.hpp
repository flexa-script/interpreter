#ifndef MD_HTTP_HPP
#define MD_HTTP_HPP

#include "module.hpp"

namespace core {

	namespace modules {

		class ModuleHTTP : public Module {
		public:
			ModuleHTTP();
			~ModuleHTTP();

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;
		};

	}

}

#endif // !MD_HTTP_HPP
