#include "practice.h"
#include "main.h"
#include "graphics.h"
#include "state.h"
#include "mp3_player.h"
#include "math_helpers.h"
#include "color_channels.h"
#include "utils/gfx.h"
#include "menus/settings.h"

#define MAX_CHECKPOINTS 100
#define CHECKPOINT_GFX_ID 6
#define PRACTICE_SONG_PATH "romfs:/songs/StayInsideMe.mp3"
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
    
} CheckpointData;

CheckpointData checkpoints[MAX_CHECKPOINTS];
int checkpoint_count = 0;
int checkpoint_pointer = 0;
static bool practice_level_music_active = false;
static float auto_checkpoint_timer = 0.f;
static float last_auto_checkpoint_x = 0.f;

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

    memcpy(check->channels, channels, sizeof(channels));
    memcpy(check->col_trigger_buffer, col_trigger_buffer, sizeof(col_trigger_buffer));

    reset_auto_checkpoint_timer();
}

void restore_checkpoint() {
    CheckpointData *check = &checkpoints[checkpoint_pointer];
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
    reset_auto_checkpoint_timer();
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
    practice_level_music_active = false;
    reset_auto_checkpoint_timer();
}

static float get_practice_level_music_time() {
    return level_info.song_offset + state.player.timeElapsed;
}

static void play_original_practice_song() {
    practice_level_music_active = false;
    song_loaded = false;
    play_mp3(PRACTICE_SONG_PATH, true, 0);
}

bool practice_uses_level_music() {
    return state.practice_mode && practiceLevelMusic && practice_level_music_active;
}

void apply_practice_music_mode() {
    if (!state.practice_mode) return;

    stop_mp3();

    if (practiceLevelMusic && play_level_song_at(get_practice_level_music_time())) {
        practice_level_music_active = true;
    } else {
        play_original_practice_song();
    }
}

void sync_practice_level_music() {
    if (!practice_uses_level_music()) return;
    seek_mp3(get_practice_level_music_time());
}

void start_practice_mode() {
    checkpoint_count = 0;
    checkpoint_pointer = 0;
    state.practice_mode = true;
    reset_auto_checkpoint_timer();
    apply_practice_music_mode();
}

void exit_practice_mode() {
    state.practice_mode = false;
    practice_level_music_active = false;
    init_variables();
    reload_level(); 
    stop_mp3();
    play_level_song();
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
