#include <grrlib.h>
#include <math.h>
#include "renderer.h"
#include "stars.h"
#include "characters.h"

#include "PressStart2P.h"

GRRLIB_ttfFont *ttf_font = NULL;
static GRRLIB_texImg *tex_idle = NULL;
static GRRLIB_texImg *tex_thrust = NULL;
static int sprites_loaded = 0;

static u32 make_color(u8 r, u8 g, u8 b, u8 a) {
    return RGBA(r, g, b, a);
}

void renderer_init(void) {
    GRRLIB_Init();
    ttf_font = GRRLIB_LoadTTF(PressStart2P, PressStart2P_size);
}

void renderer_load_sprites(const CharacterData *ch) {
    if (tex_idle) { GRRLIB_FreeTexture(tex_idle); tex_idle = NULL; }
    if (tex_thrust) { GRRLIB_FreeTexture(tex_thrust); tex_thrust = NULL; }
    sprites_loaded = 0;

    if (!ch) return;

    tex_idle = GRRLIB_LoadTextureFromFile(ch->sprite_idle);
    tex_thrust = GRRLIB_LoadTextureFromFile(ch->sprite_thrust);
    if (tex_idle && tex_thrust) {
        sprites_loaded = 1;
    }
}

void renderer_draw_background(void) {
    GRRLIB_FillScreen(0x000000FF);
}

void renderer_draw_stars(void) {
    Star *s;
    for (int i = 0; i < stars_get_count(); i++) {
        stars_get(i, &s);
        if (s && s->visible) {
            u8 brightness = s->is_pulsing ? (s->pulse_state ? 255 : 128) : 255;
            u32 color = make_color(s->color_r, s->color_g, s->color_b, brightness);

            int size;
            switch (s->pattern % 4) {
                case 0: size = 1; break;
                case 1: size = 2; break;
                case 2: size = 3; break;
                default: size = 2; break;
            }
            GRRLIB_Rectangle((f32)s->x, (f32)s->y, (f32)size, (f32)size, color, true);
        }
    }
}

void renderer_draw_player(const Player *p, int thrust_active) {
    if (sprites_loaded && tex_idle && tex_thrust) {
        GRRLIB_texImg *tex = thrust_active ? tex_thrust : tex_idle;
        const CharacterData *ch = characters_get_current();
        float dx = p->x - (tex->w - ch->hitbox_w) / 2.0f;
        float dy = p->y - (tex->h - ch->hitbox_h) / 2.0f;
        GRRLIB_DrawImg(dx, dy, tex, 0, 1, 1, RGBA(255, 255, 255, 255));
    } else {
        u32 white = RGBA(255, 255, 255, 255);
        GRRLIB_Rectangle(p->x, p->y, (f32)p->width, (f32)p->height, white, true);
    }
}

void renderer_draw_score(int score) {
    char buf[16];
    int tmp = score;
    int len = 0;
    if (tmp == 0) {
        buf[len++] = '0';
    } else {
        char rev[16];
        int rlen = 0;
        while (tmp > 0) {
            rev[rlen++] = '0' + (tmp % 10);
            tmp /= 10;
        }
        for (int i = rlen - 1; i >= 0; i--) {
            buf[len++] = rev[i];
        }
    }
    buf[len] = '\0';
    GRRLIB_PrintfTTF(20, 10, ttf_font, "SCORE:", 14, RGBA(255, 255, 255, 255));
    GRRLIB_PrintfTTF(120, 10, ttf_font, buf, 14, RGBA(255, 255, 255, 255));
}

void renderer_draw_character_name(const char *name) {
    GRRLIB_PrintfSystemFont(20, 30, name, 16, RGBA(180, 180, 180, 255));
}

void renderer_finish(void) {
    GRRLIB_Render();
}
