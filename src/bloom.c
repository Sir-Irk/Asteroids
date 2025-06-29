#include "bloom.h"
#include <raylib.h>
#include "types.h"

#include "game.h"

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
