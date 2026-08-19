#ifndef RENDERER_H
#define RENDERER_H

#include <grrlib.h>

/* Video + font bring-up. Prefer magnolia_init() in core.h, which sequences this
   with the SD mount and the rest of the engine. */
int  renderer_init(void);
void renderer_shutdown(void);
int  renderer_fonts_loaded(void);

/* Real framebuffer geometry, taken from the running video mode rather than
   assumed. NTSC is 640x480, PAL is 640x528 -- never hardcode either. */
int  renderer_screen_width(void);
int  renderer_screen_height(void);

void renderer_draw_background(void);
void renderer_finish(void);

extern GRRLIB_ttfFont *ttf_font;

#endif
