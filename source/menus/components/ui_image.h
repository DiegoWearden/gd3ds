#pragma once
#include "ui_element.h"

void ui_image_set_tint(UIImage* e, u32 color);
void ui_image_clear_tint(UIImage* e);
void ui_image_set_image(UIImage *e, int sprite_index, int sheet);
UIImage *ui_create_image(int x, int y, int sprite_index, int sheet, float sx, float sy, char (*tag)[TAG_LENGTH]);