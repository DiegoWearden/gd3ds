#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "ui_button.h"
#include "easing.h"
#include "math_helpers.h"
#include "ui_screen.h"

static void ui_action_area_update(UIElement* e, UIInput* touch) {
    UIActionArea *area = (UIActionArea *) e;
    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);

    // Check if pressed the button
    if (inside && pressedTouch && !touch->did_something) {
        area->hovered = true;
        area->pressed = true;
    }

    // If previously pressed on it, hover
    if (inside && area->pressed) {
        area->hovered = true;
    }

    // If released on button, do its action
    if (area->hovered && releasedTouch) {
        area->pressed = false;
        if (e->action)
            e->action(e);
    }
    
    // Unpress the button
    if (!inside) {
        area->hovered = false;
    }
    
    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

static void ui_action_area_draw(UIElement* e) {
    (void)e;
}

static void ui_action_area_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

UIActionArea *ui_create_action_area(
    int x, int y, float w, float h, 
    UIActionFn action,
    char (*tag)[TAG_LENGTH]
) {
    UIActionArea *e = malloc(sizeof(UIActionArea));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIActionArea));
    e->base.type = UI_ACTION_AREA;
    e->base.x = x;
    e->base.y = y;
    e->base.w = w;
    e->base.h = h;
    e->base.enabled = true;
    e->base.action = action;
    e->base.update = ui_action_area_update;
    e->base.draw = ui_action_area_draw;
    e->base.destroy = ui_action_area_destroy;

    // Copy tag
    copy_tag_array(&e->base, tag);

    return e;
}