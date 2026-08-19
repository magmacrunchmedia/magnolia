#include <grrlib.h>
#include "ui_utils.h"
#include "renderer.h"

#define CYAN   RGBA(0, 212, 255, 255)
#define SHADOW RGBA(0, 0, 0, 255)

static int overscan_pct = 6;

void ui_set_overscan_pct(int pct) {
    if (pct < 0)  pct = 0;
    if (pct > 20) pct = 20;
    overscan_pct = pct;
}

int ui_get_overscan_pct(void) { return overscan_pct; }

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
    GRRLIB_PrintfTTF(x + 2, y + 2, ttf_font, text, size, SHADOW);
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
    GRRLIB_PrintfTTF(x + 2, y + 2, ttf_font, text, size, SHADOW);
    GRRLIB_PrintfTTF(x, y, ttf_font, text, size, color);
}

void ui_draw_border(void) {
    int x = ui_safe_x(), y = ui_safe_y();
    int w = ui_safe_w(), h = ui_safe_h();
    int t = 3;
    GRRLIB_Rectangle(x, y, w, t, CYAN, true);
    GRRLIB_Rectangle(x, y + h - t, w, t, CYAN, true);
    GRRLIB_Rectangle(x, y, t, h, CYAN, true);
    GRRLIB_Rectangle(x + w - t, y, t, h, CYAN, true);
}
