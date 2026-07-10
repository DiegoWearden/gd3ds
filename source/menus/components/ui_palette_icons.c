#include "icons.h"
#include "menus/core/ui_element.h"
#include <citro2d.h>
#include "menus/core/ui_screen.h"
#include "menus/core/ui_props.h"
#include "menus/palette_kit.h"
#include "graphics.h"

#define ICON_WIDTH 30

static void ui_palette_icons_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    if (inside) touch->did_something = true;
}

static void ui_palette_icons_draw(UIElement* e) {
    UIPaletteIcons *palette_icons = (UIPaletteIcons *) e;

    bool glow_enabled = (player_glow_enabled || ((p1_color.r | p1_color.g | p1_color.b) == 0));
    
    float length = ((GAMEMODE_COUNT - 1) * (ICON_WIDTH + palette_icons->spacing)) * e->scaleX;

    float x = e->x - length * 0.5f;
    for (size_t g = 0; g < GAMEMODE_COUNT; g++) {
        float icon_x = x + (ICON_WIDTH + palette_icons->spacing) * e->scaleX * g;
        spawn_icon_at(
            g, *current_icons[g], glow_enabled, icon_x, e->y, 0, 0, 0, e->scaleX,
            C2D_Color32(p1_color.r, p1_color.g, p1_color.b, 255),
            C2D_Color32(p2_color.r, p2_color.g, p2_color.b, 255),
            C2D_Color32(glow_color.r, glow_color.g, glow_color.b, 255)
        );
    }
}

static void ui_palette_icons(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

UIPaletteIcons *ui_create_palette_icons(const UIContext *ctx) {
    UIPaletteIcons *e = malloc(sizeof(UIPaletteIcons));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIElement));
    e->base.type = UI_PALETTE_ICONS;
    e->base.enabled = true;

    e->base.update = ui_palette_icons_update;
    e->base.draw = ui_palette_icons_draw;
    e->base.destroy = ui_palette_icons;
    
    ui_element_apply_default_properties(&e->base, ctx);

    return e;
}

UIElement *ui_create_palette_icons_from_props(const UIContext *ctx, const UIPropertyList *props) {
    UIPaletteIcons *palette_icons = ui_create_palette_icons(ctx);

    if (!palette_icons) return NULL;

    ui_element_apply_properties(&palette_icons->base, ctx, props);

    palette_icons->spacing = ui_prop_float(props, "spacing", 20);

    return &palette_icons->base;
}