#include "bloom.h"
#include <raylib.h>
#include "asteroids.h"
#include "types.h"

#include "game.h"

static void InitializeBloomEffect(BloomScreenEffect *bloom, i32 start_width, i32 start_height)
{
#if defined(PLATFORM_WEB)
    bloom->blur_shader  = LoadShader(NULL, "shaders/gaussian_blur_300_es.frag");
    bloom->bloom_shader = LoadShader(NULL, "shaders/bloom_advanced_300_es.frag");
#else
    bloom->blur_shader  = LoadShader(0, "shaders/gaussian_blur.frag");
    bloom->bloom_shader = LoadShader(0, "shaders/bloom_advanced.frag");
#endif

    for (i32 i = 0; i < countof(bloom->ping_pong_buffers); ++i) {
        i32 div = 2 * (i);
        if (i == 0) div = 1;

        bloom->ping_pong_buffers[i][0] = LoadRenderTexture(start_width / div, start_height / div);
        bloom->ping_pong_buffers[i][1] = LoadRenderTexture(start_width / div, start_height / div);
    }

    bloom->texture_locations[0] = GetShaderLocation(bloom->bloom_shader, "bloomTexture1");
    bloom->texture_locations[1] = GetShaderLocation(bloom->bloom_shader, "bloomTexture2");
    bloom->texture_locations[2] = GetShaderLocation(bloom->bloom_shader, "bloomTexture3");
    bloom->texture_locations[3] = GetShaderLocation(bloom->bloom_shader, "bloomTexture4");

    // for (i32 i = 0; i < countof(bloom->texture_locations); ++i) {
    //    assert(bloom->texture_locations[i] >= 0);
    //}
}

static void UnloadBloomEffect(BloomScreenEffect *bloom)
{
    UnloadShader(bloom->blur_shader);
    UnloadShader(bloom->bloom_shader);
    for (i32 i = 0; i < countof(bloom->ping_pong_buffers); ++i) {
        UnloadRenderTexture(bloom->ping_pong_buffers[i][0]);
        UnloadRenderTexture(bloom->ping_pong_buffers[i][1]);
    }
}

void DrawFramebuffer(RenderTexture2D src, RenderTexture2D dst, b32 clear)
{
    BeginTextureMode(dst);
    if (clear) ClearBackground(BLACK);
    DrawTexturePro(src.texture,
        (Rectangle){0, 0, (float)src.texture.width, (float)-src.texture.height},
        (Rectangle){0, 0, (float)dst.texture.width, (float)-dst.texture.height},
        (Vector2){0, 0},
        0.0f,
        WHITE);
    EndTextureMode();
}

static void RenderBloomTextures(GameState *state)
{
    BloomScreenEffect *bloom = &state->bloom;

    //======== Run Blur Passes =========

    BeginShaderMode(bloom->blur_shader);

    BeginTextureMode(bloom->ping_pong_buffers[0][0]);
    i32 loc        = GetShaderLocation(bloom->blur_shader, "horizontal");
    b32 horizontal = true;
    SetShaderValue(bloom->blur_shader, loc, &horizontal, SHADER_UNIFORM_INT);

    for (i32 set = 0; set < countof(bloom->ping_pong_buffers); ++set) {
        RenderTexture *buf0 = &bloom->ping_pong_buffers[set][0];
        RenderTexture *buf1 = &bloom->ping_pong_buffers[set][1];

        if (set == 0) {
            // if (true) {
            DrawFramebuffer(state->render_target, *buf0, true);
        } else {
            DrawFramebuffer(bloom->ping_pong_buffers[set - 1][0], *buf0, true);
        }

        for (i32 i = 0; i < 6; ++i) {
            horizontal = true;
            SetShaderValue(bloom->blur_shader, loc, &horizontal, SHADER_UNIFORM_INT);
            DrawFramebuffer(*buf0, *buf1, false);

            horizontal = false;
            SetShaderValue(bloom->blur_shader, loc, &horizontal, SHADER_UNIFORM_INT);
            DrawFramebuffer(*buf1, *buf0, i == 0);
        }
    }

    //==== Draw to backbuffer using bloom =====
    BeginDrawing();

    BeginShaderMode(bloom->bloom_shader);
    for (i32 i = 0; i < countof(bloom->texture_locations); ++i) {
        SetShaderValueTexture(bloom->bloom_shader, bloom->texture_locations[i], bloom->ping_pong_buffers[i][0].texture);
    }

    DrawTexturePro(state->render_target.texture,
        (Rectangle){0, 0, (float)state->render_target.texture.width, (float)-state->render_target.texture.height},
        (Rectangle){0, 0, (float)state->render_target.texture.width, (float)-state->render_target.texture.height},
        (Vector2){0, 0},
        0.0f,
        WHITE);
    EndShaderMode();
}
