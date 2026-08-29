#include <stdio.h>
#include <string.h>
#include "ui_geom.h"

UiGeom ui_geom_make(int screen_w, int screen_h, int overscan_pct) {
    UiGeom g;
    if (overscan_pct < 0)  overscan_pct = 0;
    if (overscan_pct > 20) overscan_pct = 20;
    g.screen_w     = screen_w;
    g.screen_h     = screen_h;
    g.overscan_pct = overscan_pct;
    return g;
}

int ui_geom_safe_x(const UiGeom *g) { return (g->screen_w * g->overscan_pct) / 100; }
int ui_geom_safe_y(const UiGeom *g) { return (g->screen_h * g->overscan_pct) / 100; }
int ui_geom_safe_w(const UiGeom *g) { return g->screen_w - 2 * ui_geom_safe_x(g); }
int ui_geom_safe_h(const UiGeom *g) { return g->screen_h - 2 * ui_geom_safe_y(g); }

int ui_geom_map_x(const UiGeom *g, int design_x) {
    return ui_geom_safe_x(g) + (design_x * ui_geom_safe_w(g)) / UI_DESIGN_WIDTH;
}

int ui_geom_map_y(const UiGeom *g, int design_y) {
    return ui_geom_safe_y(g) + (design_y * ui_geom_safe_h(g)) / UI_DESIGN_HEIGHT;
}

int ui_geom_map_w(const UiGeom *g, int design_w) {
    return (design_w * ui_geom_safe_w(g)) / UI_DESIGN_WIDTH;
}

int ui_geom_map_h(const UiGeom *g, int design_h) {
    return (design_h * ui_geom_safe_h(g)) / UI_DESIGN_HEIGHT;
}

unsigned int ui_geom_map_size(const UiGeom *g, unsigned int design_size) {
    /* Scale glyphs by the vertical factor so text keeps its proportions. */
    unsigned int s = (design_size * (unsigned int)ui_geom_safe_h(g)) / UI_DESIGN_HEIGHT;
    return s < 6 ? 6 : s;
}

int ui_geom_to_design_w(const UiGeom *g, unsigned int screen_w) {
    int safe_w = ui_geom_safe_w(g);
    if (safe_w <= 0) return 0;
    return (int)((screen_w * UI_DESIGN_WIDTH) / (unsigned int)safe_w);
}

/* The wrap, moved across from ui_utils.c unchanged in what it decides -- only in
   where it puts the answer. It used to draw each line as it settled it, which is
   why it could not be checked without a console; now it hands the line back and
   ui_utils draws it.

   One difference, and it is a fix rather than a port: `out` is terminated on
   entry. The old `char line[128]` was not, and text beginning with a newline
   reached the draw call before anything had been written to it. */
const char *ui_geom_wrap_next(const char *text, char *out, int out_n,
                              int design_w, unsigned int design_size,
                              UiMeasureFn measure, void *ctx) {
    if (!text || !out || out_n <= 0 || !measure) return NULL;

    out[0] = '\0';
    int out_len = 0;

    const char *word = text;
    while (1) {
        const char *end = word;
        while (*end && *end != ' ' && *end != '\n') end++;

        int wlen = (int)(end - word);
        /* A word with nowhere to go is skipped rather than broken. Splitting one
           would be a different function; breaking the buffer would be a bug. */
        if (wlen > 0 && wlen < out_n) {
            char candidate[UI_WRAP_MAX];
            if (out_len > 0) {
                snprintf(candidate, sizeof(candidate), "%.*s %.*s",
                         out_len, out, wlen, word);
            } else {
                snprintf(candidate, sizeof(candidate), "%.*s", wlen, word);
            }

            /* Overflowing the column ends the line here and hands the word back
               to start the next one -- so the caller resumes at `word`, not past
               it. A word that overflows an empty line has nowhere better to be
               and is allowed to run over. */
            if (out_len > 0 && measure(candidate, design_size, ctx) > design_w) {
                return word;
            }

            snprintf(out, (size_t)out_n, "%s", candidate);
            out_len = (int)strlen(out);
        }

        /* An explicit newline ends the line whether or not anything reached it,
           which is how a blank line between paragraphs survives. */
        if (*end == '\n') return end + 1;

        /* End of the text: one last line if anything is holding, otherwise the
           signal that there is nothing further to draw. */
        if (!*end) return out_len > 0 ? end : NULL;

        word = end + 1;
    }
}
