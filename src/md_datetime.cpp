#include "md_datetime.hpp"

#include "interpreter.hpp"
#include "semantic_analysis.hpp"

using namespace modules;

ModuleDateTime::ModuleDateTime() {}

ModuleDateTime::~ModuleDateTime() = default;

flx_struct ModuleDateTime::tm_to_date_time(visitor::Interpreter* visitor, time_t t, tm* tm) {
	flx_struct dt_str = flx_struct();
	dt_str["timestamp"] = visitor->alocate_value(new RuntimeValue(flx_int(t)));
	dt_str["second"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_sec)));
	dt_str["minute"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_min)));
	dt_str["hour"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_hour)));
	dt_str["day"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_mday)));
	dt_str["month"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_mon + 1)));
	dt_str["year"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_year)));
	dt_str["week_day"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_wday)));
	dt_str["year_day"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_yday)));
	dt_str["is_dst"] = visitor->alocate_value(new RuntimeValue(flx_int(tm->tm_isdst)));

	return dt_str;
}

void ModuleDateTime::register_functions(visitor::SemanticAnalyser* visitor) {
	visitor->builtin_functions["create_date_time"] = nullptr;
	visitor->builtin_functions["diff_date_time"] = nullptr;
	visitor->builtin_functions["format_date_time"] = nullptr;
	visitor->builtin_functions["format_local_date_time"] = nullptr;
	visitor->builtin_functions["ascii_date_time"] = nullptr;
	visitor->builtin_functions["ascii_local_date_time"] = nullptr;
	visitor->builtin_functions["clock"] = nullptr;
}

void ModuleDateTime::register_functions(visitor::Interpreter* visitor) {

	visitor->builtin_functions["create_date_time"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();

		tm* tm = new struct tm();
		time_t t;

		if (scope->total_declared_variables() == 0) {
			t = time(nullptr);
			gmtime_s(tm, &t);
		}
		else if (scope->total_declared_variables() == 1) {
			auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("timestamp"))->value;

			t = val->get_i();
			gmtime_s(tm, &t);
		}
		else {
			auto vals = std::vector{
				std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("year"))->value,
				std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("month"))->value,
				std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("day"))->value,
				std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("hour"))->value,
				std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("min"))->value,
				std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("sec"))->value
			};

			tm->tm_year = vals[0]->get_i() - 1900;
			tm->tm_mon = vals[1]->get_i() - 1;
			tm->tm_mday = vals[2]->get_i();
			tm->tm_hour = vals[3]->get_i();
			tm->tm_min = vals[4]->get_i();
			tm->tm_sec = vals[5]->get_i();
			t = mktime(tm);
		}

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(tm_to_date_time(visitor, t, tm), "DateTime", language_namespace));

		};

	visitor->builtin_functions["diff_date_time"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("left_date_time"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("right_date_time"))->value
		};

		time_t lt = vals[0]->get_str()["timestamp"]->get_i();
		time_t rt = vals[1]->get_str()["timestamp"]->get_i();
		time_t t = difftime(lt, rt);
		tm* tm = new struct tm();
		gmtime_s(tm, &t);

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(tm_to_date_time(visitor, t, tm), "DateTime", language_namespace));

		};

	visitor->builtin_functions["format_date_time"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("date_time"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("format"))->value
		};

		time_t t = vals[0]->get_str()["timestamp"]->get_i();
		std::string fmt = vals[1]->get_s();
		tm* tm = new struct tm();
		gmtime_s(tm, &t);
		char buffer[80];
		strftime(buffer, 80, fmt.c_str(), tm);

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(std::string{ buffer }));

		};

	visitor->builtin_functions["format_local_date_time"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("date_time"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("format"))->value
		};

		time_t t = vals[0]->get_str()["timestamp"]->get_i();
		std::string fmt = vals[1]->get_s();
		tm* tm = new struct tm();
		localtime_s(tm, &t);
		char buffer[80];
		strftime(buffer, 80, fmt.c_str(), tm);

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(std::string{ buffer }));

		};

	visitor->builtin_functions["ascii_date_time"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("date_time"))->value;

		time_t t = val->get_str()["timestamp"]->get_i();
		tm* tm = new struct tm();
		gmtime_s(tm, &t);
		char buffer[26];
		
		if (asctime_s(buffer, sizeof(buffer), tm) != 0) {
			throw std::runtime_error("Error trying to convert date/time to ASCII string");
		}

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(std::string{ buffer }));

		};

	visitor->builtin_functions["ascii_local_date_time"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("date_time"))->value;

		time_t t = val->get_str()["timestamp"]->get_i();
		tm* tm = new struct tm();
		localtime_s(tm, &t);
		char buffer[26];

		if (asctime_s(buffer, sizeof(buffer), tm) != 0) {
			throw std::runtime_error("Error trying to convert date/time to ASCII string");
		}

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(std::string{ buffer }));

		};

	visitor->builtin_functions["clock"] = [this, visitor]() {
		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(flx_int(clock())));

		};

}

void ModuleDateTime::register_functions(visitor::Compiler* visitor) {}

void ModuleDateTime::register_functions(vm::VirtualMachine* vm) {}
