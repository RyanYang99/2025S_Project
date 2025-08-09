#include "leak.h"
#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SFML/Audio.h>

//한 번에 로드할 발소리 파일의 최대 개수
#define MAX_FOOTSTEP_SOUNDS 10
#define MAX_SWING_SOUNDS 3
#define MAX_HURT_SOUNDS 5
#define MAX_SFX_PLAYERS 10

typedef struct {
    sfMusic *pBGM;
} sound_manager_t;

typedef struct {
    sfSoundBuffer *ppBuffers[MAX_FOOTSTEP_SOUNDS],
                  *ppSwing_buffers[MAX_SWING_SOUNDS],
                  *ppMonster_hurt_buffers[MAX_HURT_SOUNDS];
    sfSound *ppSFX_players[MAX_SFX_PLAYERS],
            *pSound;

    int footstep_count, swing_count, monster_hurt_count;
} SFX_manager_t;

static const char * const pFootstep_sounds[MAX_FOOTSTEP_SOUNDS] = {
    "Footstep/Footstep_Dirt_00.wav",
    "Footstep/Footstep_Dirt_01.wav",
    "Footstep/Footstep_Dirt_02.wav",
    "Footstep/Footstep_Dirt_03.wav",
    "Footstep/Footstep_Dirt_04.wav",
    "Footstep/Footstep_Dirt_05.wav",
    "Footstep/Footstep_Dirt_06.wav",
    "Footstep/Footstep_Dirt_07.wav",
    "Footstep/Footstep_Dirt_08.wav",
    "Footstep/Footstep_Dirt_09.wav"
}, * const pSwing_sounds[MAX_SWING_SOUNDS] = {
    "Swing/swing.wav",
    "Swing/swing2.wav",
    "Swing/swing3.wav"
}, * const pMonster_hurt_sounds[MAX_HURT_SOUNDS] = {
    "Monster/Hurt/Hurt.wav",
    "Monster/Hurt/Hurt2.wav",
    "Monster/Hurt/Hurt3.wav",
    "Monster/Hurt/Hurt4.wav",
    "Monster/Hurt/Hurt5.wav"
};

static SFX_manager_t *pSFX_manager = NULL;
static sound_manager_t *pSound_manager = NULL;

void sound_initialize(void) {
    if (pSound_manager != NULL)
        return;

    pSound_manager = malloc(sizeof(sound_manager_t));
    if (!pSound_manager)
        return;

    pSound_manager->pBGM = NULL;

    //효과음 매니저 초기화
    pSFX_manager = malloc(sizeof(SFX_manager_t));
    if (!pSFX_manager)
        return;

    for (int i = 0; i < MAX_SFX_PLAYERS; ++i)
        pSFX_manager->ppSFX_players[i] = sfSound_create();
    pSFX_manager->pSound = sfSound_create();
    pSFX_manager->footstep_count = 0;
    pSFX_manager->swing_count = 0;
    pSFX_manager->monster_hurt_count = 0;

    for (int i = 0; i < MAX_FOOTSTEP_SOUNDS; ++i) {
        if (pFootstep_sounds[i] == NULL)
            break;

        char pFull_path[256] = { 0 };
        snprintf(pFull_path, sizeof(pFull_path), "SoundFile/%s", pFootstep_sounds[i]);

        pSFX_manager->ppBuffers[i] = sfSoundBuffer_createFromFile(pFull_path);
        if (pSFX_manager->ppBuffers[i])
            ++pSFX_manager->footstep_count;
    }

    for (int i = 0; i < MAX_SWING_SOUNDS; i++) {
        char pFull_path[256] = { 0 };
        snprintf(pFull_path, sizeof(pFull_path), "SoundFile/%s", pSwing_sounds[i]);

        pSFX_manager->ppSwing_buffers[i] = sfSoundBuffer_createFromFile(pFull_path);
        if (pSFX_manager->ppSwing_buffers[i])
            ++pSFX_manager->swing_count;
    }

    for (int i = 0; i < MAX_HURT_SOUNDS; ++i) {
        char pFull_path[256] = { 0 };
        snprintf(pFull_path, sizeof(pFull_path), "SoundFile/%s", pMonster_hurt_sounds[i]);

        pSFX_manager->ppMonster_hurt_buffers[i] = sfSoundBuffer_createFromFile(pFull_path);
        if (pSFX_manager->ppMonster_hurt_buffers[i])
            ++pSFX_manager->monster_hurt_count;
    }

    srand((unsigned int)time(NULL));
}

void sound_destroy(void) {
    if (!pSound_manager)
        return;

    if (pSound_manager->pBGM) {
        sfMusic_stop(pSound_manager->pBGM);
        sfMusic_destroy(pSound_manager->pBGM);
        pSound_manager->pBGM = NULL;
    }

    free(pSound_manager);
    pSound_manager = NULL;

    if (pSFX_manager) {
        for (int i = 0; i < pSFX_manager->footstep_count; ++i)
            if (pSFX_manager->ppBuffers[i])
                sfSoundBuffer_destroy(pSFX_manager->ppBuffers[i]);

        for (int i = 0; i < pSFX_manager->swing_count; ++i)
            if (pSFX_manager->ppSwing_buffers[i])
                sfSoundBuffer_destroy(pSFX_manager->ppSwing_buffers[i]);

        for (int i = 0; i < pSFX_manager->monster_hurt_count; ++i)
            if (pSFX_manager->ppMonster_hurt_buffers[i])
                sfSoundBuffer_destroy(pSFX_manager->ppMonster_hurt_buffers[i]);

        for (int i = 0; i < MAX_SFX_PLAYERS; ++i)
            if (pSFX_manager->ppSFX_players[i])
                sfSound_destroy(pSFX_manager->ppSFX_players[i]);

        if (pSFX_manager->pSound)
            sfSound_destroy(pSFX_manager->pSound);

        free(pSFX_manager);
        pSFX_manager = NULL;
    }
}

void sound_play_BGM(const char * const pFile_name) {
    if (!pSound_manager || !pFile_name)
        return;

    //기존 BGM 정리
    if (pSound_manager->pBGM) {
        sfMusic_stop(pSound_manager->pBGM);
        sfMusic_destroy(pSound_manager->pBGM);
        pSound_manager->pBGM = NULL;
    }

    char path[256] = { 0 };
    snprintf(path, sizeof(path), "SoundFile/%s", pFile_name);

    pSound_manager->pBGM = sfMusic_createFromFile(path);
    if (!pSound_manager->pBGM)
        return;

    sfMusic_setLoop(pSound_manager->pBGM, sfTrue);
    sfMusic_play(pSound_manager->pBGM);
}

void sound_play_menu_BGM(const char * const pFile_name) {
    if (!pSound_manager || !pFile_name)
        return;

    //다른 BGM이 재생 중이었다면 정지하고 정리
    if (pSound_manager->pBGM) {
        sfMusic_stop(pSound_manager->pBGM);
        sfMusic_destroy(pSound_manager->pBGM);
        pSound_manager->pBGM = NULL;
    }

    char path[256] = { 0 };
    snprintf(path, sizeof(path), "SoundFile/%s", pFile_name);

    pSound_manager->pBGM = sfMusic_createFromFile(path);
    if (!pSound_manager->pBGM)
        return;

    sfMusic_setLoop(pSound_manager->pBGM, sfTrue);
    sfMusic_play(pSound_manager->pBGM);
}

void sound_play_footstep(void) {
    if (!pSFX_manager || !pSFX_manager->footstep_count)
        return;

    if (sfSound_getStatus(pSFX_manager->pSound) == sfPlaying)
        return;

    // 전용 플레이어에 소리를 설정하고 재생
    sfSound_setBuffer(pSFX_manager->pSound, pSFX_manager->ppBuffers[rand() % pSFX_manager->footstep_count]);
    sfSound_play(pSFX_manager->pSound);
}

static void play_SFX(sfSoundBuffer *pBuffer) {
    if (!pSFX_manager || !pBuffer)
        return;

    //비어있는 플레이어를 찾는다
    for (int i = 0; i < MAX_SFX_PLAYERS; ++i)
        if (sfSound_getStatus(pSFX_manager->ppSFX_players[i]) != sfPlaying) {
            sfSound_setBuffer(pSFX_manager->ppSFX_players[i], pBuffer);
            sfSound_play(pSFX_manager->ppSFX_players[i]);
            return;
        }
}

void sound_play_swing(void) {
    if (!pSFX_manager || !pSFX_manager->swing_count)
        return;

    play_SFX(pSFX_manager->ppSwing_buffers[rand() % pSFX_manager->swing_count]);
}

void sound_play_monster_hurt(void) {
    if (!pSFX_manager || pSFX_manager->monster_hurt_count == 0)
        return;

    play_SFX(pSFX_manager->ppMonster_hurt_buffers[rand() % pSFX_manager->monster_hurt_count]);
}