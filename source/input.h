#ifndef INPUT_H
#define INPUT_H

/* Wiimote wrapper, held sideways: the D-pad points where you expect and 1/2 fall
 * under the thumb. Buttons are reported as edges (pressed this frame) except
 * where a held state is what the caller actually means.
 */

typedef enum {
    INPUT_DIR_UP,
    INPUT_DIR_DOWN,
    INPUT_DIR_LEFT,
    INPUT_DIR_RIGHT,
    INPUT_DIR_COUNT
} InputDir;

void input_init(void);
int  input_scan(void);

/* Edges. */
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

/* Held, for anything continuous -- thrust, charge, hold-to-aim. */
int input_a_held(void);

/* Fires on the frame the direction is first pressed, then repeatedly while it
   stays held: once after `delay`, then every `interval` frames. Menus need this
   -- stepping through a 26-letter initials editor or a 24-item roster one
   discrete press at a time is miserable -- while anything positional (sliding a
   tile, moving a piece) wants the plain edge above. Requires input_scan() once
   per frame, which is where the hold is counted. */
int  input_dir_repeat(InputDir dir);
void input_set_repeat(int delay_frames, int interval_frames);

#endif
