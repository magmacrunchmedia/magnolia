#ifndef PLAYER_H
#define PLAYER_H

typedef struct {
    float x;
    float y;
    float velocity;
    float thrust;
    float gravity;
    float maxVelocity;
    int width;
    int height;
    int offsetX;
    int offsetY;
} Player;

void player_init(Player *p);
void player_set_physics(Player *p, float thrust, float gravity, float maxVel);
void player_set_hitbox(Player *p, int w, int h, int ox, int oy);
int player_update(Player *p, int thrust_active, int canvas_height);
void player_get_hitbox(const Player *p, int *x, int *y, int *w, int *h);

#endif
