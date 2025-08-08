#pragma once
#include "astar.h"
#include "map.h"
#include "player.h"
#include "Windows.h" 
#include "conio.h"

// 보스 상태
typedef enum {
	E_BOSS_STATE_PHASE1,
	E_BOSS_STATE_PHASE2,
	E_BOSS_STATE_PHASE3,
	E_BOSS_STATE_DAMAGED,
	E_BOSS_STATE_DEFEATED
} E_BOOS_STATE;

#define BOSS_SPRITE_WIDTH 20
#define BOSS_SPRITE_HEIGHT 20
#define BOSS_DRAW_SCALE 3 // 렌더링 스케일

#define MAX_BOSS_DAMAGE_TEXTS 10
#define BOSS_DAMAGE_TEXT_DURATION 1.0f 

// 보스 구조체
typedef struct {
	int x, y;
	int hp, Max_hp;
	int atk;
	E_BOOS_STATE state;

	// 델타 타임 기반 타이머와 쿨타임
	float action_timer; // 상태 전환용 타이머
	float missile_timer;
	float horizontal_laser_timer;
	float vertical_laser_timer;

	float missile_attack_cooltime;
	float horizontal_laser_cooltime;
	float vertical_laser_cooltime;

	// 레이저 패턴 
	bool is_horizontal_laser_active;
	bool is_vertical_laser_active;

	bool horizontal_laser_from_right;
	int vertical_laser_target_x;
	int horizontal_laser_target_y;


	int current_horizontal_laser_y; 
	int current_vertical_laser_x; 
	bool is_vertical_laser_from_left; // true: 왼쪽 -> 오른쪽, false: 오른쪽 -> 왼쪽
	

	color_tchar_t sprite_data[BOSS_SPRITE_HEIGHT][BOSS_SPRITE_WIDTH];
} BossMalakh;

// 미사일 구조체
#define MAX_MISSILES 10
typedef struct {
	int x, y;
	float vel_x, vel_y; // 델타 타임 기반 이동을 위한 속도 변수
	bool is_active;
	float move_timer; // 미사일 타일 이동 타이머
} Missile;

//보스 데미지 텍스트
typedef struct {
	int damage_value;
	float precise_x, precise_y;
	float timer;
	bool active;
} BossDamageText;

static BossDamageText boss_damage_texts[MAX_BOSS_DAMAGE_TEXTS];

extern BossMalakh boss;
extern Missile missiles[MAX_MISSILES];
extern bool is_boss_spawned;

// 전역 변수
//extern COORD console;
extern player_t player;
extern block_info_t get_block_info_at(int, int);


// 함수 선언
void Boss_Init(int start_x, int start_y, int init_hp, int attack_power);
void Boss_Render();
void Boss_Update_Ai();
void Boss_Take_Damage(int damage);
static void handle_boss_click(const bool left_click);
void Boss_Update_Pattern();
void Boss_Render_Pattern();

static void create_boss_damage_text(const int damage_value);
static void update_boss_damage_texts();
static void render_boss_damage_texts();


void Boss_update();

// 미사일 관련 함수
void Boss_Launch_Missile(int count);
void Update_Missiles();
void Render_Missiles();

// 레이저 관련 함수
void Boss_Launch_Horizontal_Laser();
void Boss_Launch_Vertical_Laser();
void Boss_Launch_New_Horizontal_Laser();
void Boss_Launch_New_Vertical_Laser();