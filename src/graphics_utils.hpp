#ifndef GRAPHICS_UTILS_HPP
#define GRAPHICS_UTILS_HPP

#include <Windows.h>
#include <unordered_map>
#include <map>
#include <string>

namespace utils {

	struct Image {
		HBITMAP bitmap;
		int width;
		int height;
		~Image();
		static Image* load_image(const std::string& filename);
	};

	struct Font {
		HFONT font;
		int size;
		std::string name;
		int orientation;
		int weight;
		bool italic;
		bool underline;
		bool strike;
		~Font();
		static Font* create_font(int size = 20, const std::string& name = "Arial", int weight = 0,
			bool italic = false, bool underline = false, bool strike = false, int orientation = 0);
	};

	class Window {
	private:
		HWND hwnd;
		HDC hdc;
		HBITMAP hbm_back_buffer;
		HDC hdc_back_buffer;
		long width, height;
		MSG msg = { 0 };
		bool quit = false;

		static std::map<HWND, Window*> hwnd_map;

	public:
		Window();
		~Window();

		bool initialize(const std::string& title, int width, int height);
		int get_width();
		int get_height();
		void clear_screen(COLORREF color);
		SIZE get_text_size(const std::string& text, Font* font);
		void draw_text(int x, int y, const std::string& text, COLORREF color, Font* font);
		void draw_image(Image* image, int x, int y);
		void draw_pixel(int x, int y, COLORREF color);
		void draw_line(int x1, int y1, int x2, int y2, COLORREF color);
		void draw_rect(int x, int y, int width, int height, COLORREF color);
		void fill_rect(int x, int y, int width, int height, COLORREF color);
		void draw_circle(int xc, int yc, int radius, COLORREF color);
		void fill_circle(int xc, int yc, int radius, COLORREF color);
		void update();
		bool is_quit();
		void resize_back_buffer();

		static LRESULT CALLBACK window_proc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	private:
		LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);
	};
}

#endif // !GRAPHICS_UTILS_HPP
