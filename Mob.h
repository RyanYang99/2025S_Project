// Mob.h
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h> // COORD�� ����ϱ� ���� �߰�

#include "player.h"
#include "map.h"
#include "console.h"
#include "astar.h" 


#define Max_Mob 5


typedef struct {
    int x;  // ���� ��ǥ X
    int y;  // ���� ��ǥ Y
    int HP;
    int atk;
    long last_move_time; // ���� ���� �̵� �ð� ������ ���� Ÿ�̸�
} Mob;

extern Mob mobs[Max_Mob]; 
extern int mob_count; // mobs �迭�� ���� ��ȿ�� �� ����
extern int mob_level; // ���̵�

void Mob_Spawn_Time();
void MobSpawn(int x, int y);
void Mob_render();

void update_mob_ai(void);