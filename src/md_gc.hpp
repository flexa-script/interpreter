#ifndef MD_GC_HPP
#define MD_GC_HPP

#include "module.hpp"

namespace core {

	namespace modules {

		class ModuleGC : public Module {
		public:
			ModuleGC();
			~ModuleGC();

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;
		};

	}

}

#endif // !MD_GC_HPP
