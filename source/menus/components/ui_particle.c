#include "ui_element.h"
#include "particles/particle_definitions.h"
#include <citro2d.h>
#include "ui_screen.h"
#include "graphics.h"

void ui_particle_update_pos(UIParticle* e){
    if (!e) return;

    ParticleSystem *p = &(e->particle);
    p->emitterX = e->base.x;
    p->emitterY = e->base.y;
}

void ui_particle_emit(UIParticle* e, int emitCount){
    if (!e) return;

    spawnMultipleParticles(&(e->particle), emitCount);
}

static void ui_particle_update(UIElement* e, UIInput* touch) {
    UIParticle *particle = (UIParticle *) e;
    ParticleSystem *p = &(particle->particle);

    updateParticleSystem(p, delta);
}

static void ui_particle_draw(UIElement* e) {
    UIParticle *particle = (UIParticle *) e;
    
    change_blending(true);
    drawParticleSystem(&(particle->particle), 0, 0, 1);
    change_blending(false);
}

static void ui_particle_destroy(UIElement *e) {
    if (e) {
        UIParticle *particle = (UIParticle *) e;
        freeParticleData(&particle->particle.data);
        free(e);

        particle = NULL;
        e = NULL;
    }
}

UIParticle *ui_create_particle(int x, int y, float scale, float r, float g, float b, ParticleDefinition* const def, char (*tag)[TAG_LENGTH]) {
    UIParticle *e = malloc(sizeof(UIParticle));

    if (!e) return NULL;

    memset(e, 0, sizeof(UIParticle));
    ParticleSystem *p = &(e->particle);

    initParticleSystem(p, def);

    e->base.type = UI_PARTICLE;
    e->base.x = x;
    e->base.y = y;
    e->base.enabled = true;
    
    //Do not override colors if invalid
    if(r >= 0.f && g >= 0.f && b >= 0.f && r <= 1.f && g <= 1.f && b <= 1.f){
        p->cfg.startColorRed = r;
        p->cfg.startColorGreen = g;
        p->cfg.startColorBlue = b;
        p->cfg.finishColorRed = r;
        p->cfg.finishColorGreen = g;
        p->cfg.finishColorBlue = b;
    }

    p->emitterX = x;
    p->emitterY = y;
    p->scale = scale;
    p->affectedByMirror = false;
    p->stationary = true;

    e->base.update = ui_particle_update;
    e->base.draw = ui_particle_draw;
    e->base.destroy = ui_particle_destroy;

    copy_tag_array(&e->base, tag);

    return e;
}