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

typedef struct {
    sfMusic* bgm;
} SoundManager;

typedef struct {
    sfSoundBuffer* buffers[MAX_FOOTSTEP_SOUNDS];
    sfSound* sound;
    int footstepCount;
} SFXManager;


static SFXManager* s_sfx = NULL;
static SoundManager* s_manager = NULL;

void Sound_init() {
    if (s_manager != NULL) return;

    s_manager = (SoundManager*)malloc(sizeof(SoundManager));
    if (!s_manager) return;

    s_manager->bgm = NULL;

    // 효과음 매니저 초기화
    s_sfx = (SFXManager*)malloc(sizeof(SFXManager));
    if (!s_sfx) return;

    s_sfx->sound = sfSound_create();
    s_sfx->footstepCount = 0;

    for (int i = 0; i < MAX_FOOTSTEP_SOUNDS; i++) {
        if (footstepSounds[i] == NULL) break;

        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "SoundFile/%s", footstepSounds[i]);

        s_sfx->buffers[i] = sfSoundBuffer_createFromFile(fullPath);
        if (s_sfx->buffers[i]) {
            s_sfx->footstepCount++;
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

    // CSFML에서는 반드시 createFromFile 사용
    s_manager->bgm = sfMusic_createFromFile(path);
    if (!s_manager->bgm) {
        fprintf(stderr, "Failed to load BGM file: %s\n", path);
        return;
    }

    sfMusic_setLoop(s_manager->bgm, sfTrue);
    sfMusic_play(s_manager->bgm);
}

void Sound_playFootstep() {
    if (!s_sfx || s_sfx->footstepCount == 0) return;

    if (sfSound_getStatus(s_sfx->sound) == sfPlaying) return;


    int index = rand() % s_sfx->footstepCount;

    sfSound_stop(s_sfx->sound);  // 이전 소리 멈추기
    sfSound_setBuffer(s_sfx->sound, s_sfx->buffers[index]);
    sfSound_play(s_sfx->sound);
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
        for (int i = 0; i < s_sfx->footstepCount; i++) {
            if (s_sfx->buffers[i]) {
                sfSoundBuffer_destroy(s_sfx->buffers[i]);
            }
        }
        if (s_sfx->sound) {
            sfSound_destroy(s_sfx->sound);
        }
        free(s_sfx);
        s_sfx = NULL;
    }
}





//// 게임화면 BGM
//PlayBGM()
//{
//    PlaySound(TEXT("SoundFile\\BGM\\fixed_roop1.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
//
//}
//
////시작화면 루프에 같이 넣어두면 됨
//PlayStartMenuBGM()
//{
//    PlaySound(TEXT("SoundFile\\BGM\\Caketown-1_1.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
//}


//static const char* bgmFiles[] = { "fixed_roop1.wav", "fixed_roop2.wav" };
//static const int totalTracks = 2;
//static int currentTrack = 0;
//
//// MCI 오류를 확인하고 메시지를 출력하는 함수
//static void checkMciError(MCIERROR error, const char* command) {
//    if (error != 0) {
//        char errorText[128];
//        mciGetErrorString(error, errorText, sizeof(errorText));
//        // 오류 발생 시 콘솔에 메시지 출력
//        fprintf(stderr, "[BGM ERROR] Command: %s\n             Error: %s\n", command, errorText);
//    }
//}
//
//// BGM 시스템 초기화
//void initBgmSystem() {
//    char command[256];
//    MCIERROR error;
//
//    sprintf_s(command, sizeof(command), "open \"%s\" alias bgm", bgmFiles[currentTrack]);
//    error = mciSendString(command, NULL, 0, NULL);
//    checkMciError(error, command); // 오류 확인
//    if (error != 0) return; // 파일 열기에 실패하면 더 이상 진행하지 않음
//
//    error = mciSendString("play bgm", NULL, 0, NULL);
//    checkMciError(error, "play bgm"); // 오류 확인
//}
//
//// BGM 상태 업데이트
//void updateBgm() {
//    char status[128] = { 0 };
//    char command[256];
//    MCIERROR error;
//
//    // 'bgm' 별칭이 유효한지 먼저 확인
//    error = mciSendString("status bgm mode", status, sizeof(status), NULL);
//    if (error != 0) {
//        // 장치가 닫혔거나 오류가 발생한 상태일 수 있으므로 여기서 함수 종료
//        return;
//    }
//
//    if (strcmp(status, "stopped") == 0 || strcmp(status, "not ready") == 0) {
//        mciSendString("close bgm", NULL, 0, NULL); // 이전 리소스 정리
//
//        currentTrack = (currentTrack + 1) % totalTracks;
//
//        sprintf_s(command, sizeof(command), "open \"%s\" alias bgm", bgmFiles[currentTrack]);
//        error = mciSendString(command, NULL, 0, NULL);
//        checkMciError(error, command);
//        if (error != 0) return;
//
//        error = mciSendString("play bgm", NULL, 0, NULL);
//        checkMciError(error, "play bgm");
//    }
//}
//
//// BGM 시스템 종료
//void closeBgmSystem() {
//    mciSendString("close bgm", NULL, 0, NULL);
//}