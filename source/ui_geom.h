#ifndef UI_GEOM_H
#define UI_GEOM_H

/* The arithmetic behind ui_utils: safe-area geometry, the design-space
 * projection, and the word wrap.
 *
 * Split out of ui_utils.c for the same reason input_state.c was split out of
 * input.c and timestep.c out of clock.c -- everything here is arithmetic, and
 * everything left behind is a GRRLIB call. Kept in one file none of it could be
 * asserted, and it decides where every glyph in every game lands, on a video
 * mode most of them will never be run against. A layout that is right on NTSC
 * and wrong on PAL is a bug you find by owning two televisions.
 *
 * Like input.h, this header deliberately uses no libogc types, so the host
 * tests compile it unchanged. Colours stay in ui_utils.h, where u32 lives.
 *
 * Nothing outside the engine and its tests should call these. Games use
 * ui_utils.h, which includes this one for the design-space constants below.
 */

/* Screens are authored against this fixed space and projected onto whatever
   video mode is actually running (NTSC 640x480, PAL 640x528). Declared here
   rather than in ui_utils.h because the projection is here, and a constant
   belongs beside the code that divides by it. */
#define UI_DESIGN_WIDTH   640
#define UI_DESIGN_HEIGHT  480

/* Longest line the wrapper will assemble, terminator included. */
#define UI_WRAP_MAX 128

/* The running video mode, and how much of it the TV is assumed to eat.
   Passed by value rather than read out of a global so that a test can ask what
   a layout does on PAL at 10% overscan without a console in the room -- which
   is the question this split exists to make askable. */
typedef struct {
    int screen_w;
    int screen_h;
    int overscan_pct;
} UiGeom;

/* overscan_pct is clamped to 0..20. A negative one describes no television, and
   an enormous one silently eats the screen. */
UiGeom ui_geom_make(int screen_w, int screen_h, int overscan_pct);

int ui_geom_safe_x(const UiGeom *g);
int ui_geom_safe_y(const UiGeom *g);
int ui_geom_safe_w(const UiGeom *g);
int ui_geom_safe_h(const UiGeom *g);

int ui_geom_map_x(const UiGeom *g, int design_x);
int ui_geom_map_y(const UiGeom *g, int design_y);
int ui_geom_map_w(const UiGeom *g, int design_w);
int ui_geom_map_h(const UiGeom *g, int design_h);

/* Glyphs scale by the vertical factor so text keeps its proportions, with a
   floor of 6 -- below that Press Start 2P is a smudge rather than small text. */
unsigned int ui_geom_map_size(const UiGeom *g, unsigned int design_size);

/* Screen pixels back into design units. Layout arithmetic belongs in the design
   space; measuring in screen pixels and comparing the answer against design
   coordinates is a mistake that only shows up on the video mode you did not
   test on. 0 when the safe area has no width. */
int ui_geom_to_design_w(const UiGeom *g, unsigned int screen_w);

/* How wide `text` is in design units at `design_size`.
   The wrapper cannot measure text itself -- that needs the font, which needs
   GRRLIB -- so it is handed a measurer, exactly as input_state is handed the
   buttons rather than reading the hardware. ui_utils supplies one backed by
   GRRLIB_WidthTTF; the tests supply a monospace one, which is not a
   simplification: Press Start 2P is a monospaced pixel font. */
typedef int (*UiMeasureFn)(const char *text, unsigned int design_size, void *ctx);

/* Fills `out` with the next wrapped line of `text` and returns where to resume,
 * or NULL once the text is spent and no further line was produced. So a caller
 * draws one line per non-NULL return:
 *
 *     const char *p = text;
 *     while ((p = ui_geom_wrap_next(p, line, sizeof line, w, size, m, 0))) {
 *         draw(line, y);
 *         y += spacing;
 *     }
 *
 * Breaks on spaces, honours an explicit newline (which ends the line so far,
 * empty or not), and never splits a word.
 *
 * Two behaviours worth knowing rather than discovering: a word too long for the
 * column overflows it rather than being broken, and a word too long for `out`
 * is dropped entirely. Both are what ui_utils has always done. They are pinned
 * by the tests rather than corrected here -- three shipped games lay their text
 * out through this function, and changing where it breaks is a change to their
 * screens, not a refactor of them.
 *
 * `out` should be UI_WRAP_MAX bytes. A shorter buffer only lowers the length at
 * which a word is dropped. */
const char *ui_geom_wrap_next(const char *text, char *out, int out_n,
                              int design_w, unsigned int design_size,
                              UiMeasureFn measure, void *ctx);

#endif
