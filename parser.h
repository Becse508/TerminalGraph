#pragma once
#include "vector.h"


MAKE_VECTOR(float)

int tg_parse_file(const char *path, const char *format, size_t skip, vector_float *datax, vector_float *datay);