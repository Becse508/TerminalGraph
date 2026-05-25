#include "converters.h"
#include <stdlib.h>

#define TRUECOLOR_SIZE 19

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
// ChatGPT used
static inline int emit_truecolor(int is_bg, char *out, uint32_t rgb)
{
    uint8_t r = TG_R(rgb);
    uint8_t g = TG_G(rgb);
    uint8_t b = TG_B(rgb);

    char *p = out;

    *p++ = '\x1b';
    *p++ = '[';

    // 38;2; | 48;2;
    if (!is_bg) {
        *p++ = '3';
    }
    else {
        *p++ = '4';
    }
    *p++ = '8'; *p++ = ';';
    *p++ = '2'; *p++ = ';';

    p += u8_to_dec(p, r);
    *p++ = ';';
    p += u8_to_dec(p, g);
    *p++ = ';';
    p += u8_to_dec(p, b);

    *p++ = 'm';

    return (int)(p - out);
}

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

// TODO: more color formats
static inline int write_color_code(uint32_t bg, uint32_t fg,
                                   uint32_t prevbg, uint32_t prevfg,
                                   tg_convert_opts *opt,
                                   char *buffer, size_t bufsize,
                                   int *overflow) {
    int char_count = 0;
    int c;
    if (opt->color_format == TG_TRUECOLOR) {
        if (fg != prevfg) {
            if (char_count + TRUECOLOR_SIZE >= bufsize) {
                *overflow = 1;
                return char_count;
            }

            c = emit_truecolor(0, buffer + char_count, fg);
            if (c < 0) return char_count;
            char_count += c;
        }

        if (bg != prevbg && opt->use_background) {
            if (char_count + TRUECOLOR_SIZE >= bufsize) {
                *overflow = 1;
                return char_count;
            }

            c = emit_truecolor(0, buffer + char_count, bg);
            if (c < 0) return char_count;
            char_count += c;
        }
    }

    return char_count;
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
                      tg_convert_opts *opt,
                      size_t (*convertfn)(uint32_t ch, char *out, size_t out_size, char empty_char)) {

    int end_space_needed = opt->append_color_reset ? 5 : 1;
    int color_overflow = 0;
    
    size_t k = 0;
    size_t tmp_k, idx;
    uint32_t bg = 0xFFFFFFFF; // nonexistent color code
    uint32_t fg = 0xFFFFFFFF; // so color always emitted at first char
    
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            if (k >= out_size - end_space_needed) {
                end_conversion()
            }

            idx = y * width + x;
            k += write_color_code(buf[idx].bg, buf[idx].fg, bg, fg, opt, &out[k], out_size - k - end_space_needed, &color_overflow);
            if (color_overflow)
                end_conversion()
            
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

/// @brief Buffer size must be at least equal to the canvas size or you're fucked
void tg_to_canvas(tg_cell *buf, tg_canvas *canvas) {
    for (size_t y = 0; y < canvas->height; y++)
    {
        for (size_t x = 0; x < canvas->width; x++)
        {
            canvas->set(canvas->ctx, x, y, buf[y * canvas->width + x]);
        }
    }
}

/// @brief non-ascii characters will be displayed as '?'
size_t tg_to_ascii(tg_cell *buf, char* out, size_t out_size, size_t width, size_t height, tg_convert_opts *opt) {
    return convert(buf, out, out_size, width, height, opt, convert_ascii);
}
/// @brief invalid utf-8 characters will be displayed as '?' 
size_t tg_to_utf8(tg_cell *buf, char *out, size_t out_size, size_t width, size_t height, tg_convert_opts *opt) {
    return convert(buf, out, out_size, width, height, opt, convert_utf8);
}

// TODO: alloc for different color formats
static char *alloc_string(size_t count, tg_color_format colorf, int is_utf8, size_t *out_n)
{
    size_t n = count;

    if (is_utf8)
        n *= 4;

    if (colorf == TG_TRUECOLOR)
        n += count * 38;

    char *p = malloc(n + 1);
    if (out_n) *out_n = n;

    return p;
}

char *convert_alloc(tg_cell *buf,
                    size_t width, size_t height,
                    tg_convert_opts *opt, int is_utf8) {
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

char *tg_to_ascii_alloc(tg_cell *buf, size_t width, size_t height, tg_convert_opts *opt) {
    return convert_alloc(buf, width, height, opt, 0);
}

char *tg_to_utf8_alloc(tg_cell *buf, size_t width, size_t height, tg_convert_opts *opt) {
    return convert_alloc(buf, width, height, opt, 1);
}
