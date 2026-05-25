#include "buffer.h"

#define check_out_of_bounds(buf, x, y, width, height) \
    ((x) < 0 || (y) < 0 || (x) >= (width) || (y) >= (height))


void tg_buffer_fill(tg_cell *buf, size_t count, tg_cell cell) {
    if (!buf) return;
    for (size_t i = 0; i < count; i++)
    {
        buf[i] = cell;
    }
}

int tg_buffer_set_safe(tg_cell *buf, size_t x, size_t y, size_t width, size_t height, tg_cell cell) {
    if (check_out_of_bounds(buf, x, y, width, height))
        return 1;
    
    buf[width * y + x] = cell;
    return 0;
}
