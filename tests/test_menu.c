/* The grid cursor, checked the way its header says it was designed.
 *
 * menu.h claims the wrap-and-scroll "was settled by exhaustive simulation:
 * every item reachable, no out-of-range move, and the window follows the cursor
 * rather than the cursor being trapped inside the window." That simulation was
 * run once and thrown away, which left the next edit to menu_grid_move() with
 * nothing standing behind it. This is the same sweep, kept.
 *
 * menu.c includes nothing but its own header, so it runs here in a second:
 *
 *   make test
 */
#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "menu.h"

#define MAX_COUNT 24

static const int dcol[4] = { -1, +1,  0,  0 };
static const int drow[4] = {  0,  0, -1, +1 };

static int expected_total_rows(int count, int cols) {
    return (count + cols - 1) / cols;
}

/* Every invariant menu.h promises, over every shape a game might ask for and
 * every move it might make. Violations are accumulated rather than checked one
 * at a time: this is tens of thousands of moves, and a check() per move would
 * bury the count without telling anyone more than the first offender does.
 */
static void test_invariants(void) {
    printf("menu: invariants over every shape and move\n");

    int out_of_range = 0, cursor_offscreen = 0, bad_top_row = 0;
    char first_oor[160] = "", first_off[160] = "", first_top[160] = "";
    int shapes = 0, moves_made = 0;

    for (int count = 1; count <= MAX_COUNT; count++) {
        for (int cols = 1; cols <= 6; cols++) {
            for (int rows_visible = 1; rows_visible <= 4; rows_visible++) {
                shapes++;
                int total_rows = expected_total_rows(count, cols);
                int max_top = total_rows - rows_visible;
                if (max_top < 0) max_top = 0;

                for (int start = 0; start < count; start++) {
                    for (int m = 0; m < 4; m++) {
                        MenuGrid g;
                        menu_grid_init(&g, count, cols, rows_visible);
                        menu_grid_set_cursor(&g, start);
                        menu_grid_move(&g, dcol[m], drow[m]);
                        moves_made++;

                        if (g.cursor < 0 || g.cursor >= count) {
                            if (!out_of_range++) {
                                snprintf(first_oor, sizeof(first_oor),
                                         "count=%d cols=%d rows=%d start=%d "
                                         "move=(%d,%d) -> cursor=%d",
                                         count, cols, rows_visible, start,
                                         dcol[m], drow[m], g.cursor);
                            }
                            continue;   /* the rest would be noise */
                        }

                        int row = g.cursor / cols;
                        if (row < g.top_row || row >= g.top_row + rows_visible) {
                            if (!cursor_offscreen++) {
                                snprintf(first_off, sizeof(first_off),
                                         "count=%d cols=%d rows=%d start=%d "
                                         "move=(%d,%d) -> cursor row %d outside "
                                         "window [%d,%d)",
                                         count, cols, rows_visible, start,
                                         dcol[m], drow[m], row, g.top_row,
                                         g.top_row + rows_visible);
                            }
                        }

                        if (g.top_row < 0 || g.top_row > max_top) {
                            if (!bad_top_row++) {
                                snprintf(first_top, sizeof(first_top),
                                         "count=%d cols=%d rows=%d start=%d "
                                         "move=(%d,%d) -> top_row=%d, max=%d",
                                         count, cols, rows_visible, start,
                                         dcol[m], drow[m], g.top_row, max_top);
                            }
                        }
                    }
                }
            }
        }
    }

    printf("  swept %d shapes, %d moves\n", shapes, moves_made);

    if (out_of_range)     printf("  first out-of-range: %s\n", first_oor);
    if (cursor_offscreen) printf("  first off-screen:   %s\n", first_off);
    if (bad_top_row)      printf("  first bad top_row:  %s\n", first_top);

    check_int(out_of_range, 0, "no move leaves the cursor outside 0..count-1");
    check_int(cursor_offscreen, 0, "the window always contains the cursor");
    check_int(bad_top_row, 0, "top_row stays within 0..total_rows-rows_visible");
}

/* "Every item reachable" -- by any sequence of moves, not merely by the one
 * that happens to be a plain increment. A short final row is where this goes
 * wrong: from a column the last row does not extend to, a down-move has nowhere
 * obvious to land, and the classic bug is for it to land past the end.
 */
static void test_reachability(void) {
    printf("menu: every item reachable from item 0\n");

    int unreachable = 0;
    char first[160] = "";

    for (int count = 1; count <= MAX_COUNT; count++) {
        for (int cols = 1; cols <= 6; cols++) {
            for (int rows_visible = 1; rows_visible <= 4; rows_visible++) {
                int seen[MAX_COUNT];
                int queue[MAX_COUNT];
                int head = 0, tail = 0;

                memset(seen, 0, sizeof(seen));
                seen[0] = 1;
                queue[tail++] = 0;

                while (head < tail) {
                    int from = queue[head++];
                    for (int m = 0; m < 4; m++) {
                        MenuGrid g;
                        menu_grid_init(&g, count, cols, rows_visible);
                        menu_grid_set_cursor(&g, from);
                        menu_grid_move(&g, dcol[m], drow[m]);
                        if (g.cursor >= 0 && g.cursor < count && !seen[g.cursor]) {
                            seen[g.cursor] = 1;
                            queue[tail++] = g.cursor;
                        }
                    }
                }

                for (int i = 0; i < count; i++) {
                    if (!seen[i] && !unreachable++) {
                        snprintf(first, sizeof(first),
                                 "count=%d cols=%d rows=%d: item %d cannot be "
                                 "reached from item 0",
                                 count, cols, rows_visible, i);
                    }
                }
            }
        }
    }

    if (unreachable) printf("  first unreachable: %s\n", first);
    check_int(unreachable, 0, "every item is reachable from item 0");
}

static void test_documented_behaviour(void) {
    printf("menu: the behaviour the header promises by name\n");

    MenuGrid g;

    /* A malformed grid should still navigate rather than divide by zero. */
    menu_grid_init(&g, 5, 0, 0);
    check_int(g.cols, 1, "cols below 1 clamps to 1");
    check_int(g.rows_visible, 1, "rows_visible below 1 clamps to 1");
    menu_grid_move(&g, 0, 1);
    check(g.cursor >= 0 && g.cursor < 5, "a clamped grid still moves in range");

    menu_grid_init(&g, -3, 2, 2);
    check_int(g.count, 0, "a negative count clamps to 0");
    check_int(menu_grid_move(&g, 1, 0), 0, "an empty grid reports no movement");

    /* Opening a menu on a remembered selection: the cursor is already where the
       player left it, and only the window needs to catch up. */
    menu_grid_init(&g, 20, 2, 2);
    g.cursor = 18;
    g.top_row = 0;
    check_int(menu_grid_move(&g, 0, 0), 0, "a (0,0) move reports no movement");
    check_int(g.cursor, 18, "a (0,0) move leaves the cursor alone");
    check_int(g.top_row, 8, "a (0,0) move scrolls the window onto the cursor");

    menu_grid_init(&g, 10, 3, 2);
    menu_grid_set_cursor(&g, 99);
    check_int(g.cursor, 0, "an out-of-range set_cursor is ignored");
    menu_grid_set_cursor(&g, -1);
    check_int(g.cursor, 0, "a negative set_cursor is ignored");
    menu_grid_set_cursor(&g, 7);
    check_int(g.cursor, 7, "an in-range set_cursor selects");

    /* Ten items over three columns is four rows, the last holding one item. */
    menu_grid_init(&g, 10, 3, 2);
    check_int(menu_grid_total_rows(&g), 4, "total rows counts a short final row");
    check_int(menu_grid_page_count(&g), 2, "pages cover every row");
    check_int(menu_grid_page_index(&g), 0, "a fresh grid is on the first page");
    menu_grid_set_cursor(&g, 9);
    check_int(menu_grid_page_index(&g), 1, "the last item is on the last page");

    /* A draw loop walks slots; the ones past the end must say so. */
    menu_grid_init(&g, 5, 3, 2);
    check_int(menu_grid_item_at_slot(&g, 0), 0, "slot 0 is the first item");
    check_int(menu_grid_item_at_slot(&g, 4), 4, "slot 4 is the last item");
    check_int(menu_grid_item_at_slot(&g, 5), -1, "a slot past the end is -1");
    check_int(menu_grid_item_at_slot(&g, 6), -1, "a slot past the window is -1");
    check_int(menu_grid_item_at_slot(&g, -1), -1, "a negative slot is -1");

    /* The grids the two games actually use, so a regression shows up against a
       real layout and not only against a swept one. */
    menu_grid_init(&g, 24, 6, 4);          /* Moonlight Drift's roster */
    check_int(menu_grid_total_rows(&g), 4, "the 24-character roster is four rows");
    check_int(menu_grid_page_count(&g), 1, "the whole roster fits one page");

    menu_grid_init(&g, 8, 4, 2);           /* George Boole's mode grid */
    check_int(menu_grid_page_count(&g), 1, "all eight modes fit one page");
    menu_grid_move(&g, 0, 1);
    check_int(g.cursor, 4, "down from mode 0 reaches the second row");
    menu_grid_move(&g, 0, 1);
    check_int(g.cursor, 0, "down from the last row wraps to the first");
    menu_grid_move(&g, -1, 0);
    check_int(g.cursor, 7, "left from the first item wraps to the last");

    /* A one-column grid is a vertical list -- the case George Boole's title
       menu relies on, and the reason there is no separate list widget. */
    menu_grid_init(&g, 4, 1, 4);
    menu_grid_move(&g, 0, 1);
    check_int(g.cursor, 1, "a one-column grid steps down by one");
    menu_grid_move(&g, 0, -1);
    check_int(g.cursor, 0, "and back up by one");
    menu_grid_move(&g, 0, -1);
    check_int(g.cursor, 3, "up from the top of a list wraps to the bottom");
}

int main(void) {
    test_invariants();
    test_reachability();
    test_documented_behaviour();
    return report();
}
