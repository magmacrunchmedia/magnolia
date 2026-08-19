#ifndef GAMESTATE_H
#define GAMESTATE_H

/* The score-attack shell shared by MagmaCrunch arcade ports: title, optional
   how-to-play, play, game over, initials entry on a qualifying score, then the
   leaderboard. Games own what happens during GS_PLAYING and supply their own
   drawing; this owns the transitions and the initials editor, which is fiddly
   enough that every game copying it would mean every game copying its bugs. */

typedef enum {
    GS_TITLE,
    GS_READY,
    GS_PLAYING,
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

#endif
