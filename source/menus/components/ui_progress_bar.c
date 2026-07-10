#include "menus/core/ui_element.h"
#include <citro2d.h>
#include "menus/core/ui_screen.h"
#include "menus/core/ui_props.h"

static void ui_progress_bar_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;
}

static void draw_frame(UIProgressBar *e) {
    C2D_SpriteSetCenter(&e->frame.sprite, 0.5f, 0.5f);
    C2D_SpriteSetPos(&e->frame.sprite, e->base.x, e->base.y);
    C2D_SpriteSetScale(&e->frame.sprite, e->base.scaleX, e->base.scaleY);
    C2D_DrawSpriteTinted(&e->frame.sprite, &e->frame.tint);
}

static void draw_bar(UIProgressBar *e) {
    float bar_width = e->bar.sprite.image.subtex->width;
    
    int pixels = (e->value / e->max_value) * bar_width;

    if (pixels > 0) {
        if (pixels > bar_width) {
            pixels = bar_width;
        }

        C2D_Sprite spr;
        Tex3DS_SubTexture sub;
        C2D_Image img;


        float sx = e->base.scaleX;
        float sy = e->base.scaleY;

        if (e->style == 0) {
            // Add 1 pixel of margin
            sx *= ((float)(e->base.w - 2) / (float)e->base.w);
            sy *= ((float)(e->base.h - 2) / (float)e->base.h);
        }
        
        float x = e->base.x - e->base.w / 2;
        if (e->style == 0) {
            x += 0.5f;
        }

        sub = select_box(&e->bar.sprite.image, 0, 0, pixels, e->bar.sprite.image.subtex->height);
        img = e->bar.sprite.image; img.subtex = &sub;
        C2D_SpriteFromImage(&spr, img);
        C2D_SpriteSetCenter(&spr, 0.f, 0.5f);
        C2D_SpriteSetPos(&spr, x, e->base.y);
        C2D_SpriteSetScale(&spr, sx, sy);
        C2D_DrawSpriteTinted(&spr, &e->bar.tint);
    }
}

static void ui_progress_bar_draw(UIElement* e) {
    UIProgressBar *progress_bar = (UIProgressBar *) e;
    if (progress_bar->flip_order) {
        draw_bar(progress_bar);
        draw_frame(progress_bar);
    } else {
        draw_frame(progress_bar);
        draw_bar(progress_bar);
    }
}

static void ui_progress_bar_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

void ui_progress_bar_set_tint(UIProgressBar* e, u32 color) {
    if (!e) return;

    C2D_PlainImageTint(&e->bar.tint, color, 1.0f);
}

void ui_progress_bar_clear_tint(UIProgressBar* e) {
    if (!e) return;

    C2D_PlainImageTint(&e->bar.tint, 0xffffffff, 1.0f);
}

void ui_progress_bar_set_images(UIProgressBar *e, int style) {
    switch (style) {
        case 0:
            C2D_SpriteFromSheet(&e->bar.sprite, bar_sheet, 0);
            C3D_TexSetFilter(e->bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->frame.tint, C2D_Color32(0, 0, 0, 127), 1.0f);

            e->frame.sprite = e->bar.sprite;

            e->base.w = e->bar.sprite.image.subtex->width * e->base.scaleX;
            e->base.h = e->bar.sprite.image.subtex->height * e->base.scaleY;

            e->flip_order = false;
            break;
        case 1:
            C2D_SpriteFromSheet(&e->bar.sprite, bar_sheet, 3);
            C3D_TexSetFilter(e->bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

            C2D_SpriteFromSheet(&e->frame.sprite, bar_sheet, 2);
            C3D_TexSetFilter(e->frame.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->frame.tint, C2D_Color32(255, 255, 255, 255), 1.0f);

            e->base.w = e->bar.sprite.image.subtex->width * e->base.scaleX;
            e->base.h = e->bar.sprite.image.subtex->height * e->base.scaleY;

            e->flip_order = true;
            break;
        case 2:
            C2D_SpriteFromSheet(&e->bar.sprite, bar_sheet, 3);
            C3D_TexSetFilter(e->bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

            C2D_SpriteFromSheet(&e->frame.sprite, bar_sheet, 1);
            C3D_TexSetFilter(e->frame.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->frame.tint, C2D_Color32(255, 255, 255, 255), 1.0f);

            e->base.w = e->bar.sprite.image.subtex->width * e->base.scaleX;
            e->base.h = e->bar.sprite.image.subtex->height * e->base.scaleY;

            e->flip_order = true;
            break;
    }
}

UIProgressBar *ui_create_progress_bar(const UIContext *ctx) {
    UIProgressBar *e = malloc(sizeof(UIProgressBar));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIProgressBar));
    e->base.type = UI_PROGRESS_BAR;
    e->base.enabled = true;
    e->useTint = false;

    ui_element_apply_default_properties(&e->base, ctx);

    e->value = 0;
    e->max_value = 100;

    e->base.update = ui_progress_bar_update;
    e->base.draw = ui_progress_bar_draw;
    e->base.destroy = ui_progress_bar_destroy;

    return e;
}

UIElement *ui_create_progress_bar_from_props(const UIContext *ctx, const UIPropertyList *props) {
    UIProgressBar *progress_bar = ui_create_progress_bar(ctx);

    if (!progress_bar) return NULL;

    ui_element_apply_properties(&progress_bar->base, ctx, props);
    
    progress_bar->style = ui_prop_int(props, "style", 0);

    progress_bar->max_value = ui_prop_float(props, "max_value", 100);

    // Please no divisions by zero, thanks
    if (progress_bar->max_value == 0) {
        progress_bar->max_value = 100;
    }

    ui_progress_bar_set_images(progress_bar, progress_bar->style);

    ui_progress_bar_set_tint(progress_bar, C2D_Color32(
        ui_prop_int(props, "r", 255), 
        ui_prop_int(props, "g", 255), 
        ui_prop_int(props, "b", 255), 
        ui_prop_int(props, "a", 255)
    ));
    
    return &progress_bar->base;
}