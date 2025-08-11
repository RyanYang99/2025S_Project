#include "leak.h"
#include "sound.h"

#include <stdio.h>

#include <Windows.h>
#include <SFML/Audio.h>

#define MAX_FOOTSTEP_SOUNDS 10
#define MAX_SWING_SOUNDS 3
#define MAX_PLAYER_HURT_SOUNDS 3
#define MAX_MONSTER_HURT_SOUNDS 5
#define MAX_BOSS_HURT_SOUNDS  2
#define MAX_BOSS_LAZER_SOUNDS 2
#define MAX_BOSS_MISSILE_SOUNDS 2

#define MAX_SFX_PLAYERS 10

#define ADD_TO_BUFFER(i) \
SFX_manager.pBuffers[i] = sfSoundBuffer_createFromFile(pPath); \
++count

#define CONCAT_WAV(MAX) \
for (int i = 0; i < MAX; ++i) { \
    char pName[MAX_PATH] = { 0 }; \
    snprintf(pName, MAX_PATH, "%d.wav", i); \
    strcpy_s(pPath, MAX_PATH, pName); \
    ADD_TO_BUFFER(i); \
}

typedef struct {
    sfMusic *pBGM;

    sfSoundBuffer *pBuffers[MAX_FOOTSTEP_SOUNDS + MAX_SWING_SOUNDS + MAX_PLAYER_HURT_SOUNDS + MAX_MONSTER_HURT_SOUNDS +
                            MAX_SFX_PLAYERS + MAX_BOSS_HURT_SOUNDS + MAX_BOSS_LAZER_SOUNDS + MAX_BOSS_MISSILE_SOUNDS + 3];

    sfSound *pSFX_players[MAX_SFX_PLAYERS], *pSound;

    int footstep, swing, player_hurt, monster_hurt, monster_attack, boss_summon, boss_howl, boss_hurt, boss_lazer, boss_missile;
} SFX_manager_t;

static SFX_manager_t SFX_manager = { 0 };

void sound_initialize(void) {
    //효과음 매니저 초기화
    for (int i = 0; i < MAX_SFX_PLAYERS; ++i)
        SFX_manager.pSFX_players[i] = sfSound_create();
    SFX_manager.pSound = sfSound_create();

    const char * const pFolder = "sounds", * const pPlayer = "player", * const pMonster = "monster", * const pBoss = "boss";
    char pPath[MAX_PATH] = { 0 };
    strcpy_s(pPath, MAX_PATH, pFolder);

    int count = 0;
    snprintf(pPath, MAX_PATH, "%s\\%s\\footstep\\", pFolder, pPlayer);
    CONCAT_WAV(MAX_FOOTSTEP_SOUNDS)

    SFX_manager.swing = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\swing\\", pFolder, pPlayer);
    CONCAT_WAV(MAX_SWING_SOUNDS)

    SFX_manager.player_hurt = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\hurt\\", pFolder, pPlayer);
    CONCAT_WAV(MAX_PLAYER_HURT_SOUNDS)

    SFX_manager.monster_hurt = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\hurt\\", pFolder, pMonster);
    CONCAT_WAV(MAX_MONSTER_HURT_SOUNDS)

    SFX_manager.monster_attack = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\attack.wav", pFolder, pMonster);
    ADD_TO_BUFFER(count++);

    SFX_manager.boss_summon = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\summon.wav", pFolder, pBoss);
    ADD_TO_BUFFER(count++);

    SFX_manager.boss_howl = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\howl.wav", pFolder, pBoss);
    ADD_TO_BUFFER(count++);

    SFX_manager.boss_hurt = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\hurt\\", pFolder, pBoss);
    CONCAT_WAV(MAX_BOSS_HURT_SOUNDS)

    SFX_manager.boss_lazer = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\lazer\\", pFolder, pBoss);
    CONCAT_WAV(MAX_BOSS_LAZER_SOUNDS)

    SFX_manager.boss_missile = count;
    snprintf(pPath, MAX_PATH, "%s\\%s\\lazer\\", pFolder, pBoss);
    CONCAT_WAV(MAX_BOSS_MISSILE_SOUNDS)
}

static void stop_BGM(void) {
    if (SFX_manager.pBGM) {
        sfMusic_stop(SFX_manager.pBGM);
        sfMusic_destroy(SFX_manager.pBGM);
        SFX_manager.pBGM = NULL;
    }
}

void sound_destroy(void) {
    stop_BGM();

    for (int i = 0; i < sizeof(SFX_manager.pBuffers); ++i)
        if (SFX_manager.pBuffers[i])
            sfSoundBuffer_destroy(SFX_manager.pBuffers[i]);

    for (int i = 0; i < MAX_SFX_PLAYERS; ++i)
        if (SFX_manager.pSFX_players[i])
            sfSound_destroy(SFX_manager.pSFX_players[i]);

    if (SFX_manager.pSound) 
        sfSound_destroy(SFX_manager.pSound);
}

void sound_play_BGM(const char * const pFile_name) {
    stop_BGM();

    char path[MAX_PATH] = { 0 };
    snprintf(path, MAX_PATH, "sounds\\BGM\\%s.wav", pFile_name);

    SFX_manager.pBGM = sfMusic_createFromFile(path);
    sfMusic_setLoop(SFX_manager.pBGM, sfTrue);
    sfMusic_play(SFX_manager.pBGM);
}

static void play_sfx(const int index) {
    //비어있는 플레이어를 찾는다
    for (int i = 0; i < MAX_SFX_PLAYERS; ++i) {
        if (sfSound_getStatus(SFX_manager.pSFX_players[i]) != sfPlaying) {
            sfSound_setBuffer(SFX_manager.pSFX_players[i], SFX_manager.pBuffers[index]);
            sfSound_play(SFX_manager.pSFX_players[i]);
            return;
        }
    }
}

void sound_play_footstep(void) {
    if (sfSound_getStatus(SFX_manager.pSound) == sfPlaying)
        return;

    play_sfx(SFX_manager.footstep + (rand() % MAX_FOOTSTEP_SOUNDS));
}

void sound_play_swing(void) {
    play_sfx(SFX_manager.swing + (rand() % MAX_SWING_SOUNDS));
}

void sound_play_monster_hurt(void) {
    play_sfx(SFX_manager.monster_hurt + (rand() % MAX_MONSTER_HURT_SOUNDS));
}

void sound_play_monster_attack(void) {
    play_sfx(SFX_manager.monster_attack);
}

void sound_play_player_hurt(void) {
    play_sfx(SFX_manager.player_hurt + (rand() % MAX_PLAYER_HURT_SOUNDS));
}

void sound_play_boss_sound(const boss_sound_type_t type) {
    switch (type) {
        case BOSS_SOUND_SPAWN:
            play_sfx(SFX_manager.boss_summon);
            break;

        case BOSS_SOUND_HURT:
            play_sfx(SFX_manager.boss_hurt + (rand() % MAX_BOSS_HURT_SOUNDS));
            break;

        case BOSS_SOUND_HOWLING:
            play_sfx(SFX_manager.boss_howl);
            break;

        case BOSS_SOUND_LAZER:
            play_sfx(SFX_manager.boss_lazer + (rand() % MAX_BOSS_LAZER_SOUNDS));
            break;

        case BOSS_SOUND_MISSILE:
            play_sfx(SFX_manager.boss_missile + (rand() % MAX_BOSS_MISSILE_SOUNDS));
            break;
    }
}