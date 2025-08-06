#define _CRT_SECURE_NO_WARNINGS

#include "sound.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "SFML/Audio.h"

typedef struct {
    sfMusic* bgm;
} SoundManager;

static SoundManager* s_manager = NULL;

void Sound_init() {
    if (s_manager != NULL) return;

    s_manager = (SoundManager*)malloc(sizeof(SoundManager));
    if (!s_manager) return;

    s_manager->bgm = NULL;
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

void Sound_shutdown() {
    if (!s_manager) return;

    if (s_manager->bgm) {
        sfMusic_stop(s_manager->bgm);
        sfMusic_destroy(s_manager->bgm);
        s_manager->bgm = NULL;
    }

    free(s_manager);
    s_manager = NULL;
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