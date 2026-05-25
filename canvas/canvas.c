#include "canvas.h"


#define check_out_of_bounds(canvas, x, y) \
    ((x) < 0 || (y) < 0 ||              \
     (size_t)(x) >= (canvas)->width ||   \
     (size_t)(y) >= (canvas)->height)


void tg_canvas_fill(tg_canvas *canvas, tg_cell cell) {
    if (!canvas) return;
    for (size_t x = 0; x < canvas->width; x++)
    {
        for (size_t y = 0; y < canvas->height; y++) {
            canvas->set(canvas->ctx, x, y, cell);
        }
    }
}

int tg_canvas_set_safe(tg_canvas *canvas, int x, int y, tg_cell cell) {
    if (check_out_of_bounds(canvas, x, y)) {
        return 1;
    }
    canvas->set(canvas->ctx, x, y, cell);
    return 0;
}
