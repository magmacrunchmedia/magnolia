#ifndef RENDERER_H
#define RENDERER_H

#include <grrlib.h>
#include "player.h"
#include "characters.h"

/* Returns 0 on success. Negative values mean the engine came up degraded but
   usable -- check renderer_sd_mounted()/renderer_fonts_loaded() for specifics. */
int  renderer_init(void);
void renderer_shutdown(void);

/* Init status, for callers that want to surface a diagnostic instead of
   silently rendering placeholder graphics. */
int  renderer_sd_mounted(void);
int  renderer_fonts_loaded(void);

void renderer_load_sprites(const CharacterData *ch);
int  renderer_sprites_loaded(void);

void renderer_draw_background(void);
void renderer_draw_stars(void);
void renderer_draw_player(const Player *p, int thrust_active);
void renderer_draw_score(int score);
void renderer_draw_character_name(const char *name);
void renderer_finish(void);

/* Actual framebuffer geometry, taken from the video mode rather than assumed.
   NTSC is 640x480, PAL is 640x528 -- callers must not hardcode either. */
int  renderer_screen_width(void);
int  renderer_screen_height(void);

extern GRRLIB_ttfFont *ttf_font;

#endif
