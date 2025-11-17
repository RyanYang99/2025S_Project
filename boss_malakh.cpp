#include "leak.hpp"
#include "boss_malakh.hpp"

#include <string>
#include <format>

#include "input.hpp"
#include "map.hpp"
#include "tool.hpp"
#include "delta_time.hpp"
#include "astar.hpp"
#include "sound.hpp"
#include "player.hpp"
#include "console.hpp"
#include "inventory.hpp"
#include "block_control.hpp"
#include "item_database.hpp"

//보스 데미지 텍스트 관련
constexpr int MAX_BOSS_DAMAGE_TEXTS{ 10 }, MAX_MISSILES{ 5 };
constexpr float BOSS_DAMAGE_TEXT_DURATION{ 1.0f };

//미사일 구조체
typedef struct {
    int x, y;
    float velocity_x, velocity_y; //델타 타임 기반 이동을 위한 속도 변수
    bool is_active;
    float move_timer; //미사일 타일 이동 타이머
} boss_missile_t;

//보스 데미지 텍스트
typedef struct {
    int damage_value;
    float precise_x, precise_y, timer;
    bool active;
} boss_damage_text_t;

bool boss_spawned = false;
boss_malakh_t boss{};

static boss_missile_t pBoss_missiles[MAX_MISSILES]{};
static boss_damage_text_t pBoss_damage_texts[MAX_BOSS_DAMAGE_TEXTS]{};

//보스 스프라이트 데이터 (생략)
static const cchar pBoss_malakh_sprite_data[BOSS_SPRITE_HEIGHT][BOSS_SPRITE_WIDTH] = {
    //0              1              2              3              4              5              6              7              8              9              10             11             12             13             14             15             16             17             18             19
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //1
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::yellow, FG::yellow }, { L'█', BG::yellow, FG::yellow }, { L'█', BG::yellow, FG::yellow }, { L'█', BG::yellow, FG::yellow }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //2
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::yellow, FG::yellow }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::yellow, FG::yellow }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //3
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::blue, FG::blue }, { L'█', BG::yellow, FG::yellow }, { L'█', BG::yellow, FG::yellow }, { L'█', BG::blue, FG::blue }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //4
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //5
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { L'█', BG::cyan, FG::cyan }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { L'█', BG::red, FG::red }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { L'█', BG::red, FG::red }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { L'█', BG::cyan, FG::cyan }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //6
    { { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 } },
    //7
    { { L'█', BG::white, FG::white }, { L'█', BG::white, FG::white }, { L'█', BG::cyan, FG::cyan }, { L'█', BG::white, FG::white }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { L'█', BG::yellow, FG::yellow }, { L'█', BG::yellow, FG::yellow }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { L'█', BG::white, FG::white }, { L'█', BG::cyan, FG::cyan }, { L'█', BG::white, FG::white }, { L'█', BG::white, FG::white } },
    //8
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { L'█', BG::cyan, FG::cyan }, { L'█', BG::white, FG::white }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { L'█', BG::red, FG::red }, { L'█', BG::red, FG::red }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { L'█', BG::white, FG::white }, { L'█', BG::cyan, FG::cyan }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //9
    { { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 } },
    //10
    { { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { L'█', BG::red, FG::red }, { L'█', BG::black, FG::black }, { L'█', BG::black, FG::black }, { L'█', BG::red, FG::red }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white } },
    //11
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //12
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //13
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //14
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //15
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::white, FG::white }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //16
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //17
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //18
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
    //19
    { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } }
};

//보스 데미지 텍스트 생성 함수
static void boss_create_damage_text(const int damage_value) {
    for (int i = 0; i < MAX_BOSS_DAMAGE_TEXTS; ++i)
        if (!pBoss_damage_texts[i].active) {
            pBoss_damage_texts[i].active = true;
            pBoss_damage_texts[i].damage_value = damage_value;
            pBoss_damage_texts[i].precise_x = (float)boss.x + (float)(rand() % (BOSS_SPRITE_WIDTH - 1));
            pBoss_damage_texts[i].precise_y = (float)boss.y + (float)(rand() % (BOSS_SPRITE_HEIGHT - 1));
            pBoss_damage_texts[i].timer = BOSS_DAMAGE_TEXT_DURATION;
            break;
        }
}

//보스 데미지 입히는 함수
static void boss_damage(const int damage) {
    if (boss.state == E_BOSS_STATE_DEFEATED)
        return;

    boss.hp -= damage;
    boss_create_damage_text(damage);

    sound_play_sound_effect(BOSS_SOUND_HURT);

    if (boss.hp <= 0) {
        boss.hp = 0;
        boss.state = E_BOSS_STATE_DEFEATED;
        boss_spawned = false;
    } else {
        //보스가 데미지를 입었을 때 상태 변경 (피격 애니메이션 등을 위해)
        boss.state = E_BOSS_STATE_DAMAGED;
        boss.action_timer = 0.0f;
    }
}

//보스 핵(코어) 공격 처리 함수
static void boss_handle_player_attack(const bool left_click) {
    if (boss.state == E_BOSS_STATE_DEFEATED || !left_click || !boss_spawned)
        return;

    if (map_get_block_info(block_control_selected_x, block_control_selected_y).type == BLOCK_SEED_OF_MALAKH) {
        const player_item_t * const pItem = inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item;
        int extra = 0;
        if (pItem)
            extra = tool_get_damage_to_mob(static_cast<tool_t>(pItem->item_DB_index));

        boss_damage(player.attack_power + extra);
        return;
    }

    for (int i = 0; i < MAX_MISSILES; ++i)
        if (pBoss_missiles[i].is_active &&
            pBoss_missiles[i].x == block_control_selected_x &&
            pBoss_missiles[i].y == block_control_selected_y) {
            pBoss_missiles[i].is_active = false;
            return;
        }
}

static void boss_handle_offset(void) {
    boss.x += map.offset_x;
    
    for (int i = 0; i < MAX_MISSILES; ++i)
        if (pBoss_missiles[i].is_active)
            pBoss_missiles[i].x += map.offset_x;
}

//보스 초기화
void boss_initialize(const int start_x, const int start_y, const int hp, const int attack_power) {
    boss.x = start_x;
    boss.y = start_y;

    boss.hp = hp;
    boss.max_hp = hp;
    boss.atk = attack_power;
    boss.state = E_BOSS_STATE_PHASE_1;

    boss.action_timer =
    boss.missile_timer =
    boss.horizontal_laser_timer =
    boss.vertical_laser_timer =
    boss.horizontal_laser_damage_cool_time =
    boss.vertical_laser_damage_cool_time = 0.0f;

    boss.missile_attack_cool_time = 4.0f;
    boss.horizontal_laser_cool_time = 3.0f;
    boss.vertical_laser_cool_time = 5.0f;

    boss.is_horizontal_laser_active = boss.is_vertical_laser_active = false;

    for (int y = 0; y < BOSS_SPRITE_HEIGHT; ++y)
        for (int x = 0; x < BOSS_SPRITE_WIDTH; ++x)
            boss.pSprite_data[y][x] = pBoss_malakh_sprite_data[y][x];

    for (int i = 0; i < MAX_MISSILES; ++i) {
        pBoss_missiles[i].is_active = false;
        pBoss_missiles[i].move_timer = 0.0f;
    }

    for (int i = 0; i < MAX_BOSS_DAMAGE_TEXTS; ++i)
        pBoss_damage_texts[i].active = false;

    Input::subscribe_input_mouse_click(boss_handle_player_attack);
    map_subscribe_offset_change(boss_handle_offset);
}

//미사일 발사 함수
static void boss_launch_missile(const int count) {
    int launched_count = 0;

    sound_play_sound_effect(BOSS_SOUND_MISSILE);
    for (int i = 0; i < MAX_MISSILES && launched_count < count; ++i)
        if (!pBoss_missiles[i].is_active) {
            pBoss_missiles[i].x = boss.x;
            pBoss_missiles[i].y = boss.y;
            pBoss_missiles[i].is_active = true;
            ++launched_count;
        }
}

static void boss_launch_new_horizontal_laser(void) {
    sound_play_sound_effect(BOSS_SOUND_LASER);
    boss.is_horizontal_laser_active = true;
    boss.action_timer = 0.0f;
    //보스 머리 위부터 시작
    boss.current_horizontal_laser_y = boss.y - BOSS_SPRITE_HEIGHT / 2;
}

//세로 레이저 패턴 시작 함수
static void boss_launch_new_vertical_laser(void) {
    sound_play_sound_effect(BOSS_SOUND_LASER);
    boss.is_vertical_laser_active = true;
    boss.action_timer = 0.0f;
    //방향을 랜덤으로 결정
    boss.is_vertical_laser_from_left = (rand() % 2 == 0);
    //시작 위치 설정
    if (boss.is_vertical_laser_from_left)
        boss.current_vertical_laser_x = 0; //왼쪽에서 시작
    else
        boss.current_vertical_laser_x = map.size.x - 1; //오른쪽에서 시작
}

//보스 패턴 업데이트 및 발동 로직
static void boss_update_pattern(void) {
    if (boss.state == E_BOSS_STATE_DEFEATED)
        return;

    //타이머 누적
    boss.missile_timer += delta_time_t::delta_time;
    boss.horizontal_laser_timer += delta_time_t::delta_time;
    boss.vertical_laser_timer += delta_time_t::delta_time;

    //미사일 패턴 (모든 페이즈)
    if (boss.missile_timer >= boss.missile_attack_cool_time) {
        int missile_count_to_launch = 1;
        if (boss.state == E_BOSS_STATE_PHASE_1)
            missile_count_to_launch = 3;
        else if (boss.state == E_BOSS_STATE_PHASE_2)
            missile_count_to_launch = 5;
        else if (boss.state == E_BOSS_STATE_PHASE_3)
            missile_count_to_launch = 10;

        sound_play_sound_effect(BOSS_SOUND_HOWLING);
        boss_launch_missile(missile_count_to_launch);
        boss.missile_timer = 0.0f; //타이머 리셋
    }

    //페이즈 2: 가로 레이저 패턴
    if (boss.state == E_BOSS_STATE_PHASE_2 && boss.horizontal_laser_timer >= boss.horizontal_laser_cool_time) {
        sound_play_sound_effect(BOSS_SOUND_HOWLING);
        boss_launch_new_horizontal_laser();
        boss.horizontal_laser_timer = 0.0f;
    }

    //페이즈 3: 세로 레이저 패턴
    if (boss.state == E_BOSS_STATE_PHASE_3 && boss.vertical_laser_timer >= boss.vertical_laser_cool_time) {
        sound_play_sound_effect(BOSS_SOUND_HOWLING);
        boss_launch_new_vertical_laser();
        boss.vertical_laser_timer = 0.0f;
    }
}

//보스 AI 업데이트
static void boss_update_main(void) {
    if (boss.state == E_BOSS_STATE_DEFEATED)
        return;

    boss.horizontal_laser_damage_cool_time -= delta_time_t::delta_time;
    boss.vertical_laser_damage_cool_time -= delta_time_t::delta_time;

    // 페이즈 전환 로직
    if (boss.hp <= boss.max_hp * 0.3f && boss.state < E_BOSS_STATE_PHASE_3) {
        boss.state = E_BOSS_STATE_PHASE_3;
        boss.action_timer = 0.0f;
        boss.atk = boss.atk * 2;
        boss.missile_attack_cool_time = 1.5f;
        boss.vertical_laser_cool_time = 4.0f;
    } else if (boss.hp <= boss.max_hp * 0.6 && boss.state < E_BOSS_STATE_PHASE_2) {
        boss.state = E_BOSS_STATE_PHASE_2;
        boss.action_timer = 0.0f;
        boss.atk = (int)(boss.atk * 1.5f);
        boss.missile_attack_cool_time = 2.0f;
        boss.horizontal_laser_cool_time = 3.0f;
    } else if (boss.hp > boss.max_hp * 0.6 && boss.state != E_BOSS_STATE_PHASE_1) {
        boss.state = E_BOSS_STATE_PHASE_1;
        boss.action_timer = 0.0f;
        boss.missile_attack_cool_time = 4.0f;
    }

    if (boss.is_horizontal_laser_active && abs(boss.current_horizontal_laser_y - player.y) <= 1) {
        if (boss.horizontal_laser_damage_cool_time <= 0.0f) {
            player_take_damage(100);
            boss.horizontal_laser_damage_cool_time = 2.0f;
        }
    }

    if (boss.is_vertical_laser_active && abs(boss.current_vertical_laser_x - player.x) <= 1) {
        if (boss.vertical_laser_damage_cool_time <= 0.0f) {
            player_take_damage(100);
            boss.vertical_laser_damage_cool_time = 2.0f;
        }
    }

    boss_update_pattern();

    switch (boss.state) {
        case E_BOSS_STATE_DAMAGED:
            boss.action_timer += delta_time_t::delta_time;

            if (boss.action_timer >= 0.2f) {
                if (boss.hp > boss.max_hp * 0.6)
                    boss.state = E_BOSS_STATE_PHASE_1;
                else if (boss.hp > boss.max_hp * 0.3)
                    boss.state = E_BOSS_STATE_PHASE_2;
                else
                    boss.state = E_BOSS_STATE_PHASE_3;
            }
            break;

        case E_BOSS_STATE_DEFEATED:
            if (boss_spawned)
                boss_spawned = false;
            break;
    }
}

//미사일이 이동 가능한 블록인지 체크하는 함수
static bool boss_is_missile_movable(const int x, const int y) {
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y)
        return false;

    const block_info_t block = map_get_block_info(x, y);
    return block.type != BLOCK_STONE && block.type != BLOCK_WATER;
}

//미사일 업데이트 함수
static void boss_update_missiles(void) {
    const float missile_speed = 5.0f; //초당 5타일 이동

    for (int i = 0; i < MAX_MISSILES; ++i)
        if (pBoss_missiles[i].is_active) {
            pBoss_missiles[i].move_timer += delta_time_t::delta_time;

            //일정한 속도로 타일 이동
            if (pBoss_missiles[i].move_timer >= 1.0f / missile_speed) {
                astar::direction next_direction = astar::find_next_direction(pBoss_missiles[i].x, pBoss_missiles[i].y, player.x, player.y, boss_is_missile_movable);

                switch (next_direction) {
                    case astar::direction::up:
                        --pBoss_missiles[i].y;
                        break;

                    case astar::direction::right:
                        ++pBoss_missiles[i].x;
                        break;

                    case astar::direction::down:
                        ++pBoss_missiles[i].y;
                        break;

                    case astar::direction::left:
                        --pBoss_missiles[i].x;
                        break;
                }

                pBoss_missiles[i].move_timer -= 1.0f / missile_speed;
            }

            //플레이어와 충돌 체크
            if (pBoss_missiles[i].x == player.x && pBoss_missiles[i].y == player.y) {
                player_take_damage(boss.atk);
                pBoss_missiles[i].is_active = false;
            }

            //블록과 충돌 체크
            if (!boss_is_missile_movable(pBoss_missiles[i].x, pBoss_missiles[i].y))
                pBoss_missiles[i].is_active = false;
        }
}

//보스 대미지 텍스트 업데이트 함수
static void boss_update_damage_texts(void) {
    for (int i = 0; i < MAX_BOSS_DAMAGE_TEXTS; ++i)
        if (pBoss_damage_texts[i].active) {
            pBoss_damage_texts[i].precise_y -= delta_time_t::delta_time * 5.0f;
            pBoss_damage_texts[i].timer -= delta_time_t::delta_time;

            if (pBoss_damage_texts[i].timer <= 0.0f)
                pBoss_damage_texts[i].active = false;
        }
}

//보스 업데이트
void boss_update(void) {
    boss_update_main();
    boss_update_missiles();
    boss_update_damage_texts();
}

//미사일 렌더링
static void boss_render_missiles(void) {
    const cchar character = { '*', BG::black, FG::red };
    const COORD &size{ Console::size() };

    for (int i = 0; i < MAX_MISSILES; ++i)
        if (pBoss_missiles[i].is_active) {
            const COORD missile_position = {
                (SHORT)(size.X / 2 + (pBoss_missiles[i].x - player.x) * BOSS_DRAW_SCALE),
                (SHORT)(size.Y / 2 + (pBoss_missiles[i].y - player.y) * BOSS_DRAW_SCALE)
            };

            if (missile_position.X >= 0 && missile_position.X < size.X &&
                missile_position.Y >= 0 && missile_position.Y < size.Y)
                Console::print(character, missile_position);
        }
}

//패턴 렌더링 함수
static void boss_render_pattern(void) {
    const COORD &size{ Console::size() };

    //미사일 렌더링
    boss_render_missiles();

    //페이즈 2: 가로 레이저
    if (boss.is_horizontal_laser_active) {
        //0.2초마다 한 줄씩 아래로 이동
        if (boss.action_timer > 0.2f) {
            ++boss.current_horizontal_laser_y;
            boss.action_timer = 0.0f;
        }

        boss.action_timer += delta_time_t::delta_time;
        if (boss.current_horizontal_laser_y > map.size.y)
            boss.is_horizontal_laser_active = false;

        //3줄 레이저를 한번에 출력
        const cchar character = { L'═', BG::black, FG::red };

        for (int i = 0; i < 3; ++i) {
            COORD laser_position = {
                .Y = (SHORT)(size.Y / 2 + ((boss.current_horizontal_laser_y + i) - player.y) * BOSS_DRAW_SCALE)
            };

            for (; laser_position.X < size.X; ++laser_position.X)
                Console::print(character, laser_position);
        }
    }

    //페이즈 3: 세로 레이저 (랜덤 방향에서 3줄씩 한번에)
    if (boss.is_vertical_laser_active) {
        //0.2초마다 한 줄씩 이동
        if (boss.action_timer > 0.2f) {
            if (boss.is_vertical_laser_from_left)
                ++boss.current_vertical_laser_x;
            else
                --boss.current_vertical_laser_x;

            boss.action_timer = 0.0f;
        }
        boss.action_timer += delta_time_t::delta_time;

        //화면을 벗어나면 패턴 종료
        if (boss.is_vertical_laser_from_left && boss.current_vertical_laser_x > map.size.x ||
            !boss.is_vertical_laser_from_left && boss.current_vertical_laser_x < 0) {
            boss.is_vertical_laser_active = false;
        }

        //3줄 레이저를 한번에 출력
        const cchar character = { L'║', BG::black, FG::red };

        for (int i = 0; i < 3; ++i) {
            COORD laser_position = {
                .X = (SHORT)(size.X / 2 + ((boss.current_vertical_laser_x + (boss.is_vertical_laser_from_left ? -i : i)) - player.x) * BOSS_DRAW_SCALE)
            };

            for (; laser_position.Y < size.Y; ++laser_position.Y)
                Console::print(character, laser_position);
        }
    }
}

//보스 대미지 텍스트 렌더링 함수
static void boss_render_damage_texts(void) {
    const COORD &size{ Console::size() };

    const COORD center_position = {
        .X = static_cast<SHORT>(size.X / 2),
        .Y = static_cast<SHORT>(size.Y / 2)
    };

    for (int i = 0; i < MAX_BOSS_DAMAGE_TEXTS; ++i) {
        if (pBoss_damage_texts[i].active) {
            const COORD draw_position = {
                .X = (SHORT)(center_position.X + (pBoss_damage_texts[i].precise_x - player.x)),
                .Y = (SHORT)(center_position.Y + (pBoss_damage_texts[i].precise_y - player.y))
            };

            Console::print(std::format("Attack! -{}", pBoss_damage_texts[i].damage_value), draw_position, BG::black, FG::white);
        }
    }
}

//보스 렌더링
void boss_render(void) {
    if (boss.state == E_BOSS_STATE_DEFEATED)
        return;

    const COORD &size{ Console::size() };
    const int boss_screen_base_x = size.X / 2 + (boss.x - player.x) * BOSS_DRAW_SCALE,
              boss_screen_base_y = size.Y / 2 + (boss.y - player.y) * BOSS_DRAW_SCALE;

    //스프라이트 렌더링
    for (int y_offset = 0; y_offset < BOSS_SPRITE_HEIGHT; ++y_offset)
        for (int x_offset = 0; x_offset < BOSS_SPRITE_WIDTH; ++x_offset)
            for (int py = 0; py < BOSS_DRAW_SCALE; ++py)
                for (int px = 0; px < BOSS_DRAW_SCALE; ++px) {
                    const COORD character_position = {
                        .X = (SHORT)(boss_screen_base_x + (x_offset * BOSS_DRAW_SCALE) + px),
                        .Y = (SHORT)(boss_screen_base_y + (y_offset * BOSS_DRAW_SCALE) + py)
                    };
                    if (character_position.X >= 0 && character_position.X < size.X &&
                        character_position.Y >= 0 && character_position.Y < size.Y) {
                        const cchar char_to_print = boss.pSprite_data[y_offset][x_offset];
                        if (char_to_print.character != L' ')
                            Console::print(char_to_print, character_position);
                    }
                }

    //보스 체력 바 렌더링
    const std::string text{ std::format("HP: {} / {}", boss.hp, boss.max_hp) };
    COORD hp_position = {
        .X = (SHORT)(boss_screen_base_x + (BOSS_SPRITE_WIDTH * BOSS_DRAW_SCALE / 2.0f) - (int)(text.length() / 2.0f)),
        .Y = (SHORT)(boss_screen_base_y - 1)
    };
    if (hp_position.X < 0)
        hp_position.X = 0;
    if (hp_position.Y < 0)
        hp_position.Y = 0;
    Console::print(text, hp_position, BG::black, FG::white);

    //패턴 렌더링
    boss_render_pattern();
    boss_render_damage_texts();
}

void boss_destroy(void) {
    Input::subscribe_input_mouse_click(boss_handle_player_attack);
    map_unsubscribe_offset_change(boss_handle_offset);
}