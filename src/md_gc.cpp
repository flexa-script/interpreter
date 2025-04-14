#include "md_gc.hpp"

#include "interpreter.hpp"
#include "semantic_analysis.hpp"
#include "constants.hpp"

using namespace core::modules;
using namespace core::runtime;
using namespace core::analysis;

ModuleGC::ModuleGC() {}

ModuleGC::~ModuleGC() = default;

void ModuleGC::register_functions(SemanticAnalyser* visitor) {
	visitor->builtin_functions["gc_is_enabled"] = nullptr;
	visitor->builtin_functions["gc_enable"] = nullptr;
	visitor->builtin_functions["gc_collect"] = nullptr;
	visitor->builtin_functions["gc_get_max_heap"] = nullptr;
	visitor->builtin_functions["gc_set_max_heap"] = nullptr;
}

void ModuleGC::register_functions(Interpreter* visitor) {

	visitor->builtin_functions["gc_is_enabled"] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(flx_bool(visitor->gc.enable)));

		};

	visitor->builtin_functions["gc_enable"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("enable"))->get_value();

		visitor->gc.enable = val->get_b();

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["gc_collect"] = [this, visitor]() {

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		visitor->gc.collect();

		};

	visitor->builtin_functions["gc_get_max_heap"] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(flx_int(visitor->gc.max_heap)));

		};

	visitor->builtin_functions["gc_set_max_heap"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("max_heap"))->get_value();

		visitor->gc.max_heap = val->get_i();

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

}
