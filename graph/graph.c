#include "../canvas/canvas.h"
#include "../buffer/buffer.h"
#include "../drawing/drawing.h"
#include <string.h>
#include "graph.h"

static tg_point calc_node_pos(tg_rect area, float datax, float datay, float minx, float miny, float dx, float dy) {
    int sizex = area.right - area.left - 1;
    int sizey = area.bottom - area.top - 1;

    int x = area.left + (datax - minx) / dx * sizex;
    int y = area.bottom - 1 - (datay - miny) / dy * sizey;

    return (tg_point){x, y};
}

// TODO: variable decimal length
// 99% ChatGPT
static void tg_parse_float(tg_cell *out, float value, int max_cells, uint32_t bg, uint32_t fg)
{
    int i = 0;

    if (value < 0.0f && i < max_cells) {
        out[i].ch = '-';
        out[i].fg = fg;
        out[i].bg = bg;
        i++;
        value = -value;
    }

    int int_part = (int)value;
    float frac = value - (float)int_part;

    // scale to 2 decimals
    int frac_part = (int)(frac * 100.0f + 0.5f);

    // write integer part
    char buf[12];
    int n = 0;

    do {
        buf[n++] = '0' + (int_part % 10);
        int_part /= 10;
    } while (int_part);

    while (n-- && i < max_cells) {
        out[i].ch = buf[n];
        out[i].fg = fg;
        out[i].bg = bg;
        i++;
    }

    // decimal point
    if (i < max_cells) {
        out[i].ch = '.';
        out[i].fg = fg;
        out[i].bg = bg;
        i++;
    }

    // fractional part (always 2 digits)
    int div = 100;
    while (div && i < max_cells) {
        out[i].ch = '0' + (frac_part / div) % 10;
        out[i].fg = fg;
        out[i].bg = bg;
        i++;
        div /= 10;
    }
}

static inline int tg_axis_pos(int i, int count, int size) {
    if (count <= 1) return 0;
    return (i * (size - 1)) / (count - 1);
}

static inline int tg_map_x(tg_rect a, int i, int count)
{
    int w = a.right - a.left;
    int offset = tg_axis_pos(i, count, w);
    return a.left + offset;
}
static inline int tg_map_y(tg_rect a, int i, int count)
{
    int h = a.bottom - a.top;
    int offset = tg_axis_pos(i, count, h);
    return a.bottom - 1 - offset;
}

void tg_draw_grid(tg_cell *buf,
                  size_t width, size_t height,
                  tg_rect area,
                  const tg_grid_opts *opt)
{
    int pos;

    int w = area.right - area.left;
    int h = area.bottom - area.top;

    // vertical lines
    for (size_t i = 0; i < opt->vertical.count; i++)
    {
        pos = tg_map_x(area, i, opt->vertical.count);

        for (int y = area.top; y < area.bottom; y++)
        {
            buf[width * y + pos] = opt->vertical.cell;
        }
    }

    // horizontal lines
    for (size_t i = 0; i < opt->horizontal.count; i++)
    {
        pos = tg_map_y(area, i, opt->horizontal.count);

        for (int x = area.left; x < area.right; x++)
        {
            buf[width * pos + x] = opt->horizontal.cell;
        }
    }
}

void tg_draw_lines(tg_cell *buf,
                   int width, int height, tg_rect area,
                   const float *datax, const float *datay, size_t count,
                   float minx, float maxx,
                   float miny, float maxy,
                   const tg_line_opts *opt) {
    
    if (count < 2)
        return;
    
    float dx, dy;
    tg_point prev_pos, current_pos;
    tg_point bufsize = {width, height};

    dx = maxx - minx;
    dy = maxy - miny;
    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;


    prev_pos = calc_node_pos(area, datax[0], datay[0], minx, miny, dx, dy);
    for (size_t i = 1; i < count; i++)
    {
        current_pos = calc_node_pos(area, datax[i], datay[i], minx, miny, dx, dy);
        if (opt->mode == TG_MODE_CELLS) {
            tg_draw_line(buf, bufsize, prev_pos, current_pos, opt->cells);
        }
        else if (opt->mode == TG_MODE_BRAILLE) {
            tg_draw_line_braille(buf, bufsize, prev_pos, current_pos, opt->braille.bg, opt->braille.fg, opt->braille.line_density);
        }

        prev_pos = current_pos;
    }
}

void tg_draw_nodes(tg_cell *buf,
                   int width, int height, tg_rect area,
                   const float *datax, const float *datay, size_t count,
                   float minx, float maxx,
                   float miny, float maxy,
                   const tg_node_opts *opt) {

    if (count == 0)
        return;
    
    float dx, dy;
    tg_point pos;

    dx = maxx - minx;
    dy = maxy - miny;
    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;
    
    for (size_t i = 0; i < count; i++)
    {        
        pos = calc_node_pos(area, datax[i], datay[i], minx, miny, dx, dy);
        tg_buffer_set_safe(buf, pos.x, pos.y, width, height, opt->cell);
    }
}


void tg_draw_indicators(tg_cell *buf,
                        size_t width, size_t height, tg_rect area,
                        float minx, float maxx,
                        float miny, float maxy,
                        const tg_indicator_opts *opt)
{
    float dx = maxx - minx;
    float dy = maxy - miny;

    if (opt->y.count > 1)
    {
        int h = area.bottom - area.top;

        for (int i = 0; i < opt->y.count; i++)
        {
            float t = i / (float)(opt->y.count - 1);
            float value = miny + t * dy;

            int row = tg_map_y(area, i, opt->y.count);

            tg_parse_float(&buf[width * row], value, (int)width, opt->y.bg, opt->y.fg);
        }
    }

    if (opt->x.count > 1)
    {
        int w = area.right - area.left;

        for (int i = 0; i < opt->x.count; i++)
        {
            float t = i / (float)(opt->x.count - 1);
            float value = minx + t * dx;

            int col = tg_map_x(area, i, opt->x.count);
            int row = area.bottom < height // place it 1 row below graph or on bottom row if it doesnt fit
                      ? area.bottom
                      : area.bottom - 1;

            tg_parse_float(&buf[width * row + col],
                        value,
                        (int)(width - col),
                        opt->x.bg,
                        opt->x.fg);
        }
    }
}


void tg_render_minmax(tg_cell *buf,
                      const float *datax, const float *datay, size_t count,
                      float minx, float maxx,
                      float miny, float maxy,
                      const tg_render_opts *opt) {
    
    if (count == 0)
        return;

    tg_rect area = {
        .left = opt->padding.left,
        .top = opt->padding.top,
        .right = opt->width - opt->padding.right,
        .bottom = opt->height - opt->padding.bottom
    };

    if (opt->grid.draw) {
        tg_draw_grid(buf, opt->width, opt->height, area, &opt->grid);
    }
    if (opt->line.draw) {
        tg_draw_lines(buf,
                      opt->width, opt->height, area,
                      datax, datay, count,
                      minx, maxx, miny, maxy,
                      &opt->line);
    }
    if (opt->node.draw) {
        tg_draw_nodes(buf,
                      opt->width, opt->height, area,
                      datax, datay, count,
                      minx, maxx, miny, maxy,
                      &opt->node);
    }
    if (opt->indicator.draw) {
        tg_draw_indicators(buf,
                           opt->width, opt->height, area,
                           minx, maxx, miny, maxy,
                           &opt->indicator);
    }

}

void tg_render(tg_cell *buf,
               const float *datax, const float *datay, size_t count,
               const tg_render_opts *opt) {

    if (count == 0)
        return;
    
    float minx = datax[0];
    float maxx = datax[0]; 
    float miny = datay[0];
    float maxy = datay[0];
    for (size_t i = 1; i < count; i++)
    {
        if      (datax[i] < minx) minx = datax[i];
        else if (datax[i] > maxx) maxx = datax[i];
        if      (datay[i] < miny) miny = datay[i];
        else if (datay[i] > maxy) maxy = datay[i];
    }

    tg_render_minmax(buf,
                     datax, datay, count,
                     minx, maxx, miny, maxy,
                     opt);
}

void tg_clear(tg_cell *buf, size_t width, size_t height) {
    memset(buf, TG_EMPTY, sizeof(tg_cell) * width * height);
}
