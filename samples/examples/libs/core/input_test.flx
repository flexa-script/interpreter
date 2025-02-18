using cp.core.input;

as namespace cp;

fun test_key_press() {
    println("Press 'A' to continue...");
    while (not is_key_pressed(KEY_A)) {}
    println("Key 'A' pressed!\n");
	
    println("Press NumPad0 to continue...");
    while (not is_key_pressed(KEY_NUMPAD_0)) {}
    println("Key NumPad0 pressed!\n");

    println("Press NumPad0 F1 continue...");
    while (not is_key_pressed(KEY_F1)) {}
    println("Key F1 pressed!\n");
}

fun test_mouse_position() {
    println("Move the mouse to change its position...");
    println("Press ESC to stop");
    var pos = get_mouse_position();
    while (not is_key_pressed(KEY_ESC)) {
        var new_pos = get_mouse_position();
        if (new_pos.x != pos.x or new_pos.y != pos.y) {
            println("Mouse moved to position: ", new_pos.x, ", ", new_pos.y);
            pos = new_pos;
        }
    }
    println();
}

fun test_mouse_position_set() {
    set_mouse_position(1920/2, 1080/2);
    println("Mouse moved to center\n");
}

fun test_mouse_click() {
    println("Click the left mouse button to continue...");
    while (not is_mouse_button_pressed(MOUSE_LEFT_BUTTON)) {}
    println("Mouse left button clicked!\n");
    println("Click the right mouse button to continue...");
    while (not is_mouse_button_pressed(MOUSE_RIGHT_BUTTON)) {}
    println("Mouse right button clicked!\n");
    println("Click the middle mouse button to continue...");
    while (not is_mouse_button_pressed(MOUSE_MIDDLE_BUTTON)) {}
    println("Mouse middle button clicked!\n");
}

fun run_all_tests() {
	test_key_press();
	test_mouse_position();
	test_mouse_position_set();
	test_mouse_click();
}

run_all_tests();
