#ifndef MAGNOLIA_TEST_FAKE_INPUT_H
#define MAGNOLIA_TEST_FAKE_INPUT_H

/* A host stand-in for the *hardware* half of input.h.
 *
 * gamestate.c reaches libogc only through input.h, so a test binary that links
 * this plus source/input_state.c -- in place of source/input.c -- exercises the
 * real state machine with no #ifdef, no function pointer and no change to
 * shipping code at all.
 *
 * This used to reimplement all of input.h, which meant the edge and repeat rules
 * existed twice and the copy the tests ran was not the copy the console ran.
 * Since input.c was split, the seam sits lower: only "which buttons are down on
 * this controller this frame" is faked, and every press, release and repeat the
 * test sees is computed by the shipping code in input_state.c.
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
    /* Kept as a distinct name because the cases read better for it, but there is
       no longer anything behind it: with real edge detection a button that is
       down is held, and holding is not a separate kind of press. */
    FAKE_A_HELD,
    FAKE_BUTTON_COUNT
} FakeButton;

/* Nothing pressed, nothing remembered. Call between cases: a flag left set is
   the fake's own version of a stuck button, and it makes the next case lie. */
void fake_input_clear(void);

/* Add a button to the frame being assembled. Nothing reaches input_state.c
   until fake_input_commit(). */
void fake_input_press(FakeButton b);

/* Deliver the assembled frame to player one, preceded by an all-up frame so the
   buttons in it read as pressed this frame rather than as still-held from the
   last one. Cases that want a genuine hold across frames should commit the same
   button twice without an intervening clear. */
void fake_input_commit(void);

/* Clears, presses one button, commits -- the shape almost every case wants,
   since a frame with two buttons down is not what is being tested. */
void fake_input_only(FakeButton b);

#endif
