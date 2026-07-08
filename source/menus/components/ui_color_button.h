#pragma once
#include "ui_element.h"
#include "ui_button.h"

#define COLOR_BUTTON_HOVER_SCALE BUTTON_HOVER_SCALE
#define COLOR_BUTTON_HOVER_ANIM_TIME BUTTON_HOVER_ANIM_TIME

void ui_color_button_set_index(UIColor *e, int index, int color_index);
UIColor *ui_create_color_button(
    int x, int y, float scale, int index, int color_index, 
    UIActionFn action,
    char (*tag)[TAG_LENGTH]
);