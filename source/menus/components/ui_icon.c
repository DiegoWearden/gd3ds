#include "menus/components/ui_button.h"
#include "menus/core/common_setters.h"
#include "menus/core/ui_element.h"
#include <citro2d.h>
#include "ui_icon.h"
#include "menus/core/ui_screen.h"
#include "menus/core/ui_props.h"
#include "graphics.h"

#include "menus/icon_kit.h"

#define FIRST_TRAIL_ID 27

static void ui_icon_update(UIElement* e, UIInput* touch) {
    ui_button_update(e, touch);
}

static void ui_icon_draw(UIElement* e) {
    UIIcon *icon = (UIIcon *) e;
    UIButton *button = (UIButton *) e;

    float scale = button->hoverScale;

    float y = e->y;
    if (icon->gamemode == GAMEMODE_SHIP) y -= 4;

    if (icon->gamemode == TRAIL) {
        C2D_Sprite spr = { 0 };
        C2D_SpriteFromSheet(&spr, ui_2_sheet, FIRST_TRAIL_ID + icon->index);
        C3D_TexSetFilter(spr.image.tex, GPU_LINEAR, GPU_LINEAR);
        C2D_SpriteSetCenter(&spr, 0.5f, 0.5f);
        C2D_SpriteSetPos(&spr, e->x, y);
        C2D_SpriteSetScale(&spr, scale * button->base.scaleX, scale * button->base.scaleY);
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
            scale * button->base.scaleX,
            C2D_Color32(175, 175, 175, 255),
            C2D_Color32(255, 255, 255, 255),
            0
        );
    }

    if (icon->isSelected) {
        C2D_SpriteSetCenter(&button->image.sprite, 0.5f, 0.5f);
        C2D_SpriteSetPos(&button->image.sprite, e->x, e->y);
        C2D_SpriteSetScale(&button->image.sprite, button->base.scaleX, button->base.scaleY);
        C2D_DrawSprite(&button->image.sprite);
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

UIIcon *ui_create_icon(const UIContext *ctx) {
    UIIcon *e = malloc(sizeof(UIIcon));

    if (!e) return NULL;

    UIButton *button = (UIButton *) e;

    memset(e, 0, sizeof(UIIcon));
    button->base.type = UI_ICON;
    button->base.enabled = true;

    button->base.update = ui_icon_update;
    button->base.draw = ui_icon_draw;
    button->base.destroy = ui_icon_destroy;

    ui_element_apply_default_properties(&button->base, ctx);
    
    button->hoverScale = 1.f;

    C2D_SpriteFromSheet(&button->image.sprite, ui_sheet, 175);
    C3D_TexSetFilter(button->image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

    return e;
}

UIElement *ui_create_icon_from_props(const UIContext *ctx, const UIPropertyList *props) {
    UIIcon *icon = ui_create_icon(ctx);

    if (!icon) return NULL;

    UIButton *button = (UIButton *) icon;

    ui_element_apply_properties(&button->base, ctx, props);

    ui_element_set_size(&button->base, 30 * button->base.scaleX, 30 * button->base.scaleY);
        
    ui_icon_set_gamemode_index(icon, 
        ui_prop_int(props, "gamemode", 0), 
        ui_prop_int(props, "id", 0));

    return &button->base;
}