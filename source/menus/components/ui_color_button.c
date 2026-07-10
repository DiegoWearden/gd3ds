#include "menus/components/ui_button.h"
#include "menus/core/ui_element.h"
#include <citro2d.h>
#include "ui_color_button.h"
#include "menus/core/ui_screen.h"
#include "menus/core/ui_props.h"
#include "graphics.h"

#include "menus/palette_kit.h"

#include "main.h"
#include "ui_slider.h"

static void ui_color_button_update(UIElement* e, UIInput* touch) {
    ui_button_update(e, touch);
}

static void ui_color_button_draw(UIElement* e) {
    UIColor *color = (UIColor *) e;
    UIButton *button = (UIButton *) e;

    float scale = button->hoverScale;

    int color_idx = color->color_index;

    if (color_idx >= NUM_COLORS) color_idx = NUM_COLORS - 1;

    C2D_ImageTint tint = { 0 };
    C2D_PlainImageTint(&tint, colors[color_idx], 1.0f);
    
    C2D_SpriteSetCenter(&button->image.sprite, 0.5f, 0.5f);
    C2D_SpriteSetPos(&button->image.sprite, e->x, e->y);
    C2D_SpriteSetScale(&button->image.sprite, button->base.scaleX * scale, button->base.scaleY * scale);
    C2D_DrawSpriteTinted(&button->image.sprite, &tint);

    if (color->isSelected) {
        C2D_SpriteSetCenter(&color->image.sprite, 0.5f, 0.5f);
        C2D_SpriteSetPos(&color->image.sprite, e->x, e->y);
        C2D_SpriteSetScale(&color->image.sprite, button->base.scaleX + 0.05f, button->base.scaleY + 0.05f);
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

UIColor *ui_create_color_button(const UIContext *ctx) {
    UIColor *e = malloc(sizeof(UIColor));

    if (!e) return NULL;
    
    UIButton *button = (UIButton *) e;

    memset(e, 0, sizeof(UIColor));
    button->base.type = UI_COLOR_BUTTON;
    button->base.enabled = true;

    button->base.update = ui_color_button_update;
    button->base.draw = ui_color_button_draw;
    button->base.destroy = ui_color_button_destroy;
    
    ui_element_apply_default_properties(&button->base, ctx);
    
    button->hoverScale = 1.f;

    C2D_SpriteFromSheet(&e->image.sprite, ui_sheet, 175);
    C3D_TexSetFilter(e->image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
    
    C2D_SpriteFromSheet(&button->image.sprite, ui_sheet, 35);
    C3D_TexSetFilter(button->image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

    return e;
}

UIElement *ui_create_color_button_from_props(const UIContext *ctx, const UIPropertyList *props) {
    UIColor *color_button = ui_create_color_button(ctx);

    if (!color_button) return NULL;
    
    UIButton *button = (UIButton *) color_button;

    ui_element_apply_properties(&button->base, ctx, props);
    
    ui_element_set_size(&button->base, 30 * button->base.scaleX, 30 * button->base.scaleY);

    ui_color_button_set_index(color_button,  
        ui_prop_int(props, "id", 0), 
        ui_prop_int(props, "color_index", 0));

    return &button->base;
}