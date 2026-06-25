#pragma once

#include "ui_element.h"

UIElement ui_create_slider(int x, int y, float scale, float max, char (*tag)[TAG_LENGTH]);
float ui_slider_get_percent(UIElement* e);
void ui_slider_set_value(UIElement *e, float value);