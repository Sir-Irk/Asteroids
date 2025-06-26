#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <raylib.h>
#include <raymath.h>
#include <string.h>

#include <math.h>
#include <stdint.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

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
    f32     height;
    Vector2 vertices[4];
    Vector2 reference_vertices[4];
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
    Asteroid elements[128];
} AsteroidBuffer;

typedef struct GameState {
    b32     resources_loaded;
    b32     game_over;
    b32     game_won;
    i32     screen_width;
    i32     screen_height;
    Vector2 world_min;
    Vector2 world_max;

    Player player;

    Camera2D camera;

    BulletBuffer   bullet_buffer;
    AsteroidBuffer asteroid_buffer;

    Sound shoot_sound;
    Sound explosion_sound;
    Sound win_sound;
    Sound lose_sound;

    f32 asteroid_scale;
} GameState;

GameState global_state = {};

f32 GetRandomFloat01()
{
    return (f32)GetRandomValue(0, INT_MAX) / (f32)INT_MAX;
}

f32 GetRandomFloatRange(f32 min, f32 max)
{
    return min + (max - min) * GetRandomFloat01();
}

Vector2 GetRandomVector2UnitCircle(f32 scale)
{
    f32 x = (GetRandomFloat01() * 2.0f - 1.0f);
    f32 y = (GetRandomFloat01() * 2.0f - 1.0f);
    return Vector2Scale(Vector2Normalize((Vector2){x, y}), scale);
}

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

        f32 scale     = global_state.asteroid_scale / (1.0f + a->generation);
        i32 max_x_off = max_x + scale;
        i32 min_x_off = min_x - scale;
        i32 max_y_off = max_y + scale;
        i32 min_y_off = max_y - scale;

        if (a->position.x > max_x_off) {
            a->position.x -= max_x_off + scale;
        } else if (a->position.x < min_x_off) {
            a->position.x += max_x_off + scale;
        }

        if (a->position.y > max_y_off) {
            a->position.y -= max_y_off + scale;
        } else if (a->position.y < min_y_off) {
            a->position.y += max_y_off + scale;
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

    asteroid.position         = position;
    asteroid.velocity         = velocity;
    asteroid.generation       = generation;
    asteroid.angular_velocity = GetRandomFloatRange(-2.0f, 2.0f);

    return asteroid;
}

void UpdateBulletLives(BulletBuffer *bullets, Vector2 world_min, Vector2 world_max)
{
    for (i32 i = 0; i < bullets->count; ++i) {
        Bullet *b = &bullets->elements[i];
        if (GetTime() - b->time_spawned >= 5.0f ||
            (b->position.y > world_max.y || b->position.y < world_min.y || b->position.x > world_max.x || b->position.x < world_min.x)) {
            RemoveBullet(bullets, i--);
        }
    }
}

void InitializeGame(GameState *state)
{
    state->world_min = (Vector2){0.0f, 0.0f};
    state->world_max = (Vector2){2560.0f, 1440.0f};
    state->game_over = false;
    state->game_won  = false;

    if (!state->resources_loaded) {
        state->shoot_sound      = LoadSound("sounds/shoot.wav");
        state->explosion_sound  = LoadSound("sounds/explosion.wav");
        state->win_sound        = LoadSound("sounds/win.wav");
        state->lose_sound       = LoadSound("sounds/lose.wav");
        state->resources_loaded = true;
    }

    SetSoundVolume(state->explosion_sound, 0.7f);
    f32 player_width  = 48.0f;
    f32 player_height = 64.0f;

    state->player = (Player){};

    state->player.height                = player_height;
    state->player.reference_vertices[0] = (Vector2){0.0f, -player_height / 2.0f};
    state->player.reference_vertices[1] = (Vector2){player_width / 2.0f, player_height / 2.0f};
    state->player.reference_vertices[2] = (Vector2){0.0f, player_height / 8.0f};
    state->player.reference_vertices[3] = (Vector2){-player_width / 2.0f, player_height / 2.0f};
    memcpy(state->player.vertices, state->player.reference_vertices, sizeof(state->player.vertices));

    state->player.position = (Vector2){state->world_max.x / 2.0f, state->world_max.y / 2.0f};

    i32 screen_width  = GetScreenWidth();
    i32 screen_height = GetScreenHeight();

    state->screen_width  = screen_width;
    state->screen_height = screen_height;

    // NOTE: assumes aspect ratio will not change
    state->camera.zoom = screen_width / 2560.0f;

    state->asteroid_scale  = 128.0f;
    state->asteroid_buffer = (AsteroidBuffer){
        .count    = 0,
        .elements = {0},
        .capacity = countof(state->asteroid_buffer.elements),
    };

    state->bullet_buffer = (BulletBuffer){
        .count    = 0,
        .elements = {0},
        .capacity = countof(state->bullet_buffer.elements),
    };

    for (i32 i = 0; i < 16; ++i) {
        Vector2 screen_center = (Vector2){(f32)state->world_max.x / 2, (f32)state->world_max.y / 2};
        Vector2 random_dir    = GetRandomVector2UnitCircle(GetRandomFloatRange(state->world_max.y / 3.0f, state->world_max.y / 1.5f));
        Vector2 position      = Vector2Add(screen_center, random_dir);
        Vector2 velocity      = GetRandomVector2UnitCircle(GetRandomFloatRange(50.0f, 250.0f));
        PushAsteroid(&state->asteroid_buffer, CreateAsteroid(position, velocity, state->asteroid_scale, 0));
    }
}

void Update(GameState *state)
{
    f32 dt = GetFrameTime();

    Vector2 mouse_pos = Vector2Scale(GetMousePosition(), 1.0f / state->camera.zoom);

    i32 screen_width  = GetScreenWidth();
    i32 screen_height = GetScreenHeight();

    state->screen_width  = screen_width;
    state->screen_height = screen_height;

    if (state->game_over || state->game_won) {
        if (IsKeyPressed(KEY_SPACE)) {
            InitializeGame(state);
        }
        return;
    }

    if (state->asteroid_buffer.count == 0) {
        state->game_won = true;
        PlaySound(state->win_sound);
        return;
    }

    UpdateBulletLives(&state->bullet_buffer, state->world_min, state->world_max);
    UpdateAsteroidPositions(&state->asteroid_buffer, 0, 0, state->world_max.x, state->world_max.y, dt);

    f32    angle = Vector2Angle((Vector2){0.0f, -1.0f}, Vector2Subtract(mouse_pos, state->player.position));
    Matrix rot   = MatrixRotateZ(angle);
    for (i32 i = 0; i < countof(state->player.vertices); ++i) {
        state->player.vertices[i] = Vector2Transform(state->player.reference_vertices[i], rot);
    }

    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(0)) {
        SetSoundPitch(state->shoot_sound, GetRandomFloatRange(0.95f, 1.05f));
        PlaySound(state->shoot_sound);
        Vector2 direction = Vector2Normalize(Vector2Subtract(mouse_pos, state->player.position));
        Vector2 pos       = Vector2Add(state->player.position, Vector2Scale(direction, state->player.height / 2.0f));
        Bullet  bullet    = {pos, pos, Vector2Scale(direction, 900.0f), 8.0f, GetTime()};
        PushBullet(&state->bullet_buffer, bullet);
    }

    Vector2 direction = {};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction = Vector2Add(direction, (Vector2){0.0f, -1.0f});
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction = Vector2Add(direction, (Vector2){0.0f, 1.0f});
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction = Vector2Add(direction, (Vector2){-1.0f, 0.0f});
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction = Vector2Add(direction, (Vector2){1.0f, 0.0f});
    direction              = Vector2Normalize(direction);
    state->player.velocity = Vector2Add(state->player.velocity, Vector2Scale(direction, 30.0f * dt));

    f32 max_magnitude = 400.0f * dt;
    if (Vector2Length(state->player.velocity) > max_magnitude) {
        state->player.velocity = Vector2Scale(Vector2Normalize(state->player.velocity), max_magnitude);
    }

    state->player.position = Vector2Add(state->player.position, state->player.velocity);

    f32 min_x = state->world_min.x - state->player.height;
    f32 max_x = state->world_max.x + state->player.height;
    f32 min_y = state->world_min.y - state->player.height;
    f32 max_y = state->world_max.y + state->player.height;

    if (state->player.position.x > max_x) {
        state->player.position.x -= max_x + state->player.height;
    } else if (state->player.position.x < min_x) {
        state->player.position.x += max_x + state->player.height;
    }

    if (state->player.position.y > max_y) {
        state->player.position.y -= max_y + state->player.height;
    } else if (state->player.position.y < min_y) {
        state->player.position.y += max_y + state->player.height;
    }

    f32 drag = expf(-3.00f * dt);
    state->player.velocity.x *= drag;
    state->player.velocity.y *= drag;

    for (i32 i = 0; i < state->bullet_buffer.count; ++i) {
        Bullet *b        = &state->bullet_buffer.elements[i];
        b->prev_position = b->position;
        b->position      = Vector2Add(b->position, Vector2Scale(b->velocity, dt));
    }

    //============ Player/Asteroid and Bullet/Asteroid collision checks ===============
    Asteroid *asteroids = state->asteroid_buffer.elements;
    for (i32 i = 0; i < state->asteroid_buffer.count; ++i) {

        for (i32 v = 0; v < countof(asteroids[i].vertices); ++v) {
            i32     next = (v + 1) % countof(asteroids[i].vertices);
            Vector2 pos0 = Vector2Add(asteroids[i].vertices[v], asteroids[i].position);
            Vector2 pos1 = Vector2Add(asteroids[i].vertices[next], asteroids[i].position);

            if (CheckCollionPlayerLine(&state->player, pos0, pos1)) {
                state->game_over = true;
                PlaySound(state->lose_sound);
                break;
            }

            i32 bullet_id = CheckCollisionBulletLine(&state->bullet_buffer, pos0, pos1);

            if (bullet_id >= 0) {
                SetSoundPitch(state->explosion_sound, GetRandomFloatRange(0.90f, 1.1f));
                PlaySound(state->explosion_sound);

                Vector2 position   = asteroids[i].position;
                Vector2 velocity   = asteroids[i].velocity;
                i32     generation = asteroids[i].generation;

                RemoveAsteroid(&state->asteroid_buffer, i--);
                RemoveBullet(&state->bullet_buffer, bullet_id);

                if (generation < 2) {
                    f32     scale     = state->asteroid_scale / (2.0f * (generation + 1));
                    Vector2 split_dir = Vector2Scale(Vector2Normalize(velocity), scale * 0.5f);

                    Vector2 p0 = Vector2Add(position, split_dir);
                    Vector2 p1 = Vector2Add(position, Vector2Negate(split_dir));

                    PushAsteroid(&state->asteroid_buffer, CreateAsteroid(p0, velocity, scale, generation + 1));
                    PushAsteroid(&state->asteroid_buffer, CreateAsteroid(p1, Vector2Negate(velocity), scale, generation + 1));
                }

                break;
            }
        }
    }
}

void Draw(GameState *state)
{
    BeginDrawing();
    BeginMode2D(state->camera);
    ClearBackground(BLACK);

    Asteroid *asteroids = state->asteroid_buffer.elements;
    for (i32 i = 0; i < state->asteroid_buffer.count; ++i) {
        for (i32 v = 0; v < countof(asteroids[i].vertices); ++v) {
            i32     next = (v + 1) % countof(asteroids[i].vertices);
            Vector2 pos0 = Vector2Add(asteroids[i].vertices[v], asteroids[i].position);
            Vector2 pos1 = Vector2Add(asteroids[i].vertices[next], asteroids[i].position);
            DrawLineEx(pos0, pos1, 2.0f, WHITE);
        }
    }

    for (i32 i = 0; i < state->bullet_buffer.count; ++i) {
        Vector2 pos = state->bullet_buffer.elements[i].position;
        DrawCircle(pos.x, pos.y, 8.0f, YELLOW);
    }

    for (i32 i = 0; i < countof(state->player.vertices); ++i) {
        i32     next = (i + 1) % countof(state->player.vertices);
        Vector2 pos0 = Vector2Add(state->player.vertices[i], state->player.position);
        Vector2 pos1 = Vector2Add(state->player.vertices[next], state->player.position);
        DrawLineEx(pos0, pos1, 2.0f, ORANGE);
    }

    EndMode2D();

    if (state->game_over || state->game_won) {
        Rectangle rect = {state->screen_width / 2.0f - 256.0f, state->screen_height / 2.0f - 128.0f, 512.0f, 256.0f};
        DrawRectangleRounded(rect, 0.3f, 6, Fade(DARKGRAY, 0.5f));

        Color color = (state->game_over) ? RED : GREEN;

        const char *status_str     = (state->game_over) ? "GAME OVER!" : "YOU WIN!";
        const char *start_over_str = "PRESS SPACE TO START OVER";

        i32 font_size  = 42;
        i32 text_width = MeasureText(status_str, font_size);

        f32 x = state->screen_width / 2.0f - text_width / 2.0f;
        f32 y = state->screen_height / 2.0f - font_size;
        DrawText(status_str, x, y, font_size, color);

        font_size  = 20;
        text_width = MeasureText(start_over_str, font_size);

        x = state->screen_width / 2.0f - text_width / 2.0f;
        DrawText(start_over_str, x, y + font_size * 4, font_size, WHITE);
    } else if (state->game_won) {
    }

    DrawFPS(10, 10);
    EndDrawing();
}

void UpdateAndDraw()
{
    Update(&global_state);
    Draw(&global_state);
}

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1920, 1080, "Asteroids");
    InitAudioDevice();

    // NOTE: We use global state so that we can use "emscripten_set_main_loop" because
    //       using the "ASYNCIFY" flag with WindowShouldClose() has a significant performance penalty
    InitializeGame(&global_state);

    SetRandomSeed(time(NULL));

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateAndDraw, GetMonitorRefreshRate(GetCurrentMonitor()), 1);
#else
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    while (!WindowShouldClose()) {
        UpdateAndDraw();
    }
#endif

    UnloadSound(global_state.shoot_sound);
    UnloadSound(global_state.explosion_sound);
    UnloadSound(global_state.win_sound);
    UnloadSound(global_state.lose_sound);

    CloseAudioDevice();
    CloseWindow();
}
