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
    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);

    // Check if pressed the color
    if (inside && pressedTouch && !touch->did_something) {
        e->color.hovered = true;
        e->color.pressed = true;
    }

    // If previously pressed on it, hover
    if (inside && !sliding) {
        e->color.hovered = true;
    }
    
    EaseTypes bounce_type;
    // Animation
    if (e->color.hovered) {
        e->color.hoverTimer += DT;
        bounce_type = BOUNCE_OUT;
    } else {
        e->color.hoverTimer -= DT;
        // As the animation plays in reverse, we just use bounce in
        bounce_type = BOUNCE_IN;
    }

    e->color.hoverTimer = clampf(e->color.hoverTimer, 0.f, COLOR_BUTTON_HOVER_ANIM_TIME);
    e->color.hoverScale = easeValue(bounce_type, 1.0f, COLOR_BUTTON_HOVER_SCALE, e->color.hoverTimer, COLOR_BUTTON_HOVER_ANIM_TIME, 0);


    // If released on color, do its action
    if (e->color.hovered && releasedTouch) {
        e->color.pressed = false;
        e->color.hovered = false;
        e->color.hoverTimer = 0.f;
        e->color.hoverScale = 1.f;
        if (e->action)
            e->action(e);
    }
    
    // Unpress the color
    if (!inside) {
        e->color.hovered = false;
    }
    
    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

static void ui_color_button_draw(UIElement* e) {
    float scale = e->color.hoverScale;

    int color = e->color.color_index;

    if (color >= NUM_COLORS) color = NUM_COLORS - 1;

    C2D_ImageTint tint = { 0 };
    C2D_PlainImageTint(&tint, colors[color], 1.0f);
    
    C2D_SpriteSetCenter(&e->color.button.sprite, 0.5f, 0.5f);
    C2D_SpriteSetPos(&e->color.button.sprite, e->x, e->y);
    C2D_SpriteSetScale(&e->color.button.sprite, e->color.scaleX * scale, e->color.scaleX * scale);
    C2D_DrawSpriteTinted(&e->color.button.sprite, &tint);

    if (e->color.isSelected) {
        C2D_SpriteSetCenter(&e->color.image.sprite, 0.5f, 0.5f);
        C2D_SpriteSetPos(&e->color.image.sprite, e->x, e->y);
        C2D_SpriteSetScale(&e->color.image.sprite, e->color.scaleX + 0.05f, e->color.scaleX + 0.05f);
        C2D_DrawSprite(&e->color.image.sprite);
    }
}

void ui_color_button_set_index(UIElement *e, int index, int color_index) {
    if (e->type != UI_COLOR_BUTTON) return;

    e->color.isSelected = *current_colors[index] == color_index,
    e->color.index = index;
    e->color.color_index = color_index;
}

UIElement ui_create_color_button(
    int x, int y, float scale, int index, int color_index,
    UIActionFn action,
    char (*tag)[TAG_LENGTH]
) {
    UIElement e = {
        .type = UI_COLOR_BUTTON,
        .x = x, .y = y,
        .w = 30*scale, .h = 30*scale,
        .enabled = true,
        .action = action,
        .update = ui_color_button_update,
        .draw = ui_color_button_draw
    };

    // Copy tag
    copy_tag_array(&e, tag);

    e.color.scaleX = scale;

    C2D_SpriteFromSheet(&e.color.image.sprite, ui_sheet, 175);
    C3D_TexSetFilter(e.color.image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
    
    C2D_SpriteFromSheet(&e.color.button.sprite, ui_sheet, 35);
    C3D_TexSetFilter(e.color.button.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

    ui_color_button_set_index(&e, index, color_index);
    
    e.color.hoverScale = 1.f;

    return e;
}