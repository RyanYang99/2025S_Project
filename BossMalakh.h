#pragma once

#include "astar.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <Windows.h> //COORD

#include "player.h"
#include "console.h"
#include "map.h"
#include "astar.h"


//���� state
typedef enum
{
	E_BOOS_STATE_IDLE,  //���
	E_BOOS_STATE_CHASE, //�߰�
	E_BOOS_STATE_ATTACK, // ���� 
	E_BOOS_STATE_ULTIMATE, //Ư�� ���� (�ñر�)
	E_BOOS_STATE_DAMAGED, //�ǰ� ��

	//== ������ ���� ===
	E_BOOS_STATE_PHASE1,
	E_BOOS_STATE_PHASE2,
	E_BOOS_STATE_PHASE3,

	E_BOOS_STATE_DEFEATED // ��� (óġ��)

}BossState;


//���� ����ũ (���긮��� õ��� �ǹ� )
typedef struct 
{
	int x;
	int y;
	int hp;
	int Max_hp;
	int atk;
	char display_char;
	BossState state; //������ ���� ���� ���� ���� ����
	long last_action_time; // ������ �ൿ �ð�
	long last_move_time; //������ �̵� �ð�
	long special_attack_cooltime; //Ư�� ���� ��Ÿ��
	long last_special_attack_time; //������ Ư�� ���� �ð�

	// A* ��� ĳ���� ���� ���� �߰�
	path_t cached_path;
	COORD last_player_pos; // ���������� ��θ� ����� ������ �÷��̾� ��ġ

	color_tchar_t sprite_data[20][20];
}BossMalakh;

//���� ����
extern BossMalakh boss;

//�Լ� 
//���� �ʱ�ȭ
void Boss_Init(int start_x, int start_y, int init_hp, int attack_power);
//���� ������
void Boss_Render();
//���� AI
void Boss_Update_Ai();
//���� ������ ������ �Լ�
void Boss_Take_Damage(int damage);
//���� �浹 ó��
void Boss_Player_Collision();

//���ڿ� ���� ���
static void Boss_Print_String_Color_W(const wchar_t* wstr, const COORD position, unsigned short background_color, unsigned short foreground_color);