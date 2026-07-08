#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "fonts/chatFont.h"
#include "fonts/goldFont.h"
#include "ui_window_button.h"
#include "easing.h"
#include "math_helpers.h"
#include "ui_screen.h"
#include "menus/settings.h"
#include "menus/gameplay.h"
#include "state.h"

#include "main.h"
#include "ui_slider.h"

//if a button has been pressed with a keybind, no other button should be pressed after
static int pressedKey;

static void ui_window_button_update(UIElement* e, UIInput* touch) {
    UIWindowButton *window_button = (UIWindowButton *) e;

    //Keybinds logic
    u32 validKeybinds = window_button->keyBinds;

    if(enableDebugBindings && game_state == STATE_GAME && !game_paused && !in_level_complete){
        validKeybinds &= ~(KEY_B | KEY_X | KEY_L | KEY_R);
    }

    if((hidKeysDown() & validKeybinds) > 0){
        window_button->pressed = true;
        window_button->hovered = true;
        window_button->hoverTimer = 0.2f;
        window_button->keyPressTimer = 45;
        pressedKey = true;
    }

    if(window_button->keyPressTimer > 0){
        if(window_button->keyPressTimer == 44){
            pressedKey = false;
            if (e->action){
                e->action(e);
            }
        }
        if(--(window_button->keyPressTimer) == 0){
            window_button->pressed = false;
            window_button->hovered = false;
        }
    }

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2) && 
                  !pressedKey;

    // Check if pressed the button
    if (inside && pressedTouch && !touch->did_something) {
        window_button->hovered = true;
        window_button->pressed = true;
    }

    // If previously pressed on it, hover
    if (inside && !sliding) {
        window_button->hovered = true;
    }
    
    // If released on button, do its action
    if (window_button->hovered && releasedTouch) {
        window_button->pressed = false;
        window_button->hovered = false;
        window_button->hoverTimer = 0.f;
        window_button->hoverScale = 1.f;
        if (e->action)
            e->action(e);
    }
    
    // Unpress the button
    if (!inside) {
        window_button->hovered = false;
    }
    
    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

static void ui_window_button_draw(UIElement* e) {
    UIWindowButton *window_button = (UIWindowButton *) e;

    EaseTypes bounce_type;
    // Animation
    if (window_button->hovered) {
        window_button->hoverTimer += DT * (window_button->keyPressTimer > 0 ? 2 : 1);
        bounce_type = (window_button->keyPressTimer > 0 ? EASE_OUT : BOUNCE_OUT);
    } else {
        window_button->hoverTimer -= DT;
        // As the animation plays in reverse, we just use bounce in
        bounce_type = BOUNCE_IN;
    }

    window_button->hoverTimer = clampf(window_button->hoverTimer, 0.f, WINDOW_BUTTON_HOVER_ANIM_TIME);
    window_button->hoverScale = easeValue(bounce_type, 1.0f, WINDOW_BUTTON_HOVER_SCALE, window_button->hoverTimer, WINDOW_BUTTON_HOVER_ANIM_TIME, 0);

    int font_id = window_button->font;

    // Set to pusab if invalid
    if (font_id >= NUM_FONTS) font_id = 0;

    const LabelFont *font = &fonts[font_id];

    float scale = window_button->hoverScale;
    float text_scale;

    draw_9_slice(window_button->atlas, e->x, e->y, e->w * scale, e->h * scale, window_button->border, window_button->color);

    if (window_button->textScale == 0){
        // Get text length in pixels
        float length = get_text_length(font->charset, 1 / 0.85f, true, window_button->text);
    
        if (e->w < length) {
            text_scale = scale * (e->w / length);
        } else {
            text_scale = scale * 0.85f;
        }
    } else {
        text_scale = (window_button->textScale * scale);
    }

    draw_text(font->charset, font->sheet, e->x, e->y, text_scale, text_scale, 0.5f, true, "%s", window_button->text);
}

static void ui_window_button_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

void ui_window_button_set_style(UIWindowButton *e, int style) {
    if (!e) return;

    e->atlas = C2D_SpriteSheetGetImage(window_sheet, style);

    e->border = e->atlas.subtex->width / 3;
}

UIWindowButton *ui_create_window_button(
    int x, int y, float w, float h, int style,
    UIActionFn action,
    char *text,
    int font,
    char (*tag)[TAG_LENGTH],
    float textScale,
    u32 keyBinds
){
    UIWindowButton *e = malloc(sizeof(UIWindowButton));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIWindowButton));
    e->base.type = UI_WINDOW_BUTTON;
    e->base.x = x;
    e->base.y = y;
    e->base.w = w;
    e->base.h = h;
    e->base.enabled = true;
    e->base.action = action;
    e->base.update = ui_window_button_update;
    e->base.draw = ui_window_button_draw;
    e->base.destroy = ui_window_button_destroy;
    
    // Copy tag
    copy_tag_array(&e->base, tag);

    // Copy text
    strncpy(e->text, text, 63);

    e->color = C2D_Color32(255, 255, 255, 255);

    ui_window_button_set_style(e, style);
    
    e->hoverScale = 1.f;

    e->font = font;
    e->textScale = textScale;

    e->keyBinds = keyBinds;

    return e;
}