#ifndef GS_TYPES_H
#define GS_TYPES_H

#include <float.h>
#include <stddef.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global static
#define global_variable static

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i32 b32;

typedef float  f32;
typedef double f64;

#define F32_MIN FLT_MIN
#define F32_MAX FLT_MAX

#define F64_MIN DBL_MIN
#define F64_MAX DBL_MAX

typedef ptrdiff_t si_size;

#define si_min(a, b) ((a) < (b) ? (a) : (b))
#define si_max(a, b) ((a) > (b) ? (a) : (b))
#define si_clamp(a, min, max) (si_min(max, si_max(a, min)))

i32
si_mod32(i32 x, i32 n)
{
    return (x % n + n) % n;
}

i64
si_mod64(i64 x, i64 n)
{
    return (x % n + n) % n;
}

#endif // GS_TYPES_H
