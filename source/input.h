#ifndef INPUT_H
#define INPUT_H

/* Controller input for up to four players, sampled once per frame.
 *
 * Held sideways, a Wiimote puts the D-pad where you expect and 1/2 under the
 * thumb; that is the layout every game so far assumes and the one the button
 * names below describe.
 *
 * The engine keeps a snapshot per player -- what is held, what went down this
 * frame, what came up -- rather than asking the hardware again at every call
 * site. Three reasons, in order of how much they cost to retrofit:
 *
 *   1. A frame's input is then a value, so two calls in the same frame cannot
 *      disagree and a game may buffer past frames (a motion input is a pattern
 *      over ~8 frames, which no amount of per-button accessors can express).
 *   2. Releases become expressible. Charge moves and hold-to-block need the
 *      frame a button came *up*, which nothing here could report before.
 *   3. The arithmetic stops touching libogc, so it can be tested on the host.
 *
 * This header deliberately uses no libogc types: it is compiled by host tests as
 * well as by the console build.
 */

#define INPUT_MAX_PLAYERS 4        /* WPAD_MAX_WIIMOTES */

typedef enum {
    INPUT_DIR_UP,
    INPUT_DIR_DOWN,
    INPUT_DIR_LEFT,
    INPUT_DIR_RIGHT,
    INPUT_DIR_COUNT
} InputDir;

/* Ordered so the four directions sit last and contiguously, which lets
   input_dir_button() be arithmetic rather than a switch. */
typedef enum {
    INPUT_BTN_A,
    INPUT_BTN_B,
    INPUT_BTN_1,
    INPUT_BTN_2,
    INPUT_BTN_PLUS,
    INPUT_BTN_MINUS,
    INPUT_BTN_HOME,
    INPUT_BTN_UP,
    INPUT_BTN_DOWN,
    INPUT_BTN_LEFT,
    INPUT_BTN_RIGHT,
    INPUT_BTN_COUNT
} InputButton;

/* One player's frame, as a bit per InputButton. `pressed` and `released` are
   derived from this frame's `held` against the previous one, so a game that
   stores a snapshot keeps the edges too -- which is what makes a replay or an
   input buffer possible. */
typedef struct {
    unsigned short held;
    unsigned short pressed;
    unsigned short released;
} InputPad;

/* The button belonging to a direction, so callers holding an InputDir can reach
   the bitmask without a lookup table of their own. */
InputButton input_dir_button(InputDir dir);

void input_init(void);

/* Samples every controller. Call once per frame: the hold counters behind
   input_dir_repeat() advance here, so calling it twice in a frame makes repeat
   run at double speed, and not calling it freezes input entirely.
   Returns 1, as it always has -- ask input_player_count() how many are on. */
int  input_scan(void);

/* How many controllers reported in on the last scan, and whether a particular
   one did. A player whose controller is off reads as all-buttons-up rather than
   as a stuck frame, so a game may either pause or simply let them stand still. */
int  input_player_count(void);
int  input_connected(int player);

/* Out-of-range players and buttons read as not-pressed rather than reading off
   the end of the array: a game indexing by a player number it computed is a
   normal mistake, and it should misbehave visibly, not corrupt memory. */
int  input_held(int player, InputButton b);
int  input_pressed(int player, InputButton b);
int  input_released(int player, InputButton b);

/* The whole frame for one player. NULL for an out-of-range player. Valid until
   the next input_scan(), so copy it to keep it. */
const InputPad *input_snapshot(int player);

/* Fires on the frame the direction is first pressed, then repeatedly while it
   stays held: once after `delay`, then every `interval` frames. Menus need this
   -- stepping through a 26-letter initials editor or a 24-item roster one
   discrete press at a time is miserable -- while anything positional (sliding a
   tile, moving a piece, walking a fighter) wants the plain edge above.
   Counted per player, so one player holding left cannot advance another's
   cursor. The repeat timing itself is global: it is a feel decision about the
   game, not about the person holding the controller. */
int  input_dir_repeat_for(int player, InputDir dir);
void input_set_repeat(int delay_frames, int interval_frames);

/* --- Player one, spelled the short way -------------------------------------
 *
 * Every function below is its multi-player equivalent with player 0 filled in.
 * Single-player games -- which is all of them so far -- read better for it, and
 * the three shipped games compile against this header unchanged.
 */
int input_a_pressed(void);
int input_back_pressed(void);      /* B */
int input_home_pressed(void);
int input_button1_pressed(void);
int input_button2_pressed(void);
int input_plus_pressed(void);
int input_minus_pressed(void);

int input_left_pressed(void);
int input_right_pressed(void);
int input_up_pressed(void);
int input_down_pressed(void);

int input_a_held(void);
int input_dir_repeat(InputDir dir);

#endif
