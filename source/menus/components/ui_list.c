#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "ui_button.h"
#include "easing.h"
#include "math_helpers.h"
#include "ui_screen.h"
#include "ui_list.h"
#include "utils/gfx.h"
#include <stdlib.h>

void ui_list_reset(UIList *list) {
    list->scrollY = 0;
    list->itemCount = 0;
    list->contentHeight = 0;
    list->lastTouchY = 0;
}

void ui_list_add(UIList* list, UIElement* item) {
    if (list->itemCount >= UI_LIST_MAX_ITEMS) return;

    // Add item and increase content height
    list->items[list->itemCount++] = item;
    list->contentHeight += item->h;
}

static void ui_list_forward_touch(UIList* list, UIInput* touch) {

    float initial_y = list->base.y - (list->base.h / 2) + list->scrollY;
    
    for (int i = 0; i < list->itemCount; i++) {
        UIElement* item = list->items[i];
        // Set position
        item->y = initial_y + (item->h / 2) + (item->h) * i;

        if (!item->enabled) continue;

        item->update(item, touch);
    }
}

static void ui_list_update(UIElement* e, UIInput* touch) {
    UIList* l = (UIList *) e;

    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);

    ui_list_forward_touch(l, touch);
    if (inside) {  
        touch->did_something = true;

        // Exit if used something
        if (touch->interacted) return;
    }

    // Start dragging
    if (inside && (hidKeysDown() & KEY_TOUCH)) {
        l->dragging = true;
        l->lastTouchY = touch->touchPosition.py;
    }

    // Handle dragging
    if (l->dragging && (hidKeysHeld() & KEY_TOUCH)) {
        // Apply touch movement
        int delta = touch->touchPosition.py - l->lastTouchY;
        l->scrollY += delta;
        l->lastTouchY = touch->touchPosition.py;
    }

    circlePosition circlePad;
    //Joystick movement
    hidCircleRead(&circlePad);
    if(abs(circlePad.dy) > 48){
        l->scrollY += (circlePad.dy / 25);
    }

    if(hidKeysDown() & (KEY_UP | KEY_DOWN)){
        l->dpadHeldTime = 0;
    }

    if(l->dpadHeldTime > 60){
        l->dpadHeldTime = 60;
    }

    if(hidKeysHeld() & KEY_UP){
        l->scrollY += 4;
        if(l->dpadHeldTime >= 60){
            l->scrollY += 4;
        }

        l->dpadHeldTime++;
    }
    if(hidKeysHeld() & KEY_DOWN){
        l->scrollY -= 4;
        if(l->dpadHeldTime >= 60){
            l->scrollY -= 4;
        }

        l->dpadHeldTime++;
    }

    // Handle releasing dragging
    if (hidKeysUp() & KEY_TOUCH) {
        l->dragging = false;
    }

    // Clamp scrolling
    int minScroll = e->h - l->contentHeight;
    if (l->contentHeight <= e->h) {
        l->scrollY = 0;
    } else {
        if (l->scrollY > 0) l->scrollY = 0;
        if (l->scrollY < minScroll) l->scrollY = minScroll;
    }
}

static void ui_list_draw(UIElement* e) {
    UIList* l = (UIList *) e;

    float x = e->x - (e->w / 2);
    float y = e->y - (e->h / 2);

    // Draw border
    // C2D_DrawRectSolid(x, y, 0, e->w, e->h,
    // C2D_Color32(40,40,40,255));

    // Enable clipping
    set_scissor(GPU_SCISSOR_NORMAL, x, y, e->w, e->h);

    float initial_y = y + l->scrollY;

    for (int i = 0; i < l->itemCount; i++) {
        UIElement* item = l->items[i];

        // Set position
        item->x = e->x;
        item->y = initial_y + (item->h / 2) + (item->h) * i;
        item->w = e->w; // Set width to container width

        // Draw item
        item->draw(item);
    }

    // Disable scissor
    C2D_Flush();
    C3D_SetScissor(GPU_SCISSOR_DISABLE, 0,0,0,0);
    C2D_Prepare();
}

static void ui_list_destroy(UIElement *e) {
    if (e) {
        free(e);
        e = NULL;
    }
}

UIList *ui_create_list(
    int x, int y, int w, int h,
    char (*tag)[TAG_LENGTH]
) {
    UIList *e = malloc(sizeof(UIList));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIList));
    e->base.type = UI_LIST;
    e->base.x = x;
    e->base.y = y;
    e->base.w = w;
    e->base.h = h;
    e->base.enabled = true;

    // Copy tag
    copy_tag_array(&e->base, tag);

    e->base.update = ui_list_update;
    e->base.draw = ui_list_draw;
    e->base.destroy = ui_list_destroy;
    
    ui_list_reset(e);

    return e;
}