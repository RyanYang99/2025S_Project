#pragma once

#include <windows.h>
#include "console.h"

// 플레이어 상태 구조체
typedef struct {
    int x, y; // 맵 상의 위치
    int hp;   // 체력 등 추가 가능
    // --- 애니메이션 상태 변수 추가 ---
    int is_moving;         // 현재 움직이는 중인지 (0 또는 1)
    float animation_timer;   // 현재 프레임을 얼마나 오래 보여줬는지 추적
    int current_frame;     // 현재 보여줄 애니메이션 프레임 인덱스
    // 기타 플레이어 속성
} player_t;

// 전역 플레이어 객체
extern player_t player;

// 플레이어 초기화
void player_init(int x, int y);

// 플레이어 이동
void player_move(int dx, int dy);

void player_update(float delta_time); // Delta Time을 인자로 받음

// 플레이어 렌더링
void render_player(void);