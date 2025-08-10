#define _CRT_SECURE_NO_WARNINGS

#include "sound.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "SFML/Audio.h"

#define MAX_FOOTSTEP_SOUNDS 10
#define MAX_SWING_SOUNDS 3
#define MAX_MONSTER_HURT_SOUNDS 5
#define MAX_SFX_PLAYERS 10
#define MAX_PLAYER_HURT_SOUNDS 3
#define MAX_MONSTER_ATTACK_SOUNDS 1
#define MAX_BOSS_SPAWN_SOUNDS 1
#define MAX_BOSS_HURT_SOUNDS  5
#define MAX_BOSS_HOWLING_SOUNDS 1
#define MAX_BOSS_LAZER_SOUNDS 2
#define MAX_BOSS_MISSILE_SOUNDS 2

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

static const char* monsterHurtSounds[MAX_MONSTER_HURT_SOUNDS] = {
    "Monster/Hurt/Hurt.wav",
    "Monster/Hurt/Hurt2.wav",
    "Monster/Hurt/Hurt3.wav",
    "Monster/Hurt/Hurt4.wav",
    "Monster/Hurt/Hurt5.wav"
};

static const char* playerHurtSounds[MAX_PLAYER_HURT_SOUNDS] = {
    "Player/player_hurt.wav",
    "Player/player_hurt2.wav",
    "Player/player_hurt3.wav"
};

static const char* monsterAttackSounds[MAX_MONSTER_ATTACK_SOUNDS] = {
    "Monster/Attack/monster_attack.wav"
};

static const char* bossSpawnSounds[MAX_BOSS_SPAWN_SOUNDS] = {
    "Boss/BossSummoned/BossSummoned.wav"
};

static const char* bossHurtSounds[MAX_BOSS_HURT_SOUNDS] = {
    "Boss/BossHurt/BossHurt1.wav",
    "Boss/BossHurt/BossHurt2.wav"
};

static const char* bossHowlingSounds[MAX_BOSS_HOWLING_SOUNDS] = {
    "Boss/BossHowling/BossHowling.wav"
};

static const char* bossLazerSounds[MAX_BOSS_LAZER_SOUNDS] = {
    "Boss/BossLazer/LazerSound1.wav",
    "Boss/BossLazer/LazerSound2.wav"
};

static const char* bossMissileSounds[MAX_BOSS_MISSILE_SOUNDS] = {
    "Boss/BossMissile/MissileSound1.wav",
    "Boss/BossMissile/MissileSound2.wav"
};


typedef struct {
    sfMusic* bgm;
    char* currentBgmPath;  // 현재 BGM 경로
    char* previousBgmPath; // 이전 BGM 경로
} SoundManager;

typedef struct {
    sfSoundBuffer* buffers[MAX_FOOTSTEP_SOUNDS];
    sfSoundBuffer* swingBuffers[MAX_SWING_SOUNDS];
    sfSoundBuffer* monsterHurtBuffers[MAX_MONSTER_HURT_SOUNDS];
    sfSoundBuffer* playerHurtBuffers[MAX_PLAYER_HURT_SOUNDS];
    sfSoundBuffer* monsterAttackBuffers[MAX_MONSTER_ATTACK_SOUNDS];
    sfSoundBuffer* bossSpawnBuffers[MAX_BOSS_SPAWN_SOUNDS];
    sfSoundBuffer* bossHurtBuffers[MAX_BOSS_HURT_SOUNDS];
    sfSoundBuffer* bossHowlingBuffers[MAX_BOSS_HOWLING_SOUNDS];
    sfSoundBuffer* bossLazerBuffers[MAX_BOSS_LAZER_SOUNDS];
    sfSoundBuffer* bossMissileBuffers[MAX_BOSS_MISSILE_SOUNDS];

    sfSound* sfxPlayers[MAX_SFX_PLAYERS];
    sfSound* sound;
    int footstepCount;
    int swingCount;
    int monsterHurtCount;
    int playerHurtCount;
    int monsterAttackCount;
    int bossSpawnCount;
    int bossHurtCount;
    int bossHowlingCount;
    int bossLazerCount;
    int bossMissileCount;
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
    s_manager->currentBgmPath = NULL;
    s_manager->previousBgmPath = NULL;


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
    s_sfx->playerHurtCount = 0;
    s_sfx->monsterAttackCount = 0;
    s_sfx->bossSpawnCount = 0;
    s_sfx->bossHurtCount = 0;
    s_sfx->bossHowlingCount = 0;
    s_sfx->bossLazerCount = 0;
    s_sfx->bossMissileCount = 0;

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

    for (int i = 0; i < MAX_MONSTER_HURT_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", monsterHurtSounds[i]);
        s_sfx->monsterHurtBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->monsterHurtBuffers[i]) {
            s_sfx->monsterHurtCount++;
        }
    }

    for (int i = 0; i < MAX_PLAYER_HURT_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", playerHurtSounds[i]);
        s_sfx->playerHurtBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->playerHurtBuffers[i]) {
            s_sfx->playerHurtCount++;
        }
    }

    for (int i = 0; i < MAX_MONSTER_ATTACK_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", monsterAttackSounds[i]);
        s_sfx->monsterAttackBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->monsterAttackBuffers[i]) {
            s_sfx->monsterAttackCount++;
        }
    }

    for (int i = 0; i < MAX_BOSS_SPAWN_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", bossSpawnSounds[i]);
        s_sfx->bossSpawnBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->bossSpawnBuffers[i]) {
            s_sfx->bossSpawnCount++;
        }
    }

    for (int i = 0; i < MAX_BOSS_HURT_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", bossHurtSounds[i]);
        s_sfx->bossHurtBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->bossHurtBuffers[i]) {
            s_sfx->bossHurtCount++;
        }
    }

    for (int i = 0; i < MAX_BOSS_LAZER_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", bossLazerSounds[i]);
        s_sfx->bossLazerBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->bossLazerBuffers[i]) {
            s_sfx->bossLazerCount++;
        }
    }

    for (int i = 0; i < MAX_BOSS_MISSILE_SOUNDS; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", bossMissileSounds[i]);
        s_sfx->bossMissileBuffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->bossMissileBuffers[i]) {
            s_sfx->bossMissileCount++;
        }
    }

    srand((unsigned int)time(NULL));
}

void Sound_PushBGM(const char* new_bgm_filename) {
    if (!s_manager) return;
    // 현재브금 기억
    if (s_manager->currentBgmPath) {
        if (s_manager->previousBgmPath) free(s_manager->previousBgmPath);
        s_manager->previousBgmPath = _strdup(s_manager->currentBgmPath);
    }
    // 새로운 BGM재생
    Sound_playBGM(new_bgm_filename);
}

void Sound_PopBGM() {
    if (!s_manager || !s_manager->previousBgmPath) return;
    // 이전 BGM재생
    Sound_playBGM(s_manager->previousBgmPath);
    free(s_manager->previousBgmPath);
    s_manager->previousBgmPath = NULL;
}


void Sound_playBGM(const char* filename) {
    if (!s_manager || !filename) return;
    if (s_manager->bgm) { sfMusic_stop(s_manager->bgm); sfMusic_destroy(s_manager->bgm); }
    if (s_manager->currentBgmPath) free(s_manager->currentBgmPath);

    s_manager->currentBgmPath = _strdup(filename); // 현재 재생할 곡의 경로를 저장

    char path[256];
    snprintf(path, sizeof(path), "SoundFile/%s", filename);
    s_manager->bgm = sfMusic_createFromFile(path);
    if (!s_manager->bgm) { /*...*/ return; }
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

void Sound_playPlayerHurt() {
    if (!s_sfx || s_sfx->playerHurtCount == 0) return;
    int index = rand() % s_sfx->playerHurtCount;
    play_sfx(s_sfx->playerHurtBuffers[index]);
}

void Sound_playMosterAttack() {
    if (!s_sfx || s_sfx->monsterAttackCount == 0) return;
    int index = rand() % s_sfx->monsterAttackCount;
    play_sfx(s_sfx->monsterAttackBuffers[index]);
}

void Sound_playBossSound(boss_sound_type_t type) {
    if (!s_sfx) return;

    int index = 0;

    switch (type) {
    case BOSS_SOUND_SPAWN:
        if (s_sfx->bossSpawnCount == 0) return;
        index = rand() % s_sfx->bossSpawnCount;
        play_sfx(s_sfx->bossSpawnBuffers[index]);
        break;

    case BOSS_SOUND_HURT:
        if (s_sfx->bossHurtCount == 0) return;
        index = rand() % s_sfx->bossHurtCount;
        play_sfx(s_sfx->bossHurtBuffers[index]);
        break;
       
    case BOSS_SOUND_HOWLING:
        if (s_sfx->bossHowlingCount == 0) return;
        index = rand() % s_sfx->bossHowlingCount;
        play_sfx(s_sfx->bossHowlingBuffers[index]);
        break;

    case BOSS_SOUND_LAZER:
        if (s_sfx->bossLazerCount == 0) return;
        index = rand() % s_sfx->bossLazerCount;
        play_sfx(s_sfx->bossLazerBuffers[index]);
        break;

    case BOSS_SOUND_MISSILE:
        if (s_sfx->bossMissileCount == 0) return;
        index = rand() % s_sfx->bossMissileCount;
        play_sfx(s_sfx->bossMissileBuffers[index]);
        break;

        // 확장시 case 추가
    default:
        break;
    }
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
        for (int i = 0; i < s_sfx->playerHurtCount; i++) { if (s_sfx->playerHurtBuffers[i]) sfSoundBuffer_destroy(s_sfx->playerHurtBuffers[i]); }
        for (int i = 0; i < s_sfx->bossSpawnCount; i++) { if (s_sfx->bossSpawnBuffers[i]) sfSoundBuffer_destroy(s_sfx->bossSpawnBuffers[i]); }
        for (int i = 0; i < s_sfx->bossHurtCount; i++) { if (s_sfx->bossHurtBuffers[i]) sfSoundBuffer_destroy(s_sfx->bossHurtBuffers[i]); }
        for (int i = 0; i < s_sfx->bossHowlingCount; i++) { if (s_sfx->bossHowlingBuffers[i]) sfSoundBuffer_destroy(s_sfx->bossHowlingBuffers[i]); }
        for (int i = 0; i < s_sfx->bossLazerCount; i++) { if (s_sfx->bossLazerBuffers[i]) sfSoundBuffer_destroy(s_sfx->bossLazerBuffers[i]); }
        for (int i = 0; i < s_sfx->bossMissileCount; i++) { if (s_sfx->bossMissileBuffers[i]) sfSoundBuffer_destroy(s_sfx->bossMissileBuffers[i]); }

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

