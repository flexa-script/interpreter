#include "md_input.hpp"

#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#endif // defined(_WIN32) || defined(WIN32)

#include "interpreter.hpp"
#include "semantic_analysis.hpp"
#include "constants.hpp"

using namespace core::modules;
using namespace core::runtime;
using namespace core::analysis;

ModuleInput::ModuleInput() : running(false) {
	start();
}

ModuleInput::~ModuleInput() {
	stop();
}

void ModuleInput::register_functions(SemanticAnalyser* visitor) {
	visitor->builtin_functions["update_key_states"] = nullptr;
	visitor->builtin_functions["is_key_pressed"] = nullptr;
	visitor->builtin_functions["is_key_released"] = nullptr;
	visitor->builtin_functions["get_mouse_position"] = nullptr;
	visitor->builtin_functions["set_mouse_position"] = nullptr;
	visitor->builtin_functions["is_mouse_button_pressed"] = nullptr;
}

void ModuleInput::register_functions(Interpreter* visitor) {

	visitor->builtin_functions["update_key_states"] = [this, visitor]() {

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

#ifdef linux

		throw std::runtime_error("Not implemented yet");
		
#elif  defined(_WIN32) || defined(WIN32)

		previous_key_state = current_key_state;

		for (int i = 0; i < KEY_COUNT; ++i) {
			current_key_state[i] = GetAsyncKeyState(i) & 0x8000;
		}

#endif // linux

	};

	visitor->builtin_functions["is_key_pressed"] = [this, visitor]() {
		bool is_pressed = false;

#ifdef linux

		throw std::runtime_error("Not implemented yet");
		
#elif  defined(_WIN32) || defined(WIN32)

		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("key"))->get_value();

		int key = val->get_i();

		{
			std::lock_guard<std::mutex> lock(state_mutex);
			is_pressed = current_key_state[key];
		}

#endif // linux

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(flx_bool(is_pressed)));

		};

	visitor->builtin_functions["is_key_released"] = [this, visitor]() {
		bool is_released = false;

#ifdef linux
		
		throw std::runtime_error("Not implemented yet");
				
#elif  defined(_WIN32) || defined(WIN32)
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("key"))->get_value();

		int key = val->get_i();

		{
			std::lock_guard<std::mutex> lock(state_mutex);
			is_released = previous_key_state[key] && !current_key_state[key];
			if (is_released) {
				previous_key_state[key] = false;
			}
		}
		
#endif // linux

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(flx_bool(is_released)));

		};

	visitor->builtin_functions["get_mouse_position"] = [this, visitor]() {

		long long x, y = 0;

#ifdef linux
		
		throw std::runtime_error("Not implemented yet");
						
#elif  defined(_WIN32) || defined(WIN32)

		POINT point;
		GetCursorPos(&point);
		x = point.x;
		y = point.y;

#endif // linux

		flx_struct str = flx_struct();
		str["x"] = visitor->allocate_value(new RuntimeValue(flx_int(x * 2 * 0.905)));
		str["y"] = visitor->allocate_value(new RuntimeValue(flx_int(y * 2 * 0.875)));
		RuntimeValue* res = visitor->allocate_value(new RuntimeValue(str, "Point", Constants::STD_NAMESPACE));

		visitor->current_expression_value = res;

		};

	visitor->builtin_functions["set_mouse_position"] = [this, visitor]() {

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

#ifdef linux
		
		throw std::runtime_error("Not implemented yet");
						
#elif  defined(_WIN32) || defined(WIN32)

		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->get_value()
		};

		int x = vals[0]->get_i();
		int y = vals[1]->get_i();
		SetCursorPos(x, y);

#endif // linux

		};

	visitor->builtin_functions["is_mouse_button_pressed"] = [this, visitor]() {

		bool is_pressed = false;

#ifdef linux
				
		throw std::runtime_error("Not implemented yet");
								
#elif  defined(_WIN32) || defined(WIN32)
		
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("button"))->get_value();

		int button = val->get_i();
		
		is_pressed = (GetAsyncKeyState(button) & 0x8000) != 0;

#endif // linux

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(flx_bool(is_pressed)));
		
		};
}

void ModuleInput::key_update_loop() {
	while (running) {
		{
			std::lock_guard<std::mutex> lock(state_mutex);
			for (int i = 0; i < KEY_COUNT; ++i) {
				bool current = false;

#if defined(_WIN32) || defined(WIN32)

				current = GetAsyncKeyState(i) & 0x8000;

#endif // defined(_WIN32) || defined(WIN32)

				previous_key_state[i] = previous_key_state[i] || current;
				current_key_state[i] = current;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
