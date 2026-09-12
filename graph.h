#pragma once
#include "defs.h"
 

void tg_draw_grid(tg_cell *buf,
                  size_t width,
                  tg_rect area,
                  const tg_grid_opts *opt);

void tg_draw_lines(tg_cell *buf,
                   int width, int height, tg_rect area,
                   const float *datax, const float *datay, size_t count,
                   tg_graph_minmax minmax,
                   const tg_line_opts *opt);

void tg_draw_nodes(tg_cell *buf,
                   int width, int height, tg_rect area,
                   const float *datax, const float *datay, size_t count,
                   tg_graph_minmax minmax,
                   const tg_node_opts *opt);

void tg_draw_indicators(tg_cell *buf,
                        size_t width, size_t height, tg_rect area,
                        tg_graph_minmax minmax,
                        const tg_indicator_opts *opt);

// Renders a graph onto the tg_cell buffer
void tg_render_minmax(tg_cell *buf,
                      const float *datax, const float *datay, size_t count,
                      tg_graph_minmax minmax,
                      const tg_render_opts *opt);


// Renders a graph onto the tg_cell buffer
void tg_render(tg_cell *buf,
               const float *datax, const float *datay, size_t count,
               const tg_render_opts *opt);

// Clears a tg_cell buffer
void tg_clear(tg_cell *buf, size_t width, size_t height);



static inline tg_graph_minmax tg_calc_minmax(const float *datax, const float *datay, size_t count,
                                             const tg_axis_opts *optx, const tg_axis_opts *opty) {
    
    tg_graph_minmax minmax = {
        .minx = optx->min.unset ? datax[0] : optx->min.value,
        .maxx = optx->max.unset ? datax[0] : optx->max.value,
        .miny = opty->min.unset ? datay[0] : opty->min.value,
        .maxy = opty->max.unset ? datay[0] : opty->max.value,
    };

    if (optx->min.unset || optx->max.unset || opty->min.unset || opty->max.unset) {
        for (size_t i = 1; i < count; i++)
        {
            if      (optx->min.unset && datax[i] < minmax.minx) minmax.minx = datax[i];
            else if (optx->max.unset && datax[i] > minmax.maxx) minmax.maxx = datax[i];
            if      (opty->min.unset && datay[i] < minmax.miny) minmax.miny = datay[i];
            else if (opty->max.unset && datay[i] > minmax.maxy) minmax.maxy = datay[i];
        }
    }
    
    return minmax;
}