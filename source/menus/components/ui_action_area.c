#include "menus/core/ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "ui_button.h"
#include "easing.h"
#include "math_helpers.h"
#include "menus/core/ui_screen.h"

static void ui_action_area_update(UIElement* e, UIInput* touch, UITransform *transform) {
    UIActionArea *area = (UIActionArea *) e;
    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = ui_element_basic_bound_check(e, touch, transform);

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

static void ui_action_area_draw(UIElement* e, UITransform *transform) {
    (void)e;
}

static void ui_action_area_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

UIActionArea *ui_create_action_area(const UIContext *ctx) {
    UIActionArea *e = malloc(sizeof(UIActionArea));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIActionArea));

    e->base.type = UI_ACTION_AREA;
    e->base.enabled = true;
    e->base.update = ui_action_area_update;
    e->base.draw = ui_action_area_draw;
    e->base.destroy = ui_action_area_destroy;
    
    ui_element_apply_default_properties(&e->base, ctx);

    return e;
}

UIElement *ui_create_action_area_from_props(const UIContext *ctx, const UIPropertyList *props) {
    UIActionArea *action_area = ui_create_action_area(ctx);

    if (!action_area) return NULL;

    ui_element_apply_properties(&action_area->base, ctx, props);

    return &action_area->base;
}