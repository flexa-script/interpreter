#ifndef MD_INPUT_HPP
#define MD_INPUT_HPP

#include <thread>
#include <atomic>
#include <mutex>
#include <array>

#include "module.hpp"

namespace core {

	namespace modules {

		class ModuleInput : public Module {
		public:
			static const int KEY_COUNT = 256;

		private:
			std::array<bool, KEY_COUNT> previous_key_state = { false };
			std::array<bool, KEY_COUNT> current_key_state = { false };
			std::thread key_update_thread;
			std::mutex state_mutex;
			std::atomic<bool> running;

			void key_update_loop();

		public:
			ModuleInput();
			~ModuleInput();

			void register_functions(analysis::SemanticAnalyser* visitor) override;
			void register_functions(runtime::Interpreter* visitor) override;

			void start();
			void stop();
		};

	}

}

#endif // !MD_INPUT_HPP
