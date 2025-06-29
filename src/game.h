#include "asteroids.h"
#include "bloom.h"

typedef struct GameState {
    b32     resources_loaded;
    b32     game_over;
    b32     game_won;
    i32     screen_width;
    i32     screen_height;
    Vector2 world_min;
    Vector2 world_max;

    Player   player;
    Camera2D camera;

    BulletBuffer   bullet_buffer;
    AsteroidBuffer asteroid_buffer;

    RenderTexture2D   render_target;
    BloomScreenEffect bloom;

    Sound shoot_sound;
    Sound explosion_sound;
    Sound win_sound;
    Sound lose_sound;
} GameState;
