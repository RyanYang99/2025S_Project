#pragma once

#include <windows.h>
#include <stdbool.h>
#include "console.h"

// 플레이어 상태 구조체
typedef struct {
    int x, y; // 맵 상의 위치
    int hp;   // 체력

    // 애니메이션 상태 변수
    int is_moving;           // 현재 움직이는 중인지 (0 또는 1)
    float animation_timer;   // 현재 프레임을 얼마나 오래 보여줬는지 추적
    int current_frame;       // 현재 보여줄 애니메이션 프레임 인덱스

    // 물리 상태 변수
    float precise_y;     // 부동 소수점 정밀도의 Y 위치
    float velocity_y;    // 수직 속도 (양수: 아래, 음수: 위)
    bool is_on_ground;   // 땅에 닿아 있는지 여부

    float move_cooldown_timer;   // 수평 이동 쿨다운 타이머
} player_t;

// 전역 플레이어 객체
extern player_t player;

// 플레이어 초기화
void player_init(int x);

int find_ground_pos(int x);

// 플레이어 이동
// void player_move(int dx, int dy);

void player_update(void);

bool is_walkable(int x, int y);

// 플레이어 렌더링
void render_player(void);

//플레이어 추가 - 승준 ( 전투 시스템 )
extern void player_take_damage(int damage);