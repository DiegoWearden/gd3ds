#pragma once
#include "menus/core/ui_element.h"
#include "menus/core/ui_screen.h"


#define BUTTON_HOVER_SCALE 1.25f
#define BUTTON_HOVER_ANIM_TIME 0.4f

void ui_button_draw_text(UIElement *e);
void ui_button_update(UIElement* e, UIInput* touch);
void ui_button_set_image(UIButton *e, int sprite_index, int sheet);
UIButton *ui_create_button(const UIContext *ctx);
UIElement *ui_create_button_from_props(const UIContext *ctx, const UIPropertyList *props);