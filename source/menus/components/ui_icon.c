#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "ui_icon.h"
#include "easing.h"
#include "math_helpers.h"
#include "ui_screen.h"
#include "graphics.h"

#include "menus/icon_kit.h"

#include "main.h"
#include "ui_slider.h"

#define FIRST_TRAIL_ID 27

static void ui_icon_update(UIElement* e, UIInput* touch) {
    UIIcon *icon = (UIIcon *) e;

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);

    // Check if pressed the icon
    if (inside && pressedTouch && !touch->did_something) {
        icon->hovered = true;
        icon->pressed = true;
    }

    // If previously pressed on it, hover
    if (inside && !sliding) {
        icon->hovered = true;
    }
    
    EaseTypes bounce_type;
    // Animation
    if (icon->hovered) {
        icon->hoverTimer += DT;
        bounce_type = BOUNCE_OUT;
    } else {
        icon->hoverTimer -= DT;
        // As the animation plays in reverse, we just use bounce in
        bounce_type = BOUNCE_IN;
    }

    icon->hoverTimer = clampf(icon->hoverTimer, 0.f, ICON_HOVER_ANIM_TIME);
    icon->hoverScale = easeValue(bounce_type, 1.0f, ICON_HOVER_SCALE, icon->hoverTimer, ICON_HOVER_ANIM_TIME, 0);


    // If released on icon, do its action
    if (icon->hovered && releasedTouch) {
        icon->pressed = false;
        icon->hovered = false;
        icon->hoverTimer = 0.f;
        icon->hoverScale = 1.f;
        if (e->action)
            e->action(e);
    }
    
    // Unpress the icon
    if (!inside) {
        icon->hovered = false;
    }
    
    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

static void ui_icon_draw(UIElement* e) {
    UIIcon *icon = (UIIcon *) e;

    float scale = icon->hoverScale;

    float y = e->y;
    if (icon->gamemode == GAMEMODE_SHIP) y -= 4;

    if (icon->gamemode == TRAIL) {
        C2D_Sprite spr = { 0 };
        C2D_SpriteFromSheet(&spr, ui_2_sheet, FIRST_TRAIL_ID + icon->index);
        C3D_TexSetFilter(spr.image.tex, GPU_LINEAR, GPU_LINEAR);
        C2D_SpriteSetCenter(&spr, 0.5f, 0.5f);
        C2D_SpriteSetPos(&spr, e->x, y);
        C2D_SpriteSetScale(&spr, scale * icon->scaleX, scale * icon->scaleX);
        C2D_DrawSprite(&spr);
    } else {
        spawn_icon_at(
            icon->gamemode,
            icon->index,
            false,
            e->x, y,
            0,
            0,
            0,
            scale * icon->scaleX,
            C2D_Color32(175, 175, 175, 255),
            C2D_Color32(255, 255, 255, 255),
            0
        );
    }

    if (icon->isSelected) {
        C2D_SpriteSetCenter(&icon->image.sprite, 0.5f, 0.5f);
        C2D_SpriteSetPos(&icon->image.sprite, e->x, e->y);
        C2D_SpriteSetScale(&icon->image.sprite, icon->scaleX, icon->scaleX);
        C2D_DrawSprite(&icon->image.sprite);
    }
}

static void ui_icon_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

void ui_icon_set_gamemode_index(UIIcon*e, int gamemode, int index) {
    if (!e) return;

    e->isSelected = *current_icons[gamemode] == index,
    e->gamemode = gamemode;
    e->index = index;
}

UIIcon *ui_create_icon(
    int x, int y, float scale, int index, int gamemode, 
    UIActionFn action,
    char (*tag)[TAG_LENGTH]
) {
    UIIcon *e = malloc(sizeof(UIIcon));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIIcon));
    e->base.type = UI_ICON;
    e->base.x = x;
    e->base.y = y;
    e->base.w = 30*scale;
    e->base.h = 30*scale;
    e->base.enabled = true;
    e->base.action = action;
    e->base.update = ui_icon_update;
    e->base.draw = ui_icon_draw;
    e->base.destroy = ui_icon_destroy;

    // Copy tag
    copy_tag_array(&e->base, tag);

    e->scaleX = scale;

    C2D_SpriteFromSheet(&e->image.sprite, ui_sheet, 175);
    C3D_TexSetFilter(e->image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

    ui_icon_set_gamemode_index(e, gamemode, index);
    
    e->hoverScale = 1.f;

    return e;
}