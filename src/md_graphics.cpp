#include "md_graphics.hpp"

#include "interpreter.hpp"
#include "semantic_analysis.hpp"

using namespace modules;
using namespace visitor;

ModuleGraphics::ModuleGraphics() {}

ModuleGraphics::~ModuleGraphics() = default;

void ModuleGraphics::register_functions(visitor::SemanticAnalyser* visitor) {
	visitor->builtin_functions["create_window"] = nullptr;
	visitor->builtin_functions["get_current_width"] = nullptr;
	visitor->builtin_functions["get_current_height"] = nullptr;
	visitor->builtin_functions["clear_screen"] = nullptr;
	visitor->builtin_functions["get_text_size"] = nullptr;
	visitor->builtin_functions["draw_text"] = nullptr;
	visitor->builtin_functions["draw_pixel"] = nullptr;
	visitor->builtin_functions["draw_line"] = nullptr;
	visitor->builtin_functions["draw_rect"] = nullptr;
	visitor->builtin_functions["fill_rect"] = nullptr;
	visitor->builtin_functions["draw_circle"] = nullptr;
	visitor->builtin_functions["fill_circle"] = nullptr;
	visitor->builtin_functions["create_font"] = nullptr;
	visitor->builtin_functions["load_image"] = nullptr;
	visitor->builtin_functions["draw_image"] = nullptr;
	visitor->builtin_functions["update"] = nullptr;
	visitor->builtin_functions["destroy_window"] = nullptr;
	visitor->builtin_functions["is_quit"] = nullptr;
}

void ModuleGraphics::register_functions(visitor::Interpreter* visitor) {

	visitor->builtin_functions["create_window"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("title"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("width"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("height"))->value
		};

		// initialize window struct values
		RuntimeValue* win = visitor->alocate_value(new RuntimeValue(parser::Type::T_STRUCT));

		flx_struct str = flx_struct();
		str["title"] = visitor->alocate_value(new RuntimeValue(vals[0]));
		str["width"] = visitor->alocate_value(new RuntimeValue(vals[1]));
		str["height"] = visitor->alocate_value(new RuntimeValue(vals[2]));

		// create a new window graphic engine
		str[INSTANCE_ID_NAME] = visitor->alocate_value(new RuntimeValue(parser::Type::T_INT));
		str[INSTANCE_ID_NAME]->set(flx_int(new utils::Window()));

		// initialize window graphic engine and return value
		auto res = ((utils::Window*)str[INSTANCE_ID_NAME]->get_i())->initialize(
			str["title"]->get_s(),
			(int)str["width"]->get_i(),
			(int)str["height"]->get_i()
		);

		win->set(str, "Window", language_namespace);

		if (!res) {
			win->set_null();
		}

		visitor->current_expression_value = win;

		};

	visitor->builtin_functions["clear_screen"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!win->get_str()[INSTANCE_ID_NAME]->get_i()) {
			throw std::runtime_error("Window is corrupted");
		}
		int r, g, b;
		r = (int)vals[1]->get_str()["r"]->get_i();
		g = (int)vals[1]->get_str()["g"]->get_i();
		b = (int)vals[1]->get_str()["b"]->get_i();
		((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->clear_screen(RGB(r, g, b));

		};

	visitor->builtin_functions["get_current_width"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value;

		RuntimeValue* win = val;
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!win->get_str()[INSTANCE_ID_NAME]->get_i()) {
			throw std::runtime_error("Window is corrupted");
		}
		visitor->current_expression_value=visitor->alocate_value(new RuntimeValue(flx_int(((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->get_width())));

		};

	visitor->builtin_functions["get_current_height"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value;

		RuntimeValue* win = val;
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!win->get_str()[INSTANCE_ID_NAME]->get_i()) {
			throw std::runtime_error("Window is corrupted");
		}
		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(flx_int(((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->get_height())));

		};

	visitor->builtin_functions["draw_pixel"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!win->get_str()[INSTANCE_ID_NAME]->get_i()) {
			throw std::runtime_error("Window is corrupted");
		}
		int x, y, r, g, b;
		x = (int)vals[1]->get_i();
		y = (int)vals[2]->get_i();
		r = (int)vals[3]->get_str()["r"]->get_i();
		g = (int)vals[3]->get_str()["g"]->get_i();
		b = (int)vals[3]->get_str()["b"]->get_i();
		((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_pixel(x, y, RGB(r, g, b));

		};

	visitor->builtin_functions["draw_line"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x1"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y1"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x2"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y2"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!win->get_str()[INSTANCE_ID_NAME]->get_i()) {
			throw std::runtime_error("Window is corrupted");
		}
		int x1, y1, x2, y2, r, g, b;
		x1 = (int)vals[1]->get_i();
		y1 = (int)vals[2]->get_i();
		x2 = (int)vals[3]->get_i();
		y2 = (int)vals[4]->get_i();
		r = (int)vals[5]->get_str()["r"]->get_i();
		g = (int)vals[5]->get_str()["g"]->get_i();
		b = (int)vals[5]->get_str()["b"]->get_i();
		((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_line(x1, y1, x2, y2, RGB(r, g, b));

		};

	visitor->builtin_functions["draw_rect"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("width"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("height"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		int x, y, width, height, r, g, b;
		x = (int)vals[1]->get_i();
		y = (int)vals[2]->get_i();
		width = (int)vals[3]->get_i();
		height = (int)vals[4]->get_i();
		r = (int)vals[5]->get_str()["r"]->get_i();
		g = (int)vals[5]->get_str()["g"]->get_i();
		b = (int)vals[5]->get_str()["b"]->get_i();
		((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_rect(x, y, width, height, RGB(r, g, b));

		};

	visitor->builtin_functions["fill_rect"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("width"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("height"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		int x, y, width, height, r, g, b;
		x = (int)vals[1]->get_i();
		y = (int)vals[2]->get_i();
		width = (int)vals[3]->get_i();
		height = (int)vals[4]->get_i();
		r = (int)vals[5]->get_str()["r"]->get_i();
		g = (int)vals[5]->get_str()["g"]->get_i();
		b = (int)vals[5]->get_str()["b"]->get_i();
		((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->fill_rect(x, y, width, height, RGB(r, g, b));

		};

	visitor->builtin_functions["draw_circle"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("xc"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("yc"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("radius"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		int xc, yc, radius, r, g, b;
		xc = (int)vals[1]->get_i();
		yc = (int)vals[2]->get_i();
		radius = (int)vals[3]->get_i();
		r = (int)vals[4]->get_str()["r"]->get_i();
		g = (int)vals[4]->get_str()["g"]->get_i();
		b = (int)vals[4]->get_str()["b"]->get_i();
		((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_circle(xc, yc, radius, RGB(r, g, b));

		};

	visitor->builtin_functions["fill_circle"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("xc"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("yc"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("radius"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		int xc, yc, radius, r, g, b;
		xc = (int)vals[1]->get_i();
		yc = (int)vals[2]->get_i();
		radius = (int)vals[3]->get_i();
		r = (int)vals[4]->get_str()["r"]->get_i();
		g = (int)vals[4]->get_str()["g"]->get_i();
		b = (int)vals[4]->get_str()["b"]->get_i();
		((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->fill_circle(xc, yc, radius, RGB(r, g, b));

		};

	visitor->builtin_functions["create_font"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("size"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("name"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("weight"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("italic"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("underline"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("strike"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("orientation"))->value
		};

		// initialize image struct values
		RuntimeValue* font_value = visitor->alocate_value(new RuntimeValue(parser::Type::T_STRUCT));

		auto str = flx_struct();
		str["size"] = visitor->alocate_value(new RuntimeValue(vals[0]));
		str["name"] = visitor->alocate_value(new RuntimeValue(vals[1]));
		str["weight"] = visitor->alocate_value(new RuntimeValue(vals[2]));
		str["italic"] = visitor->alocate_value(new RuntimeValue(vals[3]));
		str["underline"] = visitor->alocate_value(new RuntimeValue(vals[4]));
		str["strike"] = visitor->alocate_value(new RuntimeValue(vals[5]));
		str["orientation"] = visitor->alocate_value(new RuntimeValue(vals[6]));

		auto font = utils::Font::create_font(
			str["size"]->get_i(),
			str["name"]->get_s(),
			str["weight"]->get_i(),
			str["italic"]->get_b(),
			str["underline"]->get_b(),
			str["strike"]->get_b(),
			str["orientation"]->get_i()
		);
		if (!font) {
			throw std::runtime_error("there was an error creating font");
		}
		str[INSTANCE_ID_NAME] = visitor->alocate_value(new RuntimeValue(parser::Type::T_INT));
		str[INSTANCE_ID_NAME]->set(flx_int(font));

		font_value->set(str, "Font", language_namespace);

		visitor->current_expression_value = font_value;

		};

	visitor->builtin_functions["draw_text"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("text"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("font"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		int x = (int)vals[1]->get_i();
		int y = (int)vals[2]->get_i();
		std::string text = vals[3]->get_s();
		int r = (int)vals[4]->get_str()["r"]->get_i();
		int g = (int)vals[4]->get_str()["g"]->get_i();
		int b = (int)vals[4]->get_str()["b"]->get_i();

		RuntimeValue* font_value = vals[5];
		if (parser::is_void(font_value->type)) {
			throw std::exception("font is null");
		}
		utils::Font* font = (utils::Font*)font_value->get_str()[INSTANCE_ID_NAME]->get_i();
		if (!font) {
			throw std::runtime_error("there was an error handling font");
		}

		((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_text(x, y, text, RGB(r, g, b), font);

		};

	visitor->builtin_functions["get_text_size"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("text"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("font"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		std::string text = vals[1]->get_s();
		RuntimeValue* font_value = vals[2];
		if (parser::is_void(font_value->type)) {
			throw std::exception("font is null");
		}
		utils::Font* font = (utils::Font*)font_value->get_str()[INSTANCE_ID_NAME]->get_i();
		if (!font) {
			throw std::runtime_error("there was an error handling font");
		}

		auto point = ((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->get_text_size(text, font);

		flx_struct str = flx_struct();
		str["width"] = visitor->alocate_value(new RuntimeValue(flx_int(point.cx * 2 * 0.905)));
		str["height"] = visitor->alocate_value(new RuntimeValue(flx_int(point.cy * 2 * 0.875)));

		RuntimeValue* res = visitor->alocate_value(new RuntimeValue(str, "Size", language_namespace));

		visitor->current_expression_value = res;

		};

	visitor->builtin_functions["load_image"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->value;

		// initialize image struct values
		RuntimeValue* img = visitor->alocate_value(new RuntimeValue(parser::Type::T_STRUCT));

		auto str = flx_struct();
		str["path"] = visitor->alocate_value(new RuntimeValue(val));

		// loads image
		auto image = utils::Image::load_image(str["path"]->get_s());
		if (!image) {
			throw std::runtime_error("there was an error loading image");
		}
		str[INSTANCE_ID_NAME] = visitor->alocate_value(new RuntimeValue(parser::Type::T_INT));
		str[INSTANCE_ID_NAME]->set(flx_int(image));

		str["width"] = visitor->alocate_value(new RuntimeValue(parser::Type::T_INT));
		str["width"]->set(flx_int(image->width));
		str["height"] = visitor->alocate_value(new RuntimeValue(parser::Type::T_INT));
		str["height"]->set(flx_int(image->height));

		img->set(str, "Image", language_namespace);

		visitor->current_expression_value = img;

		};

	visitor->builtin_functions["draw_image"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("image"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->value
		};

		RuntimeValue* win = vals[0];
		if (parser::is_void(win->type)) {
			throw std::exception("window is null");
		}
		auto window = ((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i());
		if (!window) {
			throw std::runtime_error("there was an error handling window");
		}
		RuntimeValue* img = vals[1];
		if (parser::is_void(img->type)) {
			throw std::exception("window is null");
		}
		auto image = ((utils::Image*)img->get_str()[INSTANCE_ID_NAME]->get_i());
		if (!image) {
			throw std::runtime_error("there was an error handling image");
		}
		int x = (int)vals[2]->get_i();
		int y = (int)vals[3]->get_i();
		window->draw_image(image, x, y);

		};

	visitor->builtin_functions["update"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value;

		RuntimeValue* win = val;
		if (!parser::is_void(win->type)) {
			if (((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
				((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->update();
			}
		}

		};

	visitor->builtin_functions["destroy_window"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value;

		RuntimeValue* win = val;
		if (!parser::is_void(win->type)) {
			if (((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
				((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->~Window();
				win->set_null();
			}
		}

		};

	visitor->builtin_functions["is_quit"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		RuntimeValue* win = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->value;
		auto val = visitor->alocate_value(new RuntimeValue(parser::Type::T_BOOL));
		if (!parser::is_void(win->type)) {
			if (((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
				val->set(flx_bool(((utils::Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->is_quit()));
			}
			else {
				val->set(flx_bool(true));
			}
		}
		else {
			val->set(flx_bool(true));
		}
		visitor->current_expression_value = val;

		};
}

void ModuleGraphics::register_functions(visitor::Compiler* visitor) {}

void ModuleGraphics::register_functions(vm::VirtualMachine* vm) {}
