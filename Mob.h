// Mob.h
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h> // COORD를 사용하기 위해 추가

#include "player.h"
#include "map.h"
#include "console.h"
#include "astar.h"

#define Max_Mob 5

typedef struct {
    int x;  // 월드 좌표 X
    int y;  // 월드 좌표 Y
    int HP;
    int atk;
    int state; // 현재 상태
    // 1 :
    // 2 :
    // 3 :
    int despawn_check; //디스폰 체크

    // 물리 상태 변수
    float precise_y;      // 부동 소수점 정밀도의 Y 위치
    float precise_x;
    float velocity_y;     // 수직 속도 (양수: 아래, 음수: 위)

    bool is_on_ground;    // 땅에 닿아 있는지 여부

    long last_move_time; // 몬스터 개별 이동 시간 조절을 위한 타이머
} Mob;

extern Mob mobs[Max_Mob];
extern int mob_count; // mobs 배열의 현재 유효한 몹 개수
extern int mob_level; // 난이도

//좀비 그래픽 스프라이트 데이터
extern const color_tchar_t zombie_sprite_date[5][5];




void Mob_Spawn_Time();
void MobSpawn(int x, int y);
void Mob_render();
void DespawnMob();
void Mob_deadcheck();

//플레이어 물리적용 파쿠리
void Mob_physics();

void update_mob_ai(void);