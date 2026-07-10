#pragma once
#include "menus/core/ui_element.h"
#include "menus/core/ui_screen.h"

UIActionArea *ui_create_action_area(const UIContext *ctx);
UIElement *ui_create_action_area_from_props(const UIContext *ctx, const UIPropertyList *props);