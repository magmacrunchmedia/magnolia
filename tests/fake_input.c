#include "fake_input.h"
#include "input.h"

static int down[FAKE_BUTTON_COUNT];

void fake_input_clear(void) {
    for (int i = 0; i < FAKE_BUTTON_COUNT; i++) down[i] = 0;
}

void fake_input_press(FakeButton b) {
    if (b >= 0 && b < FAKE_BUTTON_COUNT) down[b] = 1;
}

void fake_input_only(FakeButton b) {
    fake_input_clear();
    fake_input_press(b);
}

/* --- the input.h surface, as gamestate.c sees it --- */

void input_init(void) { fake_input_clear(); }
int  input_scan(void) { return 1; }

int input_a_pressed(void)       { return down[FAKE_A]; }
int input_back_pressed(void)    { return down[FAKE_B]; }
int input_home_pressed(void)    { return down[FAKE_HOME]; }
int input_button1_pressed(void) { return down[FAKE_1]; }
int input_button2_pressed(void) { return down[FAKE_2]; }
int input_plus_pressed(void)    { return down[FAKE_PLUS]; }
int input_minus_pressed(void)   { return down[FAKE_MINUS]; }

int input_left_pressed(void)    { return down[FAKE_LEFT]; }
int input_right_pressed(void)   { return down[FAKE_RIGHT]; }
int input_up_pressed(void)      { return down[FAKE_UP]; }
int input_down_pressed(void)    { return down[FAKE_DOWN]; }

int input_a_held(void)          { return down[FAKE_A_HELD] || down[FAKE_A]; }

int input_dir_repeat(InputDir dir) {
    switch (dir) {
        case INPUT_DIR_UP:    return down[FAKE_UP];
        case INPUT_DIR_DOWN:  return down[FAKE_DOWN];
        case INPUT_DIR_LEFT:  return down[FAKE_LEFT];
        case INPUT_DIR_RIGHT: return down[FAKE_RIGHT];
        default:              return 0;
    }
}

void input_set_repeat(int delay_frames, int interval_frames) {
    (void)delay_frames;
    (void)interval_frames;
}
