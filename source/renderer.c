#include <grrlib.h>
#include "renderer.h"
#include "clock.h"

#include "PressStart2P.h"

GRRLIB_ttfFont *ttf_font = NULL;

int renderer_init(void) {
    if (GRRLIB_Init() < 0) return -1;

    ttf_font = GRRLIB_LoadTTF(PressStart2P, PressStart2P_size);
    return ttf_font ? 0 : -2;
}

void renderer_shutdown(void) {
    if (ttf_font) { GRRLIB_FreeTTF(ttf_font); ttf_font = NULL; }
    GRRLIB_Exit();
}

int renderer_fonts_loaded(void) { return ttf_font != NULL; }

int renderer_screen_width(void) {
    extern GXRModeObj *rmode;
    return rmode ? rmode->fbWidth : 640;
}

int renderer_screen_height(void) {
    extern GXRModeObj *rmode;
    return rmode ? rmode->efbHeight : 480;
}

void renderer_draw_background(void) {
    GRRLIB_FillScreen(0x000000FF);
}

void renderer_finish(void) {
    GRRLIB_Render();
    clock_tick();
}

void renderer_splash(const char *line1, const char *line2) {
    GRRLIB_FillScreen(0x000000FF);
    if (ttf_font) {
        int w = renderer_screen_width();
        if (line1) {
            u32 tw = GRRLIB_WidthTTF(ttf_font, line1, 16);
            GRRLIB_PrintfTTF((w - (int)tw) / 2, 200, ttf_font, line1, 16,
                             RGBA(0, 212, 255, 255));
        }
        if (line2) {
            u32 tw = GRRLIB_WidthTTF(ttf_font, line2, 12);
            GRRLIB_PrintfTTF((w - (int)tw) / 2, 240, ttf_font, line2, 12,
                             RGBA(160, 160, 160, 255));
        }
    }
    GRRLIB_Render();
}
