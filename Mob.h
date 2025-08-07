#pragma once

#include <stdbool.h>
#include "console.h"

#define Max_Mob 5
#define MAX_MOB_DEBUG_MESSAGE_LEN 100
#define MAX_MOB_DAMAGE_TEXTS 20

#pragma pack(push, 1)
typedef struct {
    int x, y, HP, atk, state, despawn_check;

    float precise_y, precise_x, velocity_y,velocity_x;

    bool is_on_ground;

    int last_move_time;

    float ai_timer, despawn_timer;

    float last_attack_time, atk_cooltime_timer;
} Mob;
#pragma pack(pop)

typedef struct {
    int damage_value, mob_x, mob_y;        
    float precise_y;
    float timer;
    bool active;
} MobDamageText;

extern char mob_debug_message[MAX_MOB_DEBUG_MESSAGE_LEN];

// 함수 선언
void mob_init();
void save_mob(void);
void mob_update();
void Mob_render();
void destroy_mob(void);