#pragma once
#include "ui_element.h"

void ui_darken_reset_opacity(UIDarken *e);
UIDarken *ui_create_darken(float x, float y, float width, float height, float opacity, float darkenTime, bool fullScreen, char (*tag)[TAG_LENGTH]);