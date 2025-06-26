#ifndef SI_MEMORY_HEADER_GAURD
#define SI_MEMORY_HEADER_GAURD

/*
#define SI_MEMORY_IMPLEMENTATION
*/

#include <assert.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
#else
#include <sys/mman.h>
#endif

#define si_swap(a, b, type)                                                                                                                \
    do {                                                                                                                                   \
        type tmp = (a);                                                                                                                    \
        (a)      = (b);                                                                                                                    \
        (b)      = (tmp);                                                                                                                  \
    } while (0)

typedef ptrdiff_t si_size;

typedef struct si_primary_buffer
{
    si_size size;
    void   *data;
    int     is_stack_buffer;
} si_primary_buffer;

typedef struct si_memory_arena
{
    si_size  size;
    si_size  used;
    uint8_t *base;
} si_memory_arena;

typedef struct si_temp_memory
{
    si_memory_arena *arena;
    size_t           used;
} si_temp_memory;

#define si_kilobytes(value) ((value) * 1024LL)
#define si_megabytes(value) (si_kilobytes(value) * 1024LL)
#define si_gigabytes(value) (si_megabytes(value) * 1024LL)
#define si_terabytes(value) (si_gigabytes(value) * 1024LL)

#define si_push(arena, type) (type *)si__push_size(arena, sizeof(type), 0)
#define si_push_array(arena, count, type) (type *)si__push_size(arena, (count) * sizeof(type), 0)
#define si_push_size(arena, size) si__push_size(arena, (size), 0)

#define si_push_clear(arena, type) (type *)si__push_size(arena, sizeof(type), 1)
#define si_push_array_clear(arena, count, type) (type *)si__push_size(arena, (count) * sizeof(type), 1)
#define si_push_size_clear(arena, size) si__push_size(arena, (size), 1)

#define si_push_aligned(arena, type, alignment) (type *)si__push_size_aligned(arena, sizeof(type), alignment)
#define si_push_array_aligned(arena, count, type, alignment) (type *)si__push_size_aligned(arena, (count) * sizeof(type), alignment)
#define si_push_size_aligned(arena, size, alignment) si__push_size_aligned(arena, (size), alignment)

static void *
si__push_size(si_memory_arena *arena, si_size size, int clear);

static si_temp_memory
si_start_temp_memory(si_memory_arena *arena);

static void
si_pop_temp_memory(si_temp_memory temp);

static void
si_pop_and_clear_temp_memory(si_temp_memory temp);

static si_primary_buffer
si_primary_buffer_stack(size_t sizeInBytes, void *buffer);

#define si_array_count(a) (sizeof(a) / sizeof(a[0]))

#ifdef SI_MEMORY_IMPLEMENTATION

#ifdef _WIN32
#include <memoryapi.h>
static si_primary_buffer
si_allocate_primary_buffer(si_size sizeInBytes, void *baseAddress)
{
    si_primary_buffer result = {0};
    result.data              = VirtualAlloc(baseAddress, sizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    // result.data = malloc(sizeInBytes);
    assert(result.data);
    memset(result.data, 0, sizeInBytes);
    result.size = sizeInBytes;
    return result;
}

static void
si_free(si_primary_buffer *buffer)
{
    assert(buffer);
    assert(buffer->data);
    if (buffer->is_stack_buffer) return;
    // NOTE: requires windows.h
    VirtualFree(buffer->data, 0, MEM_RELEASE);
    // free(buffer);
}
#else

static si_primary_buffer
si_primary_buffer_stack(size_t sizeInBytes, void *buffer)
{
    assert(buffer);
    assert(sizeInBytes > 0);
    si_primary_buffer result = {};
    result.data              = buffer;
    result.size              = sizeInBytes;
    result.is_stack_buffer   = 1;
    return result;
}

static si_primary_buffer
si_allocate_primary_buffer(size_t sizeInBytes, void *baseAddress)
{
    assert(sizeInBytes > 0);
    si_primary_buffer result = {};
    result.data              = mmap(baseAddress, sizeInBytes, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    assert(result.data);
    result.size = sizeInBytes;
    return result;
}

static void
si_free(si_primary_buffer *buffer)
{
    assert(buffer);
    assert(buffer->data);
    if (buffer->is_stack_buffer) return;

    munmap(buffer->data, buffer->size);
    memset(buffer, 0, sizeof(*buffer));
}

#endif //_WIN32
//
static si_memory_arena
si_make_arena(si_primary_buffer *buffer, si_size size)
{
    assert(size <= buffer->size);

    si_memory_arena result = {
        .size = size,
        .used = 0,
        .base = (uint8_t *)buffer->data,
    };

    return result;
}

static si_memory_arena
si_make_arena_segment(si_primary_buffer *buffer, si_size size, si_size offset)
{
    assert(size <= buffer->size);

    si_memory_arena result = {
        .size = size,
        .used = 0,
        .base = (uint8_t *)buffer->data + offset,
    };

    return result;
}

static void
si_initialize_arena(si_memory_arena *arena, si_size size, void *base)
{
    arena->size = size;
    arena->base = (uint8_t *)base;
    arena->used = 0;
}

// TODO: remove stdlib
#include <string.h>
static void *
si__push_size(si_memory_arena *arena, si_size size, int clear)
{
    assert((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    if (clear) {
        memset(result, 0, size);
    }
    arena->used += size;
    return result;
}

static void *
si_align(void *ptr, int32_t alignment)
{
    int32_t a      = alignment - 1;
    void   *result = (void *)(((uintptr_t)(ptr) + a) & ~(uintptr_t)a);
    assert(((uintptr_t)result & a) == 0);
    return result;
}

// TODO: add versions of push that clears the memory to zero
static void *
si__push_size_aligned(si_memory_arena *arena, size_t size, int32_t alignment)
{
    void     *unaligned = arena->base + arena->used;
    void     *result    = si_align(unaligned, alignment);
    ptrdiff_t diff      = (uint8_t *)result - (uint8_t *)unaligned;
    assert(diff >= 0);
    size += diff;
    arena->used += size;
    assert(arena->used <= arena->size);
    return result;
}

static void
si_clear_arena(si_memory_arena *arena, int clearToZero)
{
    if (clearToZero) {
        memset(arena->base, 0, arena->size);
    }
    arena->used = 0;
}

static si_temp_memory
si_start_temp_memory(si_memory_arena *arena)
{
    si_temp_memory result = {0};
    assert(arena);
    result.arena = arena;
    result.used  = arena->used;
    return result;
}

static void
si_pop_temp_memory(si_temp_memory temp)
{
    assert(temp.arena);
    temp.arena->used = temp.used;
}

static void
si_pop_and_clear_temp_memory(si_temp_memory temp)
{
    assert(temp.arena);
    ptrdiff_t bytesToClear = temp.arena->used - temp.used;
    assert(bytesToClear >= 0);
    memset((temp.arena->base + temp.used), 0, bytesToClear);
    temp.arena->used = temp.used;
}

#endif // SI_MEMORY_IMPLEMENTATION

#endif // SI_MEMORY_HEADER_GAURD
