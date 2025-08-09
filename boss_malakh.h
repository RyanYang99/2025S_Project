#pragma once

#include <stdbool.h>

#include "console.h"

//보스 상태
typedef enum {
    E_BOSS_STATE_PHASE_1,
    E_BOSS_STATE_PHASE_2,
    E_BOSS_STATE_PHASE_3,
    E_BOSS_STATE_DAMAGED,
    E_BOSS_STATE_DEFEATED
} boss_state_t;

#define BOSS_SPRITE_WIDTH 20
#define BOSS_SPRITE_HEIGHT 20
#define BOSS_DRAW_SCALE 3 //렌더링 스케일

//보스 구조체
typedef struct {
    int x, y, hp, max_hp, atk;
    boss_state_t state;

    //델타 타임 기반 타이머와 쿨타임
    float action_timer, //상태 전환용 타이머
          missile_timer,
          horizontal_laser_timer,
          vertical_laser_timer,
          missile_attack_cool_time,
          horizontal_laser_cool_time,
          vertical_laser_cool_time;

    //레이저 패턴
    bool is_horizontal_laser_active,
         is_vertical_laser_active,
         horizontal_laser_from_right,
         is_vertical_laser_from_left;
    int vertical_laser_target_x, horizontal_laser_target_y,
        current_horizontal_laser_y, current_vertical_laser_x;
    float horizontal_laser_damage_cool_time, vertical_laser_damage_cool_time;

    color_character_t pSprite_data[BOSS_SPRITE_HEIGHT][BOSS_SPRITE_WIDTH];
} boss_malakh_t;

extern bool boss_spawned;
extern boss_malakh_t boss;

//함수 선언
void boss_initialize(const int start_x, const int start_y, const int hp, const int attack_power);

void boss_update(void);
void boss_render(void);

void boss_destroy(void);