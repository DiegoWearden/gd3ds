#pragma once
#include "ui_element.h"

void ui_label_set_text(UILabel *e, const char *text);
void ui_label_set_scale_from_width(UILabel *e, const char *text, float width);
UILabel *ui_create_label(int x, int y, float scale, char *text, int font, float alignment, bool parse_tags, char (*tag)[TAG_LENGTH]);