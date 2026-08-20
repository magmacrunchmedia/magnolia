#ifndef MENU_H
#define MENU_H

/* Cursor navigation over a grid of items, with a scrolling window.
 *
 * Lifted from Moonlight Drift's character selector, where the behaviour was
 * settled by exhaustive simulation: every item reachable, no out-of-range move,
 * and the window follows the cursor rather than the cursor being trapped inside
 * the window. A second game wanting a mode selector and several vertical option
 * lists is what moved it into the engine.
 *
 * A one-column grid is a vertical list, so this covers both without a separate
 * list widget.
 *
 * Drawing stays with the game: where the cells go, how a selection reads and
 * whether there are page indicators are presentation, and every game answers
 * them differently. What is shared is which item is selected and which rows are
 * on screen -- the part that is fiddly to get right and painful to get wrong.
 */

typedef struct {
    int count;         /* total items */
    int cols;
    int rows_visible;
    int cursor;        /* 0..count-1 */
    int top_row;       /* first visible row */
} MenuGrid;

/* cols and rows_visible below 1 are clamped to 1, so a malformed grid still
   navigates instead of dividing by zero. */
void menu_grid_init(MenuGrid *g, int count, int cols, int rows_visible);

/* Moves by whole columns/rows, wrapping on both axes, and scrolls the window the
   minimum needed to keep the cursor visible. Returns 1 when the cursor moved.
   menu_grid_move(g, 0, 0) just re-scrolls the window onto the current cursor,
   which is what you want when opening a menu on a remembered selection. */
int menu_grid_move(MenuGrid *g, int dcol, int drow);

/* Selects an item directly, scrolling the window onto it. Out-of-range is
   ignored. */
void menu_grid_set_cursor(MenuGrid *g, int index);

int menu_grid_total_rows(const MenuGrid *g);
int menu_grid_page_count(const MenuGrid *g);
int menu_grid_page_index(const MenuGrid *g);

/* Item index at a visible slot (0..cols*rows_visible-1), or -1 when that slot
   falls past the end of the list. Lets a draw loop walk slots without repeating
   the row/column arithmetic. */
int menu_grid_item_at_slot(const MenuGrid *g, int slot);

#endif
