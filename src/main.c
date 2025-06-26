#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include "gui.h"

#define SI_MEMORY_IMPLEMENTATION
#include "si_libs/si_memory.h"

#include "raylib.h"
#include "si_libs/types.h"

#include "threadpool.h"

#include <float.h>

#include "gui.c"

#include <limits.h>

typedef struct asteroid_t
{
    Vector2 position;
    Vector2 velocity;
    f32     angular_velocity;
    b32     child;
    Vector2 vertices[12];
} asteroid_t;

typedef struct player_t
{
    Vector2 position;
    Vector2 velocity;
    f32     rotation;
    Vector2 vertices[3];
} player_t;

typedef struct bullet_t
{
    Vector2 prev_position;
    Vector2 position;
    Vector2 velocity;
    f32     time_spawned;
    // Vector2 vertices[4];
} bullet_t;

typedef struct bullet_array_t
{
    i32       capacity;
    i32       count;
    bullet_t *bullets;

} bullet_array_t;

internal bullet_t *
push_bullet(bullet_array_t *array, bullet_t bullet)
{
    if (array->count + 1 > array->capacity) {
        return NULL;
    }

    array->bullets[array->count] = bullet;
    return &array->bullets[array->count++];
}

internal void
remove_bullet(bullet_array_t *array, i32 index)
{
    assert(index >= 0);
    assert(index < array->count);

    si_swap(array->bullets[index], array->bullets[array->count - 1], bullet_t);
    array->count--;
}

typedef struct asteroid_array_t
{
    i32         capacity;
    i32         count;
    asteroid_t *elements;
} asteroid_array_t;

internal asteroid_t *
push_asteroid(asteroid_array_t *array, asteroid_t asteroid)
{
    if (array->count + 1 > array->capacity) {
        return NULL;
    }

    array->elements[array->count] = asteroid;
    return &array->elements[array->count++];
}

internal void
remove_asteroid(asteroid_array_t *array, i32 index)
{
    assert(index >= 0);
    assert(index < array->count);

    si_swap(array->elements[index], array->elements[array->count - 1], asteroid_t);
    array->count--;
}

internal f32
GetRandomFloat01()
{
    return (f32)GetRandomValue(0, INT_MAX) / (f32)INT_MAX;
}

internal f32
GetRandomFloatRange(f32 min, f32 max)
{
    return min + (max - min) * GetRandomFloat01();
}

internal Vector2
GetRandomVector2UnitCircle(f32 scale)
{
    f32 x = (GetRandomFloat01() * 2.0f - 1.0f);
    f32 y = (GetRandomFloat01() * 2.0f - 1.0f);
    return Vector2Scale(Vector2Normalize((Vector2){x, y}), scale);
}

internal asteroid_t
create_asteroid(f32 scale, Vector2 velocity, b32 child)
{
    asteroid_t asteroid = {};
    Vector2    average  = {};
    i32        count    = si_array_count(asteroid.vertices);
    f32        step     = 2 * PI / count;

    for (i32 i = 0; i < count; ++i) {
        f32 angle = (i + 1) * step;
        f32 x     = cosf(angle);
        f32 y     = sinf(angle);
        f32 s     = scale * GetRandomFloatRange(0.5f, 1.0f);

        Vector2 vertex       = Vector2Scale(Vector2Normalize((Vector2){x, y}), s);
        asteroid.vertices[i] = vertex;
        average              = Vector2Add(average, vertex);
    }

    average = Vector2Scale(average, 1.0f / si_array_count(asteroid.vertices));

    for (i32 i = 0; i < si_array_count(asteroid.vertices); ++i) {
        asteroid.vertices[i] = Vector2Subtract(asteroid.vertices[i], average);
    }

    asteroid.velocity = velocity;
    asteroid.child    = child;

    return asteroid;
}

int
main(void)
{
    unsigned char _stack_buffer[si_megabytes(2)] = {};

    si_primary_buffer primary_buffer = si_primary_buffer_stack(sizeof(_stack_buffer), _stack_buffer);

    si_memory_arena arena          = si_make_arena(&primary_buffer, primary_buffer.size / 2);
    si_memory_arena temporal_arena = si_make_arena_segment(&primary_buffer, primary_buffer.size / 2, primary_buffer.size / 2);

    // SetConfigFlags(FLAG_FULLSCREEN_MODE);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Asteroids");
    InitAudioDevice();

    SetTargetFPS(GetMonitorRefreshRate(0));
    // SetTargetFPS(144);
    ToggleBorderlessWindowed();
    // SetRandomSeed(time(NULL));
    SetRandomSeed(8);

    Sound sound = LoadSound("sounds/spring.wav");

    i32 screen_width   = GetScreenWidth();
    i32 screen_height  = GetScreenHeight();
    f32 asteroid_scale = 128.0f;

    asteroid_array_t asteroid_array = {
        .count    = 0,
        .capacity = 64,
        .elements = si_push_array(&arena, asteroid_array.capacity, asteroid_t),
    };

    for (i32 i = 0; i < 16; ++i) {
        push_asteroid(&asteroid_array,
                      create_asteroid(asteroid_scale, GetRandomVector2UnitCircle(GetRandomFloatRange(100.0f, 250.0f)), false));
    }

    f32      player_width  = 48.0f;
    f32      player_height = 64.0f;
    player_t player        = {};

    Vector2 player_vert_ref[3] = {
        {0.0f, -player_height / 2.0f},
        {player_width / 2.0f, player_height / 2.0f},
        {-player_width / 2.0f, player_height / 2.0f},
    };

    player.position    = (Vector2){screen_width / 2.0f, screen_height / 2.0f};
    player.vertices[0] = player_vert_ref[0];
    player.vertices[1] = player_vert_ref[1];
    player.vertices[2] = player_vert_ref[2];

    bullet_array_t bullet_array = {
        .count    = 0,
        .capacity = 32,
        .bullets  = si_push_array(&arena, bullet_array.capacity, bullet_t),
    };

    Color clear_color = ColorLerp(BLACK, WHITE, 0.0f);

    asteroid_t *asteroids = asteroid_array.elements;

    for (i32 i = 0; i < asteroid_array.count; ++i) {
        Vector2 center        = (Vector2){(f32)screen_width / 2, (f32)screen_height / 2};
        asteroids[i].position = Vector2Add(center, GetRandomVector2UnitCircle(GetRandomFloatRange(400.0f, 800.0f)));
    }

    i32 red_id = -1;

    b32 game_over = false;

    while (!WindowShouldClose()) {
        screen_width  = GetScreenWidth();
        screen_height = GetScreenHeight();

        Vector2 mouse_pos = GetMousePosition();
        f32     dt        = GetFrameTime();

        if (!game_over) {
            for (i32 i = 0; i < bullet_array.count; ++i) {
                bullet_t *b = &bullet_array.bullets[i];
                if (GetTime() - b->time_spawned >= 5.0f ||
                    (b->position.y > screen_height || b->position.y < 0.0f || b->position.x > screen_width || b->position.x < 0.0f)) {
                    remove_bullet(&bullet_array, i);
                    --i;
                }
            }

            f32    angle = Vector2Angle((Vector2){0.0f, -1.0f}, Vector2Subtract(mouse_pos, player.position));
            Matrix rot   = MatrixRotateZ(angle);
            for (i32 i = 0; i < si_array_count(player.vertices); ++i) {
                player.vertices[i] = Vector2Transform(player_vert_ref[i], rot);
            }

            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(0)) {
                PlaySound(sound);
                Vector2 direction = Vector2Normalize(Vector2Subtract(mouse_pos, player.position));
                Vector2 pos       = Vector2Add(player.position, Vector2Scale(direction, player_height / 2.0f));
                push_bullet(&bullet_array, (bullet_t){pos, pos, Vector2Scale(direction, 800.0f), GetTime()});
            }

            Vector2 direction = {};
            if (IsKeyDown(KEY_W)) direction = Vector2Add(direction, (Vector2){0.0f, -1.0f});
            if (IsKeyDown(KEY_S)) direction = Vector2Add(direction, (Vector2){0.0f, 1.0f});
            if (IsKeyDown(KEY_A)) direction = Vector2Add(direction, (Vector2){-1.0f, 0.0f});
            if (IsKeyDown(KEY_D)) direction = Vector2Add(direction, (Vector2){1.0f, 0.0f});
            direction         = Vector2Normalize(direction);
            player.velocity   = Vector2Add(player.velocity, Vector2Scale(direction, 30.0f * dt));
            f32 max_magnitude = 2.5f;
            if (Vector2Length(player.velocity) > max_magnitude) {
                player.velocity = Vector2Scale(Vector2Normalize(player.velocity), max_magnitude);
            }

            player.position = Vector2Add(player.position, player.velocity);
            player.velocity = Vector2Subtract(player.velocity, Vector2Scale(Vector2Normalize(player.velocity), 3.0f * dt));

            for (i32 i = 0; i < bullet_array.count; ++i) {
                bullet_t *b      = &bullet_array.bullets[i];
                b->prev_position = b->position;
                b->position      = Vector2Add(b->position, Vector2Scale(b->velocity, dt));
            }

            for (i32 i = 0; i < asteroid_array.count; ++i) {
                asteroids[i].position.x += asteroids[i].velocity.x * dt;
                asteroids[i].position.y += asteroids[i].velocity.y * dt;

                if (asteroids[i].position.x > screen_width) {
                    asteroids[i].position.x = 0;
                } else if (asteroids[i].position.x < 0) {
                    asteroids[i].position.x = screen_width;
                }
                if (asteroids[i].position.y > screen_height) {
                    asteroids[i].position.y = 0;
                } else if (asteroids[i].position.y < 0) {
                    asteroids[i].position.y = screen_height;
                }

                Matrix mat = MatrixRotateZ(1.0f * dt);
                for (i32 v = 0; v < si_array_count(asteroids[i].vertices); ++v) {
                    asteroids[i].vertices[v] = Vector2Transform(asteroids[i].vertices[v], mat);
                }
                // DrawCircle(asteroid.position.x, asteroid.position.y, 8.0f, BLUE);

                red_id = i;

                for (i32 v = 0; v < si_array_count(asteroids[i].vertices); ++v) {
                    i32 next = (v + 1) % si_array_count(asteroids[i].vertices);
                    // Vector2 mpos  = Vector2Subtract(asteroids[i].position, mouse_pos);
                    Vector2 mpos = mouse_pos;

                    Vector2 pos0 = Vector2Add(asteroids[i].vertices[v], asteroids[i].position);
                    Vector2 pos1 = Vector2Add(asteroids[i].vertices[next], asteroids[i].position);

                    for (i32 pv = 0; pv < si_array_count(player.vertices); ++pv) {
                        i32     pv_next = (pv + 1) % si_array_count(player.vertices);
                        Vector2 vert0   = Vector2Add(player.vertices[pv], player.position);
                        Vector2 vert1   = Vector2Add(player.vertices[pv_next], player.position);
                        Vector2 collision;
                        if (CheckCollisionLines(pos0, pos1, vert0, vert1, &collision)) {
                            game_over = true;
                            goto loop_break;
                        }
                    }

                    b32 hit    = false;
                    f32 radius = 8.0f;

                    for (i32 b = 0; b < bullet_array.count; ++b) {
                        bullet_t *bullet = &bullet_array.bullets[b];
                        Vector2   collision;

                        Vector2 delta  = Vector2Normalize(Vector2Subtract(bullet->position, bullet->prev_position));
                        Vector2 newPos = Vector2Add(bullet->position, Vector2Scale(delta, radius));

                        if (CheckCollisionLines(pos0, pos1, bullet->prev_position, newPos, &collision) ||
                            CheckCollisionCircleLine(bullet->position, radius, pos0, pos1)) {

                            hit = true;

                            Vector2 position = asteroids[i].position;
                            Vector2 velocity = asteroids[i].velocity;
                            b32     is_child = asteroids[i].child;
                            remove_asteroid(&asteroid_array, i);
                            --i;
                            remove_bullet(&bullet_array, b);

                            if (!is_child) {
                                asteroid_t *a0 = push_asteroid(&asteroid_array, create_asteroid(asteroid_scale * 0.5f, velocity, true));
                                asteroid_t *a1 =
                                    push_asteroid(&asteroid_array, create_asteroid(asteroid_scale * 0.5f, Vector2Negate(velocity), true));

                                a0->position = Vector2Add(position, Vector2Scale(Vector2Normalize(velocity), asteroid_scale * 0.5f));
                                a1->position =
                                    Vector2Add(position, Vector2Scale(Vector2Normalize(Vector2Negate(velocity)), asteroid_scale * 0.5f));
                            }

                            break;
                        }
                    }
                    if (hit) break;
                }
            }
        }

    loop_break:

        BeginDrawing();
        ClearBackground(clear_color);

        Vector2 offset0 = {0.0f, screen_height};
        Vector2 offset1 = {screen_width, 0.0f};
        for (i32 i = 0; i < asteroid_array.count; ++i) {
            for (i32 r = -1; r < 2; ++r) {
                Vector2 off = Vector2Scale(offset0, (f32)r);
                for (i32 v = 0; v < si_array_count(asteroids[i].vertices); ++v) {
                    i32     next     = (v + 1) % si_array_count(asteroids[i].vertices);
                    Vector2 position = Vector2Add(asteroids[i].position, off);
                    Vector2 pos0     = Vector2Add(asteroids[i].vertices[v], position);
                    Vector2 pos1     = Vector2Add(asteroids[i].vertices[next], position);
                    DrawLineEx(pos0, pos1, 2.0f, WHITE);
                }
            }
            for (i32 r = -1; r < 2; ++r) {
                if (r == 0) continue;
                Vector2 off = Vector2Scale(offset1, (f32)r);
                for (i32 v = 0; v < si_array_count(asteroids[i].vertices); ++v) {
                    i32     next     = (v + 1) % si_array_count(asteroids[i].vertices);
                    Vector2 position = Vector2Add(asteroids[i].position, off);
                    Vector2 pos0     = Vector2Add(asteroids[i].vertices[v], position);
                    Vector2 pos1     = Vector2Add(asteroids[i].vertices[next], position);
                    DrawLineEx(pos0, pos1, 2.0f, WHITE);
                }
            }
        }

        for (i32 i = 0; i < bullet_array.count; ++i) {
            bullet_t *b = &bullet_array.bullets[i];
            DrawCircle(b->position.x, b->position.y, 8.0f, RED);
        }

        for (i32 i = 0; i < si_array_count(player.vertices); ++i) {
            i32     next = (i + 1) % si_array_count(player.vertices);
            Vector2 pos0 = Vector2Add(player.vertices[i], player.position);
            Vector2 pos1 = Vector2Add(player.vertices[next], player.position);
            DrawLineEx(pos0, pos1, 2.0f, BLUE);
        }

        // DrawText(TextFormat("Bullet Count: %d\n", bullet_array.count), 10, 40, 20, WHITE);
        // DrawText(TextFormat("Verts :  %f, %f\n", player.vertices[0].x, player.vertices[0].y), 10, 40 * 2, 20, WHITE);

        if (game_over) {
            DrawRectangleRounded(
                (Rectangle){screen_width / 2.0f - 128.0f, screen_height / 2.0f - 64.0f, 256.0f, 128.0f}, 0.3f, 6, Fade(DARKGRAY, 0.5f));
            i32 w = MeasureText("GAME OVER", 32);
            DrawText("GAME OVER", screen_width / 2.0f - w / 2.0f, screen_height / 2.0f - 32 / 2.0f, 32, RED);
        }

        // DrawFPS(10, 10);
        EndDrawing();
    }

    si_free(&primary_buffer);

    UnloadSound(sound);
    CloseAudioDevice();
    CloseWindow();
}
