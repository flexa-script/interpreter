#ifndef MD_GRAPHICS_HPP
#define MD_GRAPHICS_HPP

#include "module.hpp"
#include "graphics_utils.hpp"

namespace core {

	namespace modules {

		class ModuleGraphics : public Module {
		public:
			ModuleGraphics();
			~ModuleGraphics();

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;
		};

	}

}

#endif // !MD_GRAPHICS_HPP
