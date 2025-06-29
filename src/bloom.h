#ifndef BLOOM_EFFECT_HEADER_GUARD
#define BLOOM_EFFECT_HEADER_GUARD

#include "asteroids.h"
#include "raylib.h"

#define BLOOM_BLUR_PASS_COUNT 3
#define BLOOM_PING_PONG_BUFFER_COUNT 4

typedef struct BloomScreenEffect {
    Shader          bloom_shader;
    Shader          blur_shader;
    RenderTexture2D ping_pong_buffers[BLOOM_PING_PONG_BUFFER_COUNT][2];
    i32             texture_locations[BLOOM_PING_PONG_BUFFER_COUNT];
} BloomScreenEffect;

#endif // BLOOM_EFFECT_HEADER_GUARD
