/* The score-attack shell: the transitions, and the initials editor.
 *
 * gamestate.c reaches libogc only through input.h, so this binary links
 * tests/fake_input.c in place of source/input.c and drives the real state
 * machine on the host. No #ifdef, no function pointer, no change to shipping
 * code -- the seam was already there in the shape of a header.
 *
 * The editor is the part most worth pinning down. It is fiddly enough that
 * gamestate.h gives it as the reason the shell exists at all ("every game
 * copying it would mean every game copying its bugs"), and it has already had
 * one: running past the last slot used to clobber the terminator.
 *
 *   make test
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "harness.h"
#include "fake_input.h"
#include "gamestate.h"
#include "scoring.h"

static const char *SCORES = "/tmp/magnolia_test_gamestate_scores.json";

/* A fresh empty table. The shell asks scoring whether a run qualified, so every
   case has to state what the leaderboard already holds. */
static void fresh_table(int max_entries) {
    unlink(SCORES);
    scoring_init(SCORES, max_entries);
    scoring_reset();
    fake_input_clear();
}

/* One frame with exactly one button down. A frame with two buttons down is not
   what any of these cases is asking about. */
static int frame(GameStateMachine *g, FakeButton b, int score) {
    fake_input_only(b);
    return gamestate_update(g, score);
}

static void test_initial_state(void) {
    printf("gamestate: a freshly initialised shell\n");
    fresh_table(10);

    GameStateMachine g;
    gamestate_init(&g);

    check_int(gamestate_current(&g), GS_TITLE, "starts on the title");
    check_str(g.initials, "AAA", "initials start at AAA");
    check_int(g.cursor_pos, 0, "the editor cursor starts at the first slot");
    check_int(g.menu_enabled, 0, "the pre-run menu is off by default");
    check_int(g.is_high_score, 0, "no high score is claimed before a run");
}

static void test_title_and_menu(void) {
    printf("gamestate: title, pre-run menu and ready\n");
    fresh_table(10);

    GameStateMachine g;

    /* A game with nothing to choose keeps the shorter path. */
    gamestate_init(&g);
    check_int(frame(&g, FAKE_A, 0), 0, "A on the title does not start a run yet");
    check_int(gamestate_current(&g), GS_READY, "A goes straight to ready");

    gamestate_init(&g);
    gamestate_set_menu_enabled(&g, 1);
    frame(&g, FAKE_A, 0);
    check_int(gamestate_current(&g), GS_MENU, "with a menu enabled, A opens it");

    /* Selection is the game's; the way back out is the shell's. */
    frame(&g, FAKE_B, 0);
    check_int(gamestate_current(&g), GS_TITLE, "B backs out of the menu");

    frame(&g, FAKE_A, 0);
    check_int(gamestate_current(&g), GS_MENU, "and A opens it again");
    check_int(frame(&g, FAKE_A, 0), 0, "A alone does not confirm the menu");
    check_int(gamestate_current(&g), GS_MENU, "the game decides when it is done");

    gamestate_menu_confirm(&g);
    check_int(gamestate_current(&g), GS_READY, "confirming leads to ready");

    /* The return value is how both games know to reset their world. */
    check_int(frame(&g, FAKE_A, 0), 1, "entering play reports a fresh run");
    check_int(gamestate_current(&g), GS_PLAYING, "and the state is playing");

    check_int(frame(&g, FAKE_A, 0), 0, "play reports nothing; the game owns it");
    check_int(gamestate_current(&g), GS_PLAYING, "and the shell leaves it alone");

    /* Confirm is only meaningful from the menu. */
    gamestate_menu_confirm(&g);
    check_int(gamestate_current(&g), GS_PLAYING,
              "confirming outside the menu changes nothing");
}

static void test_pause(void) {
    printf("gamestate: pause and resume\n");
    fresh_table(10);

    GameStateMachine g;
    gamestate_init(&g);
    gamestate_set(&g, GS_PLAYING);

    gamestate_pause(&g);
    check_int(gamestate_current(&g), GS_PAUSED, "play pauses");
    check_int(frame(&g, FAKE_A, 0), 0, "a paused shell reports nothing");
    check_int(gamestate_current(&g), GS_PAUSED, "and stays paused; the game owns it");

    gamestate_resume(&g);
    check_int(gamestate_current(&g), GS_PLAYING, "and resumes");

    gamestate_set(&g, GS_TITLE);
    gamestate_pause(&g);
    check_int(gamestate_current(&g), GS_TITLE, "the title cannot be paused");
    gamestate_resume(&g);
    check_int(gamestate_current(&g), GS_TITLE, "nor resumed");
}

static void test_run_ends_qualifying(void) {
    printf("gamestate: a run that makes the table\n");
    fresh_table(10);

    GameStateMachine g;
    gamestate_init(&g);
    gamestate_set(&g, GS_PLAYING);

    gamestate_end_run(&g, 500);
    check_int(gamestate_current(&g), GS_GAME_OVER, "a finished run is game over");
    check_int(g.is_high_score, 1, "an empty table takes any score");
    check_int(g.rank, 1, "the first score is first");

    frame(&g, FAKE_A, 500);
    check_int(gamestate_current(&g), GS_INITIALS, "A opens the editor");
    check_str(g.initials, "AAA", "the editor opens on AAA");
    check_int(g.cursor_pos, 0, "on the first slot");
}

static void test_run_ends_not_qualifying(void) {
    printf("gamestate: a run that does not make the table\n");
    fresh_table(3);

    scoring_add_entry("ZZZ", 900, 0, 0);
    scoring_add_entry("YYY", 800, 0, 0);
    scoring_add_entry("XXX", 700, 0, 0);

    GameStateMachine g;
    gamestate_init(&g);
    gamestate_set(&g, GS_PLAYING);

    gamestate_end_run(&g, 10);
    check_int(gamestate_current(&g), GS_GAME_OVER, "a finished run is game over");
    check_int(g.is_high_score, 0, "a full table rejects a low score");

    /* Straight back into a run, and the caller is told to reset its world. */
    check_int(frame(&g, FAKE_A, 10), 1, "A retries and reports a fresh run");
    check_int(gamestate_current(&g), GS_PLAYING, "with no detour through the editor");
}

static void test_initials_editor(void) {
    printf("gamestate: the initials editor\n");
    fresh_table(10);

    GameStateMachine g;
    gamestate_init(&g);
    gamestate_begin_initials(&g);
    check_int(gamestate_current(&g), GS_INITIALS, "begin_initials opens the editor");

    /* Letters wrap in both directions -- 26 of them, and the player should not
       have to walk the long way round. */
    frame(&g, FAKE_RIGHT, 100);
    check_int(g.selected_letter, 1, "right advances a letter");
    frame(&g, FAKE_LEFT, 100);
    check_int(g.selected_letter, 0, "left goes back");
    frame(&g, FAKE_LEFT, 100);
    check_int(g.selected_letter, 25, "left from A wraps to Z");
    frame(&g, FAKE_RIGHT, 100);
    check_int(g.selected_letter, 0, "right from Z wraps to A");

    /* Up commits the letter without moving on. */
    for (int i = 0; i < 12; i++) frame(&g, FAKE_RIGHT, 100);
    check_int(g.selected_letter, 12, "twelve steps reaches M");
    frame(&g, FAKE_UP, 100);
    check_int(g.initials[0], 'M', "up writes the letter into the slot");
    check_int(g.cursor_pos, 0, "and does not advance");

    /* Down commits and advances. */
    frame(&g, FAKE_DOWN, 100);
    check_int(g.initials[0], 'M', "down writes the letter too");
    check_int(g.cursor_pos, 1, "and advances to the next slot");

    frame(&g, FAKE_DOWN, 100);
    check_int(g.cursor_pos, 2, "and again, to the last slot");

    /* The regression named in gamestate.c: running past the last slot used to
       clobber the terminator and lose the cursor highlight. */
    for (int i = 0; i < 5; i++) frame(&g, FAKE_DOWN, 100);
    check_int(g.cursor_pos, 2, "down stops at the last slot");
    check_int(g.initials[3], '\0', "and never clobbers the terminator");
    check_int((int)strlen(g.initials), 3, "the initials stay three letters");

    /* Committing files the entry and shows the player where they landed. */
    frame(&g, FAKE_A, 100);
    check_int(gamestate_current(&g), GS_HIGH_SCORES, "A commits and shows the table");
    check_int(scoring_get_count(), 1, "the entry reached the table");

    const ScoreEntry *e = scoring_get_entry(0);
    check(e != NULL, "the entry is readable");
    if (e) {
        check_str(e->initials, g.initials, "under the initials that were entered");
        check_int(e->score, 100, "with the score the run earned");
    }

    frame(&g, FAKE_A, 100);
    check_int(gamestate_current(&g), GS_TITLE, "A from the table returns to the title");
}

/* The path both games actually walk, start to finish, in one go. The pieces
   above pass individually; this is the one that would catch them not composing. */
static void test_full_loop(void) {
    printf("gamestate: title to leaderboard in one pass\n");
    fresh_table(10);

    GameStateMachine g;
    gamestate_init(&g);
    gamestate_set_menu_enabled(&g, 1);

    frame(&g, FAKE_A, 0);
    check_int(gamestate_current(&g), GS_MENU, "title -> menu");
    gamestate_menu_confirm(&g);
    check_int(gamestate_current(&g), GS_READY, "menu -> ready");
    check_int(frame(&g, FAKE_A, 0), 1, "ready -> playing, world reset requested");

    gamestate_end_run(&g, 250);
    check_int(gamestate_current(&g), GS_GAME_OVER, "playing -> game over");
    frame(&g, FAKE_A, 250);
    check_int(gamestate_current(&g), GS_INITIALS, "game over -> initials");

    frame(&g, FAKE_A, 250);
    check_int(gamestate_current(&g), GS_HIGH_SCORES, "initials -> high scores");
    check_int(scoring_get_count(), 1, "one entry filed");

    frame(&g, FAKE_A, 250);
    check_int(gamestate_current(&g), GS_TITLE, "high scores -> title");
}

int main(void) {
    test_initial_state();
    test_title_and_menu();
    test_pause();
    test_run_ends_qualifying();
    test_run_ends_not_qualifying();
    test_initials_editor();
    test_full_loop();

    unlink(SCORES);
    return report();
}
