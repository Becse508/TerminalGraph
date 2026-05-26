#include "converters.h"
#include <stdlib.h>

// ChatGPT used
static inline int encode_utf8(uint32_t cp, char *out, size_t out_size)
{
    if (cp <= 0x7F && out_size >= 1) {
        out[0] = cp;
        return 1;
    }
    else if (cp <= 0x7FF && out_size >= 2) {
        out[0] = 0xC0 | (cp >> 6);
        out[1] = 0x80 | (cp & 0x3F);
        return 2;
    }
    else if (cp <= 0xFFFF && out_size >= 3) {
        out[0] = 0xE0 | (cp >> 12);
        out[1] = 0x80 | ((cp >> 6) & 0x3F);
        out[2] = 0x80 | (cp & 0x3F);
        return 3;
    }
    else if (cp <= 0x10FFFF && out_size >= 4) {
        out[0] = 0xF0 | (cp >> 18);
        out[1] = 0x80 | ((cp >> 12) & 0x3F);
        out[2] = 0x80 | ((cp >> 6) & 0x3F);
        out[3] = 0x80 | (cp & 0x3F);
        return 4;
    }
    else if (cp > 0x10FFFF && out_size >= 1) { // invalid
        out[0] = '?';
        return 1;
    }

    return 0; // not enough space
}
// ChatGPT used
static inline int u8_to_dec(char *out, uint8_t v)
{
    if (v < 10) {
        out[0] = '0' + v;
        return 1;
    }
    if (v < 100) {
        out[0] = '0' + (v / 10);
        out[1] = '0' + (v % 10);
        return 2;
    }

    out[0] = '0' + (v / 100);
    out[1] = '0' + ((v / 10) % 10);
    out[2] = '0' + (v % 10);
    return 3;
}

// --- COLORS ---

static const uint8_t ansi16_rgb[16][3] = {
    {0,0,0},       // black
    {205,0,0},     // red
    {0,205,0},     // green
    {205,205,0},   // yellow
    {0,0,205},     // blue
    {205,0,205},   // magenta
    {0,205,205},   // cyan
    {229,229,229}, // white

    {127,127,127}, // bright black
    {255,0,0},     // bright red
    {0,255,0},     // bright green
    {255,255,0},   // bright yellow
    {0,0,255},     // bright blue
    {255,0,255},   // bright magenta
    {0,255,255},   // bright cyan
    {255,255,255}  // bright white
};
static const uint8_t ansi16_fg[16] = {
    30,31,32,33,34,35,36,37,
    90,91,92,93,94,95,96,97
};
static const uint8_t ansi16_bg[16] = {
    40,41,42,43,44,45,46,47,
    100,101,102,103,104,105,106,107
};

static inline int dist2(uint8_t r, uint8_t g, uint8_t b, const uint8_t c[3]) {
    int dr = r - c[0];
    int dg = g - c[1];
    int db = b - c[2];
    return dr*dr + dg*dg + db*db;
}

static inline uint8_t rgb_to_ansi16(uint32_t rgb, int is_bg)
{
    int r = TG_R(rgb);
    int g = TG_G(rgb);
    int b = TG_B(rgb);

    uint8_t best = 0;
    int best_d = 1e9;

    for (uint8_t i = 0; i < 16; i++) {
        int d = dist2(r, g, b, ansi16_rgb[i]);
        if (d < best_d) {
            best_d = d;
            best = i;
        }
    }

    return is_bg ? ansi16_bg[best] : ansi16_fg[best];
}

static inline uint8_t rgb_to_ansi256(uint32_t rgb) {
    // uint8_t r = TG_R(rgb) * 5 / 255;
    // uint8_t g = TG_G(rgb) * 5 / 255;
    // uint8_t b = TG_B(rgb) * 5 / 255;

    uint8_t r = (TG_R(rgb) * 5) >> 8;
    uint8_t g = (TG_G(rgb) * 5) >> 8;
    uint8_t b = (TG_B(rgb) * 5) >> 8;

    int n = 16 + 36*r + 6*g + b;
    return n;
}

static inline int emit_ansi16(int is_bg, char *out, uint32_t rgb) {
    char *p = out;

    *p++ = '\x1b';
    *p++ = '[';

    p += u8_to_dec(p, rgb_to_ansi16(rgb, is_bg));
    *p++ = 'm';

    return (int)(p - out);
}

static inline int emit_ansi256(int is_bg, char *out, uint32_t rgb) {
    char *p = out;

    *p++ = '\x1b';
    *p++ = '[';

    // 38 (fb) | 48 (bg)
    if (!is_bg)
        *p++ = '3';
    else
        *p++ = '4';

    *p++ = '8'; *p++ = ';';
    *p++ = '5'; *p++ = ';'; // 5 = 256 color

    p += u8_to_dec(p, rgb_to_ansi256(rgb));
    *p++ = 'm';

    return (int)(p - out);
}

// ChatGPT used
static inline int emit_truecolor(int is_bg, char *out, uint32_t rgb)
{
    uint8_t r = TG_R(rgb);
    uint8_t g = TG_G(rgb);
    uint8_t b = TG_B(rgb);

    char *p = out;

    *p++ = '\x1b';
    *p++ = '[';

    // 38 (fb) | 48 (bg)
    if (!is_bg)
        *p++ = '3';
    else
        *p++ = '4';

    *p++ = '8'; *p++ = ';';
    *p++ = '2'; *p++ = ';'; // 2 = truecolor

    p += u8_to_dec(p, r);
    *p++ = ';';
    p += u8_to_dec(p, g);
    *p++ = ';';
    p += u8_to_dec(p, b);

    *p++ = 'm';

    return (int)(p - out);
}

#define ANSI16_SIZE 6
#define ANSI256_SIZE 11
#define TRUECOLOR_SIZE 19

static const int (*emits[])(int, char*, uint32_t) = {
    emit_ansi16, emit_ansi256, emit_truecolor
};
static const int color_sizes[] = {
    0, ANSI16_SIZE, ANSI256_SIZE, TRUECOLOR_SIZE
};

static inline int write_color_code(uint32_t bg, uint32_t fg,
                                   uint32_t prevbg, uint32_t prevfg,
                                   const tg_convert_opts *opt,
                                   char *buffer, size_t bufsize,
                                   int *overflow) {

    int (*emit)(int, char*, uint32_t) = emits[opt->color_format - 1];
    int colsize = color_sizes[opt->color_format];

    int char_count = 0;
    int c;
    if (opt->color_format != TG_NOCOLOR) {
        if (fg != prevfg) {
            if (char_count + colsize >= bufsize) {
                *overflow = 1;
                return char_count;
            }

            c = emit(0, buffer + char_count, fg);
            if (c < 0) return char_count;
            char_count += c;
        }

        if (bg != prevbg && opt->use_background) {
            if (char_count + colsize >= bufsize) {
                *overflow = 1;
                return char_count;
            }

            c = emit(0, buffer + char_count, bg);
            if (c < 0) return char_count;
            char_count += c;
        }
    }

    return char_count;
}

// --------------

static inline size_t convert_ascii(uint32_t ch, char *out, size_t out_size, char empty_char) {
    if (out_size <= 1) {
        return 0;
    }

    if (ch == 0) {
        *out = empty_char;
    }
    else if (ch < 128) {
        *out = (char)ch;
    }
    else {
        *out = '?';
    }
    return 1;
}
static inline size_t convert_utf8(uint32_t ch, char *out, size_t out_size, char empty_char) {
    if (ch == 0 && out_size >= 1) {
        out[0] = empty_char;
        return 1;
    }
    return encode_utf8(ch, out, out_size);
}



// ugly ahh macro
#define end_conversion() \
    if (opt->append_color_reset ) { \
        if (k >= out_size - end_space_needed) { \
            k = out_size - end_space_needed; \
        } \
        out[k++] = '\x1b'; \
        out[k++] = '['; \
        out[k++] = '0'; \
        out[k++] = 'm'; \
    } \
    if (k < out_size) out[k] = 0; \
    else out[out_size - 1] = 0;

static size_t convert(tg_cell *buf,
                      char *out, size_t out_size,
                      size_t width, size_t height,
                      const tg_convert_opts *opt,
                      size_t (*convertfn)(uint32_t ch, char *out, size_t out_size, char empty_char)) {

    int end_space_needed = opt->append_color_reset ? 5 : 1;
    int color_overflow = 0;
    
    size_t k = 0;
    size_t tmp_k, idx;
    uint32_t bg = 0xFFFFFFFF; // nonexistent color code (32 bits instead of 24)
    uint32_t fg = 0xFFFFFFFF; // so color always emitted at first char
    
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            if (k >= out_size - end_space_needed) {
                end_conversion()
            }

            idx = y * width + x;

            if (buf[idx].ch != TG_EMPTY) { // dont waste color for empty chars
                k += write_color_code(buf[idx].bg, buf[idx].fg, bg, fg, opt, &out[k], out_size - k - end_space_needed, &color_overflow);
                if (color_overflow)
                    end_conversion()
            }
            
            bg = buf[idx].bg;
            fg = buf[idx].fg;

            tmp_k = convertfn(buf[idx].ch, out + k, out_size - k - end_space_needed, opt->empty_char); // -1 for null term
            if (!tmp_k) {
                end_conversion()
            }
            k += tmp_k;
        }
        if (k >= out_size - end_space_needed) {
            end_conversion()
        }
        out[k++] = '\n';
    }
    end_conversion()
    return k;
}

/// Buffer size must be at least equal to the canvas size or you're fucked
void tg_to_canvas(tg_cell *buf, tg_canvas *canvas) {
    for (size_t y = 0; y < canvas->height; y++)
    {
        for (size_t x = 0; x < canvas->width; x++)
        {
            canvas->set(canvas->ctx, x, y, buf[y * canvas->width + x]);
        }
    }
}

size_t tg_to_ascii(tg_cell *buf, char* out, size_t out_size, size_t width, size_t height, const tg_convert_opts *opt) {
    return convert(buf, out, out_size, width, height, opt, convert_ascii);
}

size_t tg_to_utf8(tg_cell *buf, char *out, size_t out_size, size_t width, size_t height, const tg_convert_opts *opt) {
    return convert(buf, out, out_size, width, height, opt, convert_utf8);
}

static inline char *alloc_string(size_t count, tg_color_format colorf, int is_utf8, size_t *out_n)
{
    size_t n = count;
    int colorsize = color_sizes[colorf];

    if (is_utf8)
        n *= 4;
    
    n += count * colorsize * 2;    

    char *p = malloc(n + 1);

    if (out_n)
        *out_n = p ? n : 0;
    return p;
}

char *tg_string_alloc(size_t width, size_t height, tg_color_format colorf, int is_utf8, size_t *allocated_size) {
    return alloc_string(width * height, colorf, is_utf8, allocated_size);
}

static char *convert_alloc(tg_cell *buf,
                    size_t width, size_t height,
                    const tg_convert_opts *opt, int is_utf8) {
                    // void (*convertfn)(tg_cell*, size_t, size_t, size_t, tg_convert_opts)

    size_t k;
    size_t n;
    char *ptr = alloc_string(width * height, opt->color_format, is_utf8, &n);
    if (is_utf8) {
        k = tg_to_utf8(buf, ptr, n, width, height, opt);
    }
    else {
        k = tg_to_ascii(buf, ptr, n, width, height, opt);
    }
    char *tmp = realloc(ptr, k + 1);
    if (tmp)
        ptr = tmp;
    
    return ptr;
}

char *tg_to_ascii_alloc(tg_cell *buf, size_t width, size_t height, const tg_convert_opts *opt) {
    return convert_alloc(buf, width, height, opt, 0);
}

char *tg_to_utf8_alloc(tg_cell *buf, size_t width, size_t height, const tg_convert_opts *opt) {
    return convert_alloc(buf, width, height, opt, 1);
}
