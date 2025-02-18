using cp.core.graphics;
using cp.core.console;
using cp.core.input;
using cp.core.sound;
using cp.core.datetime;
using cp.std.random;

// include namespace cp;

// global vars
enum {
	ST_MAIN_MENU,
	ST_PLAYING,
	ST_END_GAME
};
var state = ST_MAIN_MENU;

var last_time = cp::clock();
var fps = 0f;
var frame_count = 0;

var running: bool = true;
var board[4][4]: int = {0};
var score = 0;

var game_window: cp::Window;
var color_black = cp::rgb(0, 0, 0);
var color_white = cp::rgb(255, 255, 255);
var color_gray = cp::rgb(187, 173, 160);
var color_dark_gray = cp::rgb(62, 57, 51);

// function declarations
fun render();
fun update();
fun moves_up();
fun moves_down();
fun moves_left();
fun moves_right();
fun is_valid_move(v: bool, boardcopy[4][4]: int): bool;
fun counts_zero(): int;
fun spawn_random(n: int);
fun is_end_game(): bool;

fun main() {
	game_window = cp::create_window("2048", 450, 600);
    cp::randomize(cp::create_date_time().timestamp);
    board[3][3] = 2;

    while (running and not cp::is_quit(game_window)) {
		var current_time = cp::clock();
		var elapsed_time = (current_time - last_time) / 1000;

        frame_count++;
		
        if (elapsed_time >= 1.0) {
            fps = frame_count / elapsed_time;
            frame_count = 0;
            last_time = current_time;

			println("FPS: ", fps);
        }

		update();
		render();
    }
	
	cp::destroy_window(game_window);
}

fun update() {
	if (state == ST_MAIN_MENU) {
		state = ST_PLAYING;
	} else if (state == ST_PLAYING) {
		var boardcopy[4][4]: int = { 0 };
        
        for (var i: int = 0; i < 4; i++) {
            for (var j: int = 0; j < 4; j++) {
                boardcopy[i][j] = board[i][j];
            }
        }
		
		if (cp::is_key_released(cp::KEY_W) or cp::is_key_released(cp::KEY_UP) or cp::is_key_released(cp::KEY_NUMPAD_8)) {
			moves_up();
		}
		else if (cp::is_key_released(cp::KEY_S) or cp::is_key_released(cp::KEY_DOWN) or cp::is_key_released(cp::KEY_NUMPAD_2)) {
			moves_down();
		}
		else if (cp::is_key_released(cp::KEY_A) or cp::is_key_released(cp::KEY_LEFT) or cp::is_key_released(cp::KEY_NUMPAD_4)) {
			moves_left();
		}
		else if (cp::is_key_released(cp::KEY_D) or cp::is_key_released(cp::KEY_RIGHT) or cp::is_key_released(cp::KEY_NUMPAD_6)) {
			moves_right();
		}

		var valid: bool = false;
		valid = is_valid_move(valid, boardcopy);

		if (valid)  {
			var z: int = counts_zero();

			if (z > 0) {
				spawn_random(z);
			}

			z = counts_zero();
			var end_check: bool = true;

			if (z == 0) {
				end_check = is_end_game();
			}
			
			if (not end_check) {
				state = ST_END_GAME;
			}
		}
	} else if (state == ST_END_GAME) {
		state = ST_MAIN_MENU;
	}

}

fun get_color(val: int): Color {
    if (val == 2) {
        return cp::rgb(238, 228, 218);
    } else if (val == 4) {
        return cp::rgb(237, 224, 200);
    } else if (val == 8) {
        return cp::rgb(242, 177, 121);
    } else if (val == 16) {
        return cp::rgb(245, 149, 99);
    } else if (val == 32) {
        return cp::rgb(246, 124, 95);
    } else if (val == 64) {
        return cp::rgb(246, 94, 59);
    } else if (val == 128) {
        return cp::rgb(237, 207, 114);
    } else if (val == 256) {
        return cp::rgb(237, 204, 97);
    } else if (val == 512) {
        return cp::rgb(237, 200, 80);
    } else if (val == 1024) {
        return cp::rgb(237, 197, 63);
    } else if (val == 2048) {
        return cp::rgb(237, 194, 46);
    } else if (val == 4096) {
        return cp::rgb(62, 57, 51);
    } else {
        return cp::rgb(204, 192, 179);
    }
}

fun render() {
	var cw = cp::get_current_width(game_window);
	var ch = cp::get_current_height(game_window);
	
	var title_font = cp::create_font(int((cw + ch) * 0.09));
	var sub_title_font = cp::create_font(int((cw + ch) * 0.04));
	var score_font = cp::create_font(int((cw + ch) * 0.035));
	
	cp::clear_screen(game_window, color_black);
    
	var padh = cw * 0.025;
	var padv = ch * 0.166666666666666666666667;
	var x = padh;
	var y = padv + padh;
	var box_w = cw / 4 - padh * 5 / 4;
	var box_h = (ch - padv) / 4 - padh * 5 / 4;

	cp::fill_rect(game_window, 0, 0, int(cw), int(padv), color_white);
	cp::fill_rect(game_window, 0, int(padv), int(cw), int(ch - padv), color_gray);
	cp::draw_text(game_window, int(x), int(x), "2048", color_dark_gray, title_font);
	cp::draw_text(game_window, int(cw * 0.60), int(x), "SCORE:", color_dark_gray, sub_title_font);

	score = 0;
	for (var i: int = 0; i < 4; i++) {
        for (var j: int = 0; j < 4; j++) {
			score += board[i][j];
			var num_color = get_color(board[i][j]);
			var tx = string(board[i][j]);
			var num_font = cp::create_font(int((cw + ch) * 0.04));
			var font_size = cp::get_text_size(game_window, tx, num_font);
			var fw = font_size.width;
			var fh = font_size.height;

			cp::fill_rect(game_window, int(x), int(y), int(box_w), int(box_h), num_color);
			cp::draw_text(game_window, int(x + box_w / 2 - fw / 2), int(y + box_h / 2 - fh / 2), tx, board[i][j] > 4 ? color_white : color_dark_gray, num_font);
			x += box_w + padh;
        }
		y += box_h + padh;
		x = padh;
    }
	
	cp::draw_text(game_window, int(cw * 0.60), int(padv * 0.60), string(score), color_dark_gray, score_font);

	cp::update(game_window);
}

fun moves_up() {
    for (var j: int = 0; j < 4; j++) {
        // moves everything close together
        for (var l: int = 0; l < 3; l++) {
            for (var k: int = 0; k < 3; k++) {
                if (board[k][j] == 0 and board[k][j] != board[k + 1][j]) {
                    board[k][j] = board[k + 1][j];
                    board[k + 1][j] = 0;
                }
            }
        }

        // adds numbers together
        var i: int = 0;
        if (board[i][j] == board[i + 1][j] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i + 1][j] = board[i + 2][j];
            board[i + 2][j] = board[i + 3][j];
            board[i + 3][j] = 0;
        }

        i = 1;
        if (board[i][j] == board[i + 1][j] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i + 1][j] = board[i + 2][j];
            board[i + 2][j] = 0;
        }

        i = 2;
        if (board[i][j] == board[i + 1][j] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i + 1][j] = 0;
        }
    }
}

fun moves_down() {
    for (var j: int = 0; j < 4; j++) {
        // moves everything close together
        for (var l: int = 0; l < 3; l++) {
            for (var k: int = 3; k > 0; k--) {
                if (board[k][j] == 0 and board[k][j] != board[k - 1][j]) {
                    board[k][j] = board[k - 1][j];
                    board[k - 1][j] = 0;
                }
            }
        }

        // adds numbers together
        var i: int = 3;
        if (board[i][j] == board[i - 1][j] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i - 1][j] = board[i - 2][j];
            board[i - 2][j] = board[i - 3][j];
            board[i - 3][j] = 0;
        }

        i = 2;
        if (board[i][j] == board[i - 1][j] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i - 1][j] = board[i - 2][j];
            board[i - 2][j] = 0;
        }

        i = 1;
        if (board[i][j] == board[i - 1][j] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i - 1][j] = 0;
        }
    }
}

fun moves_left() {
    for (var i: int = 0; i < 4; i++) {
        // moves everything close together
        for (var l: int = 0; l < 3; l++) {
            for (var k: int = 0; k < 3; k++) {
                if (board[i][k] == 0 and board[i][k] != board[i][k + 1]) {
                    board[i][k] = board[i][k + 1];
                    board[i][k + 1] = 0;
                }
            }
        }

        // adds numbers together
        var j: int = 0;
        if (board[i][j] == board[i][j + 1] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i][j + 1] = board[i][j + 2];
            board[i][j + 2] = board[i][j + 3];
            board[i][j + 3] = 0;
        }

        j = 1;
        if (board[i][j] == board[i][j + 1] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i][j + 1] = board[i][j + 2];
            board[i][j + 2] = 0;
        }

        j = 2;
        if (board[i][j] == board[i][j + 1] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i][j + 1] = 0;
        }
    }
}

fun moves_right() {
    for (var i: int = 0; i < 4; i++) {
        // moves everything close together
        for (var l: int = 0; l < 3; l++) {
            for (var k: int = 3; k > 0; k--) {
                if (board[i][k] == 0 and board[i][k] != board[i][k - 1]) {
                    board[i][k] = board[i][k - 1];
                    board[i][k - 1] = 0;
                }
            }
        }

        // adds numbers together
        var j: int = 3;
        if (board[i][j] == board[i][j - 1] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i][j - 1] = board[i][j - 2];
            board[i][j - 2] = board[i][j - 3];
            board[i][j - 3] = 0;
        }

        j = 2;
        if (board[i][j] == board[i][j - 1] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i][j - 1] = board[i][j - 2];
            board[i][j - 2] = 0;
        }

        j = 1;
        if (board[i][j] == board[i][j - 1] and board[i][j] != 0) {
            board[i][j] = 2 * board[i][j];
            board[i][j - 1] = 0;
        }
    }
}

fun is_valid_move(v: bool, boardcopy[4][4]: int): bool {
    for (var i: int = 0; i < 4; i++) {
        for (var j: int = 0; j < 4; j++) {
            if (board[i][j] != boardcopy[i][j]) {
                v = true;
            }
        }
    }

    return v;
}

fun counts_zero(): int {
    var n: int = 0;
    for (var i: int = 0; i < 4; i++) {
        for (var j: int = 0; j < 4; j++) {
            if (board[i][j] == 0) { n++; }
        }
    }

    return n;
}

fun spawn_random(n: int) {
    var spawn: int = 1 + (cp::randi() % n);
    var p: int = 0;
    var spawn2: bool = true;
    for (var i: int = 0; i < 4; i++) {
        for (var j: int = 0; j < 4; j++) {
            if (board[i][j] == 0) {
                p++;
            }
            if (p == spawn and spawn2 == true) {
                board[i][j] = 2;
                spawn2 = false;
            }
        }
    }
}

fun is_end_game(): bool {
    var c1 = false;
    var c2 = true;
    for (var i: int = 0; i < 4; i++) {
        for (var j: int = 0; j < 4; j++) {
            if (j > 0 and board[i][j] == board[i][j - 1]) {
                c1 = true;
            }
            else if (i > 0 and board[i][j] == board[i - 1][j]) {
                c1 = true;
            }
            c2 = c1;
        }
    }

    return c2;
}

if (this == "__default__") {
	main();
}
