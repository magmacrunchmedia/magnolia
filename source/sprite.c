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

int sprite_load_mem(Sprite *s, const void *data, int origin_x, int origin_y) {
    if (!s) return 0;
    s->tex = NULL;
    s->origin_x = origin_x;
    s->origin_y = origin_y;
    if (!data) return 0;

    /* GRRLIB sniffs the format from the header bytes. */
    s->tex = GRRLIB_LoadTexture((const u8 *)data);
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

void sprite_draw_scaled_xy_tinted(const Sprite *s, float x, float y,
                                  float sx, float sy, u32 tint) {
    if (!sprite_valid(s)) return;
    /* Offset scales with the image on each axis independently, keeping the
       origin on (x, y) whatever the two factors are. */
    GRRLIB_DrawImg(x - (float)s->origin_x * sx,
                   y - (float)s->origin_y * sy,
                   s->tex, 0, sx, sy, tint);
}

void sprite_draw_scaled_xy(const Sprite *s, float x, float y,
                           float sx, float sy) {
    sprite_draw_scaled_xy_tinted(s, x, y, sx, sy, RGBA(255, 255, 255, 255));
}

void sprite_draw_scaled_tinted(const Sprite *s, float x, float y,
                               float scale, u32 tint) {
    sprite_draw_scaled_xy_tinted(s, x, y, scale, scale, tint);
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

/* --- Mirroring -------------------------------------------------------------
 *
 * GRRLIB_DrawImg and GRRLIB_DrawTile place the top-left of the scaled image at
 * the position given and take the scale factors from there, which leaves no way
 * to mirror without also moving the image: a negative scaleX reflects the quad
 * about the position rather than about the sprite, so the art lands a full width
 * to the left of where it was asked for.
 *
 * The *Quad forms take the four corners outright. Handing them the same
 * rectangle with its two left corners and its two right corners exchanged
 * mirrors the texture inside a rectangle we placed ourselves -- so position and
 * mirroring stop fighting each other, and neither depends on how GRRLIB chooses
 * to interpret a negative scale.
 *
 * The unflipped paths above deliberately still go through GRRLIB_DrawImg. Three
 * shipped games draw every pixel through them, and routing that through new
 * arithmetic to save a few lines here would put their output at risk for no gain.
 */

/* Screen rectangle for a draw, honouring the origin. Flipping reflects about
   the origin, so the origin's offset from the left edge becomes its offset from
   the right and the anchor point does not move. */
static void quad_corners(guVector pos[4], float x, float y,
                         int origin_x, int origin_y, int w, int h,
                         float sx, float sy, int flip_h) {
    float ws = (float)w * sx;
    float hs = (float)h * sy;
    float ox = (float)origin_x * sx;

    float left  = flip_h ? (x + ox - ws) : (x - ox);
    float right = left + ws;
    float top    = y - (float)origin_y * sy;
    float bottom = top + hs;

    /* Corner order is top-left, top-right, bottom-right, bottom-left as far as
       the texture is concerned; swapping which screen edge the first and second
       corners sit on is what mirrors it. */
    float near_x = flip_h ? right : left;
    float far_x  = flip_h ? left  : right;

    pos[0].x = near_x; pos[0].y = top;    pos[0].z = 0.0f;
    pos[1].x = far_x;  pos[1].y = top;    pos[1].z = 0.0f;
    pos[2].x = far_x;  pos[2].y = bottom; pos[2].z = 0.0f;
    pos[3].x = near_x; pos[3].y = bottom; pos[3].z = 0.0f;
}

void sprite_draw_ex(const Sprite *s, float x, float y,
                    float sx, float sy, int flip_h, u32 tint) {
    if (!sprite_valid(s)) return;

    if (!flip_h) {
        sprite_draw_scaled_xy_tinted(s, x, y, sx, sy, tint);
        return;
    }

    guVector pos[4];
    quad_corners(pos, x, y, s->origin_x, s->origin_y,
                 (int)s->tex->w, (int)s->tex->h, sx, sy, 1);
    GRRLIB_DrawImgQuad(pos, s->tex, tint);
}

/* --- Sprite sheets --- */

static int sheet_setup(SpriteSheet *s, int frame_w, int frame_h) {
    if (!s->base.tex) return 0;

    /* Cells that do not tile the texture mean the sheet and the code disagree
       about the grid. Every frame past the first would then be drawn from the
       wrong pixels, which reads as scrambled art rather than as a bad number. */
    if (frame_w <= 0 || frame_h <= 0 ||
        (int)s->base.tex->w % frame_w != 0 ||
        (int)s->base.tex->h % frame_h != 0) {
        GRRLIB_FreeTexture(s->base.tex);
        s->base.tex = NULL;
        return 0;
    }

    s->frame_w = frame_w;
    s->frame_h = frame_h;
    s->cols = (int)s->base.tex->w / frame_w;
    s->rows = (int)s->base.tex->h / frame_h;
    s->count = s->cols * s->rows;

    /* GRRLIB numbers tiles left-to-right then top-to-bottom, which is the order
       the shared sheet format specifies -- so frame indices mean the same thing
       here as they do in the exporter and in the other two engines. */
    GRRLIB_InitTileSet(s->base.tex, (u32)frame_w, (u32)frame_h, 0);
    return 1;
}

static void sheet_clear(SpriteSheet *s, int origin_x, int origin_y) {
    s->base.tex = NULL;
    s->base.origin_x = origin_x;
    s->base.origin_y = origin_y;
    s->frame_w = 0;
    s->frame_h = 0;
    s->cols = 0;
    s->rows = 0;
    s->count = 0;
}

int sprite_sheet_load(SpriteSheet *s, const char *path,
                      int frame_w, int frame_h, int origin_x, int origin_y) {
    if (!s) return 0;
    sheet_clear(s, origin_x, origin_y);
    if (!path) return 0;

    s->base.tex = GRRLIB_LoadTextureFromFile(path);
    return sheet_setup(s, frame_w, frame_h);
}

int sprite_sheet_load_mem(SpriteSheet *s, const void *data,
                          int frame_w, int frame_h, int origin_x, int origin_y) {
    if (!s) return 0;
    sheet_clear(s, origin_x, origin_y);
    if (!data) return 0;

    s->base.tex = GRRLIB_LoadTexture((const u8 *)data);
    return sheet_setup(s, frame_w, frame_h);
}

void sprite_sheet_free(SpriteSheet *s) {
    if (!s) return;
    sprite_free(&s->base);
    sheet_clear(s, s->base.origin_x, s->base.origin_y);
}

int sprite_sheet_valid(const SpriteSheet *s) {
    return s && s->base.tex && s->count > 0;
}

int sprite_sheet_count(const SpriteSheet *s) {
    return sprite_sheet_valid(s) ? s->count : 0;
}

void sprite_sheet_draw_ex(const SpriteSheet *s, int frame, float x, float y,
                          float sx, float sy, int flip_h, u32 tint) {
    if (!sprite_sheet_valid(s)) return;
    if (frame < 0 || frame >= s->count) return;

    if (!flip_h) {
        GRRLIB_DrawTile(x - (float)s->base.origin_x * sx,
                        y - (float)s->base.origin_y * sy,
                        s->base.tex, 0, sx, sy, tint, frame);
        return;
    }

    guVector pos[4];
    quad_corners(pos, x, y, s->base.origin_x, s->base.origin_y,
                 s->frame_w, s->frame_h, sx, sy, 1);
    GRRLIB_DrawTileQuad(pos, s->base.tex, tint, frame);
}

void sprite_sheet_draw(const SpriteSheet *s, int frame, float x, float y) {
    sprite_sheet_draw_ex(s, frame, x, y, 1.0f, 1.0f, 0, RGBA(255, 255, 255, 255));
}
