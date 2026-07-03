#pragma once
#include <stdbool.h>

extern int checkpoint_count;
extern int checkpoint_pointer;

void start_practice_mode();
void exit_practice_mode();
void apply_practice_music_mode();
bool practice_uses_level_music();
void sync_practice_level_music();

void new_checkpoint();
void restore_checkpoint();
void delete_last_checkpoint();

void handle_practice_mode();
void draw_checkpoints();
void clear_practice_mode();
