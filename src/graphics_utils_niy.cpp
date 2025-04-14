#if !defined(_WIN32) && !defined(WIN32)

#include "graphics_utils_niy.hpp"

#include <stdexcept>

using namespace core::modules;

Image* Image::load_image(const std::string& filename) {
	throw std::runtime_error("Not implemented yet");
}

Font* Font::create_font(int size, const std::string& name, int weight, bool italic, bool underline, bool strike, int orientation) {
	throw std::runtime_error("Not implemented yet");

	return new Font{};
}

Window::Window() : width(0), height(0) {}

bool Window::initialize(const std::string& title, int width, int height) {
	throw std::runtime_error("Not implemented yet");

	return true;
}

int Window::get_width() {
	throw std::runtime_error("Not implemented yet");
	return width;
}

int Window::get_height() {
	throw std::runtime_error("Not implemented yet");
	return height;
}

void Window::clear_screen(Color color) {
	throw std::runtime_error("Not implemented yet");
}

TextSize Window::get_text_size(const std::string& text, Font* font) {
	throw std::runtime_error("Not implemented yet");

	return TextSize{};
}

void Window::draw_text(int x, int y, const std::string& text, Color color, Font* font) {
	throw std::runtime_error("Not implemented yet");
}

void Window::draw_image(Image* image, int x, int y) {
	throw std::runtime_error("Not implemented yet");
}

void Window::draw_pixel(int x, int y, Color color) {
	throw std::runtime_error("Not implemented yet");
}

void Window::draw_line(int x1, int y1, int x2, int y2, Color color) {
	throw std::runtime_error("Not implemented yet");
}

void Window::draw_rect(int x, int y, int width, int height, Color color) {
	throw std::runtime_error("Not implemented yet");
}

void Window::fill_rect(int x, int y, int width, int height, Color color) {
	throw std::runtime_error("Not implemented yet");
}

void Window::draw_circle(int xc, int yc, int radius, Color color) {
	throw std::runtime_error("Not implemented yet");
}

void Window::fill_circle(int xc, int yc, int radius, Color color) {
	throw std::runtime_error("Not implemented yet");
}

void Window::update() {
	throw std::runtime_error("Not implemented yet");
}

bool Window::is_quit() {
	throw std::runtime_error("Not implemented yet");
	return quit;
}

#endif // !defined(_WIN32) && !defined(WIN32)
