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
    Vector2 vertices[8];
    f32     angular_velocity;
} asteroid_t;

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
create_asteroid(f32 scale, f32 speed)
{
    asteroid_t asteroid = {};
    Vector2    average  = {};
    i32        count    = si_array_count(asteroid.vertices);
    f32        step     = 2 * PI / count;

    for (i32 i = 0; i < count; ++i) {
        f32 angle = (i + 1) * step;
        f32 x     = cosf(angle);
        f32 y     = sinf(angle);
        f32 s     = scale * GetRandomFloatRange(0.3f, 1.0f);

        Vector2 vertex       = Vector2Scale(Vector2Normalize((Vector2){x, y}), s);
        asteroid.vertices[i] = vertex;
        average              = Vector2Add(average, vertex);
    }

    average = Vector2Scale(average, 1.0f / si_array_count(asteroid.vertices));

    for (i32 i = 0; i < si_array_count(asteroid.vertices); ++i) {
        asteroid.vertices[i] = Vector2Subtract(asteroid.vertices[i], average);
    }

    asteroid.velocity = GetRandomVector2UnitCircle(speed);

    return asteroid;
}

int
main(void)
{
    si_primary_buffer primary_buffer = si_allocate_primary_buffer(si_megabytes(64), 0);
    si_memory_arena   arena          = si_make_arena(&primary_buffer, primary_buffer.size / 2);
    si_memory_arena   temporal_arena = si_make_arena_segment(&primary_buffer, primary_buffer.size / 2, primary_buffer.size / 2);

    // SetConfigFlags(FLAG_FULLSCREEN_MODE);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1920, 1080, "Asteroids");

    SetTargetFPS(GetMonitorRefreshRate(0));
    // SetTargetFPS(144);
    ToggleBorderlessWindowed();
    SetRandomSeed(time(NULL));

    i32 screen_width  = GetScreenWidth();
    i32 screen_height = GetScreenHeight();

    const i32   asteroid_count = 32;
    asteroid_t *asteroids      = si_push_array(&arena, asteroid_count, asteroid_t);

    for (i32 i = 0; i < asteroid_count; ++i) {
        asteroids[i] = create_asteroid(128.0f, GetRandomFloatRange(100.0f, 250.0f));
    }

    Color clear_color = ColorLerp(BLACK, WHITE, 0.0f);

    for (i32 i = 0; i < asteroid_count; ++i) {
        Vector2 center        = (Vector2){(f32)screen_width / 2, (f32)screen_height / 2};
        asteroids[i].position = Vector2Add(center, GetRandomVector2UnitCircle(800));
    }

    while (!WindowShouldClose()) {
        screen_width  = GetScreenWidth();
        screen_height = GetScreenHeight();

        f32 dt = GetFrameTime();

        for (i32 i = 0; i < asteroid_count; ++i) {
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
        }

        BeginDrawing();
        ClearBackground(clear_color);

        Vector2 offset0 = {0.0f, screen_height};
        Vector2 offset1 = {screen_width, 0.0f};
        for (i32 i = 0; i < asteroid_count; ++i) {

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

        DrawFPS(10, 10);
        EndDrawing();
    }

    si_free(&primary_buffer);

    CloseWindow();
}
