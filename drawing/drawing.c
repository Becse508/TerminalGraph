#include "drawing.h"
#include <math.h>
#include <stdlib.h>

void tg_draw_borders(tg_cell *buffer, tg_point bufsize, border_cells chars) {
    if (bufsize.x * bufsize.y == 0) return;
    int bufcount = bufsize.x * bufsize.y;

    if (bufsize.x * bufsize.y == 1) {
        buffer[0] = (tg_cell){'O', chars.horizontal.bg, chars.horizontal.fg};
    }
    if (bufsize.x == 1) {
        tg_buffer_fill(buffer, bufcount, chars.horizontal);
    }
    else if (bufsize.y == 1) {
        tg_buffer_fill(buffer, bufcount, chars.vertical);
    }
    else {
        // CORNERS
        buffer[0] = chars.topleft;
        buffer[bufsize.x-1] = chars.topright;
        buffer[(bufsize.y-1) * bufsize.x], chars.bottomleft;
        buffer[bufcount-1] = chars.bottomright;

        // SIDES
        for (size_t i = 1; i < bufsize.x-1; i++)
        {
            buffer[i] = chars.horizontal;
            buffer[bufcount-1-i] = chars.horizontal;
        }
        for (size_t i = 1; i < bufsize.y-1; i++)
        {
            buffer[i * bufsize.x] = chars.vertical;
            buffer[i * bufsize.x + bufsize.x-1] = chars.vertical;
        }
    }
}

int tg_draw_line(tg_cell *buffer,
               tg_point bufsize,
               tg_point start,
               tg_point end,
               line_cells chars)
{
    tg_cell horizontal = chars.horizontal_bottom;

    int error = 0;

    int x0, x1, y0, y1;
    if (start.x < end.x) {
        x0 = start.x;
        y0 = start.y;
        x1 = end.x;
        y1 = end.y;
    }
    else {
        x0 = end.x;
        y0 = end.y;
        x1 = start.x;
        y1 = start.y;
    }

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    // line at top if going downwards
    if (sy == -1) {
        horizontal = chars.horizontal_top;
    }

    int err = dx - dy;

    while (abs(x1 - x0) >= 1 || abs(y1 - y0) >= 1)
    {
        int old_x = x0;
        int old_y = y0;

        int e2 = err << 1;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }

        /* Determine movement type */
        int moved_x = (x0 != old_x);
        int moved_y = (y0 != old_y);

        tg_cell pixel;

        if (moved_x && moved_y) {
            pixel = (sx == sy) ? chars.downtilt : chars.uptilt;
        }
        else if (moved_x) {
            pixel = horizontal;
        }
        else {
            pixel = chars.vertical;
        }
        
        error |= tg_buffer_set_safe(buffer, x0, y0, bufsize.x, bufsize.y, pixel);
    }

    return error;
}

void tg_draw_line_braille(tg_cell *buffer, tg_point bufsize, tg_point start, tg_point end, uint32_t bg, uint32_t fg, float density)
{
    // full braille pixel grid
    int pixel_width  = bufsize.x * 2;
    int pixel_height = bufsize.y * 4;

    // start/end in pixel coordinates
    float x0 = start.x * 2.0f;
    float y0 = start.y * 4.0f;
    float x1 = end.x   * 2.0f;
    float y1 = end.y   * 4.0f;

    // vector from start -> end
    float dx = x1 - x0;
    float dy = y1 - y0;

    // number of steps based on density and longest axis
    float length = fmaxf(fabsf(dx), fabsf(dy));
    if (length == 0.0f) length = 1.0f; // avoid division by zero

    int steps = (int)(length * density);

    for (int i = 0; i <= steps; i++)
    {
        float t = (float)i / steps;
        int px = (int)(x0 + dx * t + 0.5f);
        int py = (int)(y0 + dy * t + 0.5f);

        // clip to surface bounds
        if (px < 0 || px >= pixel_width || py < 0 || py >= pixel_height)
            continue;

        int cell_x = px / 2;
        int cell_y = py / 4;
        int sub_x  = px % 2;
        int sub_y  = py % 4;

        int bit;
        switch(sub_y) {
            case 0: bit = (sub_x == 0) ? 0 : 3; break;
            case 1: bit = (sub_x == 0) ? 1 : 4; break;
            case 2: bit = (sub_x == 0) ? 2 : 5; break;
            case 3: bit = (sub_x == 0) ? 6 : 7; break;
        }

        size_t idx = bufsize.x * cell_y + cell_x;
        tg_cell cell = buffer[idx];
        if (cell.ch < 0x2800 || cell.ch > 0x28FF)
            cell.ch = 0x2800;
        cell.ch |= 1 << bit;
        cell.bg = bg;
        cell.fg = fg;
        buffer[idx] = cell;
    }
}
