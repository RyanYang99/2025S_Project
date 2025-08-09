#pragma once

#include <stdbool.h>

#define MAX_DAMAGE_TEXTS 10

//플레이어 상태 구조체
typedef struct {
    int x, y, //맵 상의 위치
        max_HP, //최대 체력
        HP, //체력
        attack_power; //공격력

    //애니메이션 상태 변수
    int is_moving; // 현재 움직이는 중인지 (0 또는 1)
    float animation_timer; // 현재 프레임을 얼마나 오래 보여줬는지 추적
    int current_frame, // 현재 보여줄 애니메이션 프레임 인덱스
        facing_direction; // 플레이어가 바라보는 방향 (1: 오른쪽, -1: 왼쪽)
    bool is_swinging; // 현재 무기를 휘두르는 중인지
    float swing_timer; // 무기 휘두르기 애니메이션 타이머

    //물리 상태 변수
    float precise_y, //부동 소수점 정밀도의 Y 위치
          velocity_y; //수직 속도
    bool is_on_ground; //땅에 닿아 있는지 여부

    float move_cool_down_timer; //수평 이동 쿨다운 타이머
} player_t;

//전역 플레이어 객체
extern player_t player;

//플레이어 초기화
void player_initialize(void);
void player_update(void);
//플레이어 렌더링
void player_render(void);

const int player_find_ground_position(const int x);

//플레이어 추가 - 승준 ( 전투 시스템 )
void player_take_damage(const int damage);
void player_add_health(const int additional_health);

void player_save(void);