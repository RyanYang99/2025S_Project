#pragma once

typedef enum {
    BOSS_SOUND_SPAWN,
    BOSS_SOUND_HURT,
    BOSS_SOUND_HOWLING,
    BOSS_SOUND_MISSILE,
    BOSS_SOUND_LAZER
} boss_sound_type_t;

void sound_initialize(void);
void sound_destroy(void);

void sound_play_BGM(const char * const pFile_name);

void sound_play_footstep(void);
void sound_play_swing(void);

void sound_play_monster_hurt(void);
void sound_play_monster_attack(void);

void sound_play_player_hurt(void);

void sound_play_boss_sound(const boss_sound_type_t type);