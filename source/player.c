#include "player.h"
#include "config.h"

void player_init(Player *p) {
    p->x = PLAYER_X;
    p->y = PLAYER_Y_INITIAL;
    p->velocity = 0.0f;
    p->thrust = DEFAULT_THRUST;
    p->gravity = DEFAULT_GRAVITY;
    p->maxVelocity = DEFAULT_MAX_VELOCITY;
    p->width = 40;
    p->height = 35;
    p->offsetX = 0;
    p->offsetY = 0;
}

void player_set_physics(Player *p, float thrust, float gravity, float maxVel) {
    p->thrust = thrust;
    p->gravity = gravity;
    p->maxVelocity = maxVel;
}

void player_set_hitbox(Player *p, int w, int h, int ox, int oy) {
    p->width = w;
    p->height = h;
    p->offsetX = ox;
    p->offsetY = oy;
}

int player_update(Player *p, int thrust_active, int canvas_height) {
    if (thrust_active) {
        p->velocity += p->thrust;
    } else {
        p->velocity += p->gravity;
    }

    if (p->velocity > p->maxVelocity) p->velocity = p->maxVelocity;
    if (p->velocity < -p->maxVelocity) p->velocity = -p->maxVelocity;

    p->y += p->velocity;

    /* Boundary death is measured against the hitbox, not the sprite box, so a
       character whose art sits off its origin does not die early. Matches
       updatePlayer() in the source game's js/player.js. */
    if (p->y + p->offsetY < 0) return 1;
    if (p->y + p->offsetY + p->height > canvas_height) return 1;

    return 0;
}

void player_get_hitbox(const Player *p, int *x, int *y, int *w, int *h) {
    *x = (int)p->x + p->offsetX;
    *y = (int)p->y + p->offsetY;
    *w = p->width;
    *h = p->height;
}
