#include "menus/core/ui_element.h"
#include <citro2d.h>
#include "menus/core/ui_screen.h"
#include "menus/core/ui_props.h"

static const UIFloatEnumEntry alignment_table[] = {
    { "LEFT",   0.f },
    { "CENTER", 0.5f },
    { "RIGHT",  1.f }
};

static void ui_label_update(UIElement* e, UIInput* touch, UITransform *transform) {
    // Do absolutely nothing
    (void)e;
    (void)touch;
}

static void ui_label_draw(UIElement* e, UITransform *transform) {
    UILabel *label = (UILabel *) e;
    int font_id = label->font;

    // Set to pusab if invalid
    if (font_id >= NUM_FONTS) font_id = 0;

    const LabelFont *font = &fonts[font_id];
    draw_text(font->charset, font->sheet, transform->x, transform->y, transform->scaleX, transform->scaleY, label->alignment, label->parse_tags, "%s", label->text);
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

    float scaleX = e->base.scaleX;

    float ratio = scaleX / e->base.scaleY;

    // Get text length in pixels
    float length = get_longest_line_length(font->charset, scaleX, text);

    if (length == 0) return;

    if (width < length) {
        text_scale = scaleX * (width / length);
    } else {
        text_scale = scaleX;
    }

    e->base.scaleX = text_scale;
    e->base.scaleY = e->base.scaleX * ratio;
}

UILabel *ui_create_label(const UIContext *ctx) {
    UILabel *e = malloc(sizeof(UILabel));

    if (!e) return NULL;

    memset(e, 0, sizeof(UILabel));
    e->base.type = UI_LABEL;
    e->base.enabled = true;
    
    ui_element_apply_default_properties(&e->base, ctx);

    e->parse_tags = true;

    e->base.update = ui_label_update;
    e->base.draw = ui_label_draw;
    e->base.destroy = ui_label_destroy;

    return e;
}

UIElement *ui_create_label_from_props(const UIContext *ctx, const UIPropertyList *props) {
    UILabel *label = ui_create_label(ctx);

    if (!label) return NULL;

    ui_element_apply_properties(&label->base, ctx, props);

    ui_label_set_text(label, ui_prop_string(props, "text", ""));
    
    label->font = ui_prop_int(props, "font", 0);
    label->alignment = ui_prop_float_enum(props, "align", alignment_table, ARRAY_LEN(alignment_table), 0);
    label->parse_tags = ui_prop_bool(props, "parseTags", true);

    return &label->base;
}