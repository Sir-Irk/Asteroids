#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "raylib.h"

#include <math.h>
#include <stdint.h>

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
    Vector2 vertices[4];
} Player;

typedef struct Bullet {
    Vector2 prev_position;
    Vector2 position;
    Vector2 velocity;
    f32     radius;
    f32     time_spawned;
    // Vector2 vertices[4];
} Bullet;

typedef struct BulletBuffer {
    i32    capacity;
    i32    count;
    Bullet elements[64];
} BulletBuffer;

typedef struct AsteroidBuffer {
    i32      capacity;
    i32      count;
    Asteroid elements[64];
} AsteroidBuffer;

Bullet *PushBullet(BulletBuffer *array, Bullet bullet)
{
    if (array->count + 1 > array->capacity) {
        return NULL;
    }

    array->elements[array->count] = bullet;
    return &array->elements[array->count++];
}

void RemoveBullet(BulletBuffer *array, i32 index)
{
    assert(index >= 0);
    assert(index < array->count);

    swap(array->elements[index], array->elements[array->count - 1], Bullet);
    array->count--;
}

Asteroid *PushAsteroid(AsteroidBuffer *array, Asteroid asteroid)
{
    if (array->count + 1 > array->capacity) {
        return NULL;
    }

    array->elements[array->count] = asteroid;
    return &array->elements[array->count++];
}

void RemoveAsteroid(AsteroidBuffer *array, i32 index)
{
    assert(index >= 0);
    assert(index < array->count);

    swap(array->elements[index], array->elements[array->count - 1], Asteroid);
    array->count--;
}

f32 GetRandomFloat01() { return (f32)GetRandomValue(0, INT_MAX) / (f32)INT_MAX; }

f32 GetRandomFloatRange(f32 min, f32 max) { return min + (max - min) * GetRandomFloat01(); }

Vector2 GetRandomVector2UnitCircle(f32 scale)
{
    f32 x = (GetRandomFloat01() * 2.0f - 1.0f);
    f32 y = (GetRandomFloat01() * 2.0f - 1.0f);
    return Vector2Scale(Vector2Normalize((Vector2){x, y}), scale);
}

b32 CheckCollionPlayerLine(Player *player, Vector2 p0, Vector2 p1)
{
    for (i32 pv = 0; pv < countof(player->vertices); ++pv) {
        i32     pv_next = (pv + 1) % countof(player->vertices);
        Vector2 vert0   = Vector2Add(player->vertices[pv], player->position);
        Vector2 vert1   = Vector2Add(player->vertices[pv_next], player->position);
        Vector2 collision;
        if (CheckCollisionLines(p0, p1, vert0, vert1, &collision)) {
            return true;
        }
    }
    return false;
}

i32 CheckCollisionBulletLine(BulletBuffer *bullets, Vector2 p0, Vector2 p1)
{
    for (i32 b = 0; b < bullets->count; ++b) {
        Bullet *bullet = &bullets->elements[b];

        Vector2 delta   = Vector2Normalize(Vector2Subtract(bullet->position, bullet->prev_position));
        Vector2 new_pos = Vector2Add(bullet->position, Vector2Scale(delta, bullet->radius));

        Vector2 collision;
        if (CheckCollisionLines(p0, p1, bullet->prev_position, new_pos, &collision) ||
            CheckCollisionCircleLine(bullet->position, bullet->radius, p0, p1)) {
            return b;
        }
    }
    return -1;
}

void UpdateAsteroidPositions(AsteroidBuffer *asteroid_buffer, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 dt)
{
    for (i32 i = 0; i < asteroid_buffer->count; ++i) {
        Asteroid *a = &asteroid_buffer->elements[i];
        a->position.x += a->velocity.x * dt;
        a->position.y += a->velocity.y * dt;

        if (a->position.x > max_x) {
            a->position.x -= max_x;
        } else if (a->position.x < min_x) {
            a->position.x += max_x;
        }

        if (a->position.y > max_y) {
            a->position.y -= max_y;
        } else if (a->position.y < min_y) {
            a->position.y += max_y;
        }

        Matrix mat = MatrixRotateZ(a->angular_velocity * dt);
        for (i32 v = 0; v < countof(a->vertices); ++v) {
            a->vertices[v] = Vector2Transform(a->vertices[v], mat);
        }
    }
}

Asteroid CreateAsteroid(Vector2 position, Vector2 velocity, f32 scale, i32 generation)
{
    Asteroid asteroid = {};
    Vector2  average  = {};
    i32      count    = countof(asteroid.vertices);
    f32      step     = 2 * PI / count;

    for (i32 i = 0; i < count; ++i) {
        f32 angle = (i + 1) * step;
        f32 x     = cosf(angle);
        f32 y     = sinf(angle);
        f32 s     = scale * GetRandomFloatRange(0.5f, 1.0f);

        Vector2 vertex       = Vector2Scale(Vector2Normalize((Vector2){x, y}), s);
        asteroid.vertices[i] = vertex;
        average              = Vector2Add(average, vertex);
    }

    average = Vector2Scale(average, 1.0f / countof(asteroid.vertices));

    for (i32 i = 0; i < countof(asteroid.vertices); ++i) {
        asteroid.vertices[i] = Vector2Subtract(asteroid.vertices[i], average);
    }

    asteroid.position         = position;
    asteroid.velocity         = velocity;
    asteroid.generation       = generation;
    asteroid.angular_velocity = GetRandomFloatRange(-2.0f, 2.0f);

    return asteroid;
}

void UpdateAndDraw() { }

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    // InitWindow(2560 / 4, 1440 / 4, "Asteroids");
    InitWindow(1920, 1080, "Asteroids");
    InitAudioDevice();

    SetTargetFPS(GetMonitorRefreshRate(0));
    SetRandomSeed(time(NULL));

    Sound shoot_sound     = LoadSound("sounds/shoot.wav");
    Sound explosion_sound = LoadSound("sounds/explosion.wav");

    SetSoundVolume(explosion_sound, 0.7f);
    f32    player_width  = 48.0f;
    f32    player_height = 64.0f;
    Player player        = {};

    Vector2 player_vert_ref[countof(player.vertices)] = {
        {0.0f, -player_height / 2.0f},
        {player_width / 2.0f, player_height / 2.0f},
        {0.0f, player_height / 8.0f},
        {-player_width / 2.0f, player_height / 2.0f},
    };

    i32 screen_width  = GetScreenWidth();
    i32 screen_height = GetScreenHeight();

    Color clear_color = ColorLerp(BLACK, WHITE, 0.0f);

    Vector2 world_max = {2560.0f, 1440.0f};

    // NOTE: assumes aspect ratio will not change
    Camera2D camera = {};
    camera.zoom     = screen_width / 2560.0f;
    // camera.offset   = (Vector2){screen_width / 2.0f * (1.0f - camera.zoom), screen_height / 2.0f * (1.0f - camera.zoom)};

    f32            asteroid_scale = 128.0f;
    AsteroidBuffer asteroid_array = {
        .count    = 0,
        .elements = {0},
        .capacity = countof(asteroid_array.elements),
    };

    Asteroid *asteroids = asteroid_array.elements;

    player.position    = (Vector2){world_max.x / 2.0f, world_max.y / 2.0f};
    player.vertices[0] = player_vert_ref[0];
    player.vertices[1] = player_vert_ref[1];
    player.vertices[2] = player_vert_ref[2];

    BulletBuffer bullet_array = {
        .count    = 0,
        .elements = {0},
        .capacity = countof(bullet_array.elements),
    };

    for (i32 i = 0; i < 12; ++i) {
        Vector2 screen_center = (Vector2){(f32)world_max.x / 2, (f32)world_max.y / 2};
        Vector2 position      = Vector2Add(screen_center, GetRandomVector2UnitCircle(GetRandomFloatRange(600.0f, world_max.x)));
        Vector2 velocity      = GetRandomVector2UnitCircle(GetRandomFloatRange(50.0f, 250.0f));
        PushAsteroid(&asteroid_array, CreateAsteroid(position, velocity, asteroid_scale, 0));
    }

    b32 game_over = false;

    while (!WindowShouldClose()) {
        screen_width  = GetScreenWidth();
        screen_height = GetScreenHeight();

        Vector2 mouse_pos = Vector2Scale(GetMousePosition(), 1.0f / camera.zoom);
        f32     dt        = GetFrameTime();

        if (!game_over) {
            for (i32 i = 0; i < bullet_array.count; ++i) {
                Bullet *b = &bullet_array.elements[i];
                if (GetTime() - b->time_spawned >= 5.0f ||
                    (b->position.y > world_max.y || b->position.y < 0.0f || b->position.x > world_max.x || b->position.x < 0.0f)) {
                    RemoveBullet(&bullet_array, i--);
                }
            }

            f32    angle = Vector2Angle((Vector2){0.0f, -1.0f}, Vector2Subtract(mouse_pos, player.position));
            Matrix rot   = MatrixRotateZ(angle);
            for (i32 i = 0; i < countof(player.vertices); ++i) {
                player.vertices[i] = Vector2Transform(player_vert_ref[i], rot);
            }

            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(0)) {
                SetSoundPitch(shoot_sound, GetRandomFloatRange(0.95f, 1.05f));
                PlaySound(shoot_sound);
                Vector2 direction = Vector2Normalize(Vector2Subtract(mouse_pos, player.position));
                Vector2 pos       = Vector2Add(player.position, Vector2Scale(direction, player_height / 2.0f));
                Bullet  bullet    = {pos, pos, Vector2Scale(direction, 900.0f), 8.0f, GetTime()};
                PushBullet(&bullet_array, bullet);
            }

            Vector2 direction = {};
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction = Vector2Add(direction, (Vector2){0.0f, -1.0f});
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction = Vector2Add(direction, (Vector2){0.0f, 1.0f});
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction = Vector2Add(direction, (Vector2){-1.0f, 0.0f});
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction = Vector2Add(direction, (Vector2){1.0f, 0.0f});
            direction       = Vector2Normalize(direction);
            player.velocity = Vector2Add(player.velocity, Vector2Scale(direction, 30.0f * dt));

            f32 max_magnitude = 400.0f * dt;
            if (Vector2Length(player.velocity) > max_magnitude) {
                player.velocity = Vector2Scale(Vector2Normalize(player.velocity), max_magnitude);
            }

            player.position = Vector2Add(player.position, player.velocity);

            f32 drag = expf(-3.00f * dt);
            player.velocity.x *= drag;
            player.velocity.y *= drag;

            for (i32 i = 0; i < bullet_array.count; ++i) {
                Bullet *b        = &bullet_array.elements[i];
                b->prev_position = b->position;
                b->position      = Vector2Add(b->position, Vector2Scale(b->velocity, dt));
            }

            UpdateAsteroidPositions(&asteroid_array, 0, 0, world_max.x, world_max.y, dt);

            for (i32 i = 0; i < asteroid_array.count; ++i) {

                for (i32 v = 0; v < countof(asteroids[i].vertices); ++v) {
                    i32     next = (v + 1) % countof(asteroids[i].vertices);
                    Vector2 pos0 = Vector2Add(asteroids[i].vertices[v], asteroids[i].position);
                    Vector2 pos1 = Vector2Add(asteroids[i].vertices[next], asteroids[i].position);

                    if (CheckCollionPlayerLine(&player, pos0, pos1)) {
                        game_over = true;
                        break;
                    }

                    b32 hit       = false;
                    f32 radius    = 8.0f;
                    i32 bullet_id = CheckCollisionBulletLine(&bullet_array, pos0, pos1);

                    if (bullet_id >= 0) {
                        SetSoundPitch(explosion_sound, GetRandomFloatRange(0.90f, 1.1f));
                        PlaySound(explosion_sound);

                        Vector2 position   = asteroids[i].position;
                        Vector2 velocity   = asteroids[i].velocity;
                        i32     generation = asteroids[i].generation;

                        RemoveAsteroid(&asteroid_array, i--);
                        RemoveBullet(&bullet_array, bullet_id);

                        if (generation < 2) {
                            f32     scale     = asteroid_scale / (2.0f * (generation + 1));
                            Vector2 split_dir = Vector2Scale(Vector2Normalize(velocity), scale * 0.5f);

                            Vector2 p0 = Vector2Add(position, split_dir);
                            Vector2 p1 = Vector2Add(position, Vector2Negate(split_dir));

                            PushAsteroid(&asteroid_array, CreateAsteroid(p0, velocity, scale, generation + 1));
                            PushAsteroid(&asteroid_array, CreateAsteroid(p1, Vector2Negate(velocity), scale, generation + 1));
                        }

                        break;
                    }
                }
            }
        }

        // UpdateCamera(&camera, CAMERA_ORTHOGRAPHIC);
        BeginDrawing();
        BeginMode2D(camera);
        ClearBackground(clear_color);

        for (i32 i = 0; i < asteroid_array.count; ++i) {
            for (i32 v = 0; v < countof(asteroids[i].vertices); ++v) {
                i32     next = (v + 1) % countof(asteroids[i].vertices);
                Vector2 pos0 = Vector2Add(asteroids[i].vertices[v], asteroids[i].position);
                Vector2 pos1 = Vector2Add(asteroids[i].vertices[next], asteroids[i].position);
                DrawLineEx(pos0, pos1, 2.0f, WHITE);
            }
        }

        for (i32 i = 0; i < bullet_array.count; ++i) {
            Vector2 pos = bullet_array.elements[i].position;
            DrawCircle(pos.x, pos.y, 8.0f, YELLOW);
        }

        for (i32 i = 0; i < countof(player.vertices); ++i) {
            i32     next = (i + 1) % countof(player.vertices);
            Vector2 pos0 = Vector2Add(player.vertices[i], player.position);
            Vector2 pos1 = Vector2Add(player.vertices[next], player.position);
            DrawLineEx(pos0, pos1, 2.0f, ORANGE);
        }

        EndMode2D();

        if (game_over) {
            Rectangle rect = {screen_width / 2.0f - 128.0f, screen_height / 2.0f - 64.0f, 256.0f, 128.0f};
            DrawRectangleRounded(rect, 0.3f, 6, Fade(DARKGRAY, 0.5f));

            const char *game_over_str = "GAME OVER";

            i32 font_size  = 32;
            i32 text_width = MeasureText(game_over_str, font_size);
            DrawText(game_over_str, screen_width / 2.0f - text_width / 2.0f, screen_height / 2.0f - font_size / 2.0f, font_size, RED);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadSound(shoot_sound);
    UnloadSound(explosion_sound);
    CloseAudioDevice();
    CloseWindow();
}
