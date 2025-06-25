#include <stdio.h>
// #include "raylib.h"
#include "raylib.h"
#include "si_libs/types.h"

#include "gui.h"

LabelButton
CreateLabelButton(const char *label, i32 font_size, i32 x, i32 y, i32 pad)
{
    LabelButton result = {};
    i32         width  = MeasureText(label, font_size);
    result.pad         = pad;
    result.label       = label;
    result.font_size   = font_size;
    result.rect.width  = width + pad * 2;
    result.rect.height = font_size + pad * 2;
    result.rect.x      = (x - result.rect.width / 2) - pad;
    result.rect.y      = y - result.rect.height / 2 - pad;
    return result;
}

internal SliderF32
CreateSliderF32(f32 x, f32 y, f32 w, f32 h, f32 pad, f32 handle_radius, f32 min, f32 max, Color color)
{
    SliderF32 result     = {};
    result.min           = min;
    result.max           = max;
    result.pad           = pad;
    result.rect          = (Rectangle){x, y, w, h};
    result.handle_radius = handle_radius;
    result.t             = 0.0f;
    result.color         = color;
    result.normal_color  = color;
    return result;
}

internal Spinner
CreateSpinner(f32 x, f32 y, f32 w, f32 h, i32 min_value, i32 max_value, Color color)
{
    Spinner result      = {};
    result.rect.x       = x;
    result.rect.y       = y;
    result.rect.width   = w;
    result.rect.height  = h;
    result.value        = min_value;
    result.min_value    = min_value;
    result.max_value    = max_value;
    result.color        = color;
    result.normal_color = color;
    result.button_size  = 32;
    return result;
}

internal void
SpinnerUpdate(Spinner *spinner, Vector2 mouse_pos)
{
    b32 pressed = IsMouseButtonPressed(0);

    spinner->hovered_button = 0;

    f32 y_off = spinner->rect.height / 2 - (f32)spinner->button_size / 2;
    f32 x_off = 8;

    Vector2 left_tri[3] = {
        {spinner->rect.x + x_off, spinner->rect.y + spinner->rect.height / 2},
        {spinner->rect.x + x_off + spinner->button_size, spinner->rect.y + y_off + spinner->button_size},
        {spinner->rect.x + x_off + spinner->button_size, spinner->rect.y + y_off},
    };

    x_off = spinner->rect.width - spinner->button_size - 8;

    Vector2 right_tri[3] = {
        {spinner->rect.x + x_off, spinner->rect.y + y_off},
        {spinner->rect.x + x_off, spinner->rect.y + y_off + spinner->button_size},
        {spinner->rect.x + x_off + spinner->button_size, spinner->rect.y + spinner->rect.height / 2},
    };

    if (CheckCollisionPointTriangle(mouse_pos, left_tri[0], left_tri[1], left_tri[2])) {
        if (pressed) {
            spinner->value = si_mod32(spinner->value - 1, spinner->max_value);
        }
        spinner->hovered_button = 1;
    } else if (CheckCollisionPointTriangle(mouse_pos, right_tri[0], right_tri[1], right_tri[2])) {
        if (pressed) {
            spinner->value = si_mod32(spinner->value + 1, spinner->max_value);
        }
        spinner->hovered_button = 2;
    }
}

internal void
SpinnerDraw(const Spinner *spinner, i32 font_size)
{
    f32 button_scale = 32.0f;
    // DrawRectangleRounded(spinner->rect, 0.25f, 8, Fade(BLACK, 0.75f));

    f32 y_off = spinner->rect.height / 2 - button_scale / 2;
    f32 x_off = 8;

    Color hover_color = ColorLerp(spinner->normal_color, WHITE, 0.3f);

    {
        Vector2 triangle[3] = {
            {spinner->rect.x + x_off, spinner->rect.y + spinner->rect.height / 2},
            {spinner->rect.x + x_off + button_scale, spinner->rect.y + y_off + button_scale},
            {spinner->rect.x + x_off + button_scale, spinner->rect.y + y_off},
        };

        DrawTriangle(triangle[0], triangle[1], triangle[2], (spinner->hovered_button == 1) ? hover_color : spinner->color);
    }

    x_off = spinner->rect.width - button_scale - 8;
    {
        Vector2 triangle[3] = {
            {spinner->rect.x + x_off, spinner->rect.y + y_off},
            {spinner->rect.x + x_off, spinner->rect.y + y_off + button_scale},
            {spinner->rect.x + x_off + button_scale, spinner->rect.y + spinner->rect.height / 2},
        };

        DrawTriangle(triangle[0], triangle[1], triangle[2], (spinner->hovered_button == 2) ? hover_color : spinner->color);
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%d/%d", spinner->value + 1, spinner->max_value);
    i32 width = MeasureText(buf, font_size);
    DrawText(buf,
             spinner->rect.x + spinner->rect.width / 2 - (f32)width / 2,
             spinner->rect.y + spinner->rect.height / 2 - (f32)font_size / 2,
             font_size,
             WHITE);
}

Vector2
SliderF32GetHandleScreenPosition(const SliderF32 *slider)
{
    Vector2 result;
    result.y = slider->rect.y + slider->rect.height / 2;
    result.x = slider->rect.x + (slider->rect.width * slider->t);
    return result;
}

f32
SliderF32Move(const SliderF32 *slider, f32 pos)
{
    f32 clamped = si_clamp(pos, slider->rect.x, slider->rect.x + slider->rect.width);
    return (clamped - slider->rect.x) / slider->rect.width;
}

f32
SliderF32GetValue(const SliderF32 *slider)
{
    return slider->min + (slider->max - slider->min) * slider->t;
}

internal void
SliderF32Update(SliderF32 *slider, SliderF32 **held_slider, Vector2 mouse_pos)
{
    slider->color = slider->normal_color;

    if (*held_slider && *held_slider != slider) {
        return;
    }

    Vector2 handle_pos = SliderF32GetHandleScreenPosition(slider);

    if (CheckCollisionPointCircle(mouse_pos, handle_pos, slider->handle_radius) || CheckCollisionPointRec(mouse_pos, slider->rect)) {
        if (IsMouseButtonPressed(0)) {
            *held_slider = slider;
        } else {
            slider->color = ColorLerp(slider->color, WHITE, 0.2f);
        }
    }

    if (IsMouseButtonReleased(0)) {
        *held_slider = NULL;
    } else if (*held_slider) { // We are held :)
        slider->t     = SliderF32Move(slider, mouse_pos.x);
        slider->color = ColorLerp(slider->color, WHITE, 0.3f);
    }
}

internal void
SliderF32Draw(const SliderF32 *slider, const char *label)
{
    Vector2   handle_pos = SliderF32GetHandleScreenPosition(slider);
    Rectangle r          = slider->rect;
    i32       pad        = slider->pad;
    i32       font_size  = 20;
    r.height += 55 + font_size;
    r.y -= pad;
    r.width += pad * 2 + 64;
    r.x -= pad;

    DrawRectangleRounded(r, 0.5f, 4, Fade(BLACK, 0.5f));
    DrawRectangleRounded(slider->rect, 1.0f, 4, ColorLerp(BLACK, slider->color, 0.2f));

    Rectangle fill_rect = slider->rect;
    fill_rect.width     = handle_pos.x - fill_rect.x;
    DrawRectangleRounded(fill_rect, 1.0f, 4, ColorLerp(BLACK, slider->color, 0.7f));

    DrawCircle(handle_pos.x, handle_pos.y, slider->handle_radius, slider->color);
    DrawText(label, slider->rect.x, slider->rect.y + slider->rect.height + 16.0f, font_size, RAYWHITE);

    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", SliderF32GetValue(slider));
    DrawText(buf, slider->rect.x + slider->rect.width + 24, slider->rect.y - slider->rect.height / 2, font_size, RAYWHITE);
}

internal void
LabelButtonDraw(const LabelButton *button)
{
    DrawRectangleRounded(button->rect, 0.5f, 8, button->color);
    DrawText(button->label, (i32)button->rect.x + button->pad, (i32)button->rect.y + button->pad, button->font_size, WHITE);
}

internal b32
LabelButtonClicked(LabelButton *button, Vector2 mouse_pos)
{
    button->color = RED;
    if (CheckCollisionPointRec(mouse_pos, button->rect)) {
        button->color = GREEN;
        return IsMouseButtonPressed(0);
    }
    return false;
}
