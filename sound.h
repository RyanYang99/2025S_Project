#pragma once

void sound_initialize(void);
void sound_destroy(void);

void sound_play_BGM(const char * const pFile_name);
void sound_play_menu_BGM(const char* const pFile_name);
void sound_play_footstep(void);
void sound_play_swing(void);
void sound_play_monster_hurt(void);