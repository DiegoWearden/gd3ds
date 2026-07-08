#include "ui_element.h"

UIList *ui_create_list(
    int x, int y, int w, int h,
    char (*tag)[TAG_LENGTH]
);
void ui_list_add(UIList* list, UIElement* item);
void ui_list_reset(UIList *list);