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

/* Same, from an image already in memory -- typically a PNG linked into the
   binary. Assets embedded this way cannot go missing or fall out of step with
   the code, which file-backed assets very much can. */
int  sprite_load_mem(Sprite *s, const void *data, int origin_x, int origin_y);
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

/* Independent horizontal and vertical factors, for projecting a world onto a
   framebuffer whose pixels are not square -- 16:9 output stretches a 640x480
   frame, so a single scale that is right vertically is a third too wide. The
   origin still lands exactly on (x, y). */
void sprite_draw_scaled_xy(const Sprite *s, float x, float y, float sx, float sy);
void sprite_draw_scaled_xy_tinted(const Sprite *s, float x, float y, float sx, float sy, u32 tint);

/* Everything above, plus mirroring. See the note on flipping below. */
void sprite_draw_ex(const Sprite *s, float x, float y,
                    float sx, float sy, int flip_h, u32 tint);

/* --- Sprite sheets ---------------------------------------------------------
 *
 * A uniform grid of frames in one texture: cells of frame_w x frame_h, counted
 * left-to-right then top-to-bottom. This is the format SPRITE//FORGE exports and
 * all three MagmaCrunch engines read (adenosine, magnolia, texastoast); the
 * canonical spec is adenosine/packages/rpg/API.md and changing it is a
 * three-repo change, not a decision to take here.
 *
 * One texture rather than one per frame, because a character with a dozen
 * animation states is a dozen textures to load, track and free otherwise -- and
 * because the GPU would rather bind once.
 *
 * The origin is the frame's, not the sheet's: (0,0) is the top-left of whichever
 * cell is being drawn. It is stated at load time for the same reason the plain
 * Sprite states it there -- where a character's feet are is a property of how the
 * art was drawn, and re-deriving it per draw site is how two call sites end up
 * disagreeing about where the ground is.
 */
typedef struct {
    Sprite base;              /* the texture and the per-frame origin */
    int frame_w, frame_h;
    int cols, rows;
    int count;                /* cols * rows */
} SpriteSheet;

/* Frame size must divide the image, and both must be positive; anything else
   leaves the sheet empty and safe to draw, as a failed load does. A sheet whose
   cells do not tile its texture is a mis-exported asset, and drawing the
   plausible part of it hides that. */
int  sprite_sheet_load(SpriteSheet *s, const char *path,
                       int frame_w, int frame_h, int origin_x, int origin_y);
int  sprite_sheet_load_mem(SpriteSheet *s, const void *data,
                           int frame_w, int frame_h, int origin_x, int origin_y);
void sprite_sheet_free(SpriteSheet *s);
int  sprite_sheet_valid(const SpriteSheet *s);
int  sprite_sheet_count(const SpriteSheet *s);

/* Places the frame's origin at (x, y). A frame index outside the sheet draws
   nothing: an animation that runs off the end of its strip is a bug in the
   animation, and a wrapped or clamped frame disguises it as a glitch. */
void sprite_sheet_draw(const SpriteSheet *s, int frame, float x, float y);

/* Scaled, mirrored and tinted. Mirroring reflects the frame about its own
   origin: the origin's distance from the left edge becomes its distance from the
   right, so a character anchored at the front foot stays anchored at the front
   foot when they turn around, and a fighter does not slide half their width
   across the floor on every turn.
 *
 * Note for anyone reaching for GRRLIB_BMFX_FlipH() instead: that builds a
 * mirrored copy of the texture pixel by pixel. Fine once at load time, ruinous
 * once per frame per character -- flipping here costs nothing, because it is
 * done by handing the quad its corners in the other order. */
void sprite_sheet_draw_ex(const SpriteSheet *s, int frame, float x, float y,
                          float sx, float sy, int flip_h, u32 tint);

#endif
