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


//보스 state
typedef enum
{
	E_BOOS_STATE_IDLE,  //대기
	E_BOOS_STATE_CHASE, //추격
	E_BOOS_STATE_ATTACK, // 공격 
	E_BOOS_STATE_ULTIMATE, //특수 공격 (궁극기)
	E_BOOS_STATE_DAMAGED, //피격 시

	//== 페이즈 상태 ===
	E_BOOS_STATE_PHASE1,
	E_BOOS_STATE_PHASE2,
	E_BOOS_STATE_PHASE3,

	E_BOOS_STATE_DEFEATED // 사망 (처치됨)

}BossState;


//보스 말라크 (히브리어로 천사란 의미 )
typedef struct 
{
	int x;
	int y;
	int hp;
	int Max_hp;
	int atk;
	char display_char;
	BossState state; //페이즈 상태 포함 현재 보스 상태
	long last_action_time; // 마지막 행동 시간
	long last_move_time; //마지막 이동 시간
	long special_attack_cooltime; //특수 공격 쿨타임
	long last_special_attack_time; //마지막 특수 공격 시간

	// A* 경로 캐싱을 위한 변수 추가
	path_t cached_path;
	COORD last_player_pos; // 마지막으로 경로를 계산한 시점의 플레이어 위치

	color_tchar_t sprite_data[20][20];
}BossMalakh;

//전역 변수
extern BossMalakh boss;

//함수 
//보스 초기화
void Boss_Init(int start_x, int start_y, int init_hp, int attack_power);
//보스 렌더링
void Boss_Render();
//보스 AI
void Boss_Update_Ai();
//보스 데미지 입히는 함수
void Boss_Take_Damage(int damage);
//보스 충돌 처리
void Boss_Player_Collision();

//문자열 색상 출력
static void Boss_Print_String_Color_W(const wchar_t* wstr, const COORD position, unsigned short background_color, unsigned short foreground_color);