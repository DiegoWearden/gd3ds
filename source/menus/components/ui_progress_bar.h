#pragma once
#include "ui_element.h"

void ui_progress_bar_set_tint(UIProgressBar* e, u32 color);
void ui_progress_bar_clear_tint(UIProgressBar* e);
UIProgressBar *ui_create_progress_bar(int x, int y, int style, float scale, float max_value, char (*tag)[TAG_LENGTH]);