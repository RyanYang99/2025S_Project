#pragma once

// 한 번에 로드할 발소리 파일의 최대 개수
#define MAX_FOOTSTEP_SOUNDS 10
#define MAX_SWING_SOUNDS 3


void Sound_init();
void Sound_playBGM(const char* filename);
void Sound_shutdown();
void Sound_playFootstep();
void Sound_playSwing();





//#ifndef SOUND_H
//#define SOUND_H
//
//PlayBGM();
//PlayStartMenuBGM();
//
//#endif