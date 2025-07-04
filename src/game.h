#ifndef GAME_HEADER_GUARD
#define GAME_HEADER_GUARD

#include "asteroids.h"
#include "bloom.h"

#define STARTING_WINDOW_WIDTH 1920
#define STARTING_WINDOW_HEIGHT 1080

#define WORLD_WIDTH 2560
#define WORLD_HEIGHT 1440

typedef enum SoundNames {
    SOUND_SHOOT,
    SOUND_EXPLOSION,
    SOUND_WIN,
    SOUND_LOSE,
    SOUND_POWER_UP_SPAWNED,
    SOUND_POWER_UP_GAINED,
    SOUND_COUNT,
} SoundNames;

typedef struct GameState {
    b32 resources_loaded;
    b32 game_over;
    b32 game_won;
    i32 screen_width;
    i32 screen_height;

    Vector2 world_min;
    Vector2 world_max;

    Player   player;
    Camera2D camera;

    BulletBuffer   bullet_buffer;
    AsteroidBuffer asteroid_buffer;
    PowerUpBuffer  power_up_buffer;

    RenderTexture2D   render_targets[2];
    BloomScreenEffect bloom;
    Shader            fxaa_shader;

    Sound sounds[SOUND_COUNT];

    b32 show_fps;
} GameState;

#endif // GAME_HEADER_GUARD
