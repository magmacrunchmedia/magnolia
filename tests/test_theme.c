/* The palette generator: HSL to RGB, and the complementary colour derived from
 * it.
 *
 * Nothing but arithmetic, and until theme.h stopped requiring libogc for a
 * single typedef there was no way to run it anywhere but a console. An HSL
 * conversion is exactly the kind of code that looks right: it is short, it has
 * no branches worth worrying about, and when it is wrong the output is still a
 * colour -- just the wrong one, on a television, in a game where the palette
 * changes every ten obstacles anyway.
 *
 *   make test-theme
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "theme.h"

/* The conversion ends in a truncating cast to u8, so exact equality is the
   wrong test for anything that does not land on a whole channel value. One
   count of slack absorbs that without letting a real error through. */
static void check_rgb(ThemeColor got, int r, int g, int b, const char *what) {
    checks++;
    int dr = (int)got.r - r, dg = (int)got.g - g, db = (int)got.b - b;
    if (dr < 0) dr = -dr;
    if (dg < 0) dg = -dg;
    if (db < 0) db = -db;

    if (dr > 1 || dg > 1 || db > 1) {
        printf("  FAIL: %s (got %d,%d,%d want %d,%d,%d)\n",
               what, got.r, got.g, got.b, r, g, b);
        failures++;
    }
}

static void test_known_colours(void) {
    printf("theme: HSL conversions with known answers\n");

    /* The three primaries sit at thirds of the hue circle. */
    check_rgb(theme_hsl_to_rgb(0.0f,        1.0f, 0.5f), 255, 0, 0, "hue 0 is red");
    check_rgb(theme_hsl_to_rgb(1.0f / 3.0f, 1.0f, 0.5f), 0, 255, 0, "hue 1/3 is green");
    check_rgb(theme_hsl_to_rgb(2.0f / 3.0f, 1.0f, 0.5f), 0, 0, 255, "hue 2/3 is blue");

    /* And the secondaries between them, which is where a sector boundary error
       shows up rather than in the primaries. */
    check_rgb(theme_hsl_to_rgb(1.0f / 6.0f, 1.0f, 0.5f), 255, 255, 0, "hue 1/6 is yellow");
    check_rgb(theme_hsl_to_rgb(0.5f,        1.0f, 0.5f), 0, 255, 255, "hue 1/2 is cyan");
    check_rgb(theme_hsl_to_rgb(5.0f / 6.0f, 1.0f, 0.5f), 255, 0, 255, "hue 5/6 is magenta");

    /* Saturation and lightness override hue entirely at their extremes. */
    check_rgb(theme_hsl_to_rgb(0.42f, 0.0f, 0.5f), 127, 127, 127, "no saturation is grey");
    check_rgb(theme_hsl_to_rgb(0.42f, 1.0f, 0.0f), 0, 0, 0, "no lightness is black");
    check_rgb(theme_hsl_to_rgb(0.42f, 1.0f, 1.0f), 255, 255, 255, "full lightness is white");

    /* Grey at any hue is the same grey. */
    ThemeColor g1 = theme_hsl_to_rgb(0.1f, 0.0f, 0.25f);
    ThemeColor g2 = theme_hsl_to_rgb(0.9f, 0.0f, 0.25f);
    check(g1.r == g2.r && g1.g == g2.g && g1.b == g2.b,
          "an unsaturated colour ignores its hue");
    check(g1.r == g1.g && g1.g == g1.b, "and has equal channels");
}

static void test_hue_wraps(void) {
    printf("theme: the hue circle joins up\n");

    ThemeColor zero = theme_hsl_to_rgb(0.0f, 0.8f, 0.5f);
    ThemeColor one  = theme_hsl_to_rgb(1.0f, 0.8f, 0.5f);
    check_rgb(one, zero.r, zero.g, zero.b, "hue 1.0 is the same colour as hue 0.0");

    /* hue_to_rgb() shifts each channel by a third and folds the result back into
       0..1. Off the end in either direction has to land somewhere sensible, and
       a fold that is missing shows as a black or clipped channel. */
    ThemeColor over  = theme_hsl_to_rgb(1.25f, 1.0f, 0.5f);
    ThemeColor under = theme_hsl_to_rgb(0.25f, 1.0f, 0.5f);
    check_rgb(over, under.r, under.g, under.b, "a hue past 1 folds back into range");

    /* Walking the circle must not produce a black gap anywhere: at full
       saturation and half lightness every hue has at least one lit channel. */
    int dark = 0;
    for (int i = 0; i < 360; i++) {
        ThemeColor c = theme_hsl_to_rgb((float)i / 360.0f, 1.0f, 0.5f);
        if (c.r == 0 && c.g == 0 && c.b == 0) dark++;
    }
    check_int(dark, 0, "no hue on the circle comes out black");
}

static void test_complementary(void) {
    printf("theme: the complementary colour\n");

    /* Half a turn round the circle, which is what makes a milestone marker read
       against the obstacle it sits on. Checked by construction rather than by
       eye: the complement of a theme must equal the direct conversion of the
       opposite hue at the boosted saturation and lightness. */
    Theme t;
    memset(&t, 0, sizeof(t));
    t.hue = 0.1f;
    t.sat = 0.5f;
    t.light = 0.4f;

    ThemeColor comp = theme_complementary(&t);
    ThemeColor want = theme_hsl_to_rgb(0.6f, 0.5f * 1.3f, 0.5f);  /* 0.48 clamps up to 0.5 */
    check_rgb(comp, want.r, want.g, want.b, "the hue is rotated half a turn");

    /* The wrap: a hue already past the halfway point must come back round
       rather than running off the end. */
    t.hue = 0.8f;
    ThemeColor wrapped = theme_complementary(&t);
    ThemeColor wrapped_want = theme_hsl_to_rgb(0.3f, 0.5f * 1.3f, 0.5f);
    check_rgb(wrapped, wrapped_want.r, wrapped_want.g, wrapped_want.b,
              "a hue past 0.5 wraps rather than overflowing");

    /* Lightness is held in a band so the marker stays legible whatever the
       obstacle behind it is doing -- never darker than mid, never washed out. */
    for (int i = 0; i <= 20; i++) {
        Theme probe;
        memset(&probe, 0, sizeof(probe));
        probe.hue = 0.25f;
        probe.sat = 1.0f;
        probe.light = (float)i / 20.0f;

        ThemeColor c = theme_complementary(&probe);
        int max = c.r > c.g ? c.r : c.g;
        if (c.b > max) max = c.b;
        if (max < 100) {
            check(0, "a complementary colour is never too dark to read");
            break;
        }
        if (i == 20) check(1, "a complementary colour is never too dark to read");
    }

    /* Saturation is boosted but must not wrap past full. */
    Theme hot;
    memset(&hot, 0, sizeof(hot));
    hot.hue = 0.0f;
    hot.sat = 0.95f;          /* * 1.3 would be 1.235 without the clamp */
    hot.light = 0.5f;
    ThemeColor h = theme_complementary(&hot);
    ThemeColor clamped = theme_hsl_to_rgb(0.5f, 1.0f, 0.6f);
    check_rgb(h, clamped.r, clamped.g, clamped.b, "boosted saturation clamps at full");
}

static void test_generate(void) {
    printf("theme: generated palettes stay in their documented ranges\n");

    /* theme_generate mirrors generateRandomTheme() in the web build: saturation
       40-80%, lightness 30-60%. Derived colours are computed from the stored
       floats rather than round-tripped through 8-bit RGB, so those floats being
       in range is load-bearing, not cosmetic. */
    srand(20260822u);

    int bad_hue = 0, bad_sat = 0, bad_light = 0;
    for (int i = 0; i < 2000; i++) {
        Theme t;
        theme_generate(&t);

        if (t.hue < 0.0f || t.hue >= 1.0f) bad_hue++;
        if (t.sat < 0.40f || t.sat > 0.80f) bad_sat++;
        if (t.light < 0.30f || t.light > 0.60f) bad_light++;
    }

    check_int(bad_hue, 0, "hue stays inside 0..1");
    check_int(bad_sat, 0, "saturation stays inside the documented 40-80%");
    check_int(bad_light, 0, "lightness stays inside the documented 30-60%");

    /* Secondary is darker and more saturated, accent lighter and less so. The
       header warns the deltas are additive percentage points and not
       multipliers, because getting that wrong inverts the shading -- so check
       the ordering rather than the arithmetic. */
    srand(1u);
    int inverted = 0;
    for (int i = 0; i < 500; i++) {
        Theme t;
        theme_generate(&t);

        int prim = (int)t.primary.r + t.primary.g + t.primary.b;
        int sec  = (int)t.secondary.r + t.secondary.g + t.secondary.b;
        int acc  = (int)t.accent.r + t.accent.g + t.accent.b;

        if (sec > prim || acc < prim) inverted++;
    }
    check_int(inverted, 0, "secondary is darker than primary and accent is lighter");

    /* Two calls give two palettes; a generator that returned a constant would
       pass everything above. */
    srand(7u);
    Theme a, b;
    theme_generate(&a);
    theme_generate(&b);
    check(a.hue != b.hue || a.sat != b.sat || a.light != b.light,
          "successive palettes actually differ");
}

int main(void) {
    test_known_colours();
    test_hue_wraps();
    test_complementary();
    test_generate();
    return report();
}
