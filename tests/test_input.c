/* Edge detection and auto-repeat, for up to four players.
 *
 * This module did not exist as anything testable until input.c was split: the
 * counting lived one call below WPAD_ButtonsHeld(), so the only way to check it
 * was to hold two controllers in front of a console and watch. The cases here
 * are the ones that were being taken on trust -- that a second player has their
 * own hold counters, that a button coming up is reported once, and that asking
 * the same question twice in a frame does not advance anything.
 *
 *   make test-input
 */
#include <stdio.h>
#include "harness.h"
#include "input.h"
#include "input_state.h"

#define BIT(b) ((unsigned short)(1u << (unsigned)(b)))

/* One frame for one player, given as the buttons that are down. */
static void frame(int player, InputButton b) {
    input_state_feed(player, 1, BIT(b));
}

static void frame_empty(int player) {
    input_state_feed(player, 1, 0);
}

/* Hold a direction for n consecutive frames, counting how many of them the
   repeat fired on. */
static int repeats_over(int player, InputDir dir, int n) {
    int fired = 0;
    for (int i = 0; i < n; i++) {
        input_state_feed(player, 1, BIT(input_dir_button(dir)));
        if (input_dir_repeat_for(player, dir)) fired++;
    }
    return fired;
}

static void test_edges(void) {
    printf("input: press, hold and release edges\n");
    input_state_reset();

    check_int(input_pressed(0, INPUT_BTN_A), 0, "nothing is pressed before a frame");

    frame(0, INPUT_BTN_A);
    check_int(input_pressed(0, INPUT_BTN_A), 1, "A reads as pressed on the frame it goes down");
    check_int(input_held(0, INPUT_BTN_A), 1, "and as held");
    check_int(input_released(0, INPUT_BTN_A), 0, "and not as released");

    frame(0, INPUT_BTN_A);
    check_int(input_pressed(0, INPUT_BTN_A), 0, "a held button is not pressed again");
    check_int(input_held(0, INPUT_BTN_A), 1, "but is still held");

    frame_empty(0);
    check_int(input_released(0, INPUT_BTN_A), 1, "A reads as released on the frame it comes up");
    check_int(input_held(0, INPUT_BTN_A), 0, "and is no longer held");

    frame_empty(0);
    check_int(input_released(0, INPUT_BTN_A), 0, "the release is reported once, not every frame");
}

static void test_queries_do_not_mutate(void) {
    printf("input: asking twice in a frame\n");
    input_state_reset();

    frame(0, INPUT_BTN_B);
    check_int(input_pressed(0, INPUT_BTN_B), 1, "first ask says pressed");
    check_int(input_pressed(0, INPUT_BTN_B), 1, "second ask agrees");
    check_int(input_pressed(0, INPUT_BTN_B), 1, "and so does the third");

    /* The repeat is the one that used to disagree with itself: it settled its
       own bookkeeping inside the query, so the answer depended on how many
       times the caller had asked. It is computed in the feed now. */
    input_state_reset();
    input_set_repeat(4, 2);
    for (int i = 0; i < 3; i++) input_state_feed(0, 1, BIT(INPUT_BTN_LEFT));
    input_state_feed(0, 1, BIT(INPUT_BTN_LEFT));    /* frame 4, which is the delay */
    int first  = input_dir_repeat_for(0, INPUT_DIR_LEFT);
    int second = input_dir_repeat_for(0, INPUT_DIR_LEFT);
    check_int(first, 1, "the repeat fires on the delay frame");
    check_int(second, first, "and reports the same thing when asked again");
    input_set_repeat(20, 8);
}

static void test_repeat_timing(void) {
    printf("input: auto-repeat cadence\n");
    input_state_reset();
    input_set_repeat(10, 4);

    /* Frame 1 fires on the press. Then nothing until frame 10, then every 4th:
       10, 14, 18 -- four fires across twenty frames held. */
    check_int(repeats_over(0, INPUT_DIR_RIGHT, 20), 4, "press, then delay, then every interval");

    input_state_reset();
    check_int(repeats_over(0, INPUT_DIR_RIGHT, 1), 1, "a single frame fires exactly once");

    input_state_reset();
    check_int(repeats_over(0, INPUT_DIR_RIGHT, 9), 1, "nothing more fires before the delay");

    /* Letting go and pressing again starts the count over rather than resuming
       mid-cadence -- the difference between a menu that steps once per tap and
       one that lurches. */
    input_state_reset();
    repeats_over(0, INPUT_DIR_RIGHT, 12);
    frame_empty(0);
    check_int(repeats_over(0, INPUT_DIR_RIGHT, 1), 1, "releasing and re-pressing fires again");

    input_set_repeat(20, 8);
}

static void test_players_are_independent(void) {
    printf("input: four players, separately\n");
    input_state_reset();

    frame(0, INPUT_BTN_A);
    frame(1, INPUT_BTN_B);

    check_int(input_pressed(0, INPUT_BTN_A), 1, "player one has their own A");
    check_int(input_pressed(1, INPUT_BTN_A), 0, "which is not player two's");
    check_int(input_pressed(1, INPUT_BTN_B), 1, "player two has their own B");
    check_int(input_pressed(0, INPUT_BTN_B), 0, "which is not player one's");

    /* The hold counters were a single global before the split, so one player
       holding a direction advanced everyone's repeat. */
    input_state_reset();
    input_set_repeat(4, 2);
    for (int i = 0; i < 8; i++) {
        input_state_feed(0, 1, BIT(INPUT_BTN_LEFT));
        input_state_feed(1, 1, 0);
    }
    check_int(input_dir_repeat_for(1, INPUT_DIR_LEFT), 0,
              "player one holding left does not step player two's cursor");
    input_state_feed(1, 1, BIT(INPUT_BTN_LEFT));
    check_int(input_dir_repeat_for(1, INPUT_DIR_LEFT), 1,
              "player two's own first press fires on its own schedule");
    input_set_repeat(20, 8);
}

static void test_connection(void) {
    printf("input: controllers coming and going\n");
    input_state_reset();

    check_int(input_player_count(), 0, "no controllers before the first scan");

    input_state_feed(0, 1, 0);
    input_state_feed(1, 1, 0);
    check_int(input_player_count(), 2, "two controllers reporting in");
    check_int(input_connected(1), 1, "player two is on");

    /* A controller switched off mid-press has to release what it was holding,
       or the button stays down for the rest of the session. */
    input_state_feed(0, 1, BIT(INPUT_BTN_A));
    input_state_feed(0, 0, BIT(INPUT_BTN_A));
    check_int(input_connected(0), 0, "player one has gone");
    check_int(input_held(0, INPUT_BTN_A), 0, "a controller that is off holds nothing");
    check_int(input_released(0, INPUT_BTN_A), 1, "and releases what it was holding");
    check_int(input_player_count(), 1, "leaving one controller on");
}

static void test_snapshot(void) {
    printf("input: the frame as a value\n");
    input_state_reset();

    check(input_snapshot(-1) == 0, "an out-of-range player has no snapshot");
    check(input_snapshot(INPUT_MAX_PLAYERS) == 0, "nor does one past the last");

    frame(0, INPUT_BTN_1);
    const InputPad *pad = input_snapshot(0);
    check(pad != 0, "player one has a snapshot");
    check_int(pad->held & BIT(INPUT_BTN_1) ? 1 : 0, 1, "which carries the held mask");
    check_int(pad->pressed & BIT(INPUT_BTN_1) ? 1 : 0, 1, "and the press edge");

    /* Copying a frame is what an input buffer is made of: the copy has to keep
       its edges after the world has moved on. */
    InputPad kept = *pad;
    frame_empty(0);
    check_int(kept.pressed & BIT(INPUT_BTN_1) ? 1 : 0, 1, "a copied frame keeps its edges");
    check_int(input_pressed(0, INPUT_BTN_1), 0, "while the live one has moved on");
}

static void test_out_of_range(void) {
    printf("input: nonsense arguments\n");
    input_state_reset();
    frame(0, INPUT_BTN_A);

    check_int(input_held(-1, INPUT_BTN_A), 0, "a negative player holds nothing");
    check_int(input_held(INPUT_MAX_PLAYERS, INPUT_BTN_A), 0, "nor does one past the last");
    check_int(input_held(0, INPUT_BTN_COUNT), 0, "a button past the last is not held");
    check_int(input_dir_repeat_for(0, INPUT_DIR_COUNT), 0, "nor does a direction past the last repeat");
    check_int(input_connected(99), 0, "an absurd player is not connected");

    /* Feeding one is ignored rather than writing off the end of the array. */
    input_state_feed(INPUT_MAX_PLAYERS, 1, BIT(INPUT_BTN_A));
    check_int(input_player_count(), 1, "and feeding one changes nothing");
}

static void test_dir_button_mapping(void) {
    printf("input: directions and their buttons\n");

    check_int(input_dir_button(INPUT_DIR_UP), INPUT_BTN_UP, "up maps to up");
    check_int(input_dir_button(INPUT_DIR_DOWN), INPUT_BTN_DOWN, "down maps to down");
    check_int(input_dir_button(INPUT_DIR_LEFT), INPUT_BTN_LEFT, "left maps to left");
    check_int(input_dir_button(INPUT_DIR_RIGHT), INPUT_BTN_RIGHT, "right maps to right");
    check_int(input_dir_button(INPUT_DIR_COUNT), INPUT_BTN_COUNT, "and nothing maps to nothing");
}

static void test_player_one_shorthand(void) {
    printf("input: the zero-argument spellings\n");
    input_state_reset();

    frame(0, INPUT_BTN_A);
    check_int(input_a_pressed(), 1, "input_a_pressed() is player one's A");
    check_int(input_a_held(), 1, "input_a_held() likewise");

    frame(1, INPUT_BTN_B);
    check_int(input_back_pressed(), 0, "and reads player one, not whoever pressed last");

    input_state_reset();
    frame(0, INPUT_BTN_LEFT);
    check_int(input_dir_repeat(INPUT_DIR_LEFT), 1, "input_dir_repeat() is player one's");
    check_int(input_left_pressed(), 1, "as is input_left_pressed()");
}

int main(void) {
    test_edges();
    test_queries_do_not_mutate();
    test_repeat_timing();
    test_players_are_independent();
    test_connection();
    test_snapshot();
    test_out_of_range();
    test_dir_button_mapping();
    test_player_one_shorthand();
    return report();
}
