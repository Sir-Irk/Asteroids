#ifndef BLOOM_EFFECT_HEADER_GUARD
#define BLOOM_EFFECT_HEADER_GUARD

#include "../include/raylib.h"
#include "asteroids.h"

#if defined(PLATFORM_WEB)
#define BLOOM_BLUR_PASS_COUNT 2
#else
#define BLOOM_BLUR_PASS_COUNT 3
#endif

#define BLOOM_PING_PONG_BUFFER_COUNT 4

typedef struct BloomScreenEffect {
    Shader          bloom_shader;
    Shader          blur_shader;
    RenderTexture2D ping_pong_buffers[BLOOM_PING_PONG_BUFFER_COUNT][2];
    i32             texture_locations[BLOOM_PING_PONG_BUFFER_COUNT];
} BloomScreenEffect;

#endif // BLOOM_EFFECT_HEADER_GUARD
