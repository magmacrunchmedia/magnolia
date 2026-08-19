#ifndef THEME_H
#define THEME_H

#include <gccore.h>

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
