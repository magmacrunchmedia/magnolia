#ifndef GAMESTATE_H
#define GAMESTATE_H

/* The score-attack shell shared by MagmaCrunch arcade ports: title, optional
   how-to-play, play, game over, initials entry on a qualifying score, then the
   leaderboard. Games own what happens during GS_PLAYING and supply their own
   drawing; this owns the transitions and the initials editor, which is fiddly
   enough that every game copying it would mean every game copying its bugs. */

typedef enum {
    GS_TITLE,
    /* Pre-run selection: character, difficulty, mode. The shell owns entering and
       leaving it; what is being chosen, and how it is drawn, belongs to the game.
       Both games had grown a loose "is the selector open" flag beside the state
       machine because there was nowhere to put this. */
    GS_MENU,
    GS_READY,
    GS_PLAYING,
    GS_PAUSED,
    GS_GAME_OVER,
    GS_INITIALS,
    GS_HIGH_SCORES
} GameStateId;

typedef struct {
    GameStateId state;

    /* Set when the finished run qualifies for the table. */
    int is_high_score;
    int rank;

    /* Initials editor. */
    char initials[4];
    int  cursor_pos;
    int  selected_letter;

    /* Whether the title screen leads into GS_MENU. */
    int  menu_enabled;
} GameStateMachine;

void gamestate_init(GameStateMachine *g);

GameStateId gamestate_current(const GameStateMachine *g);
void gamestate_set(GameStateMachine *g, GameStateId s);

/* Call when a run ends. Works out whether the score qualifies and moves to
   GS_GAME_OVER. */
void gamestate_end_run(GameStateMachine *g, int score);

/* Drive the non-playing states from input. Returns 1 when the caller should
   start a fresh run (i.e. it just entered GS_PLAYING), so the game knows when
   to reset its own world. GS_PLAYING itself is left entirely to the game. */
int gamestate_update(GameStateMachine *g, int score);

/* Editor helpers, exposed for games that draw their own initials screen. */
void gamestate_begin_initials(GameStateMachine *g);
void gamestate_commit_initials(GameStateMachine *g, int score);

/* Whether A on the title screen opens GS_MENU instead of going straight to
   GS_READY. Off by default, so a game with nothing to choose keeps the shorter
   path. A game that turns it on draws the menu itself during GS_MENU and calls
   gamestate_menu_confirm() when the player has chosen; B backs out to the title. */
void gamestate_set_menu_enabled(GameStateMachine *g, int enabled);
void gamestate_menu_confirm(GameStateMachine *g);

/* Pause and resume during play. The game decides what pauses it -- the shell
   only holds the state, since what should freeze and what should keep drawing is
   never the same twice. */
void gamestate_pause(GameStateMachine *g);
void gamestate_resume(GameStateMachine *g);

#endif
