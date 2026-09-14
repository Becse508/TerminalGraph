#pragma once
#include <stdint.h>
#include <stddef.h>

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
#define TG_BORDER_CELLS_ROUNDED(bg, fg) (tg_border_cells){ \
    (tg_cell){0x2500, bg, fg}, \
    (tg_cell){0x2502, bg, fg}, \
    (tg_cell){0x256D, bg, fg}, \
    (tg_cell){0x256E, bg, fg}, \
    (tg_cell){0x2570, bg, fg}, \
    (tg_cell){0x256F, bg, fg}  \
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
    (tg_cell){0x0022, bg, fg}, \
    (tg_cell){0x007C, bg, fg}, \
    (tg_cell){0x002F, bg, fg}, \
    (tg_cell){0x005C, bg, fg}  \
}
// _‾|/\   .
#define TG_LINE_CELLS_DEFAULT(bg, fg) (tg_line_cells){ \
    (tg_cell){0x005F, bg, fg}, \
    (tg_cell){0x203E, bg, fg}, \
    (tg_cell){0x007C, bg, fg}, \
    (tg_cell){0x002F, bg, fg}, \
    (tg_cell){0x005C, bg, fg}  \
}
#define TG_GRID_CELLS_BORDER_HORIZ(bg, fg) (tg_grid_cells){ \
    (tg_cell){0x2500, bg, fg}, \
    (tg_cell){0x251C, bg, fg}, \
    (tg_cell){0x2524, bg, fg}  \
}
#define TG_GRID_CELLS_BORDER_VERT(bg, fg) (tg_grid_cells){ \
    (tg_cell){0x2502, bg, fg}, \
    (tg_cell){0x252C, bg, fg}, \
    (tg_cell){0x2534, bg, fg}  \
}
#define TG_GRID_CELLS_BORDER_SERRATED_HORIZ(bg, fg) (tg_grid_cells){ \
    (tg_cell){0x2500, bg, fg}, \
    (tg_cell){0x253C, bg, fg}, \
    (tg_cell){0x253C, bg, fg}  \
}
#define TG_GRID_CELLS_BORDER_SERRATED_VERT(bg, fg) (tg_grid_cells){ \
    (tg_cell){0x2502, bg, fg}, \
    (tg_cell){0x253C, bg, fg}, \
    (tg_cell){0x253C, bg, fg}  \
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
#define TG_LINE_CELLS_BORDER(bg, fg) (tg_line_cells){ \
    (tg_cell){0x2500, bg, fg}, \
    (tg_cell){0x2500, bg, fg}, \
    (tg_cell){0x2502, bg, fg}, \
    (tg_cell){0x2571, bg, fg}, \
    (tg_cell){0x2572, bg, fg}  \
}

typedef struct {
    float minx, maxx;
    float miny, maxy;
} tg_graph_minmax;


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
typedef struct {
    tg_cell base;
    union {
        tg_cell top;
        tg_cell left;
    };
    union {
        tg_cell bottom;
        tg_cell right;
    };
} tg_grid_cells;


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
    TG_NODRAW,
    TG_LINE_CELLS,
    TG_LINE_BRAILLE,
    TG_STEP_CELLS,
    // TG_STEP_BRAILLE,
    // TR_PILLAR_CELLS,
    // TG_PILLAR_BRAILLE,
} tg_line_mode;

typedef struct
{
    tg_line_mode mode;

    union {
        tg_line_cells line_cells; // used if mode is TG_LINE_CELLS
        tg_border_cells step_cells; // used if mode is TG_STEP_CELLS
        // tg_cell pillar_cell; // used if mode is TG_PILLAR_CELLS

        struct {
            float density;
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
    int draw_crosses; // draw a cross where 2 lines intersect to smoothly connect them. currently copies vertical.cell's color
    struct
    {
        int count;
        tg_grid_cells cells;
    } horizontal;
    struct
    {
        int count;
        tg_grid_cells cells;
    } vertical;
} tg_grid_opts;

typedef struct {
    struct {
        int count;
        int decimal_places;
        uint32_t bg;
        uint32_t fg;
    } x;
    struct {
        int count;
        int decimal_places;
        uint32_t bg;
        uint32_t fg;
    } y;
} tg_indicator_opts;

typedef struct {
    float value;
    int unset; // set to 1 to ignore value and let the program decide it
} tg_autoval;

// typedef enum {
//     TG_NOIGNORE,
//     TG_IGNORE_FULL,
//     TG_IGNORE_LEAVESPACE
// } tg_ignore_mode;
typedef struct {
    tg_autoval min;
    tg_autoval max;
    // struct {
    //     tg_ignore_mode mode;
    //     float min;
    //     float max;
    // } ignore_range;
} tg_axis_opts;

typedef struct
{
    int width, height;
    tg_rect padding;

    tg_axis_opts x;
    tg_axis_opts y;

    tg_grid_opts grid;
    tg_line_opts line;
    tg_node_opts node;
    tg_indicator_opts indicator;

} tg_render_opts;




static inline tg_render_opts tg_default_render_opts() {
    return (tg_render_opts) {
        .width = 141, .height = 25,
        .padding = {
            .left = 6,
            .top = 0,
            .right = 5,
            .bottom = 1
        },

        .x = { 
            .min = { .value = 0, .unset = 1 },
            .max = { .value = 0, .unset = 1 },
        },
        .y = {
            .min = { .value = 0, .unset = 1 },
            .max = { .value = 0, .unset = 1 },
        },

        .grid = {
            .draw_crosses = 1, // TODO: make this its own struct just like the other two
            .horizontal = {
                .count = 5,
                .cells = TG_GRID_CELLS_BORDER_HORIZ(0, 0x555555)
            },
            .vertical = {
                .count = 10,
                .cells = TG_GRID_CELLS_BORDER_VERT(0, 0x555555)
            } 
        },
        .node = {0},
        .indicator = {
            .x = {
                .count = 10,
                .decimal_places = 2,
                .bg = 0,
                .fg = 0xFF0000
            },
            .y = {
                .count = 5,
                .decimal_places = 2,
                .bg = 0,
                .fg = 0xFF0000
            }
        }
    };
}

static inline tg_render_opts tg_default_render_opts_line(uint32_t bg, uint32_t fg) {
    tg_render_opts opt = tg_default_render_opts();
    opt.line.mode = TG_LINE_CELLS;
    opt.line.line_cells = TG_LINE_CELLS_DEFAULT(bg, fg);
    return opt;
}

static inline tg_render_opts tg_default_render_opts_braille(uint32_t bg, uint32_t fg) {
    tg_render_opts opt = tg_default_render_opts();
    opt.line = (tg_line_opts){
        .mode = TG_LINE_BRAILLE,
        .braille = {
            .density = 1,
            .bg = bg,
            .fg = fg
        },
    };
    return opt;
}
static inline tg_render_opts tg_default_render_opts_steps(uint32_t bg, uint32_t fg) {
    tg_render_opts opt = tg_default_render_opts();
    opt.line.mode = TG_STEP_CELLS;
    opt.line.step_cells = TG_BORDER_CELLS_ROUNDED(bg, fg);
    return  opt;
}
