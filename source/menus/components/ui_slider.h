#pragma once

#include "ui_element.h"
#include <stdbool.h>

extern bool sliding;

UISlider *ui_create_slider(int x, int y, float scale, float max, char (*tag)[TAG_LENGTH]);
float ui_slider_get_percent(UISlider *e);
void ui_slider_set_value(UISlider *e, float value);