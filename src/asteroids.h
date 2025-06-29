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

#define si_min(a, b) (a) < (b) ? (a) : (b);
#define si_max(a, b) (a) > (b) ? (a) : (b);

#define POINTS_PER_ASTEROID 150

typedef struct Asteroid {
    Vector2 position;
    Vector2 velocity;
    f32     angular_velocity;
    i32     generation; // 3 generations. 0 = Big asteroid, 1 = Medium, 2 = Small, >=3 = dead
    Vector2 vertices[12];
} Asteroid;

enum PowerUpType {
    POWER_UP_TYPE_BOUNCEY_BULLETS,
    POWER_UP_TYPE_INVINCIBILITY,
    POWER_UP_TYPE_SHOTGUN,
    POWER_UP_TYPE_MACHINE_GUN,
    POWER_UP_TYPE_COUNT,
};

#define POWER_UP_RADIUS 80.0f
#define POWER_UP_DURATION 10.0f
#define POWER_UP_MIN_SPAWN_RATE 1.0f
#define POWER_UP_MAX_SPAWN_RATE 2.0f

#define PLAYER_SHOOTING_RATE 0.25f

typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    i32     score;
    f32     rotation;
    f32     height;
    f32     shooting_rate;
    f32     shooting_timestamp;

    u32 power_up_flags;
    f32 power_up_timestamps[POWER_UP_TYPE_COUNT];

    Vector2 vertices[4];
    Vector2 reference_vertices[4];
} Player;

typedef struct PowerUp {
    enum PowerUpType type;
    f32              time_spawned;
    Vector2          position;
} PowerUp;

typedef struct PowerUpBuffer {
    i32     capacity;
    i32     count;
    PowerUp elements[4];
} PowerUpBuffer;

typedef struct Bullet {
    Vector2 prev_position;
    Vector2 position;
    Vector2 velocity;
    f32     radius;
} Bullet;

typedef struct BulletBuffer {
    i32    capacity;
    i32    count;
    Bullet elements[128];
} BulletBuffer;

typedef struct AsteroidBuffer {
    i32      capacity;
    i32      count;
    f32      asteroid_max_scale;
    Asteroid elements[128];
} AsteroidBuffer;
#endif // ASTEROIDS_HEADER_GUARD
