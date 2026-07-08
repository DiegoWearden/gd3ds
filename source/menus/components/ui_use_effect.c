#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"
#include "graphics.h"
#include "particles/circles.h"

void ui_use_effect_clear(UIUseEffect* e){
    if (!e) return;

    UseEffectPool *pool = &(e->useEffects);
    clear_use_effects(pool);
}

void ui_use_effect_update_pos(UIUseEffect* e) {
    if (!e) return;
    
    UseEffectPool *pool = &(e->useEffects);
    if (!pool->pool) return;

    for(int i = 0; i < pool->capacity; i++){
        pool->pool[i].x = e->base.x + e->xPos[i];
        pool->pool[i].y = e->base.y + e->yPos[i];
    }
}

UseEffect *ui_add_use_effect(UIUseEffect* e, float x, float y, const UseEffectDefinition *def) {
    if (!e) return NULL;

    UseEffectPool *pool = &(e->useEffects);
    UseEffect *effect = add_use_effect(x, y, USE_EFFECT_OBJ_NOTHING, def, pool);
    e->xPos[effect->index] = x - e->base.x;
    e->yPos[effect->index] = y - e->base.y;
    return effect;
}

void ui_set_use_effect_col(UseEffect *effect, float r, float g, float b) {
    if (!effect) return;
    
    effect->def.colorR = r;
    effect->def.colorG = g;
    effect->def.colorB = b;
}

static void ui_use_effect_update(UIElement* e, UIInput* touch) {
    UIUseEffect *useEffect = (UIUseEffect *) e;

    UseEffectPool *pool = &(useEffect->useEffects);
    update_use_effects(delta, pool);
}

static void ui_use_effect_draw(UIElement* e) {
    UIUseEffect *useEffect = (UIUseEffect *) e;

    UseEffectPool *pool = &(useEffect->useEffects);
    change_blending(true);
    draw_use_effects(pool);
    change_blending(false);
}

static void ui_use_effect_destroy(UIElement *e) {
    if (e) {
        UIUseEffect *useEffect = (UIUseEffect *) e;
        // Free pool
        if (useEffect->useEffects.pool) {
            free(useEffect->useEffects.pool);
        }

        free(e);
        e = NULL;
    }
}

UIUseEffect *ui_create_use_effect(char (*tag)[TAG_LENGTH]) {
    UIUseEffect *e = malloc(sizeof(UIUseEffect));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIUseEffect));
    UseEffectPool *pool = &(e->useEffects);
    init_use_effect_pool(pool, MAX_UI_USE_EFFECTS);
    pool->stationary = true;
    for (size_t i = 0; i < pool->capacity; i++) {
        pool->pool[i].active = false;
        e->xPos[i] = 0;
        e->yPos[i] = 0;
    }

    e->base.x = 0;
    e->base.y = 0;

    e->base.type = UI_USE_EFFECT;
    e->base.enabled = true;

    e->base.update = ui_use_effect_update;
    e->base.draw = ui_use_effect_draw;
    e->base.destroy = ui_use_effect_destroy;

    copy_tag_array(&e->base, tag);

    return e;
}