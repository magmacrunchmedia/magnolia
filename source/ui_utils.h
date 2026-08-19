#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <grrlib.h>

void ui_draw_text_shadow(int x, int y, const char *text, unsigned int size, u32 color);
void ui_draw_centered_text(int y, const char *text, unsigned int size, u32 color);
void ui_draw_border(void);

#endif
