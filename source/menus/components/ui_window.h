#pragma once
#include "ui_element.h"
void ui_window_set_tint(UIWindow* e, u32 color);
UIWindow *ui_create_window(
    int x, int y, int w, int h, int style,
    char (*tag)[TAG_LENGTH]
);