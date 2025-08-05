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

#pragma pack(push, 1)
typedef struct {
    int x;  // 월드 좌표 X
    int y;  // 월드 좌표 Y
    int HP;
    int atk;
    long last_move_time; // 몬스터 개별 이동 시간 조절을 위한 타이머
} Mob;
#pragma pack(pop)

extern Mob mobs[Max_Mob]; 
extern int mob_count; // mobs 배열의 현재 유효한 몹 개수
extern int mob_level; // 난이도

void load_mob(void);

void Mob_Spawn_Time();
void MobSpawn(int x, int y);
void Mob_render();

void save_mob(void);

void update_mob_ai(void);