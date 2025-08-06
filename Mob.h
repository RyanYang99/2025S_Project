#pragma once

#include <Windows.h>
#include <stdbool.h>
#include "console.h"


#define Max_Mob 5
#define MAX_MOB_DEBUG_MESSAGE_LEN 100
#define MAX_MOB_DAMAGE_TEXTS 20


typedef struct {
	int x;
	int y;
	int HP;
	int atk;
	int state;
	int despawn_check;

	float precise_y;
	float precise_x;
	float velocity_y,velocity_x;

	bool is_on_ground;

	int last_move_time;

	float ai_timer;
	float despawn_timer;

	float last_attack_time;
	float atk_cooltime_timer;
} Mob;

typedef struct {
	int damage_value;
	int mob_x;        
	int mob_y;        
	float precise_y;
	float timer;
	bool active;
} MobDamageText;


//전역 변수
extern Mob mobs[Max_Mob];
extern int g_mob_count;
extern int g_mob_level;

//좀비 그래픽 스프라이트 
extern const color_tchar_t zombie_sprite_data[5][5];

extern char mob_debug_message[MAX_MOB_DEBUG_MESSAGE_LEN];
extern MobDamageText mob_damage_texts[MAX_MOB_DAMAGE_TEXTS];

// 함수 선언

void Mob_physics();
void update_mob_ai(void);
void mob_init();
void mob_update();
void Mob_Spawn_Time();
void MobSpawn(int x, int y);
void Mob_render();
void DespawnMob();
void Mob_deadcheck();
void handle_mob_click(const bool left_click);
void register_mob_click_handler();

static void Mob_player_collision();