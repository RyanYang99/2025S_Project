#define _CRT_SECURE_NO_WARNINGS

#include "sound.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "SFML/Audio.h"

static const char* footstepSounds[MAX_FOOTSTEP_SOUNDS] = {
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
};

static const char* swingSounds[MAX_SWING_SOUNDS] = {
    "Swing/swing.wav",
    "Swing/swing2.wav",
    "Swing/swing3.wav"
};

static const char* monsterHurtSounds[MAX_HURT_SOUNDS] = {
    "Monster/Hurt/Hurt.wav",
    "Monster/Hurt/Hurt2.wav",
    "Monster/Hurt/Hurt3.wav",
    "Monster/Hurt/Hurt4.wav",
    "Monster/Hurt/Hurt5.wav"
};

typedef struct {
    sfMusic* bgm;
} SoundManager;

typedef struct {
    sfSoundBuffer* buffers[MAX_FOOTSTEP_SOUNDS];
    sfSoundBuffer* swingBuffers[MAX_SWING_SOUNDS];
    sfSoundBuffer* monsterHurtBuffers[MAX_HURT_SOUNDS];
    sfSound* sfxPlayers[MAX_SFX_PLAYERS];
    sfSound* sound;
    int footstepCount;
    int swingCount;
    int monsterHurtCount;
} SFXManager;


static SFXManager* s_sfx = NULL;
static SoundManager* s_manager = NULL;

static void play_sfx(sfSoundBuffer* buffer) {
    if (!s_sfx || !buffer) return;

    // 비어있는 플레이어를 찾는다
    for (int i = 0; i < MAX_SFX_PLAYERS; i++) {
        if (sfSound_getStatus(s_sfx->sfxPlayers[i]) != sfPlaying) {
            sfSound_setBuffer(s_sfx->sfxPlayers[i], buffer);
            sfSound_play(s_sfx->sfxPlayers[i]);
            return;
        }
    }
}

void Sound_init() {
    if (s_manager != NULL) return;

    s_manager = (SoundManager*)malloc(sizeof(SoundManager));
    if (!s_manager) return;

    s_manager->bgm = NULL;

    // 효과음 매니저 초기화
    s_sfx = (SFXManager*)malloc(sizeof(SFXManager));
    if (!s_sfx) return;

    for (int i = 0; i < MAX_SFX_PLAYERS; i++) {
        s_sfx->sfxPlayers[i] = sfSound_create();
    }
    s_sfx->sound = sfSound_create();
    s_sfx->footstepCount = 0;
    s_sfx->swingCount = 0;
    s_sfx->monsterHurtCount = 0;

    for (int i = 0; i < MAX_FOOTSTEP_SOUNDS; i++) {
        if (footstepSounds[i] == NULL) break;

        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", footstepSounds[i]);

        s_sfx->buffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->buffers[i]) {
            s_sfx->footstepCount++;
        }
    }

    for (int i = 0; i < MAX_SWING_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", swingSounds[i]);
        s_sfx->swingBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->swingBuffers[i]) {
            s_sfx->swingCount++;
        }
    }

    for (int i = 0; i < MAX_HURT_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", monsterHurtSounds[i]);
        s_sfx->monsterHurtBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->monsterHurtBuffers[i]) {
            s_sfx->monsterHurtCount++;
        }
    }

    srand((unsigned int)time(NULL));
}

void Sound_playBGM(const char* filename) {
    if (!s_manager || !filename) return;

    // 기존 BGM 정리
    if (s_manager->bgm) {
        sfMusic_stop(s_manager->bgm);
        sfMusic_destroy(s_manager->bgm);
        s_manager->bgm = NULL;
    }

    char path[256];
    snprintf(path, sizeof(path), "SoundFile/%s", filename);

    s_manager->bgm = sfMusic_createFromFile(path);
    if (!s_manager->bgm) {
        fprintf(stderr, "Failed to load BGM file: %s\n", path);
        return;
    }

    sfMusic_setLoop(s_manager->bgm, sfTrue);
    sfMusic_play(s_manager->bgm);
}

void Sound_playMenuBGM(const char* filename) {
    if (!s_manager || !filename) return;

    // 다른 BGM이 재생 중이었다면 정지하고 정리
    if (s_manager->bgm) {
        sfMusic_stop(s_manager->bgm);
        sfMusic_destroy(s_manager->bgm);
        s_manager->bgm = NULL;
    }

    char path[256];
    snprintf(path, sizeof(path), "SoundFile/%s", filename);

    s_manager->bgm = sfMusic_createFromFile(path);
    if (!s_manager->bgm) {
        fprintf(stderr, "Failed to load Menu BGM file: %s\n", path);
        return;
    }

    sfMusic_setLoop(s_manager->bgm, sfTrue);
    sfMusic_play(s_manager->bgm);
}

void Sound_playFootstep() {
    if (!s_sfx || s_sfx->footstepCount == 0) return;

    if (sfSound_getStatus(s_sfx->sound) == sfPlaying) return;

    int index = rand() % s_sfx->footstepCount;

    // 전용 플레이어에 소리를 설정하고 재생합니다.
    sfSound_setBuffer(s_sfx->sound, s_sfx->buffers[index]);
    sfSound_play(s_sfx->sound);
}

void Sound_playSwing() {
    if (!s_sfx || s_sfx->swingCount == 0) return;
    int index = rand() % s_sfx->swingCount;
    play_sfx(s_sfx->swingBuffers[index]);
}

void Sound_playMonsterHurt() {
    if (!s_sfx || s_sfx->monsterHurtCount == 0) return;
    int index = rand() % s_sfx->monsterHurtCount;
    play_sfx(s_sfx->monsterHurtBuffers[index]);
}

void Sound_shutdown() {
    if (!s_manager) return;

    if (s_manager->bgm) {
        sfMusic_stop(s_manager->bgm);
        sfMusic_destroy(s_manager->bgm);
        s_manager->bgm = NULL;
    }

    free(s_manager);
    s_manager = NULL;

    if (s_sfx) {
        for (int i = 0; i < s_sfx->footstepCount; i++) { if (s_sfx->buffers[i]) sfSoundBuffer_destroy(s_sfx->buffers[i]); }
        for (int i = 0; i < s_sfx->swingCount; i++) { if (s_sfx->swingBuffers[i]) sfSoundBuffer_destroy(s_sfx->swingBuffers[i]); }
        for (int i = 0; i < s_sfx->monsterHurtCount; i++) { if (s_sfx->monsterHurtBuffers[i]) sfSoundBuffer_destroy(s_sfx->monsterHurtBuffers[i]); }


        for (int i = 0; i < MAX_SFX_PLAYERS; i++) {
            if (s_sfx->sfxPlayers[i]) sfSound_destroy(s_sfx->sfxPlayers[i]);
        }

        if (s_sfx->sound) {
            sfSound_destroy(s_sfx->sound);
        }

        free(s_sfx);
        s_sfx = NULL;
    }
}

