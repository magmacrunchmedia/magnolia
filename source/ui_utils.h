#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <grrlib.h>

#include "ui_geom.h"

/* Screens are authored against a fixed 640x480 design space. At draw time those
   coordinates are mapped into the TV-safe rectangle of whatever video mode is
   actually running (NTSC 640x480, PAL 640x528), so a layout does not have to know
   which console it is on or how much of the picture the TV eats.

   UI_DESIGN_WIDTH and UI_DESIGN_HEIGHT come from ui_geom.h, which holds the
   projection arithmetic and is included here so games see them exactly where
   they always did. Everything in this header still draws; everything in that one
   only computes, which is what lets the host tests reach it. */

/* Percent of each edge assumed lost to overscan. Consumer CRTs typically clip
   5-10%; 6 keeps text clear without wasting much screen. */
void ui_set_overscan_pct(int pct);
int  ui_get_overscan_pct(void);

/* Safe-area rectangle in real screen pixels. */
int ui_safe_x(void);
int ui_safe_y(void);
int ui_safe_w(void);
int ui_safe_h(void);

/* Map design-space coordinates / sizes onto the safe area. */
int ui_map_x(int design_x);
int ui_map_y(int design_y);
int ui_map_w(int design_w);
int ui_map_h(int design_h);
unsigned int ui_map_size(unsigned int design_size);

/* The drop shadow under every string drawn by this module, and the colour of
   ui_draw_border(). Both were compile-time constants -- black and cyan -- which
   is right for light text on a dark background and wrong the moment a game puts
   a dark glyph on a pale panel: the shadow turns it into a smudge. A themed game
   sets these once at startup, as it does the overscan.

   Defaults are the original black and cyan, so a game that never calls these
   looks exactly as it did. */
void ui_set_shadow_color(u32 color);
u32  ui_get_shadow_color(void);
void ui_set_border_color(u32 color);
u32  ui_get_border_color(void);

void ui_draw_text_shadow(int design_x, int design_y, const char *text,
                         unsigned int design_size, u32 color);
void ui_draw_centered_text(int design_y, const char *text,
                           unsigned int design_size, u32 color);
void ui_draw_border(void);

/* Width of `text` in DESIGN units, not screen pixels. Layout arithmetic belongs
   in the design space; measuring in screen pixels and comparing against design
   coordinates is a mistake that only shows up on the video mode you did not
   test on. Returns 0 when no font loaded. */
int ui_text_width(const char *text, unsigned int design_size);

/* Centres text inside a design-space rectangle, horizontally and vertically.
   Every tile, button and cell wants this; hand-computing it per draw site is how
   a UI ends up subtly misaligned in six places. */
void ui_draw_text_centered_in(int design_x, int design_y,
                              int design_w, int design_h,
                              const char *text, unsigned int design_size,
                              u32 color);

/* Filled panel with an optional outline and optional rounded corners. `radius`
   is in design units; 0 gives a plain rectangle. Pass a fully transparent fill
   or outline to skip that part. Tiles, modals, buttons and selection cells are
   all this shape. */
void ui_draw_panel(int design_x, int design_y, int design_w, int design_h,
                   u32 fill, u32 outline, int radius);

/* Full-screen scrim for modal layers. */
void ui_draw_dim_overlay(u32 color);

/* Word-wrapped paragraph text inside a design-space column, breaking on spaces.
   Returns the design-space y just past the last line drawn, so callers can stack
   paragraphs without counting lines themselves. */
int ui_draw_text_wrapped(int design_x, int design_y, int design_w,
                         const char *text, unsigned int design_size,
                         u32 color, int line_spacing);

#endif
