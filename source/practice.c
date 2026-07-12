#include "practice.h"
#include "level_loading.h"
#include "main.h"
#include "graphics.h"
#include "state.h"
#include "mp3_player.h"
#include "math_helpers.h"
#include "color_channels.h"
#include "utils/gfx.h"
#include "menus/settings.h"
#include "utils/json_config.h"
#include "save/saving.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#define MAX_CHECKPOINTS 100
#define CHECKPOINT_GFX_ID 6
#define AUTO_CHECKPOINT_INTERVAL 1.25f
#define AUTO_CHECKPOINT_FLYING_INTERVAL 1.0f
#define AUTO_CHECKPOINT_MIN_DISTANCE (3.5f * 30.f)
#define AUTO_CHECKPOINT_FLYING_MIN_DISTANCE (5.0f * 30.f)

typedef struct CheckpointData {
    Player p1;
    Player p2;

    float camera_x;
    float camera_y;

    float camera_intended_y;

    float ground_y;
    float ceiling_y;
    float ground_y_gfx;

    float wall_y;

    bool mirroring;
    int mirror_mult;    
    float mirror_timer;

    float original_mirror_factor;
    float intended_mirror_factor;

    float mirror_speed_factor;
    float mirror_factor;
    
    bool dual;
    
    float dual_portal_y;
    unsigned char speed;

    int current_fading_effect;
    bool p1_trail;

    ColorChannel channels[COL_CHANNEL_NUM];
    ColTriggerBuffer col_trigger_buffer[COL_CHANNEL_NUM];
    
    float song_offset;

} CheckpointData;

CheckpointData checkpoints[MAX_CHECKPOINTS];
int checkpoint_count = 0;
int checkpoint_pointer = 0;
static float auto_checkpoint_timer = 0.f;
static float last_auto_checkpoint_x = 0.f;

// Permanent checkpoints: placed in practice mode, persisted per level on the
// SD card, usable as start positions in normal mode
#define MAX_PERM_CHECKPOINTS 16
#define PERM_CP_MAGIC 0x50434447 // "GDCP"
#define PERM_CP_VERSION 1

typedef struct {
    u32 magic;
    u32 version;
    u32 struct_size;
    u32 count;
} PermCpHeader;

static CheckpointData perm_checkpoints[MAX_PERM_CHECKPOINTS];
int perm_checkpoint_count = 0;
int perm_checkpoint_selected = -1; // -1 = level start

// static const int checkpoint_size = sizeof(checkpoints);

static void reset_auto_checkpoint_timer() {
    auto_checkpoint_timer = 0.f;
    last_auto_checkpoint_x = state.player.x;
}

void new_checkpoint() {
    if (state.dead) return;

    // Wrap around
    if (++checkpoint_pointer >= MAX_CHECKPOINTS) checkpoint_pointer = 0;

    // Cap checkpoint count
    if (++checkpoint_count > MAX_CHECKPOINTS) checkpoint_count = MAX_CHECKPOINTS;

    CheckpointData *check = &checkpoints[checkpoint_pointer];

    check->camera_x = state.camera_x;
    check->camera_y = state.camera_y;

    check->p1 = state.player;
    check->p2 = state.player2;

    check->camera_intended_y = state.camera_intended_y;

    check->ground_y = state.ground_y;
    check->ceiling_y = state.ceiling_y;
    check->ground_y_gfx = state.ground_y_gfx;

    check->mirroring = state.mirroring;
    check->mirror_mult = state.mirror_mult;
    check->mirror_timer = state.mirror_timer;
    check->original_mirror_factor = state.original_mirror_factor;
    check->intended_mirror_factor = state.intended_mirror_factor;
    check->mirror_speed_factor = state.mirror_speed_factor;
    check->mirror_factor = state.mirror_factor;

    check->dual = state.dual;
    check->dual_portal_y = state.dual_portal_y;

    check->speed = state.speed;

    check->current_fading_effect = current_fading_effect;
    check->p1_trail = p1_trail;

    check->wall_y = level_info.wall_y;

    check->song_offset = level_info.song_offset + state.player.timeElapsed;

    memcpy(check->channels, channels, sizeof(channels));
    memcpy(check->col_trigger_buffer, col_trigger_buffer, sizeof(col_trigger_buffer));

    reset_auto_checkpoint_timer();
}

static void restore_checkpoint_data(CheckpointData *check) {
    state.camera_x = check->camera_x;
    state.camera_y = check->camera_y;

    state.player = check->p1;
    state.player2 = check->p2;

    state.camera_intended_y = check->camera_intended_y;

    state.ground_y = check->ground_y;
    state.ceiling_y = check->ceiling_y;
    state.ground_y_gfx = check->ground_y_gfx;

    state.mirroring = check->mirroring;
    state.mirror_mult = check->mirror_mult;
    state.mirror_timer = check->mirror_timer;
    state.original_mirror_factor = check->original_mirror_factor;
    state.intended_mirror_factor = check->intended_mirror_factor;
    state.mirror_speed_factor = check->mirror_speed_factor;
    state.mirror_factor = check->mirror_factor;
    
    state.dual = check->dual;
    state.dual_portal_y = check->dual_portal_y;

    state.speed = check->speed;

    level_info.wall_y = check->wall_y;

    current_fading_effect = check->current_fading_effect;
    p1_trail = check->p1_trail;

    memcpy(channels, check->channels, sizeof(channels));
    memcpy(col_trigger_buffer, check->col_trigger_buffer, sizeof(col_trigger_buffer));

    update_attempt_text_pos();
}

void restore_checkpoint() {
    CheckpointData *check = &checkpoints[checkpoint_pointer];

    restore_checkpoint_data(check);

    if (practiceMusicSync) seek_mp3(check->song_offset);

    reset_auto_checkpoint_timer();
}

static void perm_checkpoint_path(char *out, size_t size) {
    char key[32];
    get_level_save_key(key, sizeof(key));
    snprintf(out, size, "%s%s.cp", DATA_FOLDER, key);
}

void load_permanent_checkpoints() {
    perm_checkpoint_count = 0;
    perm_checkpoint_selected = -1;

    char path[512];
    perm_checkpoint_path(path, sizeof(path));

    FILE *f = fopen(path, "rb");
    if (!f) return;

    PermCpHeader header;
    if (fread(&header, sizeof(header), 1, f) == 1 &&
        header.magic == PERM_CP_MAGIC &&
        header.version == PERM_CP_VERSION &&
        header.struct_size == sizeof(CheckpointData) &&
        header.count <= MAX_PERM_CHECKPOINTS) {
        perm_checkpoint_count = fread(perm_checkpoints, sizeof(CheckpointData), header.count, f);
    }

    fclose(f);
}

static void save_permanent_checkpoints() {
    char path[512];
    perm_checkpoint_path(path, sizeof(path));

    mkdir(DATA_FOLDER, 0777);

    FILE *f = fopen(path, "wb");
    if (!f) return;

    PermCpHeader header = {
        .magic = PERM_CP_MAGIC,
        .version = PERM_CP_VERSION,
        .struct_size = sizeof(CheckpointData),
        .count = perm_checkpoint_count,
    };
    fwrite(&header, sizeof(header), 1, f);
    fwrite(perm_checkpoints, sizeof(CheckpointData), perm_checkpoint_count, f);
    fclose(f);
}

bool make_last_checkpoint_permanent() {
    if (!state.practice_mode || checkpoint_count == 0) return false;
    if (perm_checkpoint_count >= MAX_PERM_CHECKPOINTS) return false;

    CheckpointData *src = &checkpoints[checkpoint_pointer];

    // Ignore near-duplicates of an already saved position
    for (int i = 0; i < perm_checkpoint_count; i++) {
        if (fabsf(perm_checkpoints[i].p1.x - src->p1.x) < 1.f) return false;
    }

    // Keep the list ordered by level position so the switcher arrows walk
    // left-to-right through the level
    int pos = 0;
    while (pos < perm_checkpoint_count && perm_checkpoints[pos].p1.x < src->p1.x) pos++;
    memmove(&perm_checkpoints[pos + 1], &perm_checkpoints[pos],
            (perm_checkpoint_count - pos) * sizeof(CheckpointData));
    perm_checkpoints[pos] = *src;
    perm_checkpoint_count++;

    save_permanent_checkpoints();
    return true;
}

void restore_permanent_checkpoint(int idx) {
    if (idx < 0 || idx >= perm_checkpoint_count) return;

    restore_checkpoint_data(&perm_checkpoints[idx]);

    if (song_loaded) seek_mp3(perm_checkpoints[idx].song_offset);

    reset_auto_checkpoint_timer();
}

float perm_checkpoint_percent(int idx) {
    if (idx < 0 || idx >= perm_checkpoint_count || level_info.last_obj_x <= 0) return 0.f;
    float percent = (perm_checkpoints[idx].p1.x / level_info.last_obj_x) * 100.f;
    return percent < 0.f ? 0.f : (percent > 100.f ? 100.f : percent);
}

void delete_last_checkpoint() {
    if (checkpoint_count > 0) {
        checkpoint_count--;

        // Wrap around pointer
        if (checkpoint_pointer-- == 0) {
            checkpoint_pointer = MAX_CHECKPOINTS - 1;
        }
    }

    reset_auto_checkpoint_timer();
}

void clear_practice_mode() {
    checkpoint_count = 0;
    checkpoint_pointer = 0;
    state.practice_mode = false;
    reset_auto_checkpoint_timer();
}

void start_practice_mode() {
    checkpoint_count = 0;
    checkpoint_pointer = 0;
    state.practice_mode = true;
    reset_auto_checkpoint_timer();

    if (!practiceMusicSync) {
        stop_mp3();
        play_practice_song();
    }
}

void exit_practice_mode() {
    state.practice_mode = false;
    init_variables();
    reload_level();

    if (practiceMusicSync) {
        seek_mp3(level_info.song_offset);
    } else {
        stop_mp3();
        play_level_song(level_info.song_offset);
    }
}

static bool auto_checkpoint_is_flying_mode(Player *player) {
    return player->gamemode == GAMEMODE_SHIP ||
           player->gamemode == GAMEMODE_BIRD ||
           player->gamemode == GAMEMODE_DART;
}

static bool auto_checkpoint_is_safe_surface(Player *player) {
    return player->on_ground ||
           player->on_ceiling ||
           player->slope_data.slope_id >= 0;
}

static void handle_auto_checkpoint() {
    if (!autoCheckpoints || state.dead || level_info.completing) {
        reset_auto_checkpoint_timer();
        return;
    }

    Player *player = &state.player;
    bool flying = auto_checkpoint_is_flying_mode(player);
    float interval = flying ? AUTO_CHECKPOINT_FLYING_INTERVAL : AUTO_CHECKPOINT_INTERVAL;
    float min_distance = flying ? AUTO_CHECKPOINT_FLYING_MIN_DISTANCE : AUTO_CHECKPOINT_MIN_DISTANCE;

    auto_checkpoint_timer += delta;

    if (auto_checkpoint_timer < interval) return;
    if (player->x - last_auto_checkpoint_x < min_distance) return;
    if (!flying && !auto_checkpoint_is_safe_surface(player)) return;

    new_checkpoint();
}

void handle_practice_mode() {
    if (!state.practice_mode) return;

    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();

    if (((kDown & KEY_L) && !((kHeld & KEY_B) && enableDebugBindings)) || (kDown & KEY_ZL)) {
        new_checkpoint();
    }

    if (((kDown & KEY_R) && !((kHeld & KEY_B) && enableDebugBindings)) || (kDown & KEY_ZR)) {
        delete_last_checkpoint();
    }

    handle_auto_checkpoint();
}

static void draw_checkpoint(float x, float y) {
    C2D_Sprite spr = { 0 };
    C2D_SpriteFromSheet(&spr, spriteSheet2, CHECKPOINT_GFX_ID);
    C2D_SpriteSetCenter(&spr, 0.5f, 0.5f);
    C3D_TexSetFilter(spr.image.tex, GPU_LINEAR, GPU_LINEAR);

    C2D_SpriteSetPos(&spr, get_mirror_x(x, state.mirror_factor), y);

    C2D_DrawSprite(&spr);
}

void draw_checkpoints() {
    if (!state.practice_mode) return;

    for (u32 checkpoint = 0; checkpoint < checkpoint_count; checkpoint++) {
        // Obtain buffer index
        s32 index = WRAP((s32) (checkpoint_pointer - checkpoint), 0, MAX_CHECKPOINTS);
        CheckpointData *curr_checkpoint = &checkpoints[index];

        float calc_x = (curr_checkpoint->p1.x - state.camera_x);
        float calc_y = SCREEN_HEIGHT - ((curr_checkpoint->p1.y - state.camera_y));  

        if (calc_x < -60 || calc_x >= (SCREEN_WIDTH / SCALE) + 60) continue;
        if (calc_y < -60 || calc_y >= (SCREEN_HEIGHT / SCALE) + 60) continue;

        draw_checkpoint(calc_x, calc_y);
    }
}
