#ifndef MD_SOUND_HPP
#define MD_SOUND_HPP

#include "module.hpp"

namespace core {

	namespace modules {

		class ModuleSound : public Module {
		public:
			ModuleSound();
			~ModuleSound();

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;
		};

	}

}

#endif // !MD_SOUND_HPP
