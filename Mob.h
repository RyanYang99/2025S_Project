#pragma once

#include <stdbool.h>

#define MAX_MOB 50
#define MAX_MOB_DEBUG_MESSAGE_LEN 100
#define MAX_MOB_DAMAGE_TEXTS 20

#pragma pack(push, 1)
typedef struct {
    int x, y, HP, attack, state, despawn_check;
    float precise_y, precise_x, velocity_y, velocity_x;
    bool is_on_ground, is_dead;
    int last_move_time, current_frame;
    float ai_timer, despawn_timer, animation_timer, last_attack_time, attack_cool_time_timer, dying_timer;
} mob_t;
#pragma pack(pop)

//함수 선언
void mob_initialize(void);
void mob_update(void);
void mob_render(void);
void mob_destroy(void);

void mob_save(void);