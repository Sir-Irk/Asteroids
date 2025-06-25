#ifndef GUI_HEADER_INCLUDE_GUARD
#define GUI_HEADER_INCLUDE_GUARD

#include "raylib.h"
#include "si_libs/types.h"

typedef struct LabelButton
{
    const char *label;
    i32         font_size;
    i32         pad;
    Rectangle   rect;
    Color       color;
} LabelButton;

typedef struct SliderF32
{
    f32       min;
    f32       max;
    f32       handle_radius;
    f32       t;
    f32       pad;
    Rectangle rect;
    Color     color;
    Color     normal_color;
} SliderF32;

typedef struct Spinner
{
    i32       value;
    i32       min_value;
    i32       max_value;
    Rectangle rect;
    i32       button_size;
    Color     color;
    Color     normal_color;
    i32       hovered_button;
} Spinner;

#endif // GUI_HEADER_INCLUDE_GUARD
