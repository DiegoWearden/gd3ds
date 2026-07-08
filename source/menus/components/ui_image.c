#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"

static void ui_image_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;
}

static void ui_image_draw(UIElement* e) {
    UIImage *image = (UIImage *) e;
    C2D_SpriteSetCenter(&image->image.sprite, 0.5f, 0.5f);
    C2D_SpriteSetPos(&image->image.sprite, e->x, e->y);
    C2D_SpriteSetScale(&image->image.sprite, image->scaleX, image->scaleY);
    if (image->useTint) {
        C2D_DrawSpriteTinted(&image->image.sprite, &image->image.tint);
    } else {
        C2D_DrawSprite(&image->image.sprite);
    }
}

static void ui_image_destroy(UIElement* e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

void ui_image_set_tint(UIImage* e, u32 color) {
    if (!e) return;

    C2D_PlainImageTint(&e->image.tint, color, 1.0f);
    e->useTint = true;
}

void ui_image_clear_tint(UIImage* e) {
    if (!e) return;
    
    e->useTint = false;
}

void ui_image_set_image(UIImage *e, int sprite_index, int sheet) {
    if (!e) return;

    C2D_SpriteFromSheet(&e->image.sprite, *get_sheet(sheet), sprite_index);
    C3D_TexSetFilter(e->image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

    e->base.w = e->image.sprite.image.subtex->width * e->scaleX;
    e->base.h = e->image.sprite.image.subtex->height * e->scaleY;

    e->scaleX = e->scaleX;
    e->scaleY = e->scaleY;
}

UIImage *ui_create_image(int x, int y, int sprite_index, int sheet, float sx, float sy, char (*tag)[TAG_LENGTH]) {
    UIImage *e = malloc(sizeof(UIImage));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIImage));
    e->base.type = UI_IMAGE;
    e->base.x = x;
    e->base.y = y;
    e->base.enabled = true;
    e->useTint = false;

    e->scaleX = sx;
    e->scaleY = sy;

    // Copy tag
    copy_tag_array(&e->base, tag);

    ui_image_set_image(e, sprite_index, sheet);

    e->base.update = ui_image_update;
    e->base.draw = ui_image_draw;
    e->base.destroy = ui_image_destroy;

    return e;
}