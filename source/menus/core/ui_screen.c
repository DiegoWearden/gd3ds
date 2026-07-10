#include "menus/core/ui_screen.h"
#include "main.h"
#include "menus/core/common_setters.h"
#include "ui_element.h"
#include "ui_screen.h"
#include "ui_props.h"

#include "menus/components/ui_button.h"
#include "menus/components/ui_image.h"
#include "menus/components/ui_label.h"
#include "menus/components/ui_checkbox.h"
#include "menus/components/ui_window.h"
#include "menus/components/ui_textbox.h"
#include "menus/components/ui_list.h"
#include "menus/components/ui_bg_gradient.h"
#include "menus/components/ui_action_area.h"
#include "menus/components/ui_darken.h"
#include "menus/components/ui_icon.h"
#include "menus/components/ui_color_button.h"
#include "menus/components/ui_window_button.h"
#include "menus/components/ui_progress_bar.h"
#include "menus/components/ui_particle.h"
#include "menus/components/ui_use_effect.h"
#include "menus/components/ui_slider.h"
#include "menus/components/ui_palette_icons.h"

#include "particles/particle_definitions.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include <citro2d.h>

#include "graphics.h"

#include "fonts/bigFont.h"
#include "fonts/chatFont.h"
#include "fonts/goldFont.h"

C2D_SpriteSheet ui_sheet;
C2D_SpriteSheet ui_2_sheet;
C2D_SpriteSheet window_sheet;
C2D_SpriteSheet bigFont_sheet;
C2D_SpriteSheet chatFont_sheet;
C2D_SpriteSheet goldFont_sheet;
C2D_SpriteSheet bg_gradient_sheet;
C2D_SpriteSheet bar_sheet;

UIScreen default_screen = {
    .isBottom = true
};
UIScreen default_screen_top;

typedef struct {
    const char *name;
    UICreateFn create;
} UIElementConstructor;

static const UIElementConstructor element_constructors[] = {
    {"button",       ui_create_button_from_props },
    {"image",        ui_create_image_from_props },
    {"label",        ui_create_label_from_props },
    {"checkbox",     ui_create_checkbox_from_props },
    {"window",       ui_create_window_from_props },
    {"textbox",      ui_create_textbox_from_props },
    {"list",         ui_create_list_from_props },
    {"bggradient",   ui_create_bg_gradient_from_props },
    {"actionarea",   ui_create_action_area_from_props },
    {"darken",       ui_create_darken_from_props },
    {"icon",         ui_create_icon_from_props },
    {"colorbutton",  ui_create_color_button_from_props },
    {"windowbutton", ui_create_window_button_from_props },
    {"progressbar",  ui_create_progress_bar_from_props },
    {"particle",     ui_create_particle_from_props },
    {"useeffect",    ui_create_use_effect_from_props },
    {"paletteicons", ui_create_palette_icons_from_props },
    {"slider",       ui_create_slider_from_props },
};

const UIBitfieldEntry keybind_table[] = {
    { "A",      KEY_A },
    { "B",      KEY_B },
    { "X",      KEY_X },
    { "Y",      KEY_Y },
    { "L",      KEY_L },
    { "R",      KEY_R },
    { "ZL",     KEY_ZL },
    { "ZR",     KEY_ZR },

    { "UP",     KEY_DUP },
    { "DOWN",   KEY_DDOWN },
    { "LEFT",   KEY_DLEFT },
    { "RIGHT",  KEY_DRIGHT },

    { "START",  KEY_START },
    { "SELECT", KEY_SELECT },

    { "CSUP",    KEY_CSTICK_UP },
    { "CSDOWN",  KEY_CSTICK_DOWN },
    { "CSLEFT",  KEY_CSTICK_LEFT },
    { "CSRIGHT", KEY_CSTICK_RIGHT },

    { "CPUP",    KEY_CPAD_UP },
    { "CPDOWN",  KEY_CPAD_DOWN },
    { "CPLEFT",  KEY_CPAD_LEFT },
    { "CPRIGHT", KEY_CPAD_RIGHT },
};

const LabelFont fonts[NUM_FONTS] = {
    {
        .charset = &bigFont_fontCharset,
        .sheet = &bigFont_sheet
    },
    {
        .charset = &chatFont_fontCharset,
        .sheet = &chatFont_sheet
    },
    {
        .charset = &goldFont_fontCharset,
        .sheet = &goldFont_sheet
    }
};

void required_loading_screen_assets_init() {
    ui_sheet = C2D_SpriteSheetLoad("romfs:/gfx/ui.t3x");
    if (!ui_sheet) svcBreak(USERBREAK_PANIC);

    ui_2_sheet = C2D_SpriteSheetLoad("romfs:/gfx/ui_2.t3x");
    if (!ui_2_sheet) svcBreak(USERBREAK_PANIC);

    goldFont_sheet = C2D_SpriteSheetLoad("romfs:/gfx/goldFont.t3x");
    if (!goldFont_sheet) svcBreak(USERBREAK_PANIC);

    bar_sheet = C2D_SpriteSheetLoad("romfs:/gfx/bars.t3x");
    if (!bar_sheet) svcBreak(USERBREAK_PANIC);

    bgSheet = C2D_SpriteSheetLoad("romfs:/gfx/bg_sheet_01.t3x");
    if (!bgSheet) svcBreak(USERBREAK_PANIC);
}

void ui_assets_init() {
    window_sheet = C2D_SpriteSheetLoad("romfs:/gfx/windows.t3x");
    if (!window_sheet) svcBreak(USERBREAK_PANIC);

    bigFont_sheet = C2D_SpriteSheetLoad("romfs:/gfx/bigFont.t3x");
    if (!bigFont_sheet) svcBreak(USERBREAK_PANIC);

    chatFont_sheet = C2D_SpriteSheetLoad("romfs:/gfx/chatFont.t3x");
    if (!chatFont_sheet) svcBreak(USERBREAK_PANIC);

    bg_gradient_sheet = C2D_SpriteSheetLoad("romfs:/gfx/bg_gradient.t3x");
    if (!bg_gradient_sheet) svcBreak(USERBREAK_PANIC);
    
    bg2Sheet = C2D_SpriteSheetLoad("romfs:/gfx/bg_sheet_02.t3x");
    if (!bg2Sheet) svcBreak(USERBREAK_PANIC);
    
    groundSheet = C2D_SpriteSheetLoad("romfs:/gfx/grounds.t3x");
    if (!groundSheet) svcBreak(USERBREAK_PANIC);
}

C2D_SpriteSheet *get_sheet(int sheet) {
    switch (sheet) {
        case 0:
            return &ui_sheet;
        case 1:
            return &ui_2_sheet;
        case 2:
            return &window_sheet;
        case 3:
            return &spriteSheet;
        case 4:
            return &spriteSheet2;
        case 5:
            return &bgSheet;
        case 6:
            return &groundSheet;
        case 7:
            return &iconSheet;
        case 8:
            return &bg2Sheet;
        case 9:
            return &bar_sheet;
    }
    return NULL;
}

void run_animation_slide(UIScreen *screen, bool go_up) {
    float fade_value = easeValue(EASE_IN_OUT, go_up ? 240 : 0, go_up ? 0 : 240, screen->open_anim_time, 0.5f, 2.0f);
    float window_y_pos = -120 + fade_value;
    
    bool found_parent = false;
    float movement_y = 0;
    for (int i = 0; i < screen->count; i++) {
        UIElement *e = screen->elements[i];

        if (!found_parent) {
            found_parent = true;
            movement_y = window_y_pos - e->y;
        }

        e->y += movement_y;
    }

    if(screen->open_anim_time >= 0.5f){
        screen->open_anim_time = 0.5f;
        screen->open_anim_done = true;
    }
}

// Update all screen characters
void ui_screen_update(UIScreen* s, UIInput* touch) {
    if (!s->loaded) return;

    if (s->open_anim_time >= 1.f) {
        s->open_anim_time = 1.f;
        s->open_anim_done = true;
    }

    if(!s->open_anim_done) s->open_anim_time += 1.f / 60.f;
    
    if(s->disable_element_update) return;

    for (int i = s->count - 1; i >= 0; i--) {
        UIElement *e = s->elements[i];
        if (e->enabled) e->update(e, touch);
    }
}

// Draw all screen characters
void ui_screen_draw(UIScreen* s) {
    if (!s->loaded) return;

    C3D_Mtx originalMat;
    C3D_Mtx newMat;
    C2D_ViewSave(&originalMat);

    float offX = 0.f;
    float offY = 0.f;

    if(!s->open_anim_done){
        int width = s->isBottom ? 320 : 400;
        int height = 240;

        if(s->open_anim == ANIM_ZOOM) {
            float scale_value = easeValue(ELASTIC_OUT, 0.f, 1.f, s->open_anim_time, 0.5f, 0.6f);

            float cx = width * 0.5f;
            float cy = height * 0.5f;

            C2D_ViewTranslate(cx, cy);
            C2D_ViewScale(scale_value, scale_value);
            C2D_ViewTranslate(-cx, -cy);
        } else if(s->open_anim == ANIM_SLIDE_RIGHT) {
            float slide_value = easeValue(ELASTIC_OUT, 0.f, 1.f, s->open_anim_time, 0.5f, 0.6f);

            offX = -(1.f - slide_value) * (width / 2.f);
        }
    }

    C2D_ViewTranslate(offX, offY);

    C2D_ViewSave(&newMat);

    for (int i = 0; i < s->count; i++) {
        UIElement *e = s->elements[i];

        if(e->type == UI_DARKEN) {
            C2D_ViewRestore(&originalMat);
        }

        if (e->enabled) e->draw(e);

        if(e->type == UI_DARKEN){
            C2D_ViewRestore(&newMat);
        }
    }

    C2D_ViewRestore(&originalMat);
}

void finish_animation(UIScreen *screen) {
    // Surely no animation lasts more than 31 years
    screen->open_anim_time = 999999999.f;
    screen->open_anim_done = true;
}

// Find an action by its name
UIActionFn ui_find_action(const UIAction* actions, size_t count, const char* name) {
    for (size_t i = 0; i < count; i++)
        if (strcmp(actions[i].name, name) == 0)
            return actions[i].fn;
    return NULL;
}

// This gets a line and converts it into a null terminated string
static void trim_newline(char* s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

// This strips a string surrounded by quotes and removes those
static void strip_quotes(char* s) {
    size_t len = strlen(s);
    // Check if first char and last char is "
    if (len >= 2 && s[0] == '"' && s[len - 1] == '"') {
        // Get all but "
        memmove(s, s + 1, len - 1);
        s[len - 2] = '\0';
    }
}

// Searches for the next token
static char* next_token(char** cursor) {
    if (!*cursor) return NULL;

    char* s = *cursor;

    // Skip leading spaces
    while (*s == ' ') s++;

    // If empty string, exit
    if (*s == '\0') {
        *cursor = NULL;
        return NULL;
    }

    char* start = s;
    bool inQuotes = false;

    // Search for quotes
    while (*s) {
        if (*s == '"') {
            inQuotes = !inQuotes;
        }
        // If not in quotes and found delimiter, no more iterating
        else if ((*s == ' ' || *s == '\n' || *s == '\r') && !inQuotes) {
            break;
        }
        s++;
    }

    // Set null character
    if (*s) {
        *s = '\0';
        *cursor = s + 1;
    } else {
        *cursor = NULL;
    }

    return start;
}

// Get element by its tag, returns NULL if there is no elements with that tag
UIElement *ui_get_element_by_tag(UIScreen *screen, const char *tag) {
    for (int i = 0; i < screen->count; i++) {
        UIElement *e = screen->elements[i];
        for (int j = 0; j < TAGS_PER_ELEMENT; j++) {
            // Check for element with this tag
            if (strcmp(e->tag[j], tag) == 0) {
                return e;
            }
        }
    }
    // No element found
    return NULL;
}

// Run a function on each element with an specific tag
void ui_run_func_on_tag(UIScreen *screen, const char *tag, void (*func)(UIElement *e)) {
    for (int i = 0; i < screen->count; i++) {
        UIElement *e = screen->elements[i];
        for (int j = 0; j < TAGS_PER_ELEMENT; j++) {
            // Check for element with this tag
            if (strcmp(e->tag[j], tag) == 0) {
                func(e);
            }
        }
    }
}

// Run a function on each element with an specific tag
void ui_set_pos_on_tag(UIScreen *screen, float x, float y, const char *tag) {
    bool found_parent = false;
    float movement_x = 0;
    float movement_y = 0;
    for (int i = 0; i < screen->count; i++) {
        for (int j = 0; j < TAGS_PER_ELEMENT; j++) {
            UIElement *e = screen->elements[i];
            // Check for element with this tag
            if (strcmp(e->tag[j], tag) == 0) {
                if (!found_parent) {
                    found_parent = true;
                    movement_x = x - e->x;
                    movement_y = y - e->y;
                }

                e->x += movement_x;
                e->y += movement_y;
            }
        }
    }
}

void ui_enable_element(UIElement *e) { 
    e->enabled = true;

    if (e->on_enable) {
        e->on_enable(e);
    }
};

void ui_disable_element(UIElement *e) { 
    e->enabled = false;
    
    if (e->on_disable) {
        e->on_disable(e);
    }
};


static void split_tags(const char *input, char tag[][TAG_LENGTH]) {
    int i = 0;
    char *token = strtok((char *) input, ",");

    while (token != NULL && i < TAGS_PER_ELEMENT)
    {
        strncpy(tag[i], token, TAG_LENGTH - 1);
        tag[i][TAG_LENGTH - 1] = '\0';   // Ensure null terminator
        i++;

        token = strtok(NULL, ",");
    }
}

void copy_tag_array(UIElement *e, const char *tags) {
    if (tags == NULL) return;

    char tag[TAGS_PER_ELEMENT][TAG_LENGTH] = { 0 };

    // Split the tags
    split_tags(tags, tag);

    memcpy(e->tag, tag, sizeof(tag));
}

void ui_element_apply_properties(UIElement *e, const UIContext *ctx, const UIPropertyList *props) {
    if (!e || !ctx || !props) return;

    ui_element_set_position(e, 
        ui_prop_int(props, "x", e->x), 
        ui_prop_int(props, "y", e->y));

    ui_element_set_size(e, 
        ui_prop_int(props, "w", e->w), 
        ui_prop_int(props, "h", e->h));
    
    float scale = ui_prop_float(props, "scale", 1.0f);
    ui_element_set_scale_xy(e,
        ui_prop_float(props, "sx", scale),
        ui_prop_float(props, "sy", scale)
    );

    // Copy tag
    copy_tag_array(e, ui_prop_string(props, "tag", ""));
    
    e->opacity = ui_prop_float(props, "opacity", 1);

    e->action = ui_find_action(
        ctx->screen->actions, 
        ctx->screen->action_count,
        ui_prop_string(props, "action", "")
    );
}

void ui_element_apply_default_properties(UIElement *e, const UIContext *ctx) {
    if (!e || !ctx) return;

    ui_element_set_scale(e, 1);

    e->opacity = 1;

    e->screen = ctx->screen;
}

UNUSED static void print_props(UIPropertyList *props) {
    output_log("START\n");
    for (int i = 0; i < props->count; i++) {
        output_log("KEY %s VALUE %s\n",props->properties[i].key, props->properties[i].value);
    }
    output_log("END\n");
}

void ui_screen_add_element(UIScreen *screen, UIElement *element) {
    // Expand if out of space
    if (screen->count == screen->capacity) {
        screen->capacity *= 2;

        screen->elements = realloc(
            screen->elements,
            screen->capacity * sizeof(*screen->elements)
        );
    }

    screen->elements[screen->count++] = element;
    element->screen = screen;
}

// Load a screen from its file, needs a pointer to the actions table and the action count
void ui_load_screen(UIScreen* screen,
                    const UIAction* actions,
                    size_t actionCount,
                    const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return;
    
    // Unload screen if already loaded
    if (screen->loaded) {
        ui_unload_screen(screen);
    }

    screen->loaded = true;

    screen->disable_element_update = false;
    screen->open_anim_time = 0.f;
    screen->open_anim_done = false;

    screen->count = 0;
    screen->capacity = 16;
    screen->elements = calloc(screen->capacity, sizeof(*screen->elements));

    screen->actions = actions;
    screen->action_count = actionCount;

    // Add context
    UIContext ctx = { 0 };
    ctx.screen = screen;

    screen->ctx = ctx;

    char line[512];

    // Iterate through lines (one element per line)
    while (fgets(line, sizeof(line), f)) {

        trim_newline(line);

        // Comment or empty
        if (line[0] == '#' || line[0] == '\0')
            continue;
        
        char* cursor = line;
        char* token = next_token(&cursor);

        // Check for invalid tokens
        if (!token) continue;

        // The element type
        char type[16];
        strncpy(type, token, 15);

        UIPropertyList props = { 0 };

        // Parse element parameters
        while ((token = next_token(&cursor)) != NULL) {
            char* equal = strchr(token, '=');
            if (!equal) continue;

            // This replaces the equal sign between key and value with a null character, dividing the string in two
            *equal = '\0';

            char* key = token;
            char* value = equal + 1;
            
            strip_quotes(value);

            props.properties[props.count++] = (UIProperty){
                .key = key,
                .value = value
            };
        }

        // Execute the element constructor
        for (int i = 0; i < ARRAY_LEN(element_constructors); i++) {
            if (strcmp(type, element_constructors[i].name) == 0) {
                if (element_constructors[i].create) {
                    UIElement *e = element_constructors[i].create(&ctx, &props);
                    //print_props(&props);
                    ui_screen_add_element(screen, e);
                }
            }
        }
    }
    
    fclose(f);
}

void ui_unload_screen(UIScreen *screen) {
    if (!screen->loaded || !screen->elements) return;

    for (int i = 0; i < screen->count; i++) {
        UIElement *e = screen->elements[i];
        if (e->destroy) {
            e->destroy(e);
        }
    }

    free(screen->elements);
    screen->elements = NULL;
    screen->loaded = false;
}