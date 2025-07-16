#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "player.h"
#include "map.h"
#include "console.h"


#define Max_Mob 5


//typedef struct
//{
//	int x;
//	int y;
//}CheckSpawn;


typedef struct {
    int x;  // 월드 좌표 X
    int y;  // 월드 좌표 Y
    //int type; // 몹 종류 등 기타 정보
    int HP;
    int atk;
} Mob;


Mob mobs[Max_Mob]; // 몹 최대치와 몹관리를 위한 배열


void Mob_Spawn_Time();
void MobSpawn(int x, int y);
void Mob_render();





