#include "leak.h"
#include "sound.h"

#include <stdio.h>

#include <Windows.h>
#include <SFML/Audio.h>

#define MAX_PLAYER_FOOTSTEP_SOUNDS 10
#define MAX_PLAYER_SWING_SOUNDS 3
#define MAX_PLAYER_HURT_SOUNDS 3
#define MAX_MONSTER_HURT_SOUNDS 5
#define MAX_BOSS_HURT_SOUNDS  2
#define MAX_BOSS_LASER_SOUNDS 2
#define MAX_BOSS_MISSILE_SOUNDS 2
#define BUFFER_COUNT (MAX_PLAYER_FOOTSTEP_SOUNDS + MAX_PLAYER_SWING_SOUNDS + MAX_PLAYER_HURT_SOUNDS + MAX_MONSTER_HURT_SOUNDS + MAX_BOSS_HURT_SOUNDS + MAX_BOSS_LASER_SOUNDS + MAX_BOSS_MISSILE_SOUNDS + 4)

#define MAX_SFX_PLAYERS 10

#define ADD_TO_BUFFER(path, i, count) SFX_manager.pBuffers[i + count] = sfSoundBuffer_createFromFile(path)

#define CONCAT_WAV(MAX) \
for (int i = 0; i < MAX; ++i) { \
    char pName[MAX_PATH] = { 0 }; \
    snprintf(pName, MAX_PATH, "%d.wav", i); \
    strcpy_s(pFinal, MAX_PATH, pPath); \
    strcat_s(pFinal, MAX_PATH, pName); \
    ADD_TO_BUFFER(pFinal, i, count); \
} \
count += MAX

typedef struct {
    sfMusic *pBGM;

    sfSoundBuffer *pBuffers[BUFFER_COUNT];

    sfSound *pSFX_players[MAX_SFX_PLAYERS], *pSound;

    int player_footstep, player_swing, player_hurt,
        monster_attack, monster_hurt, monster_death,
        boss_summon, boss_howl, boss_hurt, boss_laser, boss_missile;
} SFX_manager_t;

static SFX_manager_t SFX_manager = { 0 };

void sound_initialize(void) {
    //효과음 매니저 초기화
    for (int i = 0; i < MAX_SFX_PLAYERS; ++i)
        SFX_manager.pSFX_players[i] = sfSound_create();
    SFX_manager.pSound = sfSound_create();

    const char * const pFolder = "./sounds", * const pPlayer = "player", * const pMonster = "monster", * const pBoss = "boss";
    char pPath[MAX_PATH] = { 0 }, pFinal[MAX_PATH] = { 0 };
    strcpy_s(pPath, MAX_PATH, pFolder);

    int count = 0;
    snprintf(pPath, MAX_PATH, "%s/%s/footstep/", pFolder, pPlayer);
    CONCAT_WAV(MAX_PLAYER_FOOTSTEP_SOUNDS);

    SFX_manager.player_swing = count;
    snprintf(pPath, MAX_PATH, "%s/%s/swing/", pFolder, pPlayer);
    CONCAT_WAV(MAX_PLAYER_SWING_SOUNDS);

    SFX_manager.player_hurt = count;
    snprintf(pPath, MAX_PATH, "%s/%s/hurt/", pFolder, pPlayer);
    CONCAT_WAV(MAX_PLAYER_HURT_SOUNDS);

    SFX_manager.monster_attack = count;
    snprintf(pPath, MAX_PATH, "%s/%s/attack.wav", pFolder, pMonster);
    ADD_TO_BUFFER(pPath, count++, 0);

    SFX_manager.monster_hurt = count;
    snprintf(pPath, MAX_PATH, "%s/%s/hurt/", pFolder, pMonster);
    CONCAT_WAV(MAX_MONSTER_HURT_SOUNDS);

    SFX_manager.monster_death = count;
    snprintf(pPath, MAX_PATH, "%s/%s/death.wav", pFolder, pMonster);
    ADD_TO_BUFFER(pPath, count++, 0);

    SFX_manager.boss_summon = count;
    snprintf(pPath, MAX_PATH, "%s/%s/summon.wav", pFolder, pBoss);
    ADD_TO_BUFFER(pPath, count++, 0);

    SFX_manager.boss_howl = count;
    snprintf(pPath, MAX_PATH, "%s/%s/howl.wav", pFolder, pBoss);
    ADD_TO_BUFFER(pPath, count++, 0);

    SFX_manager.boss_hurt = count;
    snprintf(pPath, MAX_PATH, "%s/%s/hurt/", pFolder, pBoss);
    CONCAT_WAV(MAX_BOSS_HURT_SOUNDS);

    SFX_manager.boss_laser = count;
    snprintf(pPath, MAX_PATH, "%s/%s/laser/", pFolder, pBoss);
    CONCAT_WAV(MAX_BOSS_LASER_SOUNDS);

    SFX_manager.boss_missile = count;
    snprintf(pPath, MAX_PATH, "%s/%s/missile/", pFolder, pBoss);
    CONCAT_WAV(MAX_BOSS_MISSILE_SOUNDS);
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

    for (int i = 0; i < BUFFER_COUNT; ++i)
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
    snprintf(path, MAX_PATH, "./sounds/BGM/%s.wav", pFile_name);

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

void sound_play_sound_effect(const sound_effect_t sound_effect) {
    switch (sound_effect) {
        case PLAYER_SOUND_FOOTSTEP:
            play_sfx(SFX_manager.player_footstep + (rand() % MAX_PLAYER_FOOTSTEP_SOUNDS));
            break;

        case PLAYER_SOUND_SWING:
            play_sfx(SFX_manager.player_swing + (rand() % MAX_PLAYER_SWING_SOUNDS));
            break;

        case PLAYER_SOUND_HURT:
            play_sfx(SFX_manager.player_hurt + (rand() % MAX_PLAYER_HURT_SOUNDS));
            break;

        case MONSTER_SOUND_ATTACK:
            play_sfx(SFX_manager.monster_attack);
            break;

        case MONSTER_SOUND_HURT:
            play_sfx(SFX_manager.monster_hurt + (rand() % MAX_MONSTER_HURT_SOUNDS));
            break;

        case MONSTER_SOUND_DEATH:
            play_sfx(SFX_manager.monster_death);
            break;

        case BOSS_SOUND_SPAWN:
            play_sfx(SFX_manager.boss_summon);
            break;

        case BOSS_SOUND_HURT:
            play_sfx(SFX_manager.boss_hurt + (rand() % MAX_BOSS_HURT_SOUNDS));
            break;

        case BOSS_SOUND_HOWLING:
            play_sfx(SFX_manager.boss_howl);
            break;

        case BOSS_SOUND_LASER:
            play_sfx(SFX_manager.boss_laser + (rand() % MAX_BOSS_LASER_SOUNDS));
            break;

        case BOSS_SOUND_MISSILE:
            play_sfx(SFX_manager.boss_missile + (rand() % MAX_BOSS_MISSILE_SOUNDS));
            break;
    }
}