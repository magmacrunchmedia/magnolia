#include <wiiuse/wpad.h>
#include "input.h"
#include "input_state.h"

/* The hardware half: read WPAD, translate to the engine's button bits, hand the
   frame to input_state.c. Everything that counts, compares or remembers lives
   over there, where it can be tested without a console.

   The mapping is one-to-one with what input.c did before the split, deliberately
   -- three shipped games are calibrated against these exact directions, and a
   "correction" here would silently rotate the D-pad under all of them. */
static const u32 wpad_mask[INPUT_BTN_COUNT] = {
    [INPUT_BTN_A]     = WPAD_BUTTON_A,
    [INPUT_BTN_B]     = WPAD_BUTTON_B,
    [INPUT_BTN_1]     = WPAD_BUTTON_1,
    [INPUT_BTN_2]     = WPAD_BUTTON_2,
    [INPUT_BTN_PLUS]  = WPAD_BUTTON_PLUS,
    [INPUT_BTN_MINUS] = WPAD_BUTTON_MINUS,
    [INPUT_BTN_HOME]  = WPAD_BUTTON_HOME,
    [INPUT_BTN_UP]    = WPAD_BUTTON_UP,
    [INPUT_BTN_DOWN]  = WPAD_BUTTON_DOWN,
    [INPUT_BTN_LEFT]  = WPAD_BUTTON_LEFT,
    [INPUT_BTN_RIGHT] = WPAD_BUTTON_RIGHT,
};

static unsigned short translate(u32 wpad_held) {
    unsigned short held = 0;
    for (int b = 0; b < INPUT_BTN_COUNT; b++) {
        if (wpad_held & wpad_mask[b]) held |= (unsigned short)(1u << (unsigned)b);
    }
    return held;
}

void input_init(void) {
    WPAD_Init();
    input_state_reset();
}

int input_scan(void) {
    WPAD_ScanPads();

    /* Every channel is fed every frame, including the ones with nothing on them.
       A controller that switches off mid-press is then reported as releasing
       what it was holding, rather than leaving a button stuck down forever. */
    for (int p = 0; p < INPUT_MAX_PLAYERS; p++) {
        u32 type = 0;
        int connected = (WPAD_Probe(p, &type) == WPAD_ERR_NONE);
        input_state_feed(p, connected, connected ? translate(WPAD_ButtonsHeld(p)) : 0);
    }

    return 1;
}
