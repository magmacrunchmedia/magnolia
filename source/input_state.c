#include "input_state.h"

/* Roughly a third of a second before repeat starts, then about 7 a second: fast
   enough to cross a long list, slow enough that a single press still selects
   exactly one item. Global rather than per-player because it describes how the
   game should feel, not who is holding the controller. */
static int repeat_delay    = 20;
static int repeat_interval = 8;

typedef struct {
    InputPad pad;
    int connected;
    /* Frames each direction has been held, and the repeat edges derived from
       them. Both are settled in input_state_feed() rather than in the query, so
       asking twice in a frame cannot advance anything. */
    int held_frames[INPUT_DIR_COUNT];
    unsigned char repeat[INPUT_DIR_COUNT];
} PlayerState;

static PlayerState players[INPUT_MAX_PLAYERS];

static int valid_player(int player) {
    return player >= 0 && player < INPUT_MAX_PLAYERS;
}

static int valid_button(InputButton b) {
    return b >= 0 && b < INPUT_BTN_COUNT;
}

static unsigned short bit(InputButton b) {
    return (unsigned short)(1u << (unsigned)b);
}

InputButton input_dir_button(InputDir dir) {
    if (dir < 0 || dir >= INPUT_DIR_COUNT) return INPUT_BTN_COUNT;
    /* InputDir and the four direction buttons are declared in the same order,
       so this stays arithmetic. The assert-by-construction is that both enums
       run UP, DOWN, LEFT, RIGHT. */
    return (InputButton)(INPUT_BTN_UP + (int)dir);
}

void input_state_reset(void) {
    for (int p = 0; p < INPUT_MAX_PLAYERS; p++) {
        players[p].pad.held = 0;
        players[p].pad.pressed = 0;
        players[p].pad.released = 0;
        players[p].connected = 0;
        for (int d = 0; d < INPUT_DIR_COUNT; d++) {
            players[p].held_frames[d] = 0;
            players[p].repeat[d] = 0;
        }
    }
}

/* Whether a direction held for `frames` should fire this frame: once on the
   press, then every `interval` frames once `delay` has passed. */
static int repeat_fires(int frames) {
    if (frames <= 0) return 0;
    if (frames == 1) return 1;
    if (frames < repeat_delay) return 0;
    return ((frames - repeat_delay) % repeat_interval) == 0;
}

void input_state_feed(int player, int connected, unsigned short held) {
    if (!valid_player(player)) return;

    PlayerState *ps = &players[player];

    /* A controller that has gone quiet holds nothing. Taking that path rather
       than returning early is what produces the release edges for whatever its
       player was mid-press on when it went. */
    if (!connected) held = 0;

    unsigned short was = ps->pad.held;
    ps->pad.held     = held;
    ps->pad.pressed  = (unsigned short)(held & ~was);
    ps->pad.released = (unsigned short)(~held & was);
    ps->connected    = connected ? 1 : 0;

    for (int d = 0; d < INPUT_DIR_COUNT; d++) {
        if (held & bit(input_dir_button((InputDir)d))) {
            ps->held_frames[d]++;
        } else {
            ps->held_frames[d] = 0;
        }
        ps->repeat[d] = (unsigned char)repeat_fires(ps->held_frames[d]);
    }
}

int input_player_count(void) {
    int n = 0;
    for (int p = 0; p < INPUT_MAX_PLAYERS; p++) n += players[p].connected;
    return n;
}

int input_connected(int player) {
    return valid_player(player) ? players[player].connected : 0;
}

int input_held(int player, InputButton b) {
    if (!valid_player(player) || !valid_button(b)) return 0;
    return (players[player].pad.held & bit(b)) != 0;
}

int input_pressed(int player, InputButton b) {
    if (!valid_player(player) || !valid_button(b)) return 0;
    return (players[player].pad.pressed & bit(b)) != 0;
}

int input_released(int player, InputButton b) {
    if (!valid_player(player) || !valid_button(b)) return 0;
    return (players[player].pad.released & bit(b)) != 0;
}

const InputPad *input_snapshot(int player) {
    return valid_player(player) ? &players[player].pad : (const InputPad *)0;
}

int input_dir_repeat_for(int player, InputDir dir) {
    if (!valid_player(player) || dir < 0 || dir >= INPUT_DIR_COUNT) return 0;
    return players[player].repeat[dir];
}

void input_set_repeat(int delay_frames, int interval_frames) {
    if (delay_frames > 0)    repeat_delay = delay_frames;
    if (interval_frames > 0) repeat_interval = interval_frames;
}

/* --- Player one, spelled the short way --- */

int input_a_pressed(void)       { return input_pressed(0, INPUT_BTN_A); }
int input_back_pressed(void)    { return input_pressed(0, INPUT_BTN_B); }
int input_home_pressed(void)    { return input_pressed(0, INPUT_BTN_HOME); }
int input_button1_pressed(void) { return input_pressed(0, INPUT_BTN_1); }
int input_button2_pressed(void) { return input_pressed(0, INPUT_BTN_2); }
int input_plus_pressed(void)    { return input_pressed(0, INPUT_BTN_PLUS); }
int input_minus_pressed(void)   { return input_pressed(0, INPUT_BTN_MINUS); }

int input_left_pressed(void)    { return input_pressed(0, INPUT_BTN_LEFT); }
int input_right_pressed(void)   { return input_pressed(0, INPUT_BTN_RIGHT); }
int input_up_pressed(void)      { return input_pressed(0, INPUT_BTN_UP); }
int input_down_pressed(void)    { return input_pressed(0, INPUT_BTN_DOWN); }

int input_a_held(void)          { return input_held(0, INPUT_BTN_A); }

int input_dir_repeat(InputDir dir) { return input_dir_repeat_for(0, dir); }
