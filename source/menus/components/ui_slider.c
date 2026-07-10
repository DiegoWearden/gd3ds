#include "menus/core/common_setters.h"
#include "menus/core/ui_element.h"
#include <citro2d.h>
#include "menus/core/ui_screen.h"
#include "menus/core/ui_props.h"
#include "ui_slider.h"

// If currently using an slider
bool sliding;

static void draw_button(UISlider *e) {
    float left_side = e->base.x - e->base.w / 2;
    
    float percent = ui_slider_get_percent(e);
    int button_x = left_side + (int)(percent * e->base.w);
    
    C2D_SpriteFromSheet(&e->button, bar_sheet, 4 + e->dragging);
    C3D_TexSetFilter(e->button.image.tex, GPU_LINEAR, GPU_LINEAR);
    C2D_SpriteSetCenter(&e->button, 0.5f, 0.5f);
    C2D_SpriteSetPos(&e->button, button_x, e->base.y);
    C2D_SpriteSetScale(&e->button, e->base.scaleX, e->base.scaleY);
    C2D_DrawSprite(&e->button);
}

static void draw_frame(UISlider *e) {
    C2D_SpriteSetCenter(&e->track_frame, 0.5f, 0.5f);
    C2D_SpriteSetPos(&e->track_frame, e->base.x, e->base.y);
    C2D_SpriteSetScale(&e->track_frame, e->base.scaleX, e->base.scaleY);
    C2D_DrawSprite(&e->track_frame);
}

static void draw_bar(UISlider *e) {
    float bar_width = e->track.image.subtex->width;
    
    int pixels = (e->value / e->max_value) * bar_width;

    if (pixels > 0) {
        if (pixels > bar_width) {
            pixels = bar_width;
        }

        C2D_Sprite spr;
        Tex3DS_SubTexture sub;
        C2D_Image img;
        C2D_ImageTint tint;
        
        C2D_PlainImageTint(&tint, C2D_Color32(50, 190, 240, 255), 1.f);

        sub = select_box(&e->track.image, 0, 0, pixels, e->track.image.subtex->height);
        img = e->track.image; img.subtex = &sub;
        C2D_SpriteFromImage(&spr, img);
        C2D_SpriteSetCenter(&spr, 0.f, 0.5f);
        C2D_SpriteSetPos(&spr, e->base.x - e->base.w / 2, e->base.y);
        C2D_SpriteSetScale(&spr, e->base.scaleX, e->base.scaleY);
        C2D_DrawSpriteTinted(&spr, &tint);
    }
}

static float slider_button_x(UISlider* e) {
    float left_side = e->base.x - e->base.w / 2;

    float t = e->value / e->max_value;

    return left_side + (t * e->base.w);
}

static void slider_set_from_x(UISlider* e, float px) {
    float left_side = e->base.x - e->base.w / 2;

    float t = (float)(px - left_side) / e->base.w;

    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;

    e->value = t * e->max_value;
}

static bool slider_touching_button(UISlider* e, UIInput* touch) {
    int button_x = slider_button_x(e);

    float width = e->button.image.subtex->width;
    float height = e->button.image.subtex->height;

    return touch->touchPosition.px >= button_x - width / 2 &&
           touch->touchPosition.px <= button_x + width / 2 &&
           touch->touchPosition.py >= e->base.y - height / 2 &&
           touch->touchPosition.py <= e->base.y + height / 2;
}

void ui_slider_set_value(UISlider *e, float value) {
    if (!e) return;

    e->value = value;
}

float ui_slider_get_percent(UISlider* e) {
    if (!e) return 0;

    return e->value / e->max_value;
}

static void ui_slider_update(UIElement* e, UIInput* touch) {
    UISlider* s = (UISlider *) e;

    bool inside = slider_touching_button(s, touch);

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool heldTouch = hidKeysHeld() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    if (pressedTouch && inside) {
        s->dragging = true;
        sliding = true;
    }

    if (s->dragging && heldTouch) {
        slider_set_from_x(s, touch->touchPosition.px);
    }

    if (releasedTouch) {
        s->dragging = false;
        sliding = false;
    }

    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

static void ui_slider_draw(UIElement* e) {
    UISlider* s = (UISlider *) e;

    // Track
    draw_bar(s);
    draw_frame(s);

    // Button
    draw_button(s);
}

static void ui_slider_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

static void ui_slider_init_graphics(UISlider *e) {
    C2D_SpriteFromSheet(&e->track, bar_sheet, 3);
    C3D_TexSetFilter(e->track.image.tex, GPU_LINEAR, GPU_LINEAR);

    C2D_SpriteFromSheet(&e->track_frame, bar_sheet, 1);
    C3D_TexSetFilter(e->track_frame.image.tex, GPU_LINEAR, GPU_LINEAR);

    e->base.w = e->track.image.subtex->width * e->base.scaleX;
    e->base.h = e->track.image.subtex->height * e->base.scaleY;
}

UISlider *ui_create_slider(const UIContext *ctx) {
    UISlider *e = malloc(sizeof(UISlider));

    if (!e) return NULL;

    memset(e, 0, sizeof(UISlider));
    e->base.type = UI_SLIDER;
    e->base.enabled = true;
    
    ui_element_apply_default_properties(&e->base, ctx);

    ui_slider_init_graphics(e);

    e->value = 1;
    e->max_value = 1;

    e->base.update = ui_slider_update;
    e->base.draw = ui_slider_draw;
    e->base.destroy = ui_slider_destroy;

    return e;
}

UIElement *ui_create_slider_from_props(const UIContext *ctx, const UIPropertyList *props) {
    UISlider *slider = ui_create_slider(ctx);

    if (!slider) return NULL;

    ui_element_apply_properties(&slider->base, ctx, props);

    slider->max_value = ui_prop_float(props, "max_value", 100);

    // Please no divisions by zero, thanks
    if (slider->max_value == 0) {
        slider->max_value = 100;
    }
    
    ui_slider_init_graphics(slider);

    return &slider->base;
}