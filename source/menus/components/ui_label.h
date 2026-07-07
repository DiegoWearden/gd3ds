#pragma once
#include "ui_element.h"

void ui_label_set_text(UIElement *e, const char *text);
void ui_label_set_scale_from_width(UIElement *e, const char *text, float width);
UIElement ui_create_label(int x, int y, float scale, char *text, int font, float alignment, bool parse_tags, char (*tag)[TAG_LENGTH]);