#include <math.h>
#include <stdlib.h>
#include "theme.h"

static float hue_to_rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

ThemeColor theme_hsl_to_rgb(float h, float s, float l) {
    ThemeColor c;
    float r, g, b;

    if (s == 0.0f) {
        r = g = b = l;
    } else {
        float q = (l < 0.5f) ? (l * (1.0f + s)) : (l + s - l * s);
        float p = 2.0f * l - q;
        r = hue_to_rgb(p, q, h + 1.0f / 3.0f);
        g = hue_to_rgb(p, q, h);
        b = hue_to_rgb(p, q, h - 1.0f / 3.0f);
    }

    c.r = (u8)(r * 255.0f);
    c.g = (u8)(g * 255.0f);
    c.b = (u8)(b * 255.0f);
    return c;
}

void theme_generate(Theme *t) {
    float h = (float)(rand() % 360) / 360.0f;
    float s = 0.5f + (float)(rand() % 300) / 1000.0f;
    float l = 0.3f + (float)(rand() % 200) / 1000.0f;

    t->primary = theme_hsl_to_rgb(h, s, l);
    t->secondary = theme_hsl_to_rgb(h, s * 0.8f, l + 0.1f);
    t->accent = theme_hsl_to_rgb(h, s * 1.2f, l + 0.15f);
}
