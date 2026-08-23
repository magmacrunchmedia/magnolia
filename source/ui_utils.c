#include <grrlib.h>
#include <stdio.h>
#include <string.h>
#include "ui_utils.h"
#include "renderer.h"


static int overscan_pct = 6;

/* The drop shadow under every string this module draws, and the border colour.
   Both were compile-time constants until a game put dark text on a pale panel:
   a black shadow under a red card rank turns the glyph into a smudge, and a
   cyan border on a lava table reads as another program's chrome. Settable for
   the same reason the overscan is -- it is a property of the game's look, not
   of the drawing. Defaults are what they always were, so a caller that never
   touches them sees no change. */
static u32 shadow_color = 0x000000FF;
static u32 border_color = 0x00D4FFFF;

void ui_set_overscan_pct(int pct) {
    if (pct < 0)  pct = 0;
    if (pct > 20) pct = 20;
    overscan_pct = pct;
}

int ui_get_overscan_pct(void) { return overscan_pct; }

void ui_set_shadow_color(u32 color) { shadow_color = color; }
u32  ui_get_shadow_color(void)      { return shadow_color; }

void ui_set_border_color(u32 color) { border_color = color; }
u32  ui_get_border_color(void)      { return border_color; }

int ui_safe_x(void) { return (renderer_screen_width()  * overscan_pct) / 100; }
int ui_safe_y(void) { return (renderer_screen_height() * overscan_pct) / 100; }
int ui_safe_w(void) { return renderer_screen_width()  - 2 * ui_safe_x(); }
int ui_safe_h(void) { return renderer_screen_height() - 2 * ui_safe_y(); }

int ui_map_x(int design_x) {
    return ui_safe_x() + (design_x * ui_safe_w()) / UI_DESIGN_WIDTH;
}

int ui_map_y(int design_y) {
    return ui_safe_y() + (design_y * ui_safe_h()) / UI_DESIGN_HEIGHT;
}

int ui_map_w(int design_w) {
    return (design_w * ui_safe_w()) / UI_DESIGN_WIDTH;
}

int ui_map_h(int design_h) {
    return (design_h * ui_safe_h()) / UI_DESIGN_HEIGHT;
}

unsigned int ui_map_size(unsigned int design_size) {
    /* Scale glyphs by the vertical factor so text keeps its proportions. */
    unsigned int s = (design_size * (unsigned int)ui_safe_h()) / UI_DESIGN_HEIGHT;
    return s < 6 ? 6 : s;
}

void ui_draw_text_shadow(int design_x, int design_y, const char *text,
                         unsigned int design_size, u32 color) {
    if (!ttf_font) return;
    int x = ui_map_x(design_x);
    int y = ui_map_y(design_y);
    unsigned int size = ui_map_size(design_size);
    GRRLIB_PrintfTTF(x + 2, y + 2, ttf_font, text, size, shadow_color);
    GRRLIB_PrintfTTF(x, y, ttf_font, text, size, color);
}

void ui_draw_centered_text(int design_y, const char *text,
                           unsigned int design_size, u32 color) {
    if (!ttf_font) return;
    unsigned int size = ui_map_size(design_size);
    u32 w = GRRLIB_WidthTTF(ttf_font, text, size);
    int x = ui_safe_x() + (ui_safe_w() - (int)w) / 2;
    int y = ui_map_y(design_y);
    if (x < 0) x = 0;
    GRRLIB_PrintfTTF(x + 2, y + 2, ttf_font, text, size, shadow_color);
    GRRLIB_PrintfTTF(x, y, ttf_font, text, size, color);
}

void ui_draw_border(void) {
    int x = ui_safe_x(), y = ui_safe_y();
    int w = ui_safe_w(), h = ui_safe_h();
    int t = 3;
    GRRLIB_Rectangle(x, y, w, t, border_color, true);
    GRRLIB_Rectangle(x, y + h - t, w, t, border_color, true);
    GRRLIB_Rectangle(x, y, t, h, border_color, true);
    GRRLIB_Rectangle(x + w - t, y, t, h, border_color, true);
}

int ui_text_width(const char *text, unsigned int design_size) {
    if (!ttf_font || !text) return 0;
    u32 w = GRRLIB_WidthTTF(ttf_font, text, ui_map_size(design_size));
    /* Back out of screen space into design space, so the result composes with
       the design-space coordinates the caller is working in. */
    int safe_w = ui_safe_w();
    if (safe_w <= 0) return 0;
    return (int)((w * UI_DESIGN_WIDTH) / (u32)safe_w);
}

void ui_draw_text_centered_in(int design_x, int design_y,
                              int design_w, int design_h,
                              const char *text, unsigned int design_size,
                              u32 color) {
    if (!ttf_font || !text) return;
    unsigned int size = ui_map_size(design_size);
    u32 tw = GRRLIB_WidthTTF(ttf_font, text, size);

    int x = ui_map_x(design_x) + (ui_map_w(design_w) - (int)tw) / 2;
    /* GRRLIB places text by its top-left, and a glyph box is close enough to the
       point size that centring on it reads correctly at these sizes. */
    int y = ui_map_y(design_y) + (ui_map_h(design_h) - (int)size) / 2;

    GRRLIB_PrintfTTF(x + 2, y + 2, ttf_font, text, size, shadow_color);
    GRRLIB_PrintfTTF(x, y, ttf_font, text, size, color);
}

/* GX has no rounded-rectangle primitive, so a radius is built from a middle
   band, two inset side bands and four quarter-circles. Cheap at UI scale, and it
   keeps every panel in the game the same shape. */
static void panel_fill(int x, int y, int w, int h, u32 color, int r) {
    if ((color & 0xFF) == 0) return;

    if (r <= 0) {
        GRRLIB_Rectangle((f32)x, (f32)y, (f32)w, (f32)h, color, true);
        return;
    }
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;

    GRRLIB_Rectangle((f32)x, (f32)(y + r), (f32)w, (f32)(h - 2 * r), color, true);
    GRRLIB_Rectangle((f32)(x + r), (f32)y, (f32)(w - 2 * r), (f32)r, color, true);
    GRRLIB_Rectangle((f32)(x + r), (f32)(y + h - r), (f32)(w - 2 * r), (f32)r, color, true);

    GRRLIB_Circle((f32)(x + r),         (f32)(y + r),         (f32)r, color, true);
    GRRLIB_Circle((f32)(x + w - r),     (f32)(y + r),         (f32)r, color, true);
    GRRLIB_Circle((f32)(x + r),         (f32)(y + h - r),     (f32)r, color, true);
    GRRLIB_Circle((f32)(x + w - r),     (f32)(y + h - r),     (f32)r, color, true);
}

void ui_draw_panel(int design_x, int design_y, int design_w, int design_h,
                   u32 fill, u32 outline, int radius) {
    int x = ui_map_x(design_x);
    int y = ui_map_y(design_y);
    int w = ui_map_w(design_w);
    int h = ui_map_h(design_h);
    int r = ui_map_w(radius);

    panel_fill(x, y, w, h, fill, r);

    if ((outline & 0xFF) != 0) {
        /* Drawn as a rectangle rather than following the corner arcs: at these
           radii the difference is invisible on a CRT and the arc version costs
           four more circles a panel. */
        GRRLIB_Rectangle((f32)x, (f32)y, (f32)w, (f32)h, outline, false);
    }
}

void ui_draw_dim_overlay(u32 color) {
    GRRLIB_Rectangle(0.0f, 0.0f,
                     (f32)renderer_screen_width(), (f32)renderer_screen_height(),
                     color, true);
}

int ui_draw_text_wrapped(int design_x, int design_y, int design_w,
                         const char *text, unsigned int design_size,
                         u32 color, int line_spacing) {
    if (!ttf_font || !text) return design_y;
    if (line_spacing <= 0) line_spacing = (int)design_size + 4;

    char line[128];
    int line_len = 0;
    int y = design_y;

    const char *word = text;
    while (1) {
        const char *end = word;
        while (*end && *end != ' ' && *end != '\n') end++;

        int wlen = (int)(end - word);
        if (wlen > 0 && wlen < (int)sizeof(line)) {
            char candidate[128];
            if (line_len > 0) {
                snprintf(candidate, sizeof(candidate), "%.*s %.*s",
                         line_len, line, wlen, word);
            } else {
                snprintf(candidate, sizeof(candidate), "%.*s", wlen, word);
            }

            if (line_len > 0 && ui_text_width(candidate, design_size) > design_w) {
                ui_draw_text_shadow(design_x, y, line, design_size, color);
                y += line_spacing;
                snprintf(line, sizeof(line), "%.*s", wlen, word);
                line_len = (int)strlen(line);
            } else {
                snprintf(line, sizeof(line), "%s", candidate);
                line_len = (int)strlen(line);
            }
        }

        if (*end == '\n') {
            ui_draw_text_shadow(design_x, y, line, design_size, color);
            y += line_spacing;
            line[0] = '\0';
            line_len = 0;
        } else if (!*end) {
            break;
        }
        word = end + 1;
    }

    if (line_len > 0) {
        ui_draw_text_shadow(design_x, y, line, design_size, color);
        y += line_spacing;
    }
    return y;
}
