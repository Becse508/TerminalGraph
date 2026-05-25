#pragma once
#include "../types.h"
#include "../canvas/canvas.h"
#include "../buffer/buffer.h"


int border_is_empty(const border_cells *b);




void tg_draw_borders(tg_cell *buffer, tg_point bufsize, border_cells chars);

/// @brief Draw a line from the characters provided.
/// @param start exclusive
/// @param end exclusive
/// @param characters use `LINE_CHARS_DEFAULT` for a normal line
/// @return 0 on success, 1 if any index was out of bounds
int tg_draw_line(tg_cell *buffer,
               tg_point bufsize,
               tg_point start,
               tg_point end,
               line_cells chars);

/// @brief (ChatGPT used) Draws a line between `start` and `end` using braille characters for precision
void tg_draw_line_braille(tg_cell *buffer, tg_point bufsize, tg_point start, tg_point end, uint32_t bg, uint32_t fg, float density);
