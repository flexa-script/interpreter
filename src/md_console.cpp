#include <Windows.h>

#include "md_console.hpp"

#include "interpreter.hpp"
#include "semantic_analysis.hpp"
#include "constants.hpp"

using namespace core::modules;
using namespace core::runtime;
using namespace core::analysis;

ModuleConsole::ModuleConsole() {}

ModuleConsole::~ModuleConsole() = default;

void ModuleConsole::register_functions(SemanticAnalyser* visitor) {
	visitor->builtin_functions["show_console"] = nullptr;
	visitor->builtin_functions["is_console_visible"] = nullptr;
	visitor->builtin_functions["set_console_color"] = nullptr;
	visitor->builtin_functions["set_console_cursor_position"] = nullptr;
	visitor->builtin_functions["set_console_font"] = nullptr;
}

void ModuleConsole::register_functions(Interpreter* visitor) {

	visitor->builtin_functions["show_console"] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("show"))->get_value();

		::ShowWindow(::GetConsoleWindow(), val->get_b());
		
		};

	visitor->builtin_functions["is_console_visible"] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(flx_bool(::IsWindowVisible(::GetConsoleWindow()))));

		};

	visitor->builtin_functions["set_console_color"] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector {
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("background_color"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("foreground_color"))->get_value()
		};

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, vals[0]->get_i() * 0x10 | vals[1]->get_i());

		};

	visitor->builtin_functions["set_console_cursor_position"] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->get_value()
		};

		COORD pos = { vals[0]->get_i(), vals[1]->get_i() };
		HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleCursorPosition(output, pos);

		};

	visitor->builtin_functions["set_console_font"] = [this, visitor]() {
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("font_name"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("font_width"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("font_height"))->get_value()
		};

		auto nfontname = vals[0]->get_s();
		auto pfontname = std::wstring(nfontname.begin(), nfontname.end());
		int pwidth = vals[1]->get_i();
		int pheight = vals[2]->get_i();

		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = pwidth;
		cfi.dwFontSize.Y = pheight;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
#pragma warning(suppress : 4996)
		std::wcscpy(cfi.FaceName, pfontname.c_str());

		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

		};

}
