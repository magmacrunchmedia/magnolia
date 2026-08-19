#include <grrlib.h>
#include <math.h>
#include <fat.h>
#include "renderer.h"
#include "stars.h"
#include "characters.h"

#include "PressStart2P.h"

GRRLIB_ttfFont *ttf_font = NULL;
static GRRLIB_texImg *tex_idle = NULL;
static GRRLIB_texImg *tex_thrust = NULL;
static int sprites_loaded = 0;
static int sd_mounted = 0;

static u32 make_color(u8 r, u8 g, u8 b, u8 a) {
    return RGBA(r, g, b, a);
}

int renderer_init(void) {
    int status = 0;

    /* Must happen before any sd:/ path is opened. Without it every
       GRRLIB_LoadTextureFromFile() call silently returns NULL and the game
       falls back to placeholder rectangles. */
    sd_mounted = fatInitDefault() ? 1 : 0;
    if (!sd_mounted) status = -1;

    if (GRRLIB_Init() < 0) return -2;

    ttf_font = GRRLIB_LoadTTF(PressStart2P, PressStart2P_size);
    if (!ttf_font) status = -3;

    return status;
}

void renderer_shutdown(void) {
    if (tex_idle)   { GRRLIB_FreeTexture(tex_idle);   tex_idle = NULL; }
    if (tex_thrust) { GRRLIB_FreeTexture(tex_thrust); tex_thrust = NULL; }
    sprites_loaded = 0;
    if (ttf_font) { GRRLIB_FreeTTF(ttf_font); ttf_font = NULL; }
    GRRLIB_Exit();
}

int renderer_sd_mounted(void) { return sd_mounted; }
int renderer_fonts_loaded(void) { return ttf_font != NULL; }
int renderer_sprites_loaded(void) { return sprites_loaded; }

int renderer_screen_width(void) {
    extern GXRModeObj *rmode;
    return rmode ? rmode->fbWidth : 640;
}

int renderer_screen_height(void) {
    extern GXRModeObj *rmode;
    return rmode ? rmode->efbHeight : 480;
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

    /* Cyan rules on the lethal top/bottom edges, as drawBackground() does in
       js/renderer.js. On a TV these also tell the player where the invisible
       kill boundary actually is once overscan eats the outer rows. */
    int w = renderer_screen_width();
    int h = renderer_screen_height();
    u32 edge = RGBA(0, 212, 255, 77);
    GRRLIB_Rectangle(0, 0, (f32)w, 2, edge, true);
    GRRLIB_Rectangle(0, (f32)(h - 2), (f32)w, 2, edge, true);
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
        /* The sprite was exported with the character's local (0,0) landing at
           sprite_origin_*, and hitbox offsets are relative to that same origin,
           so placing the origin at the player position keeps art and hitbox aligned. */
        float dx = p->x - (float)ch->sprite_origin_x;
        float dy = p->y - (float)ch->sprite_origin_y;
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
    if (!ttf_font) return;
    GRRLIB_PrintfTTF(20, 10, ttf_font, "SCORE:", 14, RGBA(255, 255, 255, 255));
    GRRLIB_PrintfTTF(120, 10, ttf_font, buf, 14, RGBA(255, 255, 255, 255));
}

void renderer_draw_character_name(const char *name) {
    GRRLIB_PrintfSystemFont(20, 30, name, 16, RGBA(180, 180, 180, 255));
}

void renderer_finish(void) {
    GRRLIB_Render();
}
