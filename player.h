#pragma once

#include <windows.h>
#include <stdbool.h>
#include "console.h"

#define MAX_DAMAGE_TEXTS 10




// 플레이어 상태 구조체
typedef struct {
    int x, y; // 맵 상의 위치
    int max_hp; //최대 체력
    int hp;   // 체력
    int atk_power; //공격력

    // 애니메이션 상태 변수
    int is_moving;           // 현재 움직이는 중인지 (0 또는 1)
    float animation_timer;   // 현재 프레임을 얼마나 오래 보여줬는지 추적
    int current_frame;       // 현재 보여줄 애니메이션 프레임 인덱스
    int facing_direction;    // 플레이어가 바라보는 방향 (1: 오른쪽, -1: 왼쪽)

    // 물리 상태 변수
    float precise_y;     // 부동 소수점 정밀도의 Y 위치
    float velocity_y;    // 수직 속도 (양수: 아래, 음수: 위)
    bool is_on_ground;   // 땅에 닿아 있는지 여부

    float move_cooldown_timer;   // 수평 이동 쿨다운 타이머
} player_t;



//데미지 텍스트 효과 추가
typedef struct
{
    int damage_value;
    float precise_y;
    float timer;
    bool active;
}DamageText;

//전역 변수
extern DamageText damage_texts[MAX_DAMAGE_TEXTS];
// 전역 플레이어 객체
extern player_t player;

// 플레이어 초기화
void player_init(void);

int find_ground_pos(int x);

// 플레이어 이동
// void player_move(int dx, int dy);

void player_update(void);

bool is_walkable(int x, int y);

// 플레이어 렌더링
void render_player(void);


void save_player(void);

//플레이어 추가 - 승준 ( 전투 시스템 )
extern void player_take_damage(int damage);
static void update_damage_texts();
static void render_damage_texts();
extern void player_take_damage(int damage);

void add_health_to_player(const int additional_health);