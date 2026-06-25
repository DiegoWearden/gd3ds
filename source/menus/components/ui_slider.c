#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"
#include "ui_slider.h"

static void draw_button(UIElement *e) {
    float left_side = e->x - e->w / 2;
    
    float percent = ui_slider_get_percent(e);
    int button_x = left_side + (int)(percent * e->w);
    
    C2D_SpriteFromSheet(&e->slider.button, bar_sheet, 4 + e->slider.dragging);
    C3D_TexSetFilter(e->slider.button.image.tex, GPU_LINEAR, GPU_LINEAR);
    C2D_SpriteSetCenter(&e->slider.button, 0.5f, 0.5f);
    C2D_SpriteSetPos(&e->slider.button, button_x, e->y);
    C2D_SpriteSetScale(&e->slider.button, e->slider.scale, e->slider.scale);
    C2D_DrawSprite(&e->slider.button);
}

static void draw_frame(UIElement *e) {
    C2D_SpriteSetCenter(&e->slider.track_frame, 0.5f, 0.5f);
    C2D_SpriteSetPos(&e->slider.track_frame, e->x, e->y);
    C2D_SpriteSetScale(&e->slider.track_frame, e->slider.scale, e->slider.scale);
    C2D_DrawSprite(&e->slider.track_frame);
}

static void draw_bar(UIElement *e) {
    float bar_width = e->slider.track.image.subtex->width;
    
    int pixels = (e->slider.value / e->slider.max_value) * bar_width;

    if (pixels > 0) {
        if (pixels > bar_width) {
            pixels = bar_width;
        }

        C2D_Sprite spr;
        Tex3DS_SubTexture sub;
        C2D_Image img;
        C2D_ImageTint tint;
        
        C2D_PlainImageTint(&tint, C2D_Color32(50, 190, 240, 255), 1.f);

        sub = select_box(&e->slider.track.image, 0, 0, pixels, e->slider.track.image.subtex->height);
        img = e->slider.track.image; img.subtex = &sub;
        C2D_SpriteFromImage(&spr, img);
        C2D_SpriteSetCenter(&spr, 0.f, 0.5f);
        C2D_SpriteSetPos(&spr, e->x - e->w / 2, e->y);
        C2D_SpriteSetScale(&spr, e->slider.scale, e->slider.scale);
        C2D_DrawSpriteTinted(&spr, &tint);
    }
}

static float slider_button_x(UIElement* e) {
    UISliderData* s = &e->slider;

    float left_side = e->x - e->w / 2;

    float t = s->value / s->max_value;

    return left_side + (t * e->w);
}

static void slider_set_from_x(UIElement* e, float px) {
    UISliderData* s = &e->slider;

    float left_side = e->x - e->w / 2;

    float t = (float)(px - left_side) / e->w;

    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;

    s->value = t * s->max_value;
}

static bool slider_touching_button(UIElement* e, UIInput* touch) {
    int button_x = slider_button_x(e);

    float width = e->slider.button.image.subtex->width;
    float height = e->slider.button.image.subtex->height;

    return touch->touchPosition.px >= button_x - width / 2 &&
           touch->touchPosition.px <= button_x + width / 2 &&
           touch->touchPosition.py >= e->y - height / 2 &&
           touch->touchPosition.py <= e->y + height / 2;
}

void ui_slider_set_value(UIElement *e, float value) {
    if (e->type != UI_SLIDER) return;

    e->slider.value = value;
}

float ui_slider_get_percent(UIElement* e) {
    if (e->type != UI_SLIDER) return 0;

    return e->slider.value / e->slider.max_value;
}

static void ui_slider_update(UIElement* e, UIInput* touch) {
    UISliderData* s = &e->slider;

    bool inside = slider_touching_button(e, touch);

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool heldTouch = hidKeysHeld() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    if (pressedTouch && inside) {
        s->dragging = true;
    }

    if (s->dragging && heldTouch) {
        slider_set_from_x(e, touch->touchPosition.px);
    }

    if (releasedTouch) {
        s->dragging = false;
    }

    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

static void ui_slider_draw(UIElement* e) {
    // Track
    draw_bar(e);
    draw_frame(e);

    // Button
    draw_button(e);
}

UIElement ui_create_slider(int x, int y, float scale, float max, char (*tag)[TAG_LENGTH]) {
    UIElement e = {0};

    e.type = UI_SLIDER;

    e.x = x;
    e.y = y;
    e.enabled = true;

    C2D_SpriteFromSheet(&e.slider.track, bar_sheet, 3);
    C3D_TexSetFilter(e.slider.track.image.tex, GPU_LINEAR, GPU_LINEAR);

    C2D_SpriteFromSheet(&e.slider.track_frame, bar_sheet, 1);
    C3D_TexSetFilter(e.slider.track_frame.image.tex, GPU_LINEAR, GPU_LINEAR);

    e.w = e.slider.track.image.subtex->width * scale;
    e.h = e.slider.track.image.subtex->height * scale;

    // Copy tag
    copy_tag_array(&e, tag);

    e.slider.scale = scale;
    e.slider.value = max;
    e.slider.max_value = max;

    e.update = ui_slider_update;
    e.draw = ui_slider_draw;

    return e;
}