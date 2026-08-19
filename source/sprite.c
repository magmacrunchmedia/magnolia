#include <stddef.h>
#include "sprite.h"

int sprite_load(Sprite *s, const char *path, int origin_x, int origin_y) {
    if (!s) return 0;
    s->tex = NULL;
    s->origin_x = origin_x;
    s->origin_y = origin_y;
    if (!path) return 0;

    s->tex = GRRLIB_LoadTextureFromFile(path);
    return s->tex != NULL;
}

void sprite_free(Sprite *s) {
    if (!s || !s->tex) return;
    GRRLIB_FreeTexture(s->tex);
    s->tex = NULL;
}

int sprite_valid(const Sprite *s) {
    return s && s->tex;
}

int sprite_width(const Sprite *s) {
    return sprite_valid(s) ? s->tex->w : 0;
}

int sprite_height(const Sprite *s) {
    return sprite_valid(s) ? s->tex->h : 0;
}

void sprite_draw_scaled_tinted(const Sprite *s, float x, float y,
                               float scale, u32 tint) {
    if (!sprite_valid(s)) return;
    /* Offset scales with the image, keeping the origin on (x, y) at any size. */
    GRRLIB_DrawImg(x - (float)s->origin_x * scale,
                   y - (float)s->origin_y * scale,
                   s->tex, 0, scale, scale, tint);
}

void sprite_draw_scaled(const Sprite *s, float x, float y, float scale) {
    sprite_draw_scaled_tinted(s, x, y, scale, RGBA(255, 255, 255, 255));
}

void sprite_draw_tinted(const Sprite *s, float x, float y, u32 tint) {
    sprite_draw_scaled_tinted(s, x, y, 1.0f, tint);
}

void sprite_draw(const Sprite *s, float x, float y) {
    sprite_draw_tinted(s, x, y, RGBA(255, 255, 255, 255));
}
