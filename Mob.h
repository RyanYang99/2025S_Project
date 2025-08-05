#pragma once

#include <Windows.h>
#include <stdbool.h>
#include "console.h"


#define Max_Mob 5
#define MAX_MOB_DEBUG_MESSAGE_LEN 100

extern char mob_debug_message[MAX_MOB_DEBUG_MESSAGE_LEN];

#pragma pack(push, 1)
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
} Mob;
#pragma pack(pop)

extern Mob mobs[Max_Mob];
extern int mob_count;
extern int mob_level;

//좀비 그래픽 스프라이트 
extern const color_tchar_t zombie_sprite_data[5][5];

// 함수 선언
void load_mob(void);
void save_mob(void);
void Mob_physics();
void update_mob_ai(void);
void mob_init();
void mob_update();
void Mob_Spawn_Time();
void MobSpawn(int x, int y);
void Mob_render();
void DespawnMob();
void Mob_deadcheck();
void handle_mob_click(const bool left_click, const COORD mouse_pos);
void register_mob_click_handler();