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
    //Keybinds logic
    u32 validKeybinds = e->window_button.keyBinds;

    if(enableDebugBindings && game_state == STATE_GAME && !game_paused && !in_level_complete){
        validKeybinds &= ~(KEY_B | KEY_X | KEY_L | KEY_R);
    }

    if((hidKeysDown() & validKeybinds) > 0){
        e->window_button.pressed = true;
        e->window_button.hovered = true;
        e->window_button.hoverTimer = 0.2f;
        e->window_button.keyPressTimer = 45;
        pressedKey = true;
    }

    if(e->window_button.keyPressTimer > 0){
        if(e->window_button.keyPressTimer == 44){
            pressedKey = false;
            if (e->action){
                e->action(e);
            }
        }
        if(--(e->window_button.keyPressTimer) == 0){
            e->window_button.pressed = false;
            e->window_button.hovered = false;
        }
    }

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2) && 
                  !pressedKey;

    // Check if pressed the button
    if (inside && pressedTouch && !touch->did_something) {
        e->window_button.hovered = true;
        e->window_button.pressed = true;
    }

    // If previously pressed on it, hover
    if (inside && !sliding) {
        e->window_button.hovered = true;
    }
    
    // If released on button, do its action
    if (e->window_button.hovered && releasedTouch) {
        e->window_button.pressed = false;
        e->window_button.hovered = false;
        e->window_button.hoverTimer = 0.f;
        e->window_button.hoverScale = 1.f;
        if (e->action)
            e->action(e);
    }
    
    // Unpress the button
    if (!inside) {
        e->window_button.hovered = false;
    }
    
    // Mask background elements
    if (inside) {
        touch->interacted = true;
        touch->did_something = true;
    }
}

static void ui_window_button_draw(UIElement* e) {
    EaseTypes bounce_type;
    // Animation
    if (e->window_button.hovered) {
        e->window_button.hoverTimer += DT * (e->window_button.keyPressTimer > 0 ? 2 : 1);
        bounce_type = (e->window_button.keyPressTimer > 0 ? EASE_OUT : BOUNCE_OUT);
    } else {
        e->window_button.hoverTimer -= DT;
        // As the animation plays in reverse, we just use bounce in
        bounce_type = BOUNCE_IN;
    }

    e->window_button.hoverTimer = clampf(e->window_button.hoverTimer, 0.f, WINDOW_BUTTON_HOVER_ANIM_TIME);
    e->window_button.hoverScale = easeValue(bounce_type, 1.0f, WINDOW_BUTTON_HOVER_SCALE, e->window_button.hoverTimer, WINDOW_BUTTON_HOVER_ANIM_TIME, 0);

    int font_id = e->window_button.font;

    // Set to pusab if invalid
    if (font_id >= NUM_FONTS) font_id = 0;

    const LabelFont *font = &fonts[font_id];

    float scale = e->window_button.hoverScale;
    float text_scale;

    draw_9_slice(e->window_button.window.atlas, e->x, e->y, e->w * scale, e->h * scale, e->window_button.window.border, e->window_button.window.color);

    if (e->window_button.textScale == 0){
        // Get text length in pixels
        float length = get_text_length(font->charset, 1 / 0.85f, e->window_button.text);
    
        if (e->w < length) {
            text_scale = scale * (e->w / length);
        } else {
            text_scale = scale * 0.85f;
        }
    } else {
        text_scale = (e->window_button.textScale * scale);
    }
    

    draw_text(font->charset, font->sheet, e->x, e->y, text_scale, text_scale, 0.5f, "%s", e->window_button.text);
}

void ui_window_button_set_style(UIElement *e, int style) {
    if (e->type != UI_WINDOW_BUTTON) return;

    e->window_button.window.atlas = C2D_SpriteSheetGetImage(window_sheet, style);

    e->window_button.window.border = e->window_button.window.atlas.subtex->width / 3;
}

UIElement ui_create_window_button(
    int x, int y, float w, float h, int style,
    UIActionFn action,
    char *text,
    int font,
    char (*tag)[TAG_LENGTH],
    float textScale,
    u32 keyBinds
){
    UIElement e = {
        .type = UI_WINDOW_BUTTON,
        .x = x, .y = y,
        .w = w, .h = h,
        .enabled = true,
        .action = action,
        .update = ui_window_button_update,
        .draw = ui_window_button_draw
    };

    // Copy tag
    copy_tag_array(&e, tag);

    // Copy text
    strncpy(e.window_button.text, text, 63);

    e.window_button.window.color = C2D_Color32(255, 255, 255, 255);

    ui_window_button_set_style(&e, style);
    
    e.window_button.hoverScale = 1.f;

    e.window_button.font = font;
    e.window_button.textScale = textScale;

    e.window_button.keyBinds = keyBinds;

    return e;
}