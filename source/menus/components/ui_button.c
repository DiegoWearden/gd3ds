#include "menus/core/common_setters.h"
#include "menus/core/ui_element.h"
#include <citro2d.h>
#include "text.h"
#include "ui_button.h"
#include "easing.h"
#include "math_helpers.h"
#include "menus/core/ui_screen.h"
#include "menus/settings.h"
#include "menus/core/ui_props.h"

#include "main.h"
#include "ui_slider.h"

//if a button has been pressed with a keybind, no other button should be pressed after
static int pressedKey;

void ui_button_update(UIElement* e, UIInput* touch) {
    UIButton *button = (UIButton *) e;

    //Keybinds logic
    u32 validKeybinds = button->keyBinds;

    if(enableDebugBindings && game_state == STATE_GAME && !game_paused && !in_level_complete){
        validKeybinds &= ~(KEY_B | KEY_X | KEY_L | KEY_R);
    }

    if((hidKeysDown() & validKeybinds) > 0){
        button->pressed = true;
        button->hovered = true;
        button->hoverTimer = 0.2f;
        button->keyPressTimer = 45;
        pressedKey = true;
    }

    if(button->keyPressTimer > 0){
        if(button->keyPressTimer == 44){
            pressedKey = false;
            if (e->action){
                e->action(e);
            }
        }
        if(--(button->keyPressTimer) == 0){
            button->pressed = false;
            button->hovered = false;
        }
    }

    EaseTypes bounce_type;
    // Animation
    if (button->hovered) {
        button->hoverTimer += DT * (button->keyPressTimer > 0 ? 2 : 1);
        bounce_type = (button->keyPressTimer > 0 ? EASE_OUT : BOUNCE_OUT);
    } else {
        button->hoverTimer -= DT;
        // As the animation plays in reverse, we just use bounce in
        bounce_type = BOUNCE_IN;
    }

    button->hoverTimer = clampf(button->hoverTimer, 0.f, BUTTON_HOVER_ANIM_TIME);
    button->hoverScale = easeValue(bounce_type, 1.0f, BUTTON_HOVER_SCALE, button->hoverTimer, BUTTON_HOVER_ANIM_TIME, 0);

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2) && 
                  !pressedKey;

    // Check if pressed the button
    if (inside && pressedTouch && !touch->did_something) {
        button->hovered = true;
        button->pressed = true;
    }

    // If previously pressed on it, hover
    if (inside && !sliding) {
        button->hovered = true;
    }
    
    // If released on button, do its action
    if (button->hovered && releasedTouch && !pressedKey) {
        button->pressed = false;
        button->hovered = false;
        button->hoverTimer = 0.f;
        button->hoverScale = 1.f;
        
        // This lets subclasses do something before the real action (checkbox uses it to flip the texture)
        if (button->pre_action) {
            button->pre_action(e);
        }

        if (e->action)
            e->action(e);
    }
    
    // Unpress the button
    if (!inside) {
        button->hovered = false;
    }

    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

void ui_button_draw_text(UIElement *e) {
    UIButton *button = (UIButton *) e;

    int font_id = button->font;

    // Set to pusab if invalid
    if (font_id >= NUM_FONTS) font_id = 0;

    const LabelFont *font = &fonts[font_id];

    float scale = button->hoverScale;
    float text_scale = button->textScale;

    if (button->textScale == 0){
        // Get text length in pixels
        float length = get_text_length(font->charset, 1 / 0.85f, true, button->text);
    
        if (e->w < length) {
            text_scale = scale * (e->w / length);
        } else {
            text_scale = scale * 0.85f;
        }
    } else {
        text_scale = (button->textScale * scale);
    }

    draw_text(font->charset, font->sheet, e->x, e->y, text_scale, text_scale, 0.5f, true, "%s", button->text);
}

static void ui_button_draw(UIElement* e) {
    UIButton *button = (UIButton *) e;

    C2D_ImageTint tint;
    float scale = button->hoverScale;

    C2D_PlainImageTint(&tint, C2D_Color32f(1, 1, 1, e->opacity), 1.f);

    C2D_SpriteSetCenter(&button->image.sprite, 0.5f, 0.5f);
    C2D_SpriteSetPos(&button->image.sprite, e->x, e->y);
    C2D_SpriteSetScale(&button->image.sprite, scale * e->scaleX, scale * e->scaleY);
    C2D_DrawSpriteTinted(&button->image.sprite, &tint);

    ui_button_draw_text(e);
}

static void ui_button_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

static void ui_button_on_disable(UIElement *e) {
    UIButton *button = (UIButton *) e;
    button->hovered = false;
    button->hoverScale = 1.f;
    button->hoverTimer = 0.f;
}

void ui_button_set_image(UIButton *e, int sprite_index, int sheet) {
    if (!e) return;

    C2D_SpriteFromSheet(&e->image.sprite, *get_sheet(sheet), sprite_index);
    C3D_TexSetFilter(e->image.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

    e->base.w = fabsf(e->image.sprite.image.subtex->width * e->base.scaleX);
    e->base.h = fabsf(e->image.sprite.image.subtex->height * e->base.scaleY);
}

UIButton *ui_create_button(const UIContext *ctx) {
    UIButton *e = malloc(sizeof(UIButton));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIButton));

    e->base.type = UI_BUTTON;
    e->base.enabled = true;
    e->base.update = ui_button_update;
    e->base.draw = ui_button_draw;
    e->base.destroy = ui_button_destroy;

    e->base.on_disable = ui_button_on_disable;

    ui_element_apply_default_properties(&e->base, ctx);

    e->hoverScale = 1.f;

    pressedKey = false;

    return e;
}

UIElement *ui_create_button_from_props(const UIContext *ctx, const UIPropertyList *props) {
    UIButton *button = ui_create_button(ctx);

    if (!button) return NULL;

    ui_element_apply_properties(&button->base, ctx, props);

    ui_button_set_image(button, 
        ui_prop_int(props, "id", 0),
        ui_prop_int(props, "sheet", 0)
    );
    
    // Copy text
    strncpy(button->text, ui_prop_string(props, "text", ""), 63);

    button->font = ui_prop_int(props, "font", 0);
    button->textScale = ui_prop_float(props, "textScale", 0);

    button->keyBinds = ui_prop_bitfield(props, "keyBinds", keybind_table, ARRAY_LEN(keybind_table));
    
    return &button->base;
}