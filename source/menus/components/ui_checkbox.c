#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "easing.h"
#include "math_helpers.h"
#include "ui_checkbox.h"
#include "ui_screen.h"

#include "main.h"

static void set_checkbox_texture(UICheckBox* e, bool enabled) {
    if (!e) return;

    int tex = enabled ? 28 : 27;
    C2D_SpriteFromSheet(&e->image.sprite, ui_sheet, tex);
    C3D_TexSetFilter(e->image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

    e->base.w = fabsf(e->image.sprite.image.subtex->width * e->scaleX);
    e->base.h = fabsf(e->image.sprite.image.subtex->height * e->scaleY);
}

// Set checkbox checked state
void set_checkbox_enabled(UICheckBox *e, bool enabled) {
    if (!e) return;

    set_checkbox_texture(e, enabled);
    e->checked = enabled;
}


static void ui_checkbox_update(UIElement* e, UIInput* touch) {
    UICheckBox *checkbox = (UICheckBox *) e;

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);

    // Check if pressed the checkbox
    if (inside && pressedTouch && !touch->did_something) {
        checkbox->hovered = true;
        checkbox->pressed = true;
    }

    // If previously pressed on it, hover
    if (inside && checkbox->pressed) {
        checkbox->hovered = true;
    }
    
    EaseTypes bounce_type;
    // Animation
    if (checkbox->hovered) {
        checkbox->hoverTimer += DT;
        bounce_type = BOUNCE_OUT;
    } else {
        checkbox->hoverTimer -= DT;
        // As the animation plays in reverse, we just use bounce in
        bounce_type = BOUNCE_IN;
    }

    checkbox->hoverTimer = clampf(checkbox->hoverTimer, 0.f, CHECKBOX_HOVER_ANIM_TIME);
    
    checkbox->hoverScale = easeValue(bounce_type, 1.0f, CHECKBOX_HOVER_SCALE, checkbox->hoverTimer, CHECKBOX_HOVER_ANIM_TIME, 0);

    // If released on checkbox, do its action
    if (checkbox->hovered && releasedTouch) {
        checkbox->checked ^= 1;
        checkbox->pressed = false;
        checkbox->hovered = false;
        checkbox->hoverTimer = 0.f;
        checkbox->hoverScale = 1.f;
        if (e->action)
            e->action(e);
    }
    
    // Unpress the checkbox
    if (!inside) {
        checkbox->hovered = false;
    }
                      
    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }

    if (checkbox->checked != checkbox->image_id) {
        set_checkbox_texture(checkbox, checkbox->checked);
        checkbox->image_id = checkbox->checked;
    }
}

static void ui_checkbox_draw(UIElement* e) {
    UICheckBox *checkbox = (UICheckBox *) e;

    float scale = checkbox->hoverScale;

    C2D_SpriteSetCenter(&checkbox->image.sprite, 0.5f, 0.5f);
    C2D_SpriteSetPos(&checkbox->image.sprite, e->x, e->y);
    C2D_SpriteSetScale(&checkbox->image.sprite, scale * checkbox->scaleX, scale * checkbox->scaleY);
    C2D_DrawSprite(&checkbox->image.sprite);
}

static void ui_checkbox_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}



static void ui_checkbox_on_disable(UIElement *e) {
    UICheckBox *checkbox = (UICheckBox *) e;
    checkbox->hovered = false;
    checkbox->hoverScale = 1.f;
    checkbox->hoverTimer = 0.f;
}

UICheckBox *ui_create_checkbox(
    int x, int y, float sx, float sy, bool enabled,
    UIActionFn action,
    char (*tag)[TAG_LENGTH]
) {
    UICheckBox *e = malloc(sizeof(UICheckBox));

    if (!e) return NULL;
    
    memset(e, 0, sizeof(UICheckBox));
    e->base.type = UI_CHECKBOX;
    e->base.x = x;
    e->base.y = y;
    e->base.w = 0;
    e->base.h = 0;
    e->base.enabled = true;
    e->base.action = action;
    e->base.update = ui_checkbox_update;
    e->base.draw = ui_checkbox_draw;
    e->base.destroy = ui_checkbox_destroy;

    e->base.on_disable = ui_checkbox_on_disable;

    // Copy tag
    copy_tag_array(&e->base, tag);

    e->scaleX = sx;
    e->scaleY = sy;

    set_checkbox_texture(e, enabled);

    return e;
}