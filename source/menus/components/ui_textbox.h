#pragma once
#include "ui_element.h"
#define TEXTBOX_STYLE 2
#define TEXTBOX_MARGIN 10
UITextbox *ui_create_textbox(
    int x, int y, int w, int limit, char *title,
    char (*tag)[TAG_LENGTH]
);