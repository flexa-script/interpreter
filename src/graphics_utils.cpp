#include <iostream>

#include "graphics_utils.hpp"

using namespace utils;

Image::~Image() {
	if (bitmap) {
		DeleteObject(bitmap);
	}
}

Image* Image::load_image(const std::string& filename) {
	std::wstring wfilename(filename.begin(), filename.end());

	HBITMAP hbm_image = (HBITMAP)LoadImage(GetModuleHandle(NULL), wfilename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!hbm_image) {
		return nullptr;
	}

	BITMAP bm;
	GetObject(hbm_image, sizeof(BITMAP), &bm);

	return new Image{ hbm_image, bm.bmWidth, bm.bmHeight };
}

Font::~Font() {
	if (font) {
		DeleteObject(font);
	}
}

Font* Font::create_font(int size, const std::string& name, int weight, bool italic, bool underline, bool strike, int orientation) {
	std::wstring wfont_name(name.begin(), name.end());

	HFONT font = CreateFont(
		size, 0, 0, orientation, weight, italic, underline, strike, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, wfont_name.c_str()
	);

	return new Font{ font, size, name, orientation, weight, italic, underline, strike };
}

std::map<HWND, Window*> Window::hwnd_map;

Window::Window()
	: hwnd(nullptr), hdc(nullptr), hbm_back_buffer(nullptr),
	hdc_back_buffer(nullptr), width(0), height(0) {}

Window::~Window() {
	if (hdc) {
		DeleteObject(hdc);
	}
	if (hbm_back_buffer) {
		DeleteObject(hbm_back_buffer);
	}
	if (hdc_back_buffer) {
		DeleteDC(hdc_back_buffer);
	}
	if (hwnd) {
		DestroyWindow(hwnd);
	}
	auto it = hwnd_map.find(hwnd);
	if (it != hwnd_map.end()) {
		hwnd_map.erase(it);
	}
}

bool Window::initialize(const std::string& title, int width, int height) {
	std::wstring wtitle(title.begin(), title.end());
	this->width = width;
	this->height = height;

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = window_proc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = wtitle.c_str();

	if (!RegisterClass(&wc)) {
		return false;
	}

	hwnd = CreateWindowEx(0, wc.lpszClassName, wtitle.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width + 16, height + 39,
		NULL, NULL, GetModuleHandle(NULL), this);

	if (!hwnd) {
		return false;
	}

	ShowWindow(hwnd, SW_SHOW);

	hdc = GetDC(hwnd);

	hdc_back_buffer = CreateCompatibleDC(hdc);
	hbm_back_buffer = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hdc_back_buffer, hbm_back_buffer);

	hwnd_map[hwnd] = this;

	return true;
}

int Window::get_width() {
	return width;
}

int Window::get_height() {
	return height;
}

void Window::clear_screen(COLORREF color) {
	if (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (hwnd_map.empty()) {
				PostQuitMessage(0);
			}
		}
		else {
			HBRUSH hBrush = CreateSolidBrush(color);
			RECT rect = { 0, 0, width, height };
			FillRect(hdc_back_buffer, &rect, hBrush);
			DeleteObject(hBrush);
		}
	}
	else {
		quit = true;
	}
}

SIZE Window::get_text_size(const std::string& text, Font* font) {
	RECT rect = { 0, 0, 0, 0 };
	std::wstring wtext(text.begin(), text.end());

	HFONT old_font = (HFONT)SelectObject(hdc_back_buffer, font);

	DrawText(hdc_back_buffer, wtext.c_str(), -1, &rect, DT_CALCRECT);

	SelectObject(hdc_back_buffer, old_font);
	DeleteObject(font);

	return SIZE{ rect.right - rect.left, rect.bottom - rect.top };
}

void Window::draw_text(int x, int y, const std::string& text, COLORREF color, Font* font) {
	HFONT old_font = (HFONT)SelectObject(hdc_back_buffer, font->font);
	SetTextColor(hdc_back_buffer, color);
	SetBkMode(hdc_back_buffer, TRANSPARENT);

	std::wstring wtext(text.begin(), text.end());

	TextOut(hdc_back_buffer, x, y, wtext.c_str(), wtext.length());

	SelectObject(hdc_back_buffer, old_font);
}

void Window::draw_image(Image* image, int x, int y) {
	if (!image) {
		std::cerr << "null image" << std::endl;
		return;
	}

	HDC hdc_image = CreateCompatibleDC(hdc_back_buffer);
	HGDIOBJ old_obj = SelectObject(hdc_image, image->bitmap);

	BitBlt(hdc_back_buffer, x, y, image->width, image->height, hdc_image, 0, 0, SRCCOPY);

	SelectObject(hdc_image, old_obj);
	DeleteDC(hdc_image);
}

void Window::draw_pixel(int x, int y, COLORREF color) {
	SetPixel(hdc_back_buffer, x, y, color);
}

void Window::draw_line(int x1, int y1, int x2, int y2, COLORREF color) {
	HPEN pen = CreatePen(PS_SOLID, 1, color);
	HPEN old_pen = (HPEN)SelectObject(hdc_back_buffer, pen);
	MoveToEx(hdc_back_buffer, x1, y1, nullptr);
	LineTo(hdc_back_buffer, x2, y2);
	SelectObject(hdc_back_buffer, old_pen);
	DeleteObject(pen);
}

void Window::draw_rect(int x, int y, int width, int height, COLORREF color) {
	HPEN pen = CreatePen(PS_SOLID, 1, color);
	HPEN old_pen = (HPEN)SelectObject(hdc_back_buffer, pen);
	HBRUSH old_brush = (HBRUSH)SelectObject(hdc_back_buffer, GetStockObject(NULL_BRUSH));
	Rectangle(hdc_back_buffer, x, y, x + width, y + height);
	SelectObject(hdc_back_buffer, old_pen);
	SelectObject(hdc_back_buffer, old_brush);
	DeleteObject(pen);
}

void Window::fill_rect(int x, int y, int width, int height, COLORREF color) {
	RECT rect = { x, y, x + width, y + height };
	HBRUSH brush = CreateSolidBrush(color);
	FillRect(hdc_back_buffer, &rect, brush);
	DeleteObject(brush);
}

void Window::draw_circle(int xc, int yc, int radius, COLORREF color) {
	HPEN pen = CreatePen(PS_SOLID, 1, color);
	HPEN old_pen = (HPEN)SelectObject(hdc_back_buffer, pen);
	HBRUSH old_brush = (HBRUSH)SelectObject(hdc_back_buffer, GetStockObject(NULL_BRUSH));
	Ellipse(hdc_back_buffer, xc - radius, yc - radius, xc + radius, yc + radius);
	SelectObject(hdc_back_buffer, old_pen);
	SelectObject(hdc_back_buffer, old_brush);
	DeleteObject(pen);
}

void Window::fill_circle(int xc, int yc, int radius, COLORREF color) {
	HBRUSH brush = CreateSolidBrush(color);
	HBRUSH old_brush = (HBRUSH)SelectObject(hdc_back_buffer, brush);
	HPEN old_pen = (HPEN)SelectObject(hdc_back_buffer, GetStockObject(NULL_PEN));
	Ellipse(hdc_back_buffer, xc - radius, yc - radius, xc + radius, yc + radius);
	SelectObject(hdc_back_buffer, old_pen);
	SelectObject(hdc_back_buffer, old_brush);
	DeleteObject(brush);
}

void Window::update() {
	BitBlt(hdc, 0, 0, width, height, hdc_back_buffer, 0, 0, SRCCOPY);
}

bool Window::is_quit() {
	return quit;
}

void Window::resize_back_buffer() {
	if (hbm_back_buffer) {
		DeleteObject(hbm_back_buffer);
	}
	if (hdc_back_buffer) {
		DeleteDC(hdc_back_buffer);
	}

	hdc_back_buffer = CreateCompatibleDC(hdc);
	hbm_back_buffer = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hdc_back_buffer, hbm_back_buffer);
}

LRESULT Window::handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) {
	switch (umsg) {
	case WM_SIZE: {
		RECT rect;
		GetClientRect(hwnd, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
		resize_back_buffer();
		break;
	}
	case WM_CLOSE:
	case WM_DESTROY:
		hwnd_map.erase(hwnd);
		if (hwnd_map.empty()) {
			PostQuitMessage(0);
		}
		quit = true;
		return 0;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		BitBlt(ps.hdc, 0, 0, width, height, hdc_back_buffer, 0, 0, SRCCOPY);
		EndPaint(hwnd, &ps);
		return 0;
	}
	}
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

LRESULT CALLBACK Window::window_proc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	Window* windows;
	
	if (umsg == WM_CREATE) {
		windows = reinterpret_cast<Window*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windows));
	}
	else {
		windows = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (windows) {
		return windows->handle_message(umsg, wparam, lparam);
	}

	if (umsg == WM_CLOSE) {
		hwnd_map.erase(hwnd);
		if (hwnd_map.empty()) {
			PostQuitMessage(0);
		}
		return 0;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

