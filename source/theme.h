#ifndef THEME_H
#define THEME_H

/* gccore is here for one typedef, and requiring it costs more than it gives:
   this header sits at the bottom of Moonlight Drift's obstacles.h, so pulling
   in libogc made that game's richest module impossible to compile on a
   development machine, and left theme.c -- which is nothing but arithmetic --
   untested in the engine too. devkitPPC defines GEKKO for the console build.
   Same trick as george-boole-wii/source/palette.h. */
#ifdef GEKKO
#include <gccore.h>
#else
typedef unsigned char u8;
#endif

typedef struct {
    u8 r, g, b;
} ThemeColor;

typedef struct {
    ThemeColor primary;
    ThemeColor secondary;
    ThemeColor accent;
    /* Generator inputs, kept so derived colours (e.g. the complementary used by
       milestone markers) can be computed exactly instead of round-tripping
       through 8-bit RGB. All normalised 0..1. */
    float hue;
    float sat;
    float light;
} Theme;

void theme_generate(Theme *t);
ThemeColor theme_hsl_to_rgb(float h, float s, float l);
/* Hue rotated 180 degrees, with saturation/lightness boosted for legibility --
   mirrors getComplementaryColor() in the source game's js/obstacles.js. */
ThemeColor theme_complementary(const Theme *t);

#endif
