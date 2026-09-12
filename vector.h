#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MAKE_VECTOR(T)      \
typedef struct vector_##T   \
{                           \
    T *data;                \
    size_t capacity;        \
    size_t count;           \
} vector_##T;               \
\
static inline int vector_##T##_init(vector_##T *v) {    \
    v->count = 0;                                       \
    v->capacity = 10;                                   \
    v->data = (float*)malloc(sizeof(T) * 10);                   \
    return v->data == 0;                                \
}                                                       \
\
static inline int vector_##T##_push(vector_##T *v, T value) {   \
    if (v->count == v->capacity) {                              \
        v->capacity *= 2;                                       \
        T *new_data = (float*)realloc(v->data, sizeof(T) * v->capacity);\
        if (new_data == 0)                                      \
            return 1;                                           \
        v->data = new_data;                                     \
    }                                                           \
    v->data[v->count++] = value;                                \
    return 0;                                                   \
}                                                               \
\
static inline void vector_##T##_remove(vector_##T *v, size_t index) {   \
    if (index >= v->count) return;                                      \
                                                                        \
    if (index == v->count-1) {                                          \
        v->count--;                                                     \
        return;                                                         \
    }                                                                   \
                                                                        \
    for (size_t i = index; i < v->count-1; i++)                         \
    {                                                                   \
        v->data[i] = v->data[i+1];                                      \
    }                                                                   \
    v->count--;                                                         \
}                                                                       \
\
static inline void vector_##T##_remove_unordered(vector_##T *v, size_t index) { \
    if (index >= v->count) return;                                              \
    v->data[index] = v->data[--(v->count)];                                     \
}                                                                               \
\
static inline void vector_##T##_destroy(vector_##T *v) {    \
    free(v->data);                                          \
    v->data = 0;                                            \
    v->count = 0;                                           \
    v->capacity = 0;                                        \
}                                                           \
\
static inline void vector_##T##_clear(vector_##T *v) {  \
    v->count = 0;                                       \
}                                                       \
\
static inline size_t vector_##T##_find_index(const vector_##T *v, int (*matchfn)(const T)) {    \
    for (size_t i = 0; i < v->count; i++) {                                                     \
        if (matchfn(v->data[i]))                                                                \
            return i;                                                                           \
    }                                                                                           \
    return SIZE_MAX;                                                                            \
}
