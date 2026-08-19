#ifndef STARS_H
#define STARS_H

typedef struct {
    int x, y;
    int pattern;
    int color_r, color_g, color_b;
    int blink_frame;
    int blink_speed;
    int frame_counter;
    int visible;
    int pulse_state;
    int is_pulsing;
} Star;

void stars_init(void);
void stars_update(void);
void stars_get(int index, Star **out);
int stars_get_count(void);

#endif
