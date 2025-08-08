#pragma once

#include <stdbool.h>
#include "console.h"

#define MAX_MOB 10
#define MAX_MOB_DEBUG_MESSAGE_LEN 100
#define MAX_MOB_DAMAGE_TEXTS 20

#pragma pack(push, 1)
typedef struct {
    int x, y, HP, atk, state, despawn_check;
    float precise_y, precise_x, velocity_y,velocity_x;
    bool is_on_ground;
    bool is_dead;
    int last_move_time;
    int current_frame;
    float ai_timer, despawn_timer, animation_timer;
    float last_attack_time, atk_cooltime_timer, dying_timer;
} Mob;
#pragma pack(pop)

extern char mob_debug_message[MAX_MOB_DEBUG_MESSAGE_LEN];

// 함수 선언
void mob_init();
void save_mob(void);
void mob_update();
void Mob_render();
void destroy_mob(void);