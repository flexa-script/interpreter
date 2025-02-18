#include <Windows.h>

#include "md_input.hpp"

#include "interpreter.hpp"
#include "semantic_analysis.hpp"

using namespace modules;

ModuleInput::ModuleInput() : running(false) {
	start();
}

ModuleInput::~ModuleInput() {
	stop();
}

void ModuleInput::register_functions(visitor::SemanticAnalyser* visitor) {
	visitor->builtin_functions["update_key_states"] = nullptr;
	visitor->builtin_functions["is_key_pressed"] = nullptr;
	visitor->builtin_functions["is_key_released"] = nullptr;
	visitor->builtin_functions["get_mouse_position"] = nullptr;
	visitor->builtin_functions["set_mouse_position"] = nullptr;
	visitor->builtin_functions["is_mouse_button_pressed"] = nullptr;
}

void ModuleInput::register_functions(visitor::Interpreter* visitor) {

	visitor->builtin_functions["update_key_states"] = [this, visitor]() {
		previous_key_state = current_key_state;

		for (int i = 0; i < KEY_COUNT; ++i) {
			current_key_state[i] = GetAsyncKeyState(i) & 0x8000;
		}

		};

	visitor->builtin_functions["is_key_pressed"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("key"))->value;

		int key = val->get_i();
		bool is_pressed = false;

		{
			std::lock_guard<std::mutex> lock(state_mutex);
			is_pressed = current_key_state[key];
		}

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(flx_bool(is_pressed)));

		};

	visitor->builtin_functions["is_key_released"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("key"))->value;

		int key = val->get_i();

		bool is_released = false;

		{
			std::lock_guard<std::mutex> lock(state_mutex);
			is_released = previous_key_state[key] && !current_key_state[key];
			previous_key_state[key] = false;
		}

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(flx_bool(is_released)));

		};

	visitor->builtin_functions["get_mouse_position"] = [this, visitor]() {
		POINT point;
		GetCursorPos(&point);

		flx_struct str = flx_struct();
		str["x"] = visitor->alocate_value(new RuntimeValue(flx_int(point.x * 2 * 0.905)));
		str["y"] = visitor->alocate_value(new RuntimeValue(flx_int(point.y * 2 * 0.875)));
		RuntimeValue* res = visitor->alocate_value(new RuntimeValue(str, "Point", language_namespace));

		visitor->current_expression_value = res;

		};

	visitor->builtin_functions["set_mouse_position"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->value
		};

		int x = vals[0]->get_i();
		int y = vals[1]->get_i();
		SetCursorPos(x, y);

		};

	visitor->builtin_functions["is_mouse_button_pressed"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("button"))->value;

		int button = val->get_i();
		bool is_pressed = (GetAsyncKeyState(button) & 0x8000) != 0;
		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(flx_bool(is_pressed)));

		};
}

void ModuleInput::key_update_loop() {
	while (running) {
		{
			std::lock_guard<std::mutex> lock(state_mutex);
			for (int i = 0; i < KEY_COUNT; ++i) {
				bool current = GetAsyncKeyState(i) & 0x8000;
				previous_key_state[i] = previous_key_state[i] || current;
				current_key_state[i] = current;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void ModuleInput::start() {
	if (!running) {
		running = true;
		key_update_thread = std::thread(&ModuleInput::key_update_loop, this);
	}
}

void ModuleInput::stop() {
	if (running) {
		running = false;
		if (key_update_thread.joinable()) {
			key_update_thread.join();
		}
	}
}

void ModuleInput::register_functions(visitor::Compiler* visitor) {}

void ModuleInput::register_functions(vm::VirtualMachine* vm) {}
