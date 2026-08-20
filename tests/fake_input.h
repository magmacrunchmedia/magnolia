#ifndef MAGNOLIA_TEST_FAKE_INPUT_H
#define MAGNOLIA_TEST_FAKE_INPUT_H

/* A host stand-in for input.h.
 *
 * gamestate.c reaches libogc only through input.h, so a test binary that links
 * this instead of source/input.c exercises the real state machine with no
 * #ifdef, no function pointer and no change to shipping code at all.
 *
 * What is faked is only "which button is down this frame". The repeat timing in
 * the real input.c is its own concern and is not what a state-machine test is
 * asking about, so input_dir_repeat() here reports the direction flag directly.
 */

typedef enum {
    FAKE_A,
    FAKE_B,
    FAKE_HOME,
    FAKE_1,
    FAKE_2,
    FAKE_PLUS,
    FAKE_MINUS,
    FAKE_UP,
    FAKE_DOWN,
    FAKE_LEFT,
    FAKE_RIGHT,
    FAKE_A_HELD,
    FAKE_BUTTON_COUNT
} FakeButton;

/* Nothing pressed. Call between cases: a flag left set is the fake's own
   version of a stuck button, and it makes the next case lie. */
void fake_input_clear(void);

void fake_input_press(FakeButton b);

/* Clears, presses one button, and returns -- the shape almost every case
   wants, since a frame with two buttons down is not what is being tested. */
void fake_input_only(FakeButton b);

#endif
