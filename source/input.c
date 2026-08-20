#include <wiiuse/wpad.h>
#include "input.h"

/* Roughly a third of a second before repeat starts, then about 7 a second: fast
   enough to cross a long list, slow enough that a single press still selects
   exactly one item. */
static int repeat_delay    = 20;
static int repeat_interval = 8;

/* Frames each direction has been held, and whether a repeat is already running
   for it. Counted in input_scan() so a caller that polls a direction twice in a
   frame gets the same answer both times. */
static int held_frames[INPUT_DIR_COUNT];
static int fired[INPUT_DIR_COUNT];

static u32 dir_mask(InputDir dir) {
    switch (dir) {
        case INPUT_DIR_UP:    return WPAD_BUTTON_UP;
        case INPUT_DIR_DOWN:  return WPAD_BUTTON_DOWN;
        case INPUT_DIR_LEFT:  return WPAD_BUTTON_LEFT;
        case INPUT_DIR_RIGHT: return WPAD_BUTTON_RIGHT;
        default:              return 0;
    }
}

void input_init(void) {
    WPAD_Init();
    for (int i = 0; i < INPUT_DIR_COUNT; i++) {
        held_frames[i] = 0;
        fired[i] = 0;
    }
}

int input_scan(void) {
    WPAD_ScanPads();

    u32 held = WPAD_ButtonsHeld(0);
    for (int i = 0; i < INPUT_DIR_COUNT; i++) {
        if (held & dir_mask((InputDir)i)) {
            held_frames[i]++;
        } else {
            held_frames[i] = 0;
            fired[i] = 0;
        }
    }
    return 1;
}

static int pressed(u32 mask) {
    return (WPAD_ButtonsDown(0) & mask) != 0;
}

int input_a_pressed(void)       { return pressed(WPAD_BUTTON_A); }
int input_back_pressed(void)    { return pressed(WPAD_BUTTON_B); }
int input_home_pressed(void)    { return pressed(WPAD_BUTTON_HOME); }
int input_button1_pressed(void) { return pressed(WPAD_BUTTON_1); }
int input_button2_pressed(void) { return pressed(WPAD_BUTTON_2); }
int input_plus_pressed(void)    { return pressed(WPAD_BUTTON_PLUS); }
int input_minus_pressed(void)   { return pressed(WPAD_BUTTON_MINUS); }

int input_left_pressed(void)    { return pressed(WPAD_BUTTON_LEFT); }
int input_right_pressed(void)   { return pressed(WPAD_BUTTON_RIGHT); }
int input_up_pressed(void)      { return pressed(WPAD_BUTTON_UP); }
int input_down_pressed(void)    { return pressed(WPAD_BUTTON_DOWN); }

int input_a_held(void) {
    return (WPAD_ButtonsHeld(0) & WPAD_BUTTON_A) != 0;
}

void input_set_repeat(int delay_frames, int interval_frames) {
    if (delay_frames > 0)    repeat_delay = delay_frames;
    if (interval_frames > 0) repeat_interval = interval_frames;
}

int input_dir_repeat(InputDir dir) {
    if (dir < 0 || dir >= INPUT_DIR_COUNT) return 0;

    int frames = held_frames[dir];
    if (frames == 0) return 0;
    if (frames == 1) return 1;                 /* the initial press */
    if (frames < repeat_delay) return 0;

    if (!fired[dir]) {
        fired[dir] = 1;
        return 1;
    }
    return ((frames - repeat_delay) % repeat_interval) == 0;
}
