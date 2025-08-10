#pragma once

typedef enum {
    BOSS_SOUND_SPAWN,
    BOSS_SOUND_HURT,
    BOSS_SOUND_HOWLING,
    BOSS_SOUND_MISSILE,
    BOSS_SOUND_LAZER

} boss_sound_type_t;

void Sound_init();
void Sound_playBGM(const char* filename);
void Sound_shutdown();
void Sound_playFootstep();
void Sound_playSwing();
void Sound_playMenuBGM(const char* filename);
void Sound_playMosterAttack();
void Sound_playPlayerHurt();
void Sound_PushBGM(const char* new_bgm_filename);
void Sound_PopBGM();
void Sound_playBossSound(boss_sound_type_t type);






//#ifndef SOUND_H
//#define SOUND_H
//
//PlayBGM();
//PlayStartMenuBGM();
//
//#endif