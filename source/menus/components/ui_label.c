#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "ui_screen.h"

static void ui_label_update(UIElement* e, UIInput* touch) {
    // Do absolutely nothing
    (void)e;
    (void)touch;
}

static void ui_label_draw(UIElement* e) {
    UILabel *label = (UILabel *) e;
    int font_id = label->font;

    // Set to pusab if invalid
    if (font_id >= NUM_FONTS) font_id = 0;

    const LabelFont *font = &fonts[font_id];
    draw_text(font->charset, font->sheet, e->x, e->y, label->scale, label->scale, label->alignment, label->parse_tags, "%s", label->text);
}

static void ui_label_destroy(UIElement* e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

void ui_label_set_text(UILabel *e, const char *text) {
    if (!e || !text) return;

    strncpy(e->text, text, sizeof(e->text) - 1);
}

void ui_label_set_scale_from_width(UILabel *e, const char *text, float width) {
    if (!e || !text) return;

    int font_id = e->font;

    // Set to pusab if invalid
    if (font_id >= NUM_FONTS) font_id = 0;

    const LabelFont *font = &fonts[font_id];

    float text_scale;

    float scale = e->scale;

    // Get text length in pixels
    float length = get_longest_line_length(font->charset, scale, text);

    if (length == 0) return;

    if (width < length) {
        text_scale = scale * (width / length);
    } else {
        text_scale = scale;
    }

    e->scale = text_scale;
}

UILabel *ui_create_label(int x, int y, float scale, char *text, int font, float alignment, bool parse_tags, char (*tag)[TAG_LENGTH]) {
    UILabel *e = malloc(sizeof(UILabel));

    if (!e) return NULL;

    memset(e, 0, sizeof(UILabel));
    e->base.type = UI_LABEL;
    e->base.x = x;
    e->base.y = y;
    e->base.w = 0;
    e->base.h = 10;
    e->base.enabled = true;
    
    e->font = font;
    e->alignment = alignment;
    e->scale = scale;
    e->parse_tags = parse_tags;
    
    // Copy tag
    copy_tag_array(&e->base, tag);

    // Copy text
    strncpy(e->text, text, 255);

    e->base.update = ui_label_update;
    e->base.draw = ui_label_draw;
    e->base.destroy = ui_label_destroy;

    return e;
}