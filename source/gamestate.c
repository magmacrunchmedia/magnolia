#include <string.h>
#include "gamestate.h"
#include "input.h"
#include "scoring.h"

#define LETTER_COUNT 26
#define INITIALS_LEN 3

void gamestate_init(GameStateMachine *g) {
    if (!g) return;
    memset(g, 0, sizeof(*g));
    g->state = GS_TITLE;
    strcpy(g->initials, "AAA");
}

GameStateId gamestate_current(const GameStateMachine *g) { return g->state; }
void gamestate_set(GameStateMachine *g, GameStateId s)   { g->state = s; }

void gamestate_end_run(GameStateMachine *g, int score) {
    g->is_high_score = scoring_is_high_score(score);
    g->rank = g->is_high_score ? scoring_get_rank(score) : 0;
    g->state = GS_GAME_OVER;
}

void gamestate_begin_initials(GameStateMachine *g) {
    g->cursor_pos = 0;
    g->selected_letter = 0;
    strcpy(g->initials, "AAA");
    g->state = GS_INITIALS;
}

void gamestate_commit_initials(GameStateMachine *g, int score) {
    g->initials[g->cursor_pos] = (char)('A' + g->selected_letter);
    scoring_add_entry(g->initials, score);
    g->state = GS_HIGH_SCORES;
}

static void update_initials(GameStateMachine *g, int score) {
    /* Repeat rather than single presses: 26 letters at one press each is the
       difference between entering initials and enduring it. */
    if (input_dir_repeat(INPUT_DIR_LEFT)) {
        g->selected_letter--;
        if (g->selected_letter < 0) g->selected_letter = LETTER_COUNT - 1;
    }
    if (input_dir_repeat(INPUT_DIR_RIGHT)) {
        g->selected_letter++;
        if (g->selected_letter >= LETTER_COUNT) g->selected_letter = 0;
    }

    if (input_up_pressed()) {
        g->initials[g->cursor_pos] = (char)('A' + g->selected_letter);
    }

    if (input_down_pressed()) {
        g->initials[g->cursor_pos] = (char)('A' + g->selected_letter);
        /* Stop at the last slot; running past it used to clobber the
           terminator and lose the cursor highlight. */
        if (g->cursor_pos < INITIALS_LEN - 1) {
            g->cursor_pos++;
            int l = g->initials[g->cursor_pos] - 'A';
            g->selected_letter = (l < 0 || l >= LETTER_COUNT) ? 0 : l;
        }
    }

    if (input_a_pressed()) {
        gamestate_commit_initials(g, score);
    }
}

void gamestate_set_menu_enabled(GameStateMachine *g, int enabled) {
    if (g) g->menu_enabled = enabled ? 1 : 0;
}

void gamestate_menu_confirm(GameStateMachine *g) {
    if (g && g->state == GS_MENU) g->state = GS_READY;
}

void gamestate_pause(GameStateMachine *g) {
    if (g && g->state == GS_PLAYING) g->state = GS_PAUSED;
}

void gamestate_resume(GameStateMachine *g) {
    if (g && g->state == GS_PAUSED) g->state = GS_PLAYING;
}

int gamestate_update(GameStateMachine *g, int score) {
    switch (g->state) {
        case GS_TITLE:
            if (input_a_pressed()) g->state = g->menu_enabled ? GS_MENU : GS_READY;
            return 0;

        case GS_MENU:
            /* Selection is the game's; the shell only offers the way back out.
               Confirming is gamestate_menu_confirm(), which the game calls once
               it knows what was chosen. */
            if (input_back_pressed()) g->state = GS_TITLE;
            return 0;

        case GS_READY:
            if (input_a_pressed()) { g->state = GS_PLAYING; return 1; }
            return 0;

        case GS_PLAYING:
        case GS_PAUSED:
            return 0;  /* the game owns these */

        case GS_GAME_OVER:
            if (input_a_pressed()) {
                if (g->is_high_score) {
                    gamestate_begin_initials(g);
                } else {
                    g->state = GS_PLAYING;
                    return 1;
                }
            }
            return 0;

        case GS_INITIALS:
            update_initials(g, score);
            return 0;

        case GS_HIGH_SCORES:
            if (input_a_pressed()) g->state = GS_TITLE;
            return 0;
    }
    return 0;
}
