#pragma once
#include "../types.h"

void tg_buffer_fill(tg_cell *buf, size_t count, tg_cell cell);
int tg_buffer_set_safe(tg_cell *buf, size_t x, size_t y, size_t width, size_t height, tg_cell cell);
