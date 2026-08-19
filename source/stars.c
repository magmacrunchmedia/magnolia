#include <stdlib.h>
#include "stars.h"
#include "config.h"

static Star stars[STAR_COUNT];

void stars_init(void) {
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].x = rand() % CANVAS_WIDTH;
        stars[i].y = rand() % CANVAS_HEIGHT;
        stars[i].pattern = rand() % 7;
        stars[i].color_r = 200 + rand() % 55;
        stars[i].color_g = 200 + rand() % 55;
        stars[i].color_b = 200 + rand() % 55;
        stars[i].blink_frame = 0;
        stars[i].blink_speed = 30 + rand() % 40;
        stars[i].frame_counter = 0;
        stars[i].visible = 1;
        stars[i].pulse_state = 0;
        stars[i].is_pulsing = (rand() % 100 < 30) ? 1 : 0;
    }
}

void stars_update(void) {
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].frame_counter++;
        if (stars[i].frame_counter >= stars[i].blink_speed) {
            stars[i].frame_counter = 0;
            stars[i].visible = !stars[i].visible;
        }
        if (stars[i].is_pulsing) {
            stars[i].pulse_state = stars[i].visible ? 1 : 0;
        }
    }
}

void stars_get(int index, Star **out) {
    if (index >= 0 && index < STAR_COUNT) {
        *out = &stars[index];
    } else {
        *out = NULL;
    }
}

int stars_get_count(void) {
    return STAR_COUNT;
}
