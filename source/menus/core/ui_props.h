#pragma once

#include "ui_screen.h"

typedef struct {
    const char *name;
    float value;
} UIFloatEnumEntry;

typedef struct {
    const char *name;
    int value;
} UIIntEnumEntry;

const char *ui_prop_string(const UIPropertyList *props, const char *key, const char *default_value);
int ui_prop_int(const UIPropertyList *props, const char *key, int default_value);
float ui_prop_float(const UIPropertyList *props, const char *key, float default_value);
bool ui_prop_bool(const UIPropertyList *props, const char *key, bool default_value);
u32 ui_prop_bitfield(const UIPropertyList *props, const char *key, const UIBitfieldEntry *table, size_t count);

int ui_prop_int_enum(const UIPropertyList *props, const char *key, const UIIntEnumEntry *table, size_t count, int default_value);
float ui_prop_float_enum(const UIPropertyList *props, const char *key, const UIFloatEnumEntry *table, size_t count, float default_value);