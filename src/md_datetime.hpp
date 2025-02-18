#ifndef MD_DATETIME_HPP
#define MD_DATETIME_HPP

#include <ctime>

#include "module.hpp"
#include "types.hpp"

namespace modules {
	class ModuleDateTime : public Module {
	public:
		ModuleDateTime();
		~ModuleDateTime();

		flx_struct tm_to_date_time(visitor::Interpreter* visitor, time_t t, tm* tm);

		void register_functions(visitor::SemanticAnalyser* visitor) override;
		void register_functions(visitor::Interpreter* visitor) override;
		void register_functions(visitor::Compiler* visitor) override;
		void register_functions(vm::VirtualMachine* vm) override;
	};
}

#endif // !MD_DATETIME_HPP
