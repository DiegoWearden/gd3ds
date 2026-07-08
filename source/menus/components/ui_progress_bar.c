#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"

static void ui_progress_bar_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;
}

static void draw_frame(UIProgressBar *e) {
    C2D_SpriteSetCenter(&e->frame.sprite, 0.5f, 0.5f);
    C2D_SpriteSetPos(&e->frame.sprite, e->base.x, e->base.y);
    C2D_SpriteSetScale(&e->frame.sprite, e->scale, e->scale);
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

        
        float x = e->base.x - e->base.w / 2;
        if (e->style == 0) {
            x += 0.5f;
        }

        sub = select_box(&e->bar.sprite.image, 0, 0, pixels, e->bar.sprite.image.subtex->height);
        img = e->bar.sprite.image; img.subtex = &sub;
        C2D_SpriteFromImage(&spr, img);
        C2D_SpriteSetCenter(&spr, 0.f, 0.5f);
        C2D_SpriteSetPos(&spr, x, e->base.y);
        C2D_SpriteSetScale(&spr, e->fill_scaleX, e->fill_scaleY);
        if (e->useTint) {
            C2D_DrawSpriteTinted(&spr, &e->bar.tint);
        } else {
            C2D_DrawSprite(&spr);
        }
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
    e->useTint = true;
}

void ui_progress_bar_clear_tint(UIProgressBar* e) {
    if (!e) return;
    
    e->useTint = false;
}

void ui_progress_bar_set_images(UIProgressBar *e, int style, float scale) {
    switch (style) {
        case 0:
            C2D_SpriteFromSheet(&e->bar.sprite, bar_sheet, 0);
            C3D_TexSetFilter(e->bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->frame.tint, C2D_Color32(0, 0, 0, 127), 1.0f);

            e->frame.sprite = e->bar.sprite;

            e->base.w = e->bar.sprite.image.subtex->width * scale;
            e->base.h = e->bar.sprite.image.subtex->height * scale;

            e->fill_scaleX = scale * ((float)(e->base.w - 2) / (float)e->base.w);
            e->fill_scaleY = scale * ((float)(e->base.h - 2) / (float)e->base.h);
            e->flip_order = false;
            break;
        case 1:
            C2D_SpriteFromSheet(&e->bar.sprite, bar_sheet, 3);
            C3D_TexSetFilter(e->bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

            C2D_SpriteFromSheet(&e->frame.sprite, bar_sheet, 2);
            C3D_TexSetFilter(e->frame.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->frame.tint, C2D_Color32(255, 255, 255, 255), 1.0f);

            e->base.w = e->bar.sprite.image.subtex->width * scale;
            e->base.h = e->bar.sprite.image.subtex->height * scale;

            e->flip_order = true;
            break;
        case 2:
            C2D_SpriteFromSheet(&e->bar.sprite, bar_sheet, 3);
            C3D_TexSetFilter(e->bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

            C2D_SpriteFromSheet(&e->frame.sprite, bar_sheet, 1);
            C3D_TexSetFilter(e->frame.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->frame.tint, C2D_Color32(255, 255, 255, 255), 1.0f);

            e->base.w = e->bar.sprite.image.subtex->width * scale;
            e->base.h = e->bar.sprite.image.subtex->height * scale;

            e->flip_order = true;
            break;
    }
}

UIProgressBar *ui_create_progress_bar(int x, int y, int style, float scale, float max_value, char (*tag)[TAG_LENGTH]) {
    UIProgressBar *e = malloc(sizeof(UIProgressBar));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIProgressBar));
    e->base.type = UI_PROGRESS_BAR;
    e->base.x = x;
    e->base.y = y;
    e->base.enabled = true;
    e->useTint = false;

    e->style = style;

    // Copy tag
    copy_tag_array(&e->base, tag);

    e->scale = scale;
    e->fill_scaleX = scale;
    e->fill_scaleY = scale;

    ui_progress_bar_set_images(e, style, scale);

    // Please no divisions by zero, thanks
    e->max_value = (max_value == 0 ? 100 : max_value);
    e->value = 0;

    e->base.update = ui_progress_bar_update;
    e->base.draw = ui_progress_bar_draw;
    e->base.destroy = ui_progress_bar_destroy;

    return e;
}