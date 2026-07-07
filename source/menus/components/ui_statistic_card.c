#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/goldFont.h"
#include "ui_screen.h"
#include "ui_button.h"
#include "easing.h"
#include "math_helpers.h"

#include "main.h"

static void ui_statistic_card_update(UIElement* e, UIInput* touch) {
    // Do absolutely nothing
    (void)e;
    (void)touch;
}

static void ui_statistic_card_draw(UIElement* e) {
    float left_side = e->x - e->w / 2;
    float right_side = e->x + e->w / 2;
    float top_side = e->y - e->h / 2;
    float name_pos = left_side + 6;
    float value_pos = right_side - 6;
    
    
    C2D_DrawRectSolid(left_side, top_side, 0, e->w, e->h, (e->statistic_card.swap_color ? C2D_Color32(194,114,62,255) :  C2D_Color32(161,88,48,255)));

    draw_text(&goldFont_fontCharset, &goldFont_sheet, name_pos, e->y+1, e->statistic_card.stat_name.scale, e->statistic_card.stat_name.scale, e->statistic_card.stat_name.alignment, true, "%s", e->statistic_card.stat_name.text);
    draw_text(&goldFont_fontCharset, &goldFont_sheet, value_pos, e->y+1, e->statistic_card.stat_name.scale, e->statistic_card.stat_name.scale, 1.f, true, "%d", e->statistic_card.value);
}

UIElement ui_create_statistic_card(int x, int y, bool swap_color, char *name, int value, char (*tag)[TAG_LENGTH]) {
    UIElement e = {0};

    e.type = UI_STATISTIC_CARD;
    e.x = x;
    e.y = y;
    e.w = 200;
    e.h = 28;
    e.enabled = true;
    
    // Copy tag
    copy_tag_array(&e, tag);
    
    // Copy text
    strncpy(e.statistic_card.stat_name.text, name, 255);

    e.statistic_card.stat_name.scale = 0.54f;
    e.statistic_card.stat_name.alignment = 0;

    e.statistic_card.value = value;
    e.statistic_card.swap_color = swap_color;

    e.update = ui_statistic_card_update;
    e.draw = ui_statistic_card_draw;

    return e;
}