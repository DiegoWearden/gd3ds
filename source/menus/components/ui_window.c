#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "easing.h"
#include "utils/gfx.h"
#include "ui_checkbox.h"
#include "ui_screen.h"

void ui_window_set_tint(UIWindow* e, u32 color) {
    if (!e) return;

    e->color = color;
    e->useTint = true;
}

static void ui_window_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;
}

static void ui_window_draw(UIElement* e) {
    UIWindow *window = (UIWindow *) e;

    draw_9_slice(window->atlas, e->x, e->y, e->w, e->h, window->border, window->color);
}

static void ui_window_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

UIWindow *ui_create_window(
    int x, int y, int w, int h, int style,
    char (*tag)[TAG_LENGTH]
) {
    UIWindow *e = malloc(sizeof(UIWindow));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIWindow));
    e->base.type = UI_WINDOW;
    e->base.x = x;
    e->base.y = y;
    e->base.w = w;
    e->base.h = h;
    e->base.enabled = true;
    e->base.update = ui_window_update;
    e->base.draw = ui_window_draw;
    e->base.destroy = ui_window_destroy;

    e->color = C2D_Color32(255, 255, 255, 255);

    // Copy tag
    copy_tag_array(&e->base, tag);
    e->atlas = C2D_SpriteSheetGetImage(window_sheet, style);

    e->border = e->atlas.subtex->width / 3;

    return e;
}