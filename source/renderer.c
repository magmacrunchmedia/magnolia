#include <grrlib.h>
#include "renderer.h"

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
}
