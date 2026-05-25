#pragma once
#include <stdint.h>

#define TG_EMPTY 0

typedef struct
{
    uint32_t ch;
    uint32_t bg;
    uint32_t fg;
} tg_cell;

// ─│┌┐└┘
#define TG_BORDER_CELLS_SINGLE(bg, fg) (tg_border_cells){ \
    (tg_cell){0x2500, bg, fg}, \
    (tg_cell){0x2502, bg, fg}, \
    (tg_cell){0x250C, bg, fg}, \
    (tg_cell){0x2510, bg, fg}, \
    (tg_cell){0x2514, bg, fg}, \
    (tg_cell){0x2518, bg, fg}  \
}
// ━┃┏┓┗┛
#define TG_BORDER_CELLS_BOLD(bg, fg) (tg_border_cells){ \
    (tg_cell){0x2501, bg, fg}, \
    (tg_cell){0x2503, bg, fg}, \
    (tg_cell){0x250F, bg, fg}, \
    (tg_cell){0x2513, bg, fg}, \
    (tg_cell){0x2517, bg, fg}, \
    (tg_cell){0x251B, bg, fg}  \
}
// _"|/\  .
#define TG_LINE_CELLS_ASCII(bg, fg) (tg_line_cells){ \
    (tg_cell){0x005F, bg, fg}, \
    (tg_cell){0x0022 bg, fg}, \
    (tg_cell){0x007C bg, fg}, \
    (tg_cell){0x002F bg, fg}, \
    (tg_cell){0x005C bg, fg} \
}
// _‾|/\   .
#define TG_LINE_CELLS_DEFAULT(bg, fg) (tg_line_cells){ \
    (tg_cell){0x005F, bg, fg}, \
    (tg_cell){0x203E, bg, fg}, \
    (tg_cell){0x007C, bg, fg}, \
    (tg_cell){0x002F, bg, fg}, \
    (tg_cell){0x005C, bg, fg} \
}
// _‾│╱╲
#define TG_LINE_CELLS_SMOOTH(bg, fg) (tg_line_cells){ \
    (tg_cell){0x005F, bg, fg}, \
    (tg_cell){0x203E, bg, fg}, \
    (tg_cell){0x2502, bg, fg}, \
    (tg_cell){0x2571, bg, fg}, \
    (tg_cell){0x2572, bg, fg}  \
}
// ──│╱╲
#define TG_LINE_CELLS_SMOOTH_BORDER(bg, fg) (tg_line_cells){ \
    (tg_cell){0x2500, bg, fg}, \
    (tg_cell){0x2500, bg, fg}, \
    (tg_cell){0x2502, bg, fg}, \
    (tg_cell){0x2571, bg, fg}, \
    (tg_cell){0x2572, bg, fg}  \
}

typedef struct {
    tg_cell horizontal;
    tg_cell vertical;
    tg_cell topleft;
    tg_cell topright;
    tg_cell bottomleft;
    tg_cell bottomright;
} tg_border_cells;
typedef struct {
    tg_cell horizontal_bottom;
    tg_cell horizontal_top;
    tg_cell vertical;
    tg_cell uptilt;
    tg_cell downtilt;
} tg_line_cells;

typedef struct
{
    int x, y;
} tg_point;
typedef struct
{
    int left, top, right, bottom;
} tg_rect;

typedef struct
{
    int width, height;
    void (*set)(void *ctx, int x, int y, tg_cell ch);
    void *ctx;
} tg_canvas;

typedef enum {
    TG_MODE_CELLS,
    TG_MODE_BRAILLE
} tg_line_mode;

typedef struct
{
    int draw;
    tg_line_mode mode;

    union {
        tg_line_cells cells;

        struct {
            float line_density;
            uint32_t bg;
            uint32_t fg;
        } braille;
    };
} tg_line_opts;

typedef struct {
    int draw;
    tg_cell cell;
} tg_node_opts;

typedef struct {
    int draw;
    struct
    {
        int count;
        tg_cell cell;
    } horizontal;
    struct
    {
        int count;
        tg_cell cell;
    } vertical;
} tg_grid_opts;

typedef struct {
    int draw;
    struct {
        int count;
        uint32_t bg;
        uint32_t fg;
    } x;
    struct {
        int count;
        uint32_t bg;
        uint32_t fg;
    } y;
} tg_indicator_opts;

typedef struct
{
    int width, height;
    tg_rect padding;

    tg_grid_opts grid;
    tg_line_opts line;
    tg_node_opts node;
    tg_indicator_opts indicator;

} tg_render_opts;

static inline tg_render_opts tg_default_render_opts() {
    return (tg_render_opts) {
        .width = 100, .height = 50,
        .padding = {
            .left = 6,
            .top = 0,
            .right = 5,
            .bottom = 1
        },

        .line = {
            .mode = TG_MODE_CELLS,
            .draw = 1,
            .cells = TG_LINE_CELLS_DEFAULT(0, 0x00FF00)
        },
        .node = {0},
        .grid = {
            .draw = 1,
            .horizontal = {
                .count = 5,
                .cell = {.ch=0x2500, .bg=0, .fg=0x555555}
            },
            .vertical = {
                .count = 10,
                .cell = {.ch=0x2502, .bg=0, .fg=0x555555}
            } 
        },
        .indicator = {
            .draw = 1,
            .x = {
                .count = 10,
                .bg = 0,
                .fg = 0xFF0000
            },
            .y = {
                .count = 5,
                .bg = 0,
                .fg = 0xFF0000
            }
        }
    };
}

static inline tg_render_opts tg_default_render_opts_braille() {
    return (tg_render_opts) {
        .width = 141, .height = 51,
        .padding = {
            .left = 6,
            .top = 0,
            .right = 5,
            .bottom = 1
        },

        .line = {
            .draw = 1,
            .mode = TG_MODE_BRAILLE,
            .braille = {
                .line_density = 5,
                .bg = 0,
                .fg = 0x00FF00
            },
        },
        .node = {0},
        .grid = {
            .draw = 1,
            .horizontal = {
                .count = 5,
                .cell = {.ch=0x2500, .bg=0, .fg=0x555555}
            },
            .vertical = {
                .count = 10,
                .cell = {.ch=0x2502, .bg=0, .fg=0x555555}
            } 
        },
        .indicator = {
            .draw = 1,
            .x = {
                .count = 10,
                .bg = 0,
                .fg = 0xFF0000
            },
            .y = {
                .count = 5,
                .bg = 0,
                .fg = 0xFF0000
            }
        }
    };
}
