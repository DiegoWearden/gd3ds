#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"
#include "ui_bg_gradient.h"

static void ui_bg_gradient_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;
}

static void ui_bg_gradient_draw(UIElement* e) {
    UIImage *image = (UIImage *) e;
    C2D_SpriteSetPos(&image->image.sprite, e->x, e->y);
    C2D_SpriteSetScale(&image->image.sprite, image->scaleX, image->scaleY);
    if (image->useTint) {
        C2D_DrawSpriteTinted(&image->image.sprite, &image->image.tint);
    } else {
        C2D_DrawSprite(&image->image.sprite);
    }
}

static void ui_bg_gradient_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

UIImage *ui_create_bg_gradient(char (*tag)[TAG_LENGTH]) {
    UIImage *e = malloc(sizeof(UIImage));

    if (!e) return NULL;

    memset(e, 0, sizeof(UI_IMAGE));
    e->base.type = UI_IMAGE;
    e->base.x = 0;
    e->base.y = 0;
    e->base.enabled = true;
    e->useTint = false;

    C2D_SpriteFromSheet(&e->image.sprite, bg_gradient_sheet, 0);

    // Copy tag
    copy_tag_array(&e->base, tag);

    e->base.w = e->image.sprite.image.subtex->width * BG_GRADIENT_XSCALE;
    e->base.h = e->image.sprite.image.subtex->height * BG_GRADIENT_YSCALE;

    e->scaleX = BG_GRADIENT_XSCALE;
    e->scaleY = BG_GRADIENT_YSCALE;

    e->base.update = ui_bg_gradient_update;
    e->base.draw = ui_bg_gradient_draw;
    e->base.destroy = ui_bg_gradient_destroy;

    return e;
}