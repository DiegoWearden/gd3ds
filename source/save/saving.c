#include "saving.h"
#include "main.h"
#include "utils/json_config.h"
#include <sys/stat.h>
#include <sys/types.h>

#include <stdint.h>
#include "level/main_levels.h"

Config curr_level_config;
Config main_level_config[MAIN_LEVELS_NUM];

LevelData level_data;
LevelData main_level_data[MAIN_LEVELS_NUM];

uint64_t fnv1a64(const char* str) {
    uint64_t hash = 14695981039346656037ULL;

    while (*str) {
        hash ^= (unsigned char)*str++;
        hash *= 1099511628211ULL;
    }

    return hash;
}

static void init_default(Config *config) {
    config_init_int(config, DATA_ATTEMPTS, 0);
    config_init_int(config, DATA_JUMPS, 0);
    config_init_int(config, DATA_NORMAL, 0);
    config_init_int(config, DATA_PRACTICE, 0);
    config_init_bool(config, DATA_COIN1, false);
    config_init_bool(config, DATA_COIN2, false);
    config_init_bool(config, DATA_COIN3, false);
}

static void init_values(Config *config, LevelData *level_data) {
    level_data->attempts = config_get_int(config, DATA_ATTEMPTS, 0);
    level_data->jumps = config_get_int(config, DATA_JUMPS, 0);
    level_data->normal_progress = config_get_int(config, DATA_NORMAL, 0);
    level_data->practice_progress = config_get_int(config, DATA_PRACTICE, 0);
    level_data->coin1 = config_get_bool(config, DATA_COIN1, false);
    level_data->coin2 = config_get_bool(config, DATA_COIN2, false);
    level_data->coin3 = config_get_bool(config, DATA_COIN3, false);
}

static void save_values(Config *config, LevelData *level_data) {
    config_set_int(config, DATA_ATTEMPTS, level_data->attempts);
    config_set_int(config, DATA_JUMPS, level_data->jumps);
    config_set_int(config, DATA_NORMAL, level_data->normal_progress);
    config_set_int(config, DATA_PRACTICE, level_data->practice_progress);
    config_set_bool(config, DATA_COIN1, level_data->coin1);
    config_set_bool(config, DATA_COIN2, level_data->coin2);
    config_set_bool(config, DATA_COIN3, level_data->coin3);
}

void load_main_level_progress() {
    for (int i = 0; i < MAIN_LEVELS_NUM; i++) {
        char file[256];
        snprintf(file, sizeof(file), "main_%d", i);
        
        char tmp[512];
        mkdir(DATA_FOLDER, 0777);

        snprintf(tmp, sizeof(tmp), "%s%016llX.d", DATA_FOLDER, fnv1a64(file));

        config_load(&main_level_config[i], tmp);

        init_default(&main_level_config[i]);
        init_values(&main_level_config[i], &main_level_data[i]);

        config_save(&main_level_config[i]);
    }
}

void save_main_level_progress(int level) {
    save_values(&main_level_config[level], &main_level_data[level]);
    config_save(&main_level_config[level]);
}

void free_main_level_progress() {
    for (int i = 0; i < MAIN_LEVELS_NUM; i++) {
        config_free(&main_level_config[i]);
    }
}

void load_level_progress(char *filename) {
    char tmp[512];

    mkdir(DATA_FOLDER, 0777);

    snprintf(tmp, sizeof(tmp), "%s%016llX.d", DATA_FOLDER, fnv1a64(filename));

    // Do not load again
    if (curr_level_config.root) {
        free_level_progress();
    }

    config_load(&curr_level_config, tmp);

    init_default(&curr_level_config);
    init_values(&curr_level_config, &level_data);

    config_save(&curr_level_config);
}


void save_level_progress() {
    save_values(&curr_level_config, &level_data);

    config_save(&curr_level_config);
}

void free_level_progress() {
    config_free(&curr_level_config);
}