using cp.core.graphics;
// using cp.std.random;
// using cp.std.math;

var window1: cp::Window = cp::create_window("CP Graphics 1", 800, 450);
var window2: cp::Window = cp::create_window("CP Graphics 2", 400, 225);
var img1: cp::Image = cp::load_image("C:\\Users\\Carlos\\repos\\cp-lang\\interpreter\\samples\\cp-example\\file\\logo.bmp");
var img2: cp::Image = cp::load_image("C:\\Users\\Carlos\\repos\\cp-lang\\interpreter\\samples\\cp-example\\file\\logo-old.bmp");
var font: cp::Font = cp::create_font();
var font2: cp::Font = cp::create_font(30, "Arial", 0, false, false, true);

var fontsize = cp::get_text_size(window2, "center text", font2);
var fw = fontsize.width;
var fh = fontsize.height;
var ww = window2.width;
var wh = window2.height;

while (not cp::is_quit(window1) or not cp::is_quit(window2)) {
	if (not cp::is_quit(window1)) {
		cp::clear_screen(window1, cp::Color{r=0,g=0,b=0});

		cp::draw_pixel(window1, 100, 50, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 101, 50, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 102, 50, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 103, 50, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 104, 50, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 105, 50, cp::Color{r=255,g=0,b=0});
		
		cp::draw_pixel(window1, 105, 51, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 105, 52, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 105, 53, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 105, 54, cp::Color{r=255,g=0,b=0});

		cp::draw_pixel(window1, 100, 55, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 101, 55, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 102, 55, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 103, 55, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 104, 55, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 105, 55, cp::Color{r=255,g=0,b=0});
		
		cp::draw_pixel(window1, 100, 51, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 100, 52, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 100, 53, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window1, 100, 54, cp::Color{r=255,g=0,b=0});

		cp::draw_line(window1, 90, 40, 600, 80, cp::Color{r=0,g=255,b=0});

		cp::draw_rect(window1, 500, 200, 100, 80, cp::Color{r=0,g=255,b=255});
		cp::fill_rect(window1, 200, 200, 100, 80, cp::Color{r=0,g=0,b=255});

		cp::draw_circle(window1, 375, 100, 50, cp::Color{r=255,g=255,b=0});
		cp::fill_circle(window1, 375, 300, 50, cp::Color{r=255,g=0,b=255});

		cp::update(window1);
	}
	else {
		cp::destroy_window(window1);
	}
	if (not cp::is_quit(window2)) {
		cp::clear_screen(window2, cp::Color{r=0,g=0,b=0});
		cp::fill_rect(window2, 5, 5, cp::get_current_width(window2) - 10, cp::get_current_height(window2) - 10, cp::Color{r=255,g=0,b=0});
		cp::draw_pixel(window2, 100, 50, cp::Color{r=255,g=255,b=255});
		cp::draw_image(window2, img1, 150, 50);
		cp::draw_image(window2, img2, 230, 100);
    cp::draw_text(window2, 0, 0, "test", cp::Color{r=255,g=255,b=255}, font);
    cp::draw_text(window2, int(cp::get_current_width(window2) / 2 - fw / 2), int(cp::get_current_height(window2) / 2 - fh / 2), "center text", cp::rgb(0, 255, 0), font2);
		cp::update(window2);
	}
	else {
		cp::destroy_window(window2);
	}
}

cp::destroy_window(window1);
cp::destroy_window(window2);
