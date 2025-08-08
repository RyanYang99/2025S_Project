
#include "BossMalakh.h"
#include "astar.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


extern map_t map;
extern player_t player;
extern console_t console;
extern block_info_t get_block_info_at(int x, int y);
extern void player_take_damage(int damage); // 플레이어 데미지 함수 선언 추가

BossMalakh boss;

// 가독성을 위해 매크로 재정의
#define BG_BLACK BACKGROUND_T_BLACK
#define FG_WHITE FOREGROUND_T_WHITE
#define FG_YELLOW FOREGROUND_T_YELLOW
#define FG_CYAN FOREGROUND_T_CYAN
#define FG_MAGENTA FOREGROUND_T_MAGENTA
#define FG_RED FOREGROUND_T_RED

#define BOSS_SPRITE_WIDTH 20
#define BOSS_SPRITE_HEIGHT 20
#define BOSS_DRAW_SCALE 3

static const color_tchar_t boss_malakh_sprite_data[BOSS_SPRITE_HEIGHT][BOSS_SPRITE_WIDTH] = {
    // 0~4: 상단 여백
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'-',BG_BLACK,FG_WHITE},{L'-',BG_BLACK,FG_WHITE},{L'-',BG_BLACK,FG_WHITE},{L'-',BG_BLACK,FG_WHITE},{L'-',BG_BLACK,FG_WHITE},{L'-',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},

    // 5~14: 눈동자 부분
    {{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L'/',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_WHITE},{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L'O',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_WHITE},{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_WHITE},{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_WHITE},{L'|',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_RED},{L' ',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'\\',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'_',BG_BLACK,FG_WHITE},{L'/',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},

    // 15~19: 하단 여백
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}
};


// 문자열 출력 도움 함수
static void Boss_Print_String_Color_W(const wchar_t* wstr, const COORD position, unsigned short background_color, unsigned short foreground_color) {
    COORD current_pos = position;
    for (int i = 0; wstr[i] != L'\0'; ++i) {
        print_color_tchar((color_tchar_t) { wstr[i], background_color, foreground_color }, current_pos);
        current_pos.X++;
    }
}

// 보스 전용 추적 에이스타
static bool is_boss_chasing(int x, int y)
{
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y) {
        return false;
    }
    block_info_t block = get_block_info_at(x, y);
    return block.type == BLOCK_AIR || block.type == BLOCK_GRASS || block.type == BLOCK_DIRT || block.type == BLOCK_SNOW;
}

// 보스 하늘에 생성하기 위해 위치 찾는 함수
static int find_airpos_for_boss(int x)
{
    if (map.size.y <= 0) return 0;
    for (int y = 0; y < map.size.y; ++y)
    {
        block_info_t block = get_block_info_at(x, y);
        if (block.type == BLOCK_AIR)
        {
            return y;
        }
    }
    int default_y = map.size.y / 4;
    return default_y;
}

// 보스 초기화
void Boss_Init(int start_x, int start_y, int init_hp, int attack_power)
{
    boss.x = map.size.x / 2;
    boss.y = map.size.y / 4;

    boss.hp = init_hp;
    boss.Max_hp = init_hp;
    boss.atk = attack_power;

    boss.state = E_BOOS_STATE_PHASE1;
    boss.last_action_time = clock();
    boss.last_move_time = clock();
    boss.special_attack_cooltime = 10000;
    boss.last_special_attack_time = clock();

    // 경로 캐싱 변수 초기화
    boss.cached_path.count = 0;
    boss.cached_path.current_index = 0;
    boss.last_player_pos.X = -1;
    boss.last_player_pos.Y = -1;

    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 20; x++)
        {
            boss.sprite_data[y][x] = boss_malakh_sprite_data[y][x];
        }
    }
}

// 보스 렌더링
void Boss_Render()
{
    if (boss.state == E_BOOS_STATE_DEFEATED) return;

    COORD center_m = { console.size.X / 2 , console.size.Y / 2 };

    int boss_screen_base_x = center_m.X + (boss.x - player.x) * BOSS_DRAW_SCALE - (BOSS_SPRITE_WIDTH * BOSS_DRAW_SCALE / 2);
    int boss_screen_base_y = center_m.Y + (boss.y - player.y) * BOSS_DRAW_SCALE - (BOSS_SPRITE_HEIGHT * BOSS_DRAW_SCALE / 2) - 30;

    // 스프라이트 렌더링
    for (int y_offset = 0; y_offset < 20; ++y_offset) {
        for (int x_offset = 0; x_offset < 20; ++x_offset) {
            for (int py = 0; py < 3; ++py) {
                for (int px = 0; px < 3; ++px) {
                    COORD char_pos = { (SHORT)(boss_screen_base_x + (x_offset * 3) + px), (SHORT)(boss_screen_base_y + (y_offset * 3) + py) };
                    if (char_pos.X >= 0 && char_pos.X < console.size.X &&
                        char_pos.Y >= 0 && char_pos.Y < console.size.Y) {
                        color_tchar_t char_to_print = boss.sprite_data[y_offset][x_offset];
                        if (char_to_print.character != L' ') {
                            print_color_tchar(char_to_print, char_pos);
                        }
                    }
                }
            }
        }
    }

    // 보스 체력 바 렌더링
    wchar_t hp_text[50];
    swprintf(hp_text, sizeof(hp_text) / sizeof(wchar_t), L"HP: %d/%d", boss.hp, boss.Max_hp);
    COORD hp_pos = { (SHORT)(boss_screen_base_x + (20 * 3 / 2) - (int)(wcslen(hp_text) / 2.0)), (SHORT)(boss_screen_base_y - 1) };
    if (hp_pos.X < 0) hp_pos.X = 0;
    if (hp_pos.Y < 0) hp_pos.Y = 0;
    Boss_Print_String_Color_W(hp_text, hp_pos, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);
}

// 보스 AI
void Boss_Update_Ai()
{
    if (boss.state == E_BOOS_STATE_DEFEATED) return;

    long current_time = clock();
    int dist_x = abs(boss.x - player.x);
    int dist_y = abs(boss.y - player.y);
    int distance = dist_x + dist_y;

    // 페이즈 전환 로직
    if (boss.hp <= boss.Max_hp * 0.3 && boss.state < E_BOOS_STATE_PHASE3)
    {
        boss.state = E_BOOS_STATE_PHASE3;
#if _DEBUG
        printf("Boss enters Phase 3 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
        boss.atk = boss.atk * 2;
        boss.special_attack_cooltime = 4000;
        boss.last_special_attack_time = current_time;
    }
    else if (boss.hp <= boss.Max_hp * 0.6 && boss.state < E_BOOS_STATE_PHASE2)
    {
        boss.state = E_BOOS_STATE_PHASE2;
#if _DEBUG
        printf("Boss enters Phase 2 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
        boss.atk = (int)(boss.atk * 1.5f);
        boss.special_attack_cooltime = 6000;
        boss.last_special_attack_time = current_time;
    }
    else if (boss.hp > boss.Max_hp * 0.6 && boss.state != E_BOOS_STATE_PHASE1)
    {
        boss.state = E_BOOS_STATE_PHASE1;
#if _DEBUG
        printf("Boss is in  Phase 1 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
    }

    // 각 페이즈별 행동 로직
    switch (boss.state) {
    case E_BOOS_STATE_PHASE1:
    case E_BOOS_STATE_PHASE2:
    case E_BOOS_STATE_PHASE3:
    {
        // 이동 로직 (경로 캐싱 사용)
        if ((current_time - boss.last_move_time) / (double)CLOCKS_PER_SEC >= 0.5) {

            if (boss.cached_path.count == 0 || boss.cached_path.current_index >= boss.cached_path.count ||
                (boss.last_player_pos.X != player.x || boss.last_player_pos.Y != player.y)) {

                // 플레이어 위치가 바뀌었거나 경로가 소진되면 새 경로 계산
                boss.cached_path = find_path(boss.x, boss.y, player.x, player.y, is_boss_chasing);

                if (boss.cached_path.count > 0)
                {
                    boss.last_player_pos.X = (SHORT)player.x;
                    boss.last_player_pos.Y = (SHORT)player.y;
                    boss.cached_path.current_index = 0;
                }
                else
                {
                    boss.last_move_time = current_time;
                    return;
                }
            }

            if (boss.cached_path.current_index < boss.cached_path.count) {
                COORD next_pos = boss.cached_path.path[boss.cached_path.current_index];
                boss.x = next_pos.X;
                boss.y = next_pos.Y;
                boss.cached_path.current_index++;
            }

            boss.last_move_time = current_time;
        }

        // 공격 로직 (이전 코드와 동일)
        if (distance <= 10) {
            if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 1.0) {
                player_take_damage(boss.atk);
#if _DEBUG
                printf("Boss Phase %d attacks player! (%d HP left)\n", boss.state, player.hp);
#endif
                boss.last_action_time = current_time;
            }
        }

        // 특수 공격 로직 (이전 코드와 동일)
        if ((current_time - boss.last_special_attack_time) / (double)CLOCKS_PER_SEC * 1000 >= boss.special_attack_cooltime) {
#if _DEBUG
            printf("Boss Phase %d uses Special Attack!\n", boss.state);
#endif
            player_take_damage((int)(boss.atk * 1.5f));
            boss.last_special_attack_time = current_time;
        }

        break;
    }

    case E_BOOS_STATE_DAMAGED: {
        if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 0.2) {
            if (boss.hp > boss.Max_hp * 0.6) boss.state = E_BOOS_STATE_PHASE1;
            else if (boss.hp > boss.Max_hp * 0.3) boss.state = E_BOOS_STATE_PHASE2;
            else boss.state = E_BOOS_STATE_PHASE3;
        }
        break;
    }
    case E_BOOS_STATE_DEFEATED:
        break;
    default:
        break;
    }
}

// 보스 데미지 입히는 함수
void Boss_Take_Damage(int damage)
{
    if (boss.state == E_BOOS_STATE_DEFEATED) return;

    boss.hp -= damage;
#if _DEBUG
    printf("Boss %d damaged. current HP : %d\n ", damage, boss.hp);
#endif

    if (boss.hp <= 0)
    {
        boss.hp = 0;
        boss.state = E_BOOS_STATE_DEFEATED;
        printf("Boss Malakh Defeated . . . \n");
    }
    else
    {
        boss.state = E_BOOS_STATE_DAMAGED;
        boss.last_action_time = clock();
    }
}

// 보스 충돌 처리
void Boss_Player_Collision()
{
    if (boss.state == E_BOOS_STATE_DEFEATED) return;
    if (abs(boss.x - player.x) <= 1 && abs(boss.y - player.y) <= 1)
    {
        // 충돌 시 로직 추가 (예: 데미지)
    }
}