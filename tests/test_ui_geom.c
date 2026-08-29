/* Host-side tests for the UI arithmetic: safe-area geometry, the design-space
 * projection, and the word wrap.
 *
 * All of this used to sit in ui_utils.c behind a GRRLIB include, which meant the
 * only way to find out where a layout landed was to cross-compile it and look at
 * a television. That is a poor way to learn that PAL is 528 lines tall.
 *
 *   make test-ui-geom
 */
#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "ui_geom.h"

/* The two video modes magnolia actually runs on. */
static UiGeom ntsc(int pct) { return ui_geom_make(640, 480, pct); }
static UiGeom pal(int pct)  { return ui_geom_make(640, 528, pct); }

/* Press Start 2P is a monospaced pixel font whose advance is its point size, so
   this is what the real measurer approximates rather than a stand-in for it. */
static int mono(const char *text, unsigned int design_size, void *ctx) {
    (void)ctx;
    return (int)strlen(text) * (int)design_size;
}

static void test_overscan_clamp(void) {
    printf("ui_geom: overscan is clamped to something a television could do\n");

    check_int(ui_geom_make(640, 480, -1).overscan_pct, 0, "negative clamps to 0");
    check_int(ui_geom_make(640, 480, -999).overscan_pct, 0, "very negative clamps to 0");
    check_int(ui_geom_make(640, 480, 0).overscan_pct, 0, "0 is allowed");
    check_int(ui_geom_make(640, 480, 6).overscan_pct, 6, "the default passes through");
    check_int(ui_geom_make(640, 480, 20).overscan_pct, 20, "20 is the ceiling");
    check_int(ui_geom_make(640, 480, 21).overscan_pct, 20, "21 clamps to 20");
    check_int(ui_geom_make(640, 480, 100).overscan_pct, 20,
              "a whole-screen overscan clamps rather than leaving nothing to draw on");
}

static void test_safe_area(void) {
    printf("ui_geom: the safe area on both video modes\n");

    /* No overscan is the whole screen -- the identity case, and the one a
       developer looking at an emulator window is actually seeing. */
    UiGeom g = ntsc(0);
    check_int(ui_geom_safe_x(&g), 0, "no overscan starts at the left edge");
    check_int(ui_geom_safe_y(&g), 0, "no overscan starts at the top edge");
    check_int(ui_geom_safe_w(&g), 640, "no overscan is the full width");
    check_int(ui_geom_safe_h(&g), 480, "no overscan is the full height");

    g = ntsc(6);
    check_int(ui_geom_safe_x(&g), 38, "NTSC at 6%: 640*6/100");
    check_int(ui_geom_safe_y(&g), 28, "NTSC at 6%: 480*6/100");
    check_int(ui_geom_safe_w(&g), 640 - 76, "width loses both margins");
    check_int(ui_geom_safe_h(&g), 480 - 56, "height loses both margins");

    /* PAL is 528 lines, not 480. Every vertical number changes and no horizontal
       one does, which is exactly the class of bug this module exists to catch. */
    g = pal(6);
    check_int(ui_geom_safe_x(&g), 38, "PAL is the same width as NTSC");
    check_int(ui_geom_safe_y(&g), 31, "PAL at 6%: 528*6/100");
    check_int(ui_geom_safe_h(&g), 528 - 62, "PAL height loses both margins");

    /* The safe area stays inside the screen at the extreme. */
    g = ntsc(20);
    check(ui_geom_safe_w(&g) > 0, "the widest overscan still leaves something");
    check(ui_geom_safe_x(&g) + ui_geom_safe_w(&g) <= 640, "and stays on the screen");
    check(ui_geom_safe_y(&g) + ui_geom_safe_h(&g) <= 480, "vertically too");
}

static void test_mapping(void) {
    printf("ui_geom: design space onto the safe area\n");

    UiGeom g = ntsc(6);

    /* The corners of the design space land on the corners of the safe area.
       Everything else in the UI is somewhere between these two claims. */
    check_int(ui_geom_map_x(&g, 0), ui_geom_safe_x(&g), "design 0 maps to the safe left edge");
    check_int(ui_geom_map_y(&g, 0), ui_geom_safe_y(&g), "design 0 maps to the safe top edge");
    check_int(ui_geom_map_x(&g, UI_DESIGN_WIDTH),
              ui_geom_safe_x(&g) + ui_geom_safe_w(&g), "design 640 maps to the safe right edge");
    check_int(ui_geom_map_y(&g, UI_DESIGN_HEIGHT),
              ui_geom_safe_y(&g) + ui_geom_safe_h(&g), "design 480 maps to the safe bottom edge");

    /* A width is a distance, not a position: it carries no origin. */
    check_int(ui_geom_map_w(&g, UI_DESIGN_WIDTH), ui_geom_safe_w(&g), "a full-width span is the safe width");
    check_int(ui_geom_map_h(&g, UI_DESIGN_HEIGHT), ui_geom_safe_h(&g), "a full-height span is the safe height");
    check_int(ui_geom_map_w(&g, 0), 0, "a zero span stays zero");

    /* The centre stays the centre on both modes -- a layout centred in design
       space must not drift when the video mode changes underneath it. */
    UiGeom n = ntsc(6), p = pal(6);
    int n_mid = ui_geom_map_x(&n, UI_DESIGN_WIDTH / 2) - ui_geom_safe_x(&n);
    int p_mid = ui_geom_map_x(&p, UI_DESIGN_WIDTH / 2) - ui_geom_safe_x(&p);
    check_int(n_mid, ui_geom_safe_w(&n) / 2, "the horizontal centre is the centre on NTSC");
    check_int(p_mid, ui_geom_safe_w(&p) / 2, "and on PAL");

    /* Monotonic: further right in design space is never further left on screen.
       Integer division makes this worth asserting rather than assuming. */
    int prev = ui_geom_map_x(&g, 0);
    int monotonic = 1;
    for (int d = 1; d <= UI_DESIGN_WIDTH; d++) {
        int cur = ui_geom_map_x(&g, d);
        if (cur < prev) monotonic = 0;
        prev = cur;
    }
    check(monotonic, "mapping never runs backwards across the whole design width");
}

static void test_map_size(void) {
    printf("ui_geom: glyph sizes scale vertically and have a floor\n");

    UiGeom g = ntsc(0);
    check_int((int)ui_geom_map_size(&g, 24), 24, "no overscan leaves the size alone");

    g = ntsc(6);
    check_int((int)ui_geom_map_size(&g, 24), (24 * (480 - 56)) / 480, "scaled by the safe height");

    /* Below six pixels Press Start 2P stops being small text and starts being a
       smudge, so the floor is a legibility decision, not a rounding one. */
    check_int((int)ui_geom_map_size(&g, 1), 6, "a tiny size is floored at 6");
    check_int((int)ui_geom_map_size(&g, 0), 6, "so is zero");

    g = ntsc(20);
    check(ui_geom_map_size(&g, 8) >= 6, "the floor holds at the widest overscan");
}

static void test_to_design_w(void) {
    printf("ui_geom: screen widths back into design units\n");

    UiGeom g = ntsc(0);
    check_int(ui_geom_to_design_w(&g, 320), 320, "with no overscan the spaces coincide");

    g = ntsc(6);
    /* The safe area is narrower than the design space, so a given number of
       screen pixels is more design units than it looks. */
    check_int(ui_geom_to_design_w(&g, (unsigned)ui_geom_safe_w(&g)), UI_DESIGN_WIDTH,
              "the whole safe width is the whole design width");
    check_int(ui_geom_to_design_w(&g, 0), 0, "nothing measures as nothing");

    /* A degenerate geometry must answer 0 rather than divide by it. */
    UiGeom none = ui_geom_make(0, 0, 6);
    check_int(ui_geom_to_design_w(&none, 100), 0, "a screen with no width answers 0, not a crash");
}

/* --- Word wrap -------------------------------------------------------------
 *
 * Collects the lines the wrapper settles on, so a case can state the whole
 * paragraph it expects rather than one line at a time.
 */
#define MAX_LINES 32
static char lines[MAX_LINES][UI_WRAP_MAX];
static int  line_count;

static void wrap_all(const char *text, int design_w, unsigned int size) {
    char buf[UI_WRAP_MAX];
    const char *p = text;
    line_count = 0;
    memset(lines, 0, sizeof(lines));
    while (line_count < MAX_LINES &&
           (p = ui_geom_wrap_next(p, buf, (int)sizeof(buf), design_w, size,
                                  mono, NULL)) != NULL) {
        snprintf(lines[line_count++], UI_WRAP_MAX, "%s", buf);
    }
}

static void test_wrap_basics(void) {
    printf("ui_geom: wrapping on spaces\n");

    /* Ten design units per character at size 10, so a 100-unit column holds ten
       characters and the arithmetic in each case below is checkable by eye. */
    wrap_all("one two three", 100, 10);
    check_int(line_count, 2, "thirteen characters need two lines of ten");
    check_str(lines[0], "one two", "the first line takes what fits");
    check_str(lines[1], "three", "the rest starts the next");

    wrap_all("one two three", 1000, 10);
    check_int(line_count, 1, "a wide enough column needs only one line");
    check_str(lines[0], "one two three", "and holds the whole text");

    wrap_all("", 100, 10);
    check_int(line_count, 0, "empty text produces no lines at all");

    wrap_all("word", 100, 10);
    check_int(line_count, 1, "a single word is a single line");
    check_str(lines[0], "word", "unchanged");

    /* Runs of spaces collapse -- the wrapper joins words with exactly one. */
    wrap_all("a  b", 1000, 10);
    check_int(line_count, 1, "a double space does not start a line");
    check_str(lines[0], "a b", "words are rejoined with one space");
}

static void test_wrap_newlines(void) {
    printf("ui_geom: explicit newlines\n");

    wrap_all("first\nsecond", 1000, 10);
    check_int(line_count, 2, "a newline breaks the line regardless of width");
    check_str(lines[0], "first", "before the break");
    check_str(lines[1], "second", "after it");

    /* A blank line between paragraphs has to survive, or every wrapped screen
       in the two games that use this runs together. */
    wrap_all("para\n\nnext", 1000, 10);
    check_int(line_count, 3, "a doubled newline leaves an empty line standing");
    check_str(lines[0], "para", "first paragraph");
    check_str(lines[1], "", "the blank line between them");
    check_str(lines[2], "next", "second paragraph");

    wrap_all("trailing\n", 1000, 10);
    check_int(line_count, 1, "a trailing newline does not invent an empty line after it");
    check_str(lines[0], "trailing", "just the text");

    /* Text opening with a newline used to reach the draw call with the line
       buffer never written to -- an uninitialised read that put whatever was on
       the stack onto the screen. It emits an empty line now. */
    wrap_all("\nleading", 1000, 10);
    check_int(line_count, 2, "a leading newline emits an empty line first");
    check_str(lines[0], "", "and that line is empty, not whatever was on the stack");
    check_str(lines[1], "leading", "then the text");

    wrap_all("\n", 1000, 10);
    check_int(line_count, 1, "a lone newline is one empty line");
    check_str(lines[0], "", "which is empty");
}

/* Two behaviours that are not obviously right, pinned so that changing either is
   a decision someone makes on purpose. Three shipped games lay their text out
   through this function; where it breaks is part of their screens. */
static void test_wrap_pinned_quirks(void) {
    printf("ui_geom: the awkward cases, as they have always behaved\n");

    /* A word wider than the column runs over rather than being split. Breaking
       words mid-glyph would need hyphenation rules nobody has asked for. */
    wrap_all("supercalifragilistic", 50, 10);
    check_int(line_count, 1, "a word too wide for the column is not broken");
    check_str(lines[0], "supercalifragilistic", "it overflows instead");

    /* ...and it still starts a fresh line rather than dragging its neighbour
       over with it. */
    wrap_all("hi supercalifragilistic", 50, 10);
    check_int(line_count, 2, "an over-long word still starts its own line");
    check_str(lines[0], "hi", "the short word keeps its line");
    check_str(lines[1], "supercalifragilistic", "the long one follows");

    /* A word too long for the line buffer is dropped outright. This is the one
       genuinely questionable behaviour here: nothing is drawn and nothing says
       so. It is left alone because correcting it changes what two games render,
       which is a change to make deliberately and not inside a refactor. */
    static char huge[UI_WRAP_MAX + 40];
    memset(huge, 'x', sizeof(huge) - 1);
    huge[sizeof(huge) - 1] = '\0';

    wrap_all(huge, 100000, 1);
    check_int(line_count, 0, "a word longer than the line buffer is dropped, not truncated");

    static char around[UI_WRAP_MAX + 80];
    snprintf(around, sizeof(around), "before %s after", huge);
    wrap_all(around, 100000, 1);
    check_int(line_count, 1, "the words around it still make a line");
    check_str(lines[0], "before after", "with the over-long word simply absent");
}

static void test_wrap_arguments(void) {
    printf("ui_geom: wrapping nonsense arguments\n");

    char buf[UI_WRAP_MAX];
    check(ui_geom_wrap_next(NULL, buf, (int)sizeof(buf), 100, 10, mono, NULL) == NULL,
          "no text is no lines");
    check(ui_geom_wrap_next("hello", NULL, (int)sizeof(buf), 100, 10, mono, NULL) == NULL,
          "nowhere to put the line is no lines");
    check(ui_geom_wrap_next("hello", buf, 0, 100, 10, mono, NULL) == NULL,
          "a zero-length buffer is no lines");
    check(ui_geom_wrap_next("hello", buf, (int)sizeof(buf), 100, 10, NULL, NULL) == NULL,
          "no measurer is no lines, rather than a null call");

    /* A column of no width cannot fit anything, but must still terminate --
       an unwrappable paragraph should draw badly, not hang. */
    wrap_all("a b c", 0, 10);
    check(line_count > 0 && line_count <= MAX_LINES, "a zero-width column still terminates");
}

int main(void) {
    test_overscan_clamp();
    test_safe_area();
    test_mapping();
    test_map_size();
    test_to_design_w();
    test_wrap_basics();
    test_wrap_newlines();
    test_wrap_pinned_quirks();
    test_wrap_arguments();

    return report();
}
