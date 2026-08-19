#include <grrlib.h>
#include "ui_utils.h"
#include "renderer.h"

#define CYAN  RGBA(0, 212, 255, 255)
#define SHADOW RGBA(0, 0, 0, 255)

static int get_screen_width(void) {
    extern GXRModeObj *rmode;
    return rmode ? rmode->fbWidth : 640;
}

void ui_draw_text_shadow(int x, int y, const char *text, unsigned int size, u32 color) {
    GRRLIB_PrintfTTF(x + 2, y + 2, ttf_font, text, size, SHADOW);
    GRRLIB_PrintfTTF(x, y, ttf_font, text, size, color);
}

void ui_draw_centered_text(int y, const char *text, unsigned int size, u32 color) {
    u32 w = GRRLIB_WidthTTF(ttf_font, text, size);
    int screen_w = get_screen_width();
    int x = (screen_w - (int)w) / 2;
    if (x < 0) x = 0;
    ui_draw_text_shadow(x, y, text, size, color);
}

void ui_draw_border(void) {
    int screen_w = get_screen_width();
    extern GXRModeObj *rmode;
    int screen_h = rmode ? rmode->efbHeight : 480;
    int t = 3;
    GRRLIB_Rectangle(0, 0, screen_w, t, CYAN, true);
    GRRLIB_Rectangle(0, screen_h - t, screen_w, t, CYAN, true);
    GRRLIB_Rectangle(0, 0, t, screen_h, CYAN, true);
    GRRLIB_Rectangle(screen_w - t, 0, t, screen_h, CYAN, true);
}
