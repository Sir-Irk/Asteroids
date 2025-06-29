#ifndef ASTEROIDS_HEADER_GUARD
#define ASTEROIDS_HEADER_GUARD

#include "raylib.h"
#include "raymath.h"
#include "types.h"

#define swap(a, b, type)                                                                                                                   \
    do {                                                                                                                                   \
        type tmp = (a);                                                                                                                    \
        (a)      = (b);                                                                                                                    \
        (b)      = (tmp);                                                                                                                  \
    } while (0)

#define countof(a) (sizeof(a) / sizeof(a[0]))

typedef struct Asteroid {
    Vector2 position;
    Vector2 velocity;
    f32     angular_velocity;
    i32     generation; // 3 generations. 0 = Big asteroid, 1 = Medium, 2 = Small, >=3 = dead
    Vector2 vertices[12];
} Asteroid;

typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    f32     rotation;
    f32     height;
    f32     shootingRateSeconds;
    f32     shootingTimestamp;
    Vector2 vertices[4];
    Vector2 reference_vertices[4];
} Player;

typedef struct Bullet {
    Vector2 prev_position;
    Vector2 position;
    Vector2 velocity;
    f32     radius;
} Bullet;

typedef struct BulletBuffer {
    i32    capacity;
    i32    count;
    Bullet elements[64];
} BulletBuffer;

typedef struct AsteroidBuffer {
    i32      capacity;
    i32      count;
    f32      asteroid_max_scale;
    Asteroid elements[128];
} AsteroidBuffer;
#endif // ASTEROIDS_HEADER_GUARD
