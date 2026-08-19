#ifndef RENDERER_H
#define RENDERER_H

#include <grrlib.h>
#include "player.h"
#include "characters.h"

void renderer_init(void);
void renderer_load_sprites(const CharacterData *ch);
void renderer_draw_background(void);
void renderer_draw_stars(void);
void renderer_draw_player(const Player *p, int thrust_active);
void renderer_draw_score(int score);
void renderer_draw_character_name(const char *name);
void renderer_finish(void);

extern GRRLIB_ttfFont *ttf_font;

#endif
