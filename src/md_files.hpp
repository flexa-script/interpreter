#ifndef MD_FILES_HPP
#define MD_FILES_HPP

#include <fstream>
#include <sstream>

#include "module.hpp"

namespace core {

	namespace modules {

		class ModuleFiles : public Module {
		public:
			ModuleFiles();
			~ModuleFiles();

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;
		};

	}

}

#endif // !MD_FILES_HPP
