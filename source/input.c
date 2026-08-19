#include <wiiuse/wpad.h>
#include "input.h"

void input_init(void) {
    WPAD_Init();
}

int input_scan(void) {
    WPAD_ScanPads();
    return 1;
}

int input_thrust_pressed(void) {
    u32 held = WPAD_ButtonsHeld(0);
    return (held & WPAD_BUTTON_A) != 0;
}

int input_home_pressed(void) {
    u32 pressed = WPAD_ButtonsDown(0);
    return (pressed & WPAD_BUTTON_HOME) != 0;
}

int input_start_pressed(void) {
    u32 pressed = WPAD_ButtonsDown(0);
    return (pressed & WPAD_BUTTON_A) != 0;
}

int input_left_pressed(void) {
    u32 pressed = WPAD_ButtonsDown(0);
    return (pressed & WPAD_BUTTON_LEFT) != 0;
}

int input_right_pressed(void) {
    u32 pressed = WPAD_ButtonsDown(0);
    return (pressed & WPAD_BUTTON_RIGHT) != 0;
}
