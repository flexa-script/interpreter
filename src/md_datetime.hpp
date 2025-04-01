#ifndef MD_DATETIME_HPP
#define MD_DATETIME_HPP

#include <ctime>

#include "module.hpp"
#include "types.hpp"

namespace core {

	namespace modules {

		class ModuleDateTime : public Module {
		public:
			ModuleDateTime();
			~ModuleDateTime();

			flx_struct tm_to_date_time(runtime::Interpreter* visitor, time_t t, tm* tm);

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;
		};

	}

}

#endif // !MD_DATETIME_HPP
