#ifndef SPRITE_H
#define SPRITE_H

#include <grrlib.h>

/* A texture plus the point inside it that should land on the caller's draw
   position. Exported art rarely has its logical origin at pixel (0,0) -- the
   origin is a property of how the asset was produced, so it travels with the
   sprite rather than being re-derived at every draw site. */
typedef struct {
    GRRLIB_texImg *tex;
    int origin_x;
    int origin_y;
} Sprite;

/* Returns 1 on success. Failure leaves the Sprite empty and safe to draw. */
int  sprite_load(Sprite *s, const char *path, int origin_x, int origin_y);
void sprite_free(Sprite *s);
int  sprite_valid(const Sprite *s);

int  sprite_width(const Sprite *s);
int  sprite_height(const Sprite *s);

/* Places the sprite's origin at (x, y). No-ops when the sprite failed to load,
   so a missing asset degrades to nothing drawn rather than a crash. */
void sprite_draw(const Sprite *s, float x, float y);
void sprite_draw_tinted(const Sprite *s, float x, float y, u32 tint);

/* Same placement rule, scaled about the origin -- so a sprite drawn at half
   size still has its origin land exactly on (x, y). For thumbnail grids. */
void sprite_draw_scaled(const Sprite *s, float x, float y, float scale);
void sprite_draw_scaled_tinted(const Sprite *s, float x, float y, float scale, u32 tint);

#endif
