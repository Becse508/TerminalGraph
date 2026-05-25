#pragma once
#include "../types.h"
#include "../canvas/canvas.h"

#define TG_RGB(r, g, b) ((uint32_t)(((r)<<16) | ((g)<<8) | (b)))
#define TG_R(c) ((uint8_t)((c >> 16) & 0xFF))
#define TG_G(c) ((uint8_t)((c >> 8)  & 0xFF))
#define TG_B(c) ((uint8_t)(c & 0xFF))

typedef enum {
    TG_NOCOLOR,
    TG_ANSI_16,
    TG_ANSI_256,
    TG_TRUECOLOR
} tg_color_format;

typedef struct
{
    tg_color_format color_format;
    int use_background;
    int append_color_reset;
    uint32_t empty_char;
} tg_convert_opts;

static inline tg_convert_opts tg_default_convert_opts() {
    return (tg_convert_opts) {
        .color_format = TG_TRUECOLOR,
        .use_background = 1,
        .append_color_reset = 1,
        .empty_char = ' '
    };
}


void tg_to_canvas(tg_cell *buf, tg_canvas *canvas);
size_t tg_to_ascii(tg_cell *buf, char* out, size_t out_size, size_t width, size_t height, const tg_convert_opts *opt);
size_t tg_to_utf8(tg_cell *buf, char *out, size_t out_size, size_t width, size_t height, const tg_convert_opts *opt);

char *tg_to_ascii_alloc(tg_cell *buf, size_t width, size_t height, const tg_convert_opts *opt);
char *tg_to_utf8_alloc(tg_cell *buf, size_t width, size_t height, const tg_convert_opts *opt);
