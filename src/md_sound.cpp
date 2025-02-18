#include <Windows.h>
#include <mmsystem.h>

#include "md_sound.hpp"

#include "interpreter.hpp"
#include "semantic_analysis.hpp"

#pragma comment(lib, "winmm.lib")

using namespace modules;

ModuleSound::ModuleSound() {}

ModuleSound::~ModuleSound() = default;

void ModuleSound::register_functions(visitor::SemanticAnalyser* visitor) {
	visitor->builtin_functions["play_sound"] = nullptr;
	visitor->builtin_functions["stop_sound_once"] = nullptr;
	visitor->builtin_functions["stop_sound"] = nullptr;
	visitor->builtin_functions["set_volume"] = nullptr;
}

void ModuleSound::register_functions(visitor::Interpreter* visitor) {

	visitor->builtin_functions["play_sound"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->value;

		auto file_path = val->get_s();
		std::wstring wfile_path = std::wstring(file_path.begin(), file_path.end());
		PlaySound(wfile_path.c_str(), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);

		};

	visitor->builtin_functions["play_sound_once"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->value;

		auto file_path = val->get_s();
		std::wstring wfile_path = std::wstring(file_path.begin(), file_path.end());
		PlaySound(wfile_path.c_str(), NULL, SND_ASYNC | SND_FILENAME);

		};

	visitor->builtin_functions["stop_sound"] = [this, visitor]() {
		PlaySound(NULL, 0, 0);

		};

	visitor->builtin_functions["set_volume"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("volume"))->value;

		unsigned long volume = val->get_f() * 65535;
		waveOutSetVolume(0, MAKELONG(volume, volume));

		};
}

void ModuleSound::register_functions(visitor::Compiler* visitor) {}

void ModuleSound::register_functions(vm::VirtualMachine* vm) {}
