﻿#pragma once

#include <windows.h>
#include "console.h"

// 플레이어 상태 구조체
typedef struct {
    int x, y; // 맵 상의 위치
    int hp;   // 체력 등 추가 가능
    // 기타 플레이어 속성
} player_t;

// 전역 플레이어 객체
extern player_t player;

// 플레이어 초기화
void player_init(int x, int y);

// 플레이어 이동
void player_move(int dx, int dy);

// 플레이어 렌더링
void render_player(void);