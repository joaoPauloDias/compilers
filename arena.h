// Copyright 2022 Alexey Kutepov <reximkut@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef ARENA_NOSTDIO
#include <stdarg.h>
#include <stdio.h>
#endif // ARENA_NOSTDIO

#define ARENA_BACKEND_LIBC_MALLOC 0
#define ARENA_BACKEND_LINUX_MMAP 1
#define ARENA_BACKEND_WIN32_VIRTUALALLOC 2
#define ARENA_BACKEND_WASM_HEAPBASE 3

#ifndef ARENA_BACKEND
#define ARENA_BACKEND ARENA_BACKEND_LIBC_MALLOC
#endif // ARENA_BACKEND

typedef struct Region Region;

struct Region {
    Region *next;
    size_t count;
    size_t capacity;
    uintptr_t data[];
};

typedef struct {
    Region *begin, *end;
} Arena;

typedef struct  {
    Region *region;
    size_t count;
} Arena_Mark;

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (8*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY

Region *new_region(size_t capacity);
void free_region(Region *r);

void *arena_alloc(Arena *a, size_t size_bytes);
void *arena_realloc(Arena *a, void *oldptr, size_t oldsz, size_t newsz);
char *arena_strdup(Arena *a, const char *cstr);
void *arena_memdup(Arena *a, void *data, size_t size);
#ifndef ARENA_NOSTDIO
char *arena_sprintf(Arena *a, const char *format, ...);
char *arena_vsprintf(Arena *a, const char *format, va_list args);
#endif // ARENA_NOSTDIO

Arena_Mark arena_snapshot(Arena *a);
void arena_reset(Arena *a);
void arena_rewind(Arena *a, Arena_Mark m);
void arena_free(Arena *a);
void arena_trim(Arena *a);

#ifndef ARENA_DA_INIT_CAP
#define ARENA_DA_INIT_CAP 256
#endif // ARENA_DA_INIT_CAP

#ifdef __cplusplus
    #define cast_ptr(ptr) (decltype(ptr))
#else
    #define cast_ptr(...)
#endif

#define arena_da_append(a, da, item)                                                          \
    do {                                                                                      \
        if ((da)->count >= (da)->capacity) {                                                  \
            size_t new_capacity = (da)->capacity == 0 ? ARENA_DA_INIT_CAP : (da)->capacity*2; \
            (da)->items = cast_ptr((da)->items)arena_realloc(                                 \
                (a), (da)->items,                                                             \
                (da)->capacity*sizeof(*(da)->items),                                          \
                new_capacity*sizeof(*(da)->items));                                           \
            (da)->capacity = new_capacity;                                                    \
        }                                                                                     \
                                                                                              \
        (da)->items[(da)->count++] = (item);                                                  \
    } while (0)

// Append several items to a dynamic array
#define arena_da_append_many(a, da, new_items, new_items_count)                                       \
    do {                                                                                              \
        if ((da)->count + (new_items_count) > (da)->capacity) {                                       \
            size_t new_capacity = (da)->capacity;                                                     \
            if (new_capacity == 0) new_capacity = ARENA_DA_INIT_CAP;                                  \
            while ((da)->count + (new_items_count) > new_capacity) new_capacity *= 2;                 \
            (da)->items = cast_ptr((da)->items)arena_realloc(                                         \
                (a), (da)->items,                                                                     \
                (da)->capacity*sizeof(*(da)->items),                                                  \
                new_capacity*sizeof(*(da)->items));                                                   \
            (da)->capacity = new_capacity;                                                            \
        }                                                                                             \
        arena_memcpy((da)->items + (da)->count, (new_items), (new_items_count)*sizeof(*(da)->items)); \
        (da)->count += (new_items_count);                                                             \
    } while (0)

// Append a sized buffer to a string builder
#define arena_sb_append_buf arena_da_append_many

// Append a NULL-terminated string to a string builder
#define arena_sb_append_cstr(a, sb, cstr)  \
    do {                                   \
        const char *s = (cstr);            \
        size_t n = arena_strlen(s);        \
        arena_da_append_many(a, sb, s, n); \
    } while (0)

// Append a single NULL character at the end of a string builder. So then you can
// use it a NULL-terminated C string
#define arena_sb_append_null(a, sb) arena_da_append(a, sb, 0)

extern Arena* allocator;
