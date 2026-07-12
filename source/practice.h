#pragma once
#include <stdbool.h>

extern int checkpoint_count;
extern int checkpoint_pointer;

// Permanent checkpoints (practice-placed start positions for normal mode)
extern int perm_checkpoint_count;
extern int perm_checkpoint_selected; // -1 = level start

void load_permanent_checkpoints();
bool make_last_checkpoint_permanent();
void restore_permanent_checkpoint(int idx);
void delete_permanent_checkpoint(int idx);
float perm_checkpoint_percent(int idx);

void start_practice_mode();
void exit_practice_mode();

void new_checkpoint();
void restore_checkpoint();
void delete_last_checkpoint();

void handle_practice_mode();
void draw_checkpoints();
void clear_practice_mode();
