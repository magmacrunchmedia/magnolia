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

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

/* Mirrors generateRandomTheme() in the source game's js/obstacles.js:
   saturation 40-80%, lightness 30-60%, with secondary darker/more saturated and
   accent lighter/less saturated. The deltas are additive percentage points, not
   multipliers -- getting that wrong inverts the shading. */
void theme_generate(Theme *t) {
    float h = (float)(rand() % 360) / 360.0f;
    float s = (40.0f + (float)(rand() % 40)) / 100.0f;
    float l = (30.0f + (float)(rand() % 30)) / 100.0f;

    t->hue = h;
    t->sat = s;
    t->light = l;

    t->primary   = theme_hsl_to_rgb(h, s, l);
    t->secondary = theme_hsl_to_rgb(h, clamp01(s + 0.10f), clamp01(l - 0.10f));
    t->accent    = theme_hsl_to_rgb(h, clamp01(s - 0.10f), clamp01(l + 0.20f));
}

ThemeColor theme_complementary(const Theme *t) {
    float h = t->hue + 0.5f;
    if (h >= 1.0f) h -= 1.0f;

    float s = t->sat * 1.3f;
    if (s > 1.0f) s = 1.0f;

    float l = t->light * 1.2f;
    if (l < 0.5f) l = 0.5f;
    if (l > 0.7f) l = 0.7f;

    return theme_hsl_to_rgb(h, s, l);
}
