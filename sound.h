#pragma once

typedef enum {
    PLAYER_SOUND_FOOTSTEP,
    PLAYER_SOUND_SWING,
    PLAYER_SOUND_HURT,

    MONSTER_SOUND_ATTACK,
    MONSTER_SOUND_HURT,
    MONSTER_SOUND_DEATH,

    BOSS_SOUND_SPAWN,
    BOSS_SOUND_HURT,
    BOSS_SOUND_HOWLING,
    BOSS_SOUND_MISSILE,
    BOSS_SOUND_LASER
} sound_effect_t;

void sound_initialize(void);
void sound_destroy(void);

void sound_play_BGM(const char * const pFile_name);

void sound_play_sound_effect(const sound_effect_t sound_effect);