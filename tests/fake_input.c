#include "fake_input.h"
#include "input.h"
#include "input_state.h"

static const InputButton mapping[FAKE_BUTTON_COUNT] = {
    [FAKE_A]      = INPUT_BTN_A,
    [FAKE_B]      = INPUT_BTN_B,
    [FAKE_HOME]   = INPUT_BTN_HOME,
    [FAKE_1]      = INPUT_BTN_1,
    [FAKE_2]      = INPUT_BTN_2,
    [FAKE_PLUS]   = INPUT_BTN_PLUS,
    [FAKE_MINUS]  = INPUT_BTN_MINUS,
    [FAKE_UP]     = INPUT_BTN_UP,
    [FAKE_DOWN]   = INPUT_BTN_DOWN,
    [FAKE_LEFT]   = INPUT_BTN_LEFT,
    [FAKE_RIGHT]  = INPUT_BTN_RIGHT,
    [FAKE_A_HELD] = INPUT_BTN_A,
};

static unsigned short pending;

void fake_input_clear(void) {
    pending = 0;
    input_state_reset();
}

void fake_input_press(FakeButton b) {
    if (b < 0 || b >= FAKE_BUTTON_COUNT) return;
    pending |= (unsigned short)(1u << (unsigned)mapping[b]);
}

void fake_input_commit(void) {
    input_state_feed(0, 1, 0);
    input_state_feed(0, 1, pending);
}

void fake_input_only(FakeButton b) {
    pending = 0;
    fake_input_press(b);
    fake_input_commit();
}

/* The rest of the hardware surface, which gamestate.c never calls but a test
   binary must still be able to link. */
void input_init(void) { fake_input_clear(); }
int  input_scan(void) { return 1; }
