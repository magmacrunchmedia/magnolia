#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <grrlib.h>

/* Screens are authored against a fixed 640x480 design space. At draw time those
   coordinates are mapped into the TV-safe rectangle of whatever video mode is
   actually running (NTSC 640x480, PAL 640x528), so a layout does not have to know
   which console it is on or how much of the picture the TV eats. */
#define UI_DESIGN_WIDTH   640
#define UI_DESIGN_HEIGHT  480

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

void ui_draw_text_shadow(int design_x, int design_y, const char *text,
                         unsigned int design_size, u32 color);
void ui_draw_centered_text(int design_y, const char *text,
                           unsigned int design_size, u32 color);
void ui_draw_border(void);

#endif
