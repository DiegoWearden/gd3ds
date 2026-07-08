#pragma once
#include "ui_element.h"
#include "particles/particle_definitions.h"

void ui_particle_update_pos(UIParticle* e);
void ui_particle_emit(UIParticle* e, int emitCount);
UIParticle *ui_create_particle(int x, int y, float scale, float r, float g, float b, const ParticleDefinition* particle, char (*tag)[TAG_LENGTH]);