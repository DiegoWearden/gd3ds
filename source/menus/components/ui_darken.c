#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"

void ui_darken_reset_opacity(UIDarken* e){
    if (!e) return;

    C2D_PlainImageTint(&e->image.tint, C2D_Color32f(0, 0, 0, e->base.opacity), 1.0f);
}

static void ui_darken_update(UIElement* e, UIInput* touch) {
    UIDarken *darken = (UIDarken *) e;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;

    if(!darken->darkenOver){
        e->opacity = (darken->darkenTimeElapsed / darken->darkenTime) * darken->targetOpacity;
        darken->darkenTimeElapsed += 1.f / 60.f;
    }

    if(darken->darkenTimeElapsed > darken->darkenTime && !darken->darkenOver){
        darken->darkenOver = true;
    }
}

static void ui_darken_draw(UIElement* e) {
    UIDarken *darken = (UIDarken *) e;

    if(!darken->darkenOver){
        ui_darken_reset_opacity(darken);
    }

    if (darken->fullScreen) {
        C2D_SpriteSetPos(&darken->image.sprite, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
        C2D_SpriteSetScale(&darken->image.sprite, SCREEN_WIDTH/16.f, SCREEN_HEIGHT/16.f);
    } else {
        C2D_SpriteSetPos(&darken->image.sprite, e->x, e->y);
        C2D_SpriteSetScale(&darken->image.sprite, e->w/16.f, e->h/16.f);
    }
    C2D_DrawSpriteTinted(&darken->image.sprite, &darken->image.tint);
}

static void ui_darken_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

UIDarken *ui_create_darken(float x, float y, float width, float height, float opacity, float darkenTime, bool fullScreen, char (*tag)[TAG_LENGTH]) {
    UIDarken *e = malloc(sizeof(UIDarken));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIDarken));
    e->base.type = UI_DARKEN;
    e->base.x = x;
    e->base.y = y;
    e->base.w = width;
    e->base.h = height;
    e->base.enabled = true;
    e->base.opacity = 0.0f;
    e->fullScreen = fullScreen;

    if (darkenTime <= 0.f) {
        ui_darken_reset_opacity(e);
        
        e->base.opacity = opacity;
        e->darkenOver = true;
    } else {
        e->darkenTime = darkenTime;
        e->darkenTimeElapsed = 0.f;
        e->darkenOver = false;
        e->targetOpacity = opacity;
    }

    // Copy tag
    copy_tag_array(&e->base, tag);

    C2D_PlainImageTint(&e->image.tint, C2D_Color32f(0, 0, 0, opacity), 1.0f);
    C2D_SpriteFromSheet(&e->image.sprite, ui_sheet, 416);
    C2D_SpriteSetCenter(&e->image.sprite, 0.5f, 0.5f);

    e->base.update = ui_darken_update;
    e->base.draw = ui_darken_draw;
    e->base.destroy = ui_darken_destroy;

    return e;
}