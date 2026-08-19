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
} Theme;

void theme_generate(Theme *t);
ThemeColor theme_hsl_to_rgb(float h, float s, float l);

#endif
