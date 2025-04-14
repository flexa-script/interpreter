#include "md_graphics.hpp"

#include <memory>

#include "interpreter.hpp"
#include "semantic_analysis.hpp"
#include "constants.hpp"

#if defined(_WIN32) || defined(WIN32)
#include "graphics_utils_win.hpp"
#else
#include "graphics_utils_niy.hpp"
#endif // defined(_WIN32) || defined(WIN32)

using namespace core::modules;
using namespace core::runtime;
using namespace core::analysis;

ModuleGraphics::ModuleGraphics() {}

ModuleGraphics::~ModuleGraphics() = default;

void ModuleGraphics::register_functions(SemanticAnalyser* visitor) {
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
	visitor->builtin_functions["destroy_font"] = nullptr;
	visitor->builtin_functions["destroy_image"] = nullptr;
	visitor->builtin_functions["is_quit"] = nullptr;
}

void ModuleGraphics::register_functions(Interpreter* visitor) {

	visitor->builtin_functions["create_window"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("title"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("width"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("height"))->get_value()
		};

		// initialize window struct values
		RuntimeValue* win = visitor->allocate_value(new RuntimeValue(Type::T_STRUCT));

		flx_struct str = flx_struct();
		str["title"] = visitor->allocate_value(new RuntimeValue(vals[0]));
		str["width"] = visitor->allocate_value(new RuntimeValue(vals[1]));
		str["height"] = visitor->allocate_value(new RuntimeValue(vals[2]));

		// create a new window graphic engine
		str[INSTANCE_ID_NAME] = visitor->allocate_value(new RuntimeValue(Type::T_INT));
		str[INSTANCE_ID_NAME]->set(flx_int(new Window()));

		// initialize window graphic engine and return value
		auto res = ((Window*)str[INSTANCE_ID_NAME]->get_i())->initialize(
			str["title"]->get_s(),
			(int)str["width"]->get_i(),
			(int)str["height"]->get_i()
		);

		win->set(str, "Window", Constants::STD_NAMESPACE);

		if (!res) {
			win->set_null();
		}

		visitor->current_expression_value = win;

		};

	visitor->builtin_functions["clear_screen"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!win->get_str()[INSTANCE_ID_NAME]->get_i()) {
			throw std::runtime_error("Window is corrupted");
		}
		int r, g, b;
		r = (int)vals[1]->get_str()["r"]->get_i();
		g = (int)vals[1]->get_str()["g"]->get_i();
		b = (int)vals[1]->get_str()["b"]->get_i();
		((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->clear_screen(Color(r, g, b));

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["get_current_width"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value();

		RuntimeValue* win = val;
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!win->get_str()[INSTANCE_ID_NAME]->get_i()) {
			throw std::runtime_error("Window is corrupted");
		}
		visitor->current_expression_value=visitor->allocate_value(new RuntimeValue(flx_int(((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->get_width())));

		};

	visitor->builtin_functions["get_current_height"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value();

		RuntimeValue* win = val;
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!win->get_str()[INSTANCE_ID_NAME]->get_i()) {
			throw std::runtime_error("Window is corrupted");
		}
		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(flx_int(((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->get_height())));

		};

	visitor->builtin_functions["draw_pixel"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
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
		((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_pixel(x, y, Color(r, g, b));

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["draw_line"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x1"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y1"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x2"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y2"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
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
		((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_line(x1, y1, x2, y2, Color(r, g, b));

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["draw_rect"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("width"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("height"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
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
		((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_rect(x, y, width, height, Color(r, g, b));

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["fill_rect"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("width"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("height"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
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
		((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->fill_rect(x, y, width, height, Color(r, g, b));

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["draw_circle"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("xc"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("yc"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("radius"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		int xc, yc, radius, r, g, b;
		xc = (int)vals[1]->get_i();
		yc = (int)vals[2]->get_i();
		radius = (int)vals[3]->get_i();
		r = (int)vals[4]->get_str()["r"]->get_i();
		g = (int)vals[4]->get_str()["g"]->get_i();
		b = (int)vals[4]->get_str()["b"]->get_i();
		((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_circle(xc, yc, radius, Color(r, g, b));

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["fill_circle"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("xc"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("yc"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("radius"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		int xc, yc, radius, r, g, b;
		xc = (int)vals[1]->get_i();
		yc = (int)vals[2]->get_i();
		radius = (int)vals[3]->get_i();
		r = (int)vals[4]->get_str()["r"]->get_i();
		g = (int)vals[4]->get_str()["g"]->get_i();
		b = (int)vals[4]->get_str()["b"]->get_i();
		((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->fill_circle(xc, yc, radius, Color(r, g, b));

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["create_font"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("size"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("name"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("weight"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("italic"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("underline"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("strike"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("orientation"))->get_value()
		};

		// initialize image struct values
		RuntimeValue* font_value = visitor->allocate_value(new RuntimeValue(Type::T_STRUCT));

		auto str = flx_struct();
		str["size"] = visitor->allocate_value(new RuntimeValue(vals[0]));
		str["name"] = visitor->allocate_value(new RuntimeValue(vals[1]));
		str["weight"] = visitor->allocate_value(new RuntimeValue(vals[2]));
		str["italic"] = visitor->allocate_value(new RuntimeValue(vals[3]));
		str["underline"] = visitor->allocate_value(new RuntimeValue(vals[4]));
		str["strike"] = visitor->allocate_value(new RuntimeValue(vals[5]));
		str["orientation"] = visitor->allocate_value(new RuntimeValue(vals[6]));

		auto font = Font::create_font(
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
		str[INSTANCE_ID_NAME] = visitor->allocate_value(new RuntimeValue(Type::T_INT));
		str[INSTANCE_ID_NAME]->set(flx_int(font));

		font_value->set(str, "Font", Constants::STD_NAMESPACE);

		visitor->current_expression_value = font_value;

		};

	visitor->builtin_functions["draw_text"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("text"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("color"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("font"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		int x = (int)vals[1]->get_i();
		int y = (int)vals[2]->get_i();
		std::string text = vals[3]->get_s();
		int r = (int)vals[4]->get_str()["r"]->get_i();
		int g = (int)vals[4]->get_str()["g"]->get_i();
		int b = (int)vals[4]->get_str()["b"]->get_i();

		RuntimeValue* font_value = vals[5];
		if (TypeUtils::is_void(font_value->type)) {
			throw std::runtime_error("font is null");
		}
		Font* font = (Font*)font_value->get_str()[INSTANCE_ID_NAME]->get_i();
		if (!font) {
			throw std::runtime_error("there was an error handling font");
		}

		((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->draw_text(x, y, text, Color(r, g, b), font);

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["get_text_size"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("text"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("font"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("Window is null");
		}
		if (!((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
			throw std::runtime_error("Window is corrupted");
		}
		std::string text = vals[1]->get_s();
		RuntimeValue* font_value = vals[2];
		if (TypeUtils::is_void(font_value->type)) {
			throw std::runtime_error("font is null");
		}
		Font* font = (Font*)font_value->get_str()[INSTANCE_ID_NAME]->get_i();
		if (!font) {
			throw std::runtime_error("there was an error handling font");
		}

		auto point = ((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->get_text_size(text, font);

		flx_struct str = flx_struct();
		str["width"] = visitor->allocate_value(new RuntimeValue(flx_int(point.width * 2 * 0.905)));
		str["height"] = visitor->allocate_value(new RuntimeValue(flx_int(point.height * 2 * 0.875)));

		RuntimeValue* res = visitor->allocate_value(new RuntimeValue(str, "Size", Constants::STD_NAMESPACE));

		visitor->current_expression_value = res;

		};

	visitor->builtin_functions["load_image"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->get_value();

		// initialize image struct values
		RuntimeValue* img = visitor->allocate_value(new RuntimeValue(Type::T_STRUCT));

		auto str = flx_struct();
		str["path"] = visitor->allocate_value(new RuntimeValue(val));

		// loads image
		auto image = Image::load_image(str["path"]->get_s());
		if (!image) {
			throw std::runtime_error("there was an error loading image");
		}
		str[INSTANCE_ID_NAME] = visitor->allocate_value(new RuntimeValue(Type::T_INT));
		str[INSTANCE_ID_NAME]->set(flx_int(image));

		str["width"] = visitor->allocate_value(new RuntimeValue(Type::T_INT));
		str["width"]->set(flx_int(image->width));
		str["height"] = visitor->allocate_value(new RuntimeValue(Type::T_INT));
		str["height"]->set(flx_int(image->height));

		img->set(str, "Image", Constants::STD_NAMESPACE);

		visitor->current_expression_value = img;

		};

	visitor->builtin_functions["draw_image"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("image"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("x"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("y"))->get_value()
		};

		RuntimeValue* win = vals[0];
		if (TypeUtils::is_void(win->type)) {
			throw std::runtime_error("window is null");
		}
		auto window = ((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i());
		if (!window) {
			throw std::runtime_error("there was an error handling window");
		}
		RuntimeValue* img = vals[1];
		if (TypeUtils::is_void(img->type)) {
			throw std::runtime_error("window is null");
		}
		auto image = ((Image*)img->get_str()[INSTANCE_ID_NAME]->get_i());
		if (!image) {
			throw std::runtime_error("there was an error handling image");
		}
		int x = (int)vals[2]->get_i();
		int y = (int)vals[3]->get_i();
		window->draw_image(image, x, y);

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["update"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value();

		RuntimeValue* win = val;
		if (!TypeUtils::is_void(win->type)) {
			if (((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
				((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->update();
			}
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["destroy_window"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value();

		RuntimeValue* win = val;
		if (!TypeUtils::is_void(win->type)) {
			Window* window = (Window*)win->get_str()[INSTANCE_ID_NAME]->get_i();
			if (window) {
				delete window;
				win->set_null();
			}
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["destroy_font"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("font"))->get_value();

		RuntimeValue* font_value = val;
		if (!TypeUtils::is_void(font_value->type)) {
			Font* font = (Font*)font_value->get_str()[INSTANCE_ID_NAME]->get_i();
			if (font) {
				delete font;
				font_value->set_null();
			}
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["destroy_image"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("image"))->get_value();

		RuntimeValue* img_value = val;
		if (!TypeUtils::is_void(img_value->type)) {
			Image* img = (Image*)img_value->get_str()[INSTANCE_ID_NAME]->get_i();
			if (img) {
				delete img;
				img_value->set_null();
			}
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["is_quit"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		RuntimeValue* win = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("window"))->get_value();
		auto val = visitor->allocate_value(new RuntimeValue(Type::T_BOOL));
		if (!TypeUtils::is_void(win->type)) {
			if (((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())) {
				val->set(flx_bool(((Window*)win->get_str()[INSTANCE_ID_NAME]->get_i())->is_quit()));
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
