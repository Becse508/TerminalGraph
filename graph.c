#include <math.h>
#include <string.h>
#include "buffer.h"
#include "defs.h"
#include "drawing.h"
#include "graph.h"

static tg_point calc_node_pos(tg_rect area, float datax, float datay, float minx, float miny, float dx, float dy) {
    int sizex = area.right - area.left - 1;
    int sizey = area.bottom - area.top - 1;

    int x = area.left + (datax - minx) / dx * sizex;
    int y = area.bottom - 1 - (datay - miny) / dy * sizey;

    return (tg_point){x, y};
}

// 70% AI
static void tg_parse_float(tg_cell *out, float value, int decimal_places, int max_cells, uint32_t bg, uint32_t fg)
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
    
    
    // write integer part
    do {
        out[i].ch = '0' + (int_part % 10);
        out[i].fg = fg;
        out[i].bg = bg;
        i++;
        int_part /= 10;
    } while (int_part && i < max_cells - 1);
    
    if (decimal_places <= 0)
        return;

    float frac = value - (float)int_part;
    int frac_part = (int)(frac * pow(10, decimal_places - 1) + 0.5f);


    // decimal point
    if (i < max_cells) {
        out[i].ch = '.';
        out[i].fg = fg;
        out[i].bg = bg;
        i++;
    }

    // fractional part
    int div = pow(10, decimal_places - 1);
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

#define GRID_CROSS 0x253C

void tg_draw_grid(tg_cell *buf,
                  size_t width,
                  tg_rect area,
                  const tg_grid_opts *opt)
{
    int pos;

    // vertical lines
    for (int i = 0; i < opt->vertical.count; i++)
    {
        pos = tg_map_x(area, i, opt->vertical.count);
        
        for (int y = area.top + 1; y < area.bottom - 1; y++)
        {
            buf[width * y + pos] = opt->vertical.cells.cell;
        }
    }

    // horizontal lines
    for (int i = 0; i < opt->horizontal.count; i++)
    {
        pos = tg_map_y(area, i, opt->horizontal.count);

        for (int x = area.left + 1; x < area.right - 1; x++)
        {
            buf[width * pos + x] = opt->horizontal.cells.cell;
        }
    }

    // vertical line endings
    for (int i = 0; i < opt->vertical.count; i++)
    {
        pos = tg_map_x(area, i, opt->vertical.count);

        buf[width * area.top + pos] = opt->vertical.cells.top;
        buf[width * (area.bottom-1) + pos] = opt->vertical.cells.bottom;
    }

    // horizontal line endings + crosses
    for (int i = 0; i < opt->horizontal.count; i++)
    {
        pos = tg_map_y(area, i, opt->horizontal.count);

        buf[width * pos + area.left] = opt->horizontal.cells.left;
        buf[width * pos + (area.right-1)] = opt->horizontal.cells.right;

        if (opt->draw_crosses) {
            for (int i = 0; i < opt->vertical.count; i++) {
                buf[width * pos + tg_map_x(area, i, opt->vertical.count)] = (tg_cell){
                    GRID_CROSS, opt->horizontal.cells.cell.bg, opt->horizontal.cells.cell.fg
                };
            }
        }
    }
}

void tg_draw_lines(tg_cell *buf,
                   int width, int height, tg_rect area,
                   const float *datax, const float *datay, size_t count,
                   tg_graph_minmax m,
                   const tg_line_opts *opt) {
    
    if (count < 2)
        return;
    
    float dx, dy;
    tg_point prev_pos, current_pos;
    tg_point bufsize = {width, height};

    dx = m.maxx - m.minx;
    dy = m.maxy - m.miny;
    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;


    prev_pos = calc_node_pos(area, datax[0], datay[0], m.minx, m.miny, dx, dy);
    for (size_t i = 1; i < count; i++)
    {
        current_pos = calc_node_pos(area, datax[i], datay[i], m.minx, m.miny, dx, dy);
        if (opt->mode == TG_MODE_CELLS) {
            tg_draw_line(buf, bufsize, prev_pos, current_pos, opt->cells);
        }
        else if (opt->mode == TG_MODE_BRAILLE) {
            tg_draw_line_braille(buf, bufsize, prev_pos, current_pos, opt->braille.bg, opt->braille.fg, opt->braille.density);
        }

        prev_pos = current_pos;
    }
}

void tg_draw_nodes(tg_cell *buf,
                   int width, int height, tg_rect area,
                   const float *datax, const float *datay, size_t count,
                   tg_graph_minmax m,
                   const tg_node_opts *opt) {

    if (count == 0)
        return;
    
    float dx, dy;
    tg_point pos;

    dx = m.maxx - m.minx;
    dy = m.maxy - m.miny;
    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;
    
    for (size_t i = 0; i < count; i++)
    {        
        pos = calc_node_pos(area, datax[i], datay[i], m.minx, m.miny, dx, dy);
        tg_buffer_set_safe(buf, pos.x, pos.y, width, height, opt->cell);
    }
}


void tg_draw_indicators(tg_cell *buf,
                        size_t width, size_t height, tg_rect area,
                        tg_graph_minmax m,
                        const tg_indicator_opts *opt)
{
    float dx = m.maxx - m.minx;
    float dy = m.maxy - m.miny;

    if (opt->y.count > 1)
    {
        for (int i = 0; i < opt->y.count; i++)
        {
            float t = i / (float)(opt->y.count - 1);
            float value = m.miny + t * dy;

            int row = tg_map_y(area, i, opt->y.count);

            tg_parse_float(&buf[width * row],
                        value,
                        opt->x.decimal_places,
                        (int)width,
                        opt->y.bg,
                        opt->y.fg);
        }
    }

    if (opt->x.count > 1)
    {
        for (int i = 0; i < opt->x.count; i++)
        {
            float t = i / (float)(opt->x.count - 1);
            float value = m.minx + t * dx;

            int col = tg_map_x(area, i, opt->x.count);
            int row = area.bottom < height // place it 1 row below graph or on bottom row if it doesnt fit
                      ? area.bottom
                      : area.bottom - 1;

            tg_parse_float(&buf[width * row + col],
                        value,
                        opt->x.decimal_places,
                        (int)(width - col),
                        opt->x.bg,
                        opt->x.fg);
        }
    }
}


void tg_render_minmax(tg_cell *buf,
                      const float *datax, const float *datay, size_t count,
                      tg_graph_minmax minmax,
                      const tg_render_opts *opt) {
    
    if (count == 0)
        return;

    tg_rect area = {
        .left = opt->padding.left,
        .top = opt->padding.top,
        .right = opt->width - opt->padding.right,
        .bottom = opt->height - opt->padding.bottom
    };

    if (opt->grid.vertical.count > 0 || opt->grid.horizontal.count > 0) {
        tg_draw_grid(buf, opt->width, area, &opt->grid);
    }
    if (opt->line.mode != TG_MODE_NODRAW) {
        tg_draw_lines(buf,
                      opt->width, opt->height, area,
                      datax, datay, count,
                      minmax,
                      &opt->line);
    }
    if (opt->node.draw) {
        tg_draw_nodes(buf,
                      opt->width, opt->height, area,
                      datax, datay, count,
                      minmax,
                      &opt->node);
    }
    if (opt->indicator.x.count > 1 || opt->indicator.y.count > 1) {
        tg_draw_indicators(buf,
                           opt->width, opt->height, area,
                           minmax,
                           &opt->indicator);
    }

}

void tg_render(tg_cell *buf,
               const float *datax, const float *datay, size_t count,
               const tg_render_opts *opt) {

    if (count == 0)
        return;
    
    tg_graph_minmax minmax = tg_calc_minmax(datax, datay, count, &opt->x, &opt->y);
    

    tg_render_minmax(buf,
                     datax, datay, count,
                     minmax,
                     opt);
}

void tg_clear(tg_cell *buf, size_t width, size_t height) {
    memset(buf, TG_EMPTY, sizeof(tg_cell) * width * height);
}
