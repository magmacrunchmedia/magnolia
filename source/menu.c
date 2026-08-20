#include "menu.h"

static void scroll_to_cursor(MenuGrid *g) {
    int row = g->cursor / g->cols;

    if (row < g->top_row) g->top_row = row;
    if (row >= g->top_row + g->rows_visible) g->top_row = row - g->rows_visible + 1;

    int max_top = menu_grid_total_rows(g) - g->rows_visible;
    if (max_top < 0) max_top = 0;
    if (g->top_row > max_top) g->top_row = max_top;
    if (g->top_row < 0) g->top_row = 0;
}

void menu_grid_init(MenuGrid *g, int count, int cols, int rows_visible) {
    if (!g) return;
    g->count        = count < 0 ? 0 : count;
    g->cols         = cols < 1 ? 1 : cols;
    g->rows_visible = rows_visible < 1 ? 1 : rows_visible;
    g->cursor       = 0;
    g->top_row      = 0;
}

int menu_grid_move(MenuGrid *g, int dcol, int drow) {
    if (!g || g->count <= 0) return 0;

    int before = g->cursor;

    if (dcol) {
        g->cursor += dcol;
        if (g->cursor < 0) g->cursor = g->count - 1;
        if (g->cursor >= g->count) g->cursor = 0;
    }

    if (drow) {
        int next = g->cursor + drow * g->cols;
        if (next < 0) {
            /* Wrap to the same column on the last populated row, then step back
               up until the slot actually exists -- the final row is usually
               short, and landing past the end there is the classic grid bug. */
            int col = g->cursor % g->cols;
            next = (menu_grid_total_rows(g) - 1) * g->cols + col;
            while (next >= g->count) next -= g->cols;
        } else if (next >= g->count) {
            next = g->cursor % g->cols;
        }
        g->cursor = next;
    }

    scroll_to_cursor(g);
    return g->cursor != before;
}

void menu_grid_set_cursor(MenuGrid *g, int index) {
    if (!g || index < 0 || index >= g->count) return;
    g->cursor = index;
    scroll_to_cursor(g);
}

int menu_grid_total_rows(const MenuGrid *g) {
    if (!g || g->count <= 0) return 0;
    return (g->count + g->cols - 1) / g->cols;
}

int menu_grid_page_count(const MenuGrid *g) {
    if (!g) return 0;
    int rows = menu_grid_total_rows(g);
    if (rows <= 0) return 0;
    return (rows + g->rows_visible - 1) / g->rows_visible;
}

int menu_grid_page_index(const MenuGrid *g) {
    if (!g || g->rows_visible <= 0) return 0;
    return g->top_row / g->rows_visible;
}

int menu_grid_item_at_slot(const MenuGrid *g, int slot) {
    if (!g || slot < 0 || slot >= g->cols * g->rows_visible) return -1;
    int col = slot % g->cols;
    int row = g->top_row + slot / g->cols;
    int index = row * g->cols + col;
    return (index >= 0 && index < g->count) ? index : -1;
}
