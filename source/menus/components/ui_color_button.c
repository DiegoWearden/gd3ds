#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "ui_color_button.h"
#include "easing.h"
#include "math_helpers.h"
#include "ui_screen.h"
#include "graphics.h"

#include "menus/palette_kit.h"

#include "main.h"
#include "ui_slider.h"

static void ui_color_button_update(UIElement* e, UIInput* touch) {
    UIColor *color = (UIColor *) e;

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);

    // Check if pressed the color
    if (inside && pressedTouch && !touch->did_something) {
        color->hovered = true;
        color->pressed = true;
    }

    // If previously pressed on it, hover
    if (inside && !sliding) {
        color->hovered = true;
    }
    
    EaseTypes bounce_type;
    // Animation
    if (color->hovered) {
        color->hoverTimer += DT;
        bounce_type = BOUNCE_OUT;
    } else {
        color->hoverTimer -= DT;
        // As the animation plays in reverse, we just use bounce in
        bounce_type = BOUNCE_IN;
    }

    color->hoverTimer = clampf(color->hoverTimer, 0.f, COLOR_BUTTON_HOVER_ANIM_TIME);
    color->hoverScale = easeValue(bounce_type, 1.0f, COLOR_BUTTON_HOVER_SCALE, color->hoverTimer, COLOR_BUTTON_HOVER_ANIM_TIME, 0);


    // If released on color, do its action
    if (color->hovered && releasedTouch) {
        color->pressed = false;
        color->hovered = false;
        color->hoverTimer = 0.f;
        color->hoverScale = 1.f;
        if (e->action)
            e->action(e);
    }
    
    // Unpress the color
    if (!inside) {
        color->hovered = false;
    }
    
    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

static void ui_color_button_draw(UIElement* e) {
    UIColor *color = (UIColor *) e;
    float scale = color->hoverScale;

    int color_idx = color->color_index;

    if (color_idx >= NUM_COLORS) color_idx = NUM_COLORS - 1;

    C2D_ImageTint tint = { 0 };
    C2D_PlainImageTint(&tint, colors[color_idx], 1.0f);
    
    C2D_SpriteSetCenter(&color->button.sprite, 0.5f, 0.5f);
    C2D_SpriteSetPos(&color->button.sprite, e->x, e->y);
    C2D_SpriteSetScale(&color->button.sprite, color->scaleX * scale, color->scaleX * scale);
    C2D_DrawSpriteTinted(&color->button.sprite, &tint);

    if (color->isSelected) {
        C2D_SpriteSetCenter(&color->image.sprite, 0.5f, 0.5f);
        C2D_SpriteSetPos(&color->image.sprite, e->x, e->y);
        C2D_SpriteSetScale(&color->image.sprite, color->scaleX + 0.05f, color->scaleX + 0.05f);
        C2D_DrawSprite(&color->image.sprite);
    }
}

void ui_color_button_set_index(UIColor *e, int index, int color_index) {
    if (!e) return;

    e->isSelected = *current_colors[index] == color_index,
    e->index = index;
    e->color_index = color_index;
}

static void ui_color_button_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

UIColor *ui_create_color_button(
    int x, int y, float scale, int index, int color_index,
    UIActionFn action,
    char (*tag)[TAG_LENGTH]
) {
    UIColor *e = malloc(sizeof(UIColor));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIColor));
    e->base.type = UI_COLOR_BUTTON;
    e->base.x = x;
    e->base.y = y;
    e->base.w = 30*scale;
    e->base.h = 30*scale;
    e->base.enabled = true;
    e->base.action = action;
    e->base.update = ui_color_button_update;
    e->base.draw = ui_color_button_draw;
    e->base.destroy = ui_color_button_destroy;

    // Copy tag
    copy_tag_array(&e->base, tag);

    e->scaleX = scale;

    C2D_SpriteFromSheet(&e->image.sprite, ui_sheet, 175);
    C3D_TexSetFilter(e->image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
    
    C2D_SpriteFromSheet(&e->button.sprite, ui_sheet, 35);
    C3D_TexSetFilter(e->button.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

    ui_color_button_set_index(e, index, color_index);
    
    e->hoverScale = 1.f;

    return e;
}