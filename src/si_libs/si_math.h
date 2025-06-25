
#ifndef SI_MATH_IMPLEMENTATION

// #ifndef SI_MATH_HEADER_GAURD
// #define SI_MATH_HEADER_GAURD

/*copyright 2024 Jeremy Montgomery(sir_irk)
 * use #define SI_MATH_IMPLEMENTATION to get the implementation
 * use #define SI_MATH_NO_CPP to disable cpp only features for C compatability
 */

#ifdef _MSC_VER
#define SI_VECTORCALL __vectorcall
#else
#define SI_VECTORCALL
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>

#include <immintrin.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i32 b32;

typedef float f32;
typedef double f64;

#define si_min(a, b) ((a) < (b) ? (a) : (b))
#define si_max(a, b) ((a) > (b) ? (a) : (b))
#define si_clamp(a, min, max) (si_min(max, si_max(a, min)))

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define si_sqr(a) ((a) * (a))

typedef union si_v2
{
    struct
    {
        f32 x, y;
    };
    f32 v[2];

#ifndef SI_MATH_NO_CPP
    void lerp(si_v2 end, f32 percent);
    f32 sqr_length() const;
    f32 length() const;
#endif
} si_v2;

typedef union
{
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        f32 r, g, b;
    };
    f32 v[3];
} si_v3;

typedef union
{
    struct
    {
        f32 x, y, z, w;
    };
    struct
    {
        f32 r, g, b, a;
    };
    f32 v[4];
} si_v4;

typedef struct
{
    f32 v[4][4];
} si_mat4x4;

typedef struct si_aabb_2d
{
    si_v2 min;
    si_v2 max;

#ifndef SI_MATH_NO_CPP
    b32 contains(si_v2 point) const;
    b32 intersects(si_aabb_2d other) const;
    si_aabb_2d half(si_v2 center, si_v2 dim) const;
#endif
} si_aabb_2d;

#ifdef _MSC_VER
#define SI_INLINE __forceinline
#else
#define SI_INLINE inline __attribute__((always_inline))
#endif

SI_INLINE si_v2
si_v2_add(si_v2 a, si_v2 b);
SI_INLINE si_v2
si_v2_mul(si_v2 v, f32 s);

#else // SI_MATH_IMPLEMENTATION
// From Stephan Brumme https://bits.stephan-brumme.com/absFloat.html
SI_INLINE f32
si_absf(f32 x)
{
    i32 casted = *(i32 *)&x;
    // clear highest bit
    casted &= 0x7FFFFFFF;
    return *(f32 *)&casted;
}

// from https://bits.stephan-brumme.com/inverse.html
// NOTE: approximate. May not be accurate enough for some things
SI_INLINE f32
si_inversef(f32 x)
{
    u32 *i = (u32 *)&x;
    // adjust exponent
    *i = 0x7F000000 - *i;
    return x;
}

SI_INLINE f32
si_radians(f32 degrees)
{
    return degrees * ((f32)M_PI / 180);
}

SI_INLINE f32
si_randf(f32 scalar)
{
    return ((float)rand() / (float)(RAND_MAX)) * scalar;
}
SI_INLINE f32
si_randf_range(f32 min, f32 max)
{
    return min + si_randf(max - min);
}

SI_INLINE f32
si_lerp(f32 start, f32 end, f32 percent)
{
    return start + ((end - start) * percent);
}

SI_INLINE si_v2
si_v2_lerp(si_v2 start, si_v2 end, f32 percent)
{
    si_v2 result = {
        .x = si_lerp(start.x, end.x, percent),
        .y = si_lerp(start.y, end.y, percent),
    };
    return result;
}

SI_INLINE si_v3
si_v3_lerp(si_v3 start, si_v3 end, f32 percent)
{
    si_v3 result = {
        .x = si_lerp(start.x, end.x, percent),
        .y = si_lerp(start.y, end.y, percent),
        .z = si_lerp(start.z, end.z, percent),
    };
    return result;
}

SI_INLINE si_v4
si_v4_lerp(si_v4 start, si_v4 end, f32 percent)
{
    si_v4 result = {
        .x = si_lerp(start.x, end.x, percent),
        .y = si_lerp(start.y, end.y, percent),
        .z = si_lerp(start.z, end.z, percent),
        .w = si_lerp(start.w, end.w, percent),
    };
    return result;
}

SI_INLINE si_v4
si_v4_new(f32 x, f32 y, f32 z, f32 w)
{
    return (si_v4){x, y, z, w};
}

#ifndef SI_MATH_NO_CPP
SI_INLINE void
si_v2::lerp(si_v2 end, f32 percent)
{
    *this = si_v2_lerp(*this, end, percent);
}

#endif

SI_INLINE si_v3
new_si_v3(f32 x, f32 y, f32 z)
{
    si_v3 result = {x, y, z};
    return result;
}

SI_INLINE si_v3
new_si_v3_0()
{
    si_v3 result = {0};
    return result;
}

SI_INLINE si_mat4x4
si_mat4x4_identity()
{
    si_mat4x4 result = {0};
    result.v[0][0] = 1.0f;
    result.v[1][1] = 1.0f;
    result.v[2][2] = 1.0f;
    result.v[3][3] = 1.0f;
    return result;
}

SI_INLINE si_mat4x4
si_mat4x4_translate(si_mat4x4 m, si_v3 v)
{
    m.v[3][0] = v.x;
    m.v[3][1] = v.y;
    m.v[3][2] = v.z;
    return m;
}

SI_INLINE si_mat4x4
si_mat4x4_translate_2d(si_mat4x4 m, si_v2 v, f32 z)
{
    m.v[3][0] = v.x;
    m.v[3][1] = v.y;
    m.v[3][2] = z;
    return m;
}

SI_INLINE si_mat4x4
si_mat4x4_scale_f(float scale)
{
    si_mat4x4 result = si_mat4x4_identity();
    result.v[0][0] = scale;
    result.v[1][1] = scale;
    result.v[2][2] = scale;
    return result;
}

SI_INLINE si_mat4x4
si_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearDist, f32 farDist)
{
    si_mat4x4 result = {0};

    result.v[0][0] = 2.0f / (right - left);
    result.v[1][1] = 2.0f / (top - bottom);
    result.v[2][2] = 2.0f / (nearDist - farDist);

    result.v[3][0] = (left + right) / (left - right);
    result.v[3][1] = (bottom + top) / (bottom - top);
    result.v[3][2] = (farDist + nearDist) / (nearDist - farDist);

    result.v[3][3] = 1.0f;

    return result;
}

SI_INLINE si_mat4x4
si_perspective(f32 fov, f32 aspectRatio, f32 nearDist, f32 farDist)
{
    si_mat4x4 result = si_mat4x4_identity();
    f32 tanThetaOver2 = tanf(fov * ((f32)M_PI / 360.0f));

    result.v[0][0] = 1.0f / tanThetaOver2;
    result.v[1][1] = (aspectRatio / tanThetaOver2);
    result.v[2][3] = -1.0f;
    result.v[2][2] = (nearDist + farDist) / (nearDist - farDist);
    result.v[3][2] = (2.0f * nearDist * farDist) / (nearDist - farDist);
    result.v[3][3] = 0.0f;

    return result;
}

SI_INLINE si_mat4x4
si_mat4x4_mul(si_mat4x4 a, si_mat4x4 b)
{
    si_mat4x4 result = {0};
    for (i32 x = 0; x < 4; ++x) {
        for (i32 y = 0; y < 4; ++y) {
            f32 sum = 0;
            for (i32 i = 0; i < 4; ++i) {
                sum += a.v[i][y] * b.v[x][i];
            }
            result.v[x][y] = sum;
        }
    }
    return result;
}

SI_INLINE si_mat4x4
si_mat4x4_mul3(si_mat4x4 a, si_mat4x4 b, si_mat4x4 c)
{
    si_mat4x4 result = si_mat4x4_mul(a, b);
    result = si_mat4x4_mul(result, c);
    return result;
}

SI_INLINE si_v3
si_v3_invert(si_v3 a)
{
    si_v3 result = {-a.x, -a.y, -a.z};
    return result;
}

SI_INLINE si_v3
si_v3_add(si_v3 a, si_v3 b)
{
    si_v3 result = {a.x + b.x, a.y + b.y, b.z + b.z};
    return result;
}

SI_INLINE si_v3
si_v3_sub(si_v3 a, si_v3 b)
{
    si_v3 result = {a.x - b.x, a.y - b.y, b.z - b.z};
    return result;
}

SI_INLINE si_v3
si_v3_mul(si_v3 v, float s)
{
    si_v3 result = {v.x * s, v.y * s, v.z * s};
    return result;
}

SI_INLINE float
si_v3_sqr_len(si_v3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

SI_INLINE float
si_v3_len(si_v3 v)
{
    return sqrtf(si_v3_sqr_len(v));
}

SI_INLINE si_v3
si_v3_normalized(si_v3 v)
{
    float len = si_v3_len(v);
    if (len == 0.0f) {
        return v;
    }
    float invLen = 1.0f / len;
    v.x *= invLen;
    v.y *= invLen;
    v.z *= invLen;
    return v;
}

SI_INLINE si_v3
si_v3_cross(si_v3 a, si_v3 b)
{
    si_v3 result = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    return result;
}

SI_INLINE float
si_v3_dot(si_v3 a, si_v3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

SI_INLINE si_v2
si_v2_new(float x, float y)
{
    si_v2 result = {x, y};
    return result;
}

SI_INLINE si_v2
si_v2_add(si_v2 a, si_v2 b)
{
    si_v2 result = {a.x + b.x, a.y + b.y};
    return result;
}

#ifndef SI_MATH_NO_CPP

inline si_v2
operator+(si_v2 a, si_v2 b)
{
    return {a.x + b.x, a.y + b.y};
}

inline si_v2
operator-(si_v2 a, si_v2 b)
{
    return {a.x - b.x, a.y - b.y};
}

inline si_v2
operator*(si_v2 a, f32 s)
{
    return {a.x * s, a.y * s};
}

inline si_v2 &
operator+=(si_v2 &a, si_v2 b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline si_v2 &
operator-=(si_v2 &a, si_v2 b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

inline si_v2 &
operator*=(si_v2 &a, f32 s)
{
    a.x *= s;
    a.y *= s;
    return a;
}

inline b32
operator==(si_v2 a, si_v2 b)
{
    return a.x == b.x && a.y == b.y;
}

inline si_v3
operator+(si_v3 a, si_v3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline si_v3
operator-(si_v3 a, si_v3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline si_v3
operator*(si_v3 a, f32 s)
{
    return {a.x * s, a.y * s, a.z * s};
}

inline si_v3 &
operator+=(si_v3 &a, si_v3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

inline si_v3 &
operator-=(si_v3 &a, si_v3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

inline si_v3 &
operator*=(si_v3 &a, f32 s)
{
    a.x *= s;
    a.y *= s;
    a.z *= s;
    return a;
}

inline b32
operator==(si_v3 a, si_v3 b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline si_v4
operator+(si_v4 a, si_v4 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
inline si_v4
operator-(si_v4 a, si_v4 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

inline si_v4
operator*(si_v4 a, f32 s)
{
    return {a.x * s, a.y * s, a.z * s, a.w};
}

inline si_v4 &
operator+=(si_v4 &a, si_v4 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

inline si_v4 &
operator-=(si_v4 &a, si_v4 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
    return a;
}

inline si_v4 &
operator*=(si_v4 &a, f32 s)
{
    a.x *= s;
    a.y *= s;
    a.z *= s;
    a.w *= s;
    return a;
}

inline b32
operator==(si_v4 a, si_v4 b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

#endif // OPERATOR_OVERLOAD

SI_INLINE si_v2
si_v2_sub(si_v2 a, si_v2 b)
{
    si_v2 result = {a.x - b.x, a.y - b.y};
    return result;
}

SI_INLINE si_v2
si_v2_mul(si_v2 v, float s)
{
    v.x *= s;
    v.y *= s;
    return v;
}

SI_INLINE float
si_v2_dot(si_v2 a, si_v2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

SI_INLINE si_v2
si_v2_reflect(si_v2 a, si_v2 normal)
{
    si_v2 dir = si_v2_mul(normal, si_v2_dot(a, normal) * -2.0f);
    return si_v2_add(a, dir);
    // return a + (normal * (-2 * si_v2_dot(a, normal)));
}

SI_INLINE float
si_v2_sqr_len(si_v2 v)
{
    return v.x * v.x + v.y * v.y;
}

SI_INLINE float
si_v2_len(si_v2 v)
{
    return sqrtf(si_v2_sqr_len(v));
}

#ifndef SI_MATH_NO_CPP
SI_INLINE f32
si_v2::sqr_length() const
{
    return si_v2_sqr_len(*this);
}

SI_INLINE f32
si_v2::length() const
{
    return sqrtf(this->sqr_length());
}
#endif

SI_INLINE si_v2
si_v2_normalized(si_v2 v)
{
    float len = si_v2_len(v);
    if (len == 0.0f) {
        return v;
    }
    float invLen = 1.0f / len;
    v.x *= invLen;
    v.y *= invLen;
    return v;
}

SI_INLINE si_mat4x4
si_look_at(si_v3 position, si_v3 target, si_v3 up)
{
    si_v3 f = si_v3_normalized(si_v3_sub(target, position));
    si_v3 r = si_v3_normalized(si_v3_cross(f, si_v3_normalized(up)));
    si_v3 u = si_v3_cross(r, f);

    si_mat4x4 result;

    result.v[0][0] = r.x;
    result.v[0][1] = u.x;
    result.v[0][2] = -f.x;
    result.v[0][3] = 0.0f;

    result.v[1][0] = r.y;
    result.v[1][1] = u.y;
    result.v[1][2] = -f.y;
    result.v[1][3] = 0.0f;

    result.v[2][0] = r.z;
    result.v[2][1] = u.z;
    result.v[2][2] = -f.z;
    result.v[2][3] = 0.0f;

    result.v[3][0] = -si_v3_dot(r, position);
    result.v[3][1] = -si_v3_dot(u, position);
    result.v[3][2] = si_v3_dot(f, position);
    result.v[3][3] = 1.0f;

    return result;
}

SI_INLINE si_mat4x4
si_mat4x4_rot(si_mat4x4 mat, f32 radians, si_v3 axis)
{
    axis = si_v3_normalized(axis);
    f32 sinT = sinf(radians);
    f32 cosT = cosf(radians);
    f32 cos = 1.0f - cosT;

    mat.v[0][0] = (axis.x * axis.x * cos) + cosT;
    mat.v[0][1] = (axis.x * axis.y * cos) + (axis.z * sinT);
    mat.v[0][2] = (axis.x * axis.z * cos) - (axis.y * sinT);

    mat.v[1][0] = (axis.y * axis.x * cos) - (axis.z * sinT);
    mat.v[1][1] = (axis.y * axis.y * cos) + cosT;
    mat.v[1][2] = (axis.y * axis.z * cos) + (axis.x * sinT);

    mat.v[2][0] = (axis.z * axis.x * cos) + (axis.y * sinT);
    mat.v[2][1] = (axis.z * axis.y * cos) - (axis.x * sinT);
    mat.v[2][2] = (axis.z * axis.z * cos) + cosT;

    return mat;
}

#if 0
//=======================================================================================
// si_aabb_2d impl
//=======================================================================================
SI_INLINE si_aabb_2d
si_aabb_2d_new(f32 minX, f32 minY, f32 maxX, f32 maxY)
{
    return (si_aabb_2d){{minX, minY}, {maxX, maxY}};
}

SI_INLINE si_aabb_2d
si_aabb_2d_new(si_v2 min, si_v2 max)
{
    si_aabb_2d result = (si_aabb_2d){min, max};
    return result;
}

SI_INLINE si_aabb_2d
operator+(si_aabb_2d a, si_v2 b)
{
    si_aabb_2d result = a;
    result.min = b + a.min;
    result.max = b + a.max;
    return result;
}

SI_INLINE si_aabb_2d
operator+(si_aabb_2d a, si_aabb_2d b)
{
    si_aabb_2d result = a;
    result.min += b.min;
    result.max += b.max;
    return result;
}

SI_INLINE si_aabb_2d
operator*(si_aabb_2d a, f32 b)
{
    si_aabb_2d result;
    result.min = a.min * b;
    result.max = a.max * b;
    return result;
}

SI_INLINE si_aabb_2d
si_aabb_2d_half(si_v2 center, si_v2 dim)
{
    si_aabb_2d result;
    result.min = center - dim * 0.5f;
    result.max = center + dim * 0.5f;
    return result;
}

SI_INLINE b32
si_aabb_2d_contains(si_aabb_2d bounds, si_v2 point)
{
    return (point.x <= bounds.max.x && point.y <= bounds.max.y && point.x >= bounds.min.x && point.y >= bounds.min.y);
}

SI_INLINE b32
si_aabb_2d_contains(si_aabb_2d a, si_aabb_2d b)
{
    return (b.min.x >= a.min.x && b.min.y >= a.min.y && b.max.x <= a.max.x && b.max.y <= a.max.y);
}
SI_INLINE b32
si_aabb_2d_intersects(si_aabb_2d a, si_aabb_2d b)
{
    for (i32 i = 0; i < 2; i++) {
        if (a.min.v[i] > b.max.v[i])
            return false;
        if (a.max.v[i] < b.min.v[i])
            return false;
    }
    return true;
}

SI_INLINE b32
si_aabb_2d_clip_line(i32 d, si_aabb_2d aabb, si_v2 v0, si_v2 v1, f32 *out_low, f32 high)
{
    f32 low = *out_low;
    f32 dim_low = (aabb.min.v[d] - v0.v[d]) / (v1.v[d] - v0.v[d]);
    f32 dim_high = (aabb.max.v[d] - v0.v[d]) / (v1.v[d] - v0.v[d]);
    if (dim_high < dim_low)
        std::swap(dim_high, dim_low);
    if (dim_high < low)
        return false;
    if (dim_low > high)
        return false;

    low = max(dim_low, low);
    high = min(dim_high, high);

    if (low > high)
        return false;
    *out_low = low;
    return true;
}

SI_INLINE b32
si_aabb_2d__line_intersection(si_aabb_2d aabb, si_v2 v0, si_v2 v1, si_v2 *out_intersection, f32 *out_fraction)
{
    f32 low = 0;
    f32 high = 1;

    if (!si_aabb_2d_clip_line(0, aabb, v0, v1, &low, high))
        return false;
    if (!si_aabb_2d_clip_line(1, aabb, v0, v1, &low, high))
        return false;

    si_v2 b = v1 - v0;
    *out_intersection = v0 + b * low;
    *out_fraction = low;
    return true;
}

SI_INLINE b32
si_aabb_2d_trace_line(si_aabb_2d box, si_v2 box_position, si_v2 v0, si_v2 v1, si_v2 *intersection)
{
    f32 lowest_fraction = 1.0f;
    f32 test_fraction;
    si_v2 test_inter;

    if (si_aabb_2d__line_intersection((box + box_position), v0, v1, &test_inter, &test_fraction) && test_fraction < lowest_fraction) {
        *intersection = test_inter;
        lowest_fraction = test_fraction;
    }

    if (lowest_fraction < 1)
        return true;

    return false;
}

#define SI_IS_NEAR_REAL32(x, y) (fabsf((x) - (y)) < 0.00000001f)
#define SI_IS_NEAR_ROUGH_REAL32(x, y) (fabsf((x) - (y)) < 1.0f)

SI_INLINE si_v2
si_aabb_2d_get_normal_from_intersection(si_aabb_2d bounds, si_v2 world_position, si_v2 intersection)
{
    si_v2 normal = intersection - world_position;
    if (!SI_IS_NEAR_ROUGH_REAL32(normal.x, bounds.min.x) && !SI_IS_NEAR_ROUGH_REAL32(normal.x, bounds.max.x))
        normal.x = 0;
    if (!SI_IS_NEAR_ROUGH_REAL32(normal.y, bounds.min.y) && !SI_IS_NEAR_ROUGH_REAL32(normal.y, bounds.max.y))
        normal.y = 0;

#if 1
    if (normal.x > bounds.min.x && normal.x < bounds.max.x) {
        if (SI_IS_NEAR_ROUGH_REAL32(normal.y, bounds.min.y))
            normal.y = -1;
        else
            normal.y = 1;
        normal.x = 0;
    } else {
        if (SI_IS_NEAR_ROUGH_REAL32(normal.x, bounds.min.x))
            normal.x = -1;
        else
            normal.x = 1;
        normal.y = 0;
    }
#endif

    return si_v2_normalized(normal);
}

#ifndef SI_MATH_NO_CPP
SI_INLINE b32
si_aabb_2d::contains(si_v2 point) const
{
    return si_aabb_2d_contains(*this, point);
}

SI_INLINE b32
si_aabb_2d::intersects(si_aabb_2d other) const
{
    return si_aabb_2d_intersects(*this, other);
}

SI_INLINE si_aabb_2d
si_aabb_2d::half(si_v2 center, si_v2 dim) const
{
    return si_aabb_2d_half(center, dim);
}
#endif
#endif

SI_INLINE __m256 SI_VECTORCALL
si_v2_avx_sqr_len(__m256 x, __m256 y)
{
    return _mm256_add_ps(_mm256_mul_ps(x, x), _mm256_mul_ps(y, y));
}

SI_INLINE __m256 SI_VECTORCALL
si_v2_avx_len(__m256 x, __m256 y)
{
    return _mm256_sqrt_ps(si_v2_avx_sqr_len(x, y));
}

SI_INLINE __m256 SI_VECTORCALL
si_avx_nr_sqrt(__m256 x)
{
    __m256 nr = _mm256_rsqrt_ps(x);
    __m256 xnr = _mm256_mul_ps(x, nr);
    __m256 hnr = _mm256_mul_ps(_mm256_set1_ps(0.5f), nr);
    __m256 result = _mm256_fnmadd_ps(xnr, nr, _mm256_set1_ps(3.0f));
    result = _mm256_mul_ps(hnr, result);
    return result;
}

SI_INLINE __m256 SI_VECTORCALL
si_v2_avx_nr_sqrt(__m256 x, __m256 y)
{
    return si_avx_nr_sqrt(_mm256_add_ps(_mm256_mul_ps(x, x), _mm256_mul_ps(y, y)));
}
#endif // SI_MATH_IMPLEMENTATION
// #endif // SI_MATH_HEADER_GAURD
