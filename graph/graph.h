#pragma once
#include "../types.h"
 

void tg_draw_grid(tg_cell *buf,
                  size_t width, size_t height,
                  tg_rect area,
                  const tg_grid_opts *opt);

void tg_draw_lines(tg_cell *buf,
                   int width, int height, tg_rect area,
                   const float *datax, const float *datay, size_t count,
                   float minx, float maxx,
                   float miny, float maxy,
                   const tg_line_opts *opt);

void tg_draw_nodes(tg_cell *buf,
                   int width, int height, tg_rect area,
                   const float *datax, const float *datay, size_t count,
                   float minx, float maxx,
                   float miny, float maxy,
                   const tg_node_opts *opt);

void tg_draw_indicators(tg_cell *buf,
                        size_t width, size_t height, tg_rect area,
                        float minx, float maxx,
                        float miny, float maxy,
                        const tg_indicator_opts *opt);

/// @brief Renders a graph onto the tg_cell buffer
void tg_render_minmax(tg_cell *buf,
                      const float *datax, const float *datay, size_t count,
                      float minx, float maxx,
                      float miny, float maxy,
                      const tg_render_opts *opt);


/// @brief Renders a graph onto the tg_cell buffer
void tg_render(tg_cell *buf,
               const float *datax, const float *datay, size_t count,
               const tg_render_opts *opt);

/// @brief Clears a tg_cell buffer
void tg_clear(tg_cell *buf, size_t width, size_t height);
