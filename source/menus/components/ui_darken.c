#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"

void ui_darken_reset_opacity(UIElement* e){
    C2D_PlainImageTint(&e->darken.tint, C2D_Color32f(0, 0, 0, e->opacity), 1.0f);
}

static void ui_darken_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;

    if(!e->darken.darkenOver){
        e->opacity = (e->darken.darkenTimeElapsed / e->darken.darkenTime) * e->darken.targetOpacity;
        e->darken.darkenTimeElapsed += 1.f / 60.f;
    }

    if(e->darken.darkenTimeElapsed > e->darken.darkenTime && !e->darken.darkenOver){
        e->darken.darkenOver = true;
    }
}

static void ui_darken_draw(UIElement* e) {
    if(!e->darken.darkenOver){
        ui_darken_reset_opacity(e);
    }

    if (e->darken.fullScreen) {
        C2D_SpriteSetPos(&e->darken.sprite, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
        C2D_SpriteSetScale(&e->darken.sprite, SCREEN_WIDTH/16.f, SCREEN_HEIGHT/16.f);
    } else {
        C2D_SpriteSetPos(&e->darken.sprite, e->x, e->y);
        C2D_SpriteSetScale(&e->darken.sprite, e->w/16.f, e->h/16.f);
    }
    C2D_DrawSpriteTinted(&e->darken.sprite, &e->darken.tint);
}

UIElement ui_create_darken(float x, float y, float width, float height, float opacity, float darkenTime, bool fullScreen, char (*tag)[TAG_LENGTH]) {
    UIElement e = {0};

    e.type = UI_DARKEN;
    e.x = x;
    e.y = y;
    e.w = width;
    e.h = height;
    e.enabled = true;
    e.opacity = 0.0f;
    e.darken.fullScreen = fullScreen;

    if (darkenTime <= 0.f) {
        ui_darken_reset_opacity(&e);
        
        e.opacity = opacity;
        e.darken.darkenOver = true;
    } else {
        e.darken.darkenTime = darkenTime;
        e.darken.darkenTimeElapsed = 0.f;
        e.darken.darkenOver = false;
        e.darken.targetOpacity = opacity;
    }

    // Copy tag
    copy_tag_array(&e, tag);

    C2D_PlainImageTint(&e.darken.tint, C2D_Color32f(0, 0, 0, opacity), 1.0f);
    C2D_SpriteFromSheet(&e.darken.sprite, ui_sheet, 416);
    C2D_SpriteSetCenter(&e.darken.sprite, 0.5f, 0.5f);

    e.update = ui_darken_update;
    e.draw = ui_darken_draw;

    return e;
}