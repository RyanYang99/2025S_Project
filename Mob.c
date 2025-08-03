#include "leak.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <stdbool.h>
#include <math.h>

#include "Mob.h"
#include "player.h"
#include "map.h"
#include "console.h"
#include "astar.h"
#include "delta.h"


#define GRAVITY 25.0f // 중력 가속도
// #define MOB_DRAW_SCALE 3 // 이 매크로는 더 이상 사용하지 않습니다.

#define BG_BLACK BACKGROUND_T_BLACK
#define FG_WHITE FOREGROUND_T_WHITE
#define FG_YELLOW FOREGROUND_T_YELLOW
#define FG_CYAN FOREGROUND_T_CYAN
#define FG_MAGENTA FOREGROUND_T_MAGENTA
#define FG_RED FOREGROUND_T_RED
#define FG_GREEN FOREGROUND_T_GREEN
#define FG_DARKYELLOW FOREGROUND_T_DARKYELLOW
#define FG_DARKGREEN FOREGROUND_T_DARKGREEN

// 몹 스프라이트의 가로/세로 크기 (플레이어와 동일하게 설정)
#define MOB_SPRITE_WIDTH 5
#define MOB_SPRITE_HEIGHT 5


static const color_tchar_t zombie_sprite_data[MOB_SPRITE_HEIGHT][MOB_SPRITE_WIDTH] = {
    // {문자, 배경색, 전경색}
    { {L' ',0,0}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L' ',0,0} }, // 머리 (위: 머리카락, 아래:피부) -> 전부 빨간색
    { {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0} }, // 눈 (가운데는 비어있음) -> 하얀색 눈
    { {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN} }, // 몸통과 팔 -> 짙은 초록색
    { {L' ',0,0}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L' ',0,0} }, // 허리, 바지 -> 갈색
    { {L' ',0,0}, {L'█', BG_BLACK, FG_GREEN}, {L' ',0,0}, {L'█', BG_BLACK, FG_GREEN}, {L' ',0,0} }  // 신발 -> 초록색
};


Mob mobs[Max_Mob];

int mob_count = 0; // 몹의 개체수 확인
int mob_level = 1; // 난이도 (시간비례 증가)

long startTime = 0; // 게임 시작 시간
long last_spawn_time = 0;// 마지막 스폰시간

COORD console_c() // 콘솔 중앙 찾기
{
    COORD center_m = { console.size.X / 2, console.size.Y / 2 };
    return center_m;
}

// Mob.c 파일의 수정된 Mob_Spawn_Time 함수
void Mob_Spawn_Time()
{
    static float spawnTimer = 0.0f;
    static float levelUpTimer = 0.0f;

    spawnTimer += delta_time;
    levelUpTimer += delta_time;

    if (spawnTimer >= 3.0f) // 소환 간격을 3초로 
    {
        spawnTimer = 0;
        if (mob_count < Max_Mob) // 일정수 이상 몹생성 제한
        {
            MobSpawn(player.x, player.y); // 플레이어 좌표를 받아 플레이어 기준으로 생성
        }
    }

    if (levelUpTimer >= 100.0f && mob_level < 10) // 일정시간마다 몹 레벨 상승
    {
        mob_level++;
        levelUpTimer = 0.0f; // 타이머 리셋
    }
}

// Mob.c의 MobSpawn 함수
void MobSpawn(int player_x, int player_y)
{
    srand((unsigned int)time(NULL)); // 랜덤값 초기화

    int spawn_x, spawn_y;
    int max_attempts = 1000;

    for (int attempt = 0; attempt < max_attempts; attempt++)
    {
        spawn_x = player_x + (rand() % 101) - 50;
        spawn_y = player_y + (rand() % 101) - 50;

        if (spawn_x >= 0 && spawn_x < map.size.x && spawn_y >= 0 && spawn_y < map.size.y - 1)
        {
            if (abs(spawn_x - player_x) <= 20 && abs(spawn_y - player.y) <= 20)
            {
                continue;
            }

            block_info_t block_at_y = get_block_info_at(spawn_x, spawn_y);
            block_info_t block_at_y_plus_1 = get_block_info_at(spawn_x, spawn_y + 1);

            //y 위치는 BLOCK_AIR이고, y+1 위치는 공기나 물이 아닌 블록
            if (block_at_y.type == BLOCK_AIR && block_at_y_plus_1.type != BLOCK_AIR && block_at_y_plus_1.type != BLOCK_WATER)
            {
                mobs[mob_count].x = spawn_x;
                mobs[mob_count].y = spawn_y; 
                mobs[mob_count].HP = mob_level * 10;
                mobs[mob_count].atk = mob_level * 2; //초기 공격력은 2, 체력은 10으로 설정 
                mobs[mob_count].precise_x = (float)mobs[mob_count].x;
                mobs[mob_count].precise_y = (float)mobs[mob_count].y;
                mobs[mob_count].velocity_y = 0;
                mobs[mob_count].is_on_ground = false;
                mobs[mob_count].despawn_check = clock();
                mobs[mob_count].last_move_time = clock();
                mob_count++;
                break;
            }
        }
    }
}

void Mob_render()
{
    COORD center_m = console_c();

    for (int i = 0; i < mob_count; i++)
    {
        // 몹 스프라이트의 좌측 상단 좌표 계산 (플레이어와 동일하게 맵 오프셋 적용)
        int screen_x = center_m.X + (mobs[i].x - player.x);
        int screen_y = center_m.Y + (mobs[i].y - player.y);

        // 스프라이트 크기 (5x5)를 고려하여 화면 밖 몹은 그리지 않음
        int sprite_width = MOB_SPRITE_WIDTH;
        int sprite_height = MOB_SPRITE_HEIGHT;
        if (screen_x + sprite_width < 0 || screen_x >= console.size.X ||
            screen_y + sprite_height < 0 || screen_y >= console.size.Y)
        {
            continue;
        }

        // 몬스터 체력(HP) 바 렌더링
        // 체력 문자열을 몬스터 스프라이트 바로 위에 출력하기 위한 위치 계산
        COORD hp_pos;
        hp_pos.X = screen_x;
        hp_pos.Y = screen_y - 1; // 몬스터 스프라이트 바로 위 (y-1)에 위치

        // HP 문자열을 "HP: X" 
        wchar_t hp_str[16];
        swprintf(hp_str, 16, L"HP: %d", mobs[i].HP);

        // HP 문자열을 출력
        for (int j = 0; hp_str[j] != L'\0'; ++j) {
            COORD char_pos = { hp_pos.X + j, hp_pos.Y };

            // 색상과 문자를 설정하여 출력
            color_tchar_t hp_char = { hp_str[j], BACKGROUND_T_BLACK, FOREGROUND_T_RED };
            print_color_tchar(hp_char, char_pos);
        }

        // 5x5 스프라이트를 1:1 비율로 출력
        for (int j = 0; j < MOB_SPRITE_HEIGHT; ++j) {
            for (int k = 0; k < MOB_SPRITE_WIDTH; ++k) {
                color_tchar_t mob_pixel = zombie_sprite_data[j][k];

                COORD current_pos = { (SHORT)(screen_x + k), (SHORT)(screen_y + j) };

                if (current_pos.X >= 0 && current_pos.X < console.size.X &&
                    current_pos.Y >= 0 && current_pos.Y < console.size.Y) {
                    if (mob_pixel.character != L' ') {
                        print_color_tchar(mob_pixel, current_pos);
                    }
                }
            }
        }
    }
}

void DespawnMob()
{
    for (int i = 0; i < mob_count; )
    {
        int x = abs(mobs[i].x - player.x);
        int y = abs(mobs[i].y - player.y);
        if (x > 70 || y > 70)
        {
            if ((clock() - mobs[i].despawn_check) / CLOCKS_PER_SEC >= 5)
            {
                for (int j = i; j < mob_count - 1; j++)
                {
                    mobs[j] = mobs[j + 1];
                }
                mob_count--;
                continue;
            }
        }
        mobs[i].despawn_check = clock();
        i++;
    }
}

void Mob_deadcheck()
{
    for (int i = 0; i < mob_count; )
    {
        if (mobs[i].HP <= 0)
        {
            for (int j = i; j < mob_count - 1; j++)
            {
                mobs[j] = mobs[j + 1];
            }
            mob_count--;
            continue;
        }
        i++;
    }
}

void Mob_physics()
{
    for (int i = 0; i < mob_count; i++)
    {
        // 땅에 있는지 확인
        block_info_t block_below = get_block_info_at(mobs[i].x, mobs[i].y + 1);
        if (block_below.type != BLOCK_AIR && block_below.type != BLOCK_WATER) {
            mobs[i].is_on_ground = true;
            if (mobs[i].velocity_y > 0) {
                mobs[i].velocity_y = 0;
            }
        }
        else {
            mobs[i].is_on_ground = false;
        }

        // 중력 적용
        if (!mobs[i].is_on_ground) {
            mobs[i].velocity_y += GRAVITY * delta_time;
        }

        // Y축 이동
        mobs[i].precise_y += mobs[i].velocity_y * delta_time;
        int new_y = (int)mobs[i].precise_y;
        mobs[i].y = new_y;

        // X축 이동
        mobs[i].x = (int)mobs[i].precise_x;
    }
}


static bool is_mob_movable(int x, int y) {
    // 맵 범위를 벗어난 좌표는 이동 불가능
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y) {
        return false;
    }

    // 몹의 위치(y)는 공기
    block_info_t block_at_y = get_block_info_at(x, y);
    if (block_at_y.type != BLOCK_AIR) {
        return false;
    }

    // 몹의 발밑은 공기(BLOCK_AIR)와 물(BLOCK_WATER)이 아닌 모든 블록 위에서 이동 가능
    block_info_t block_at_y_plus_1 = get_block_info_at(x, y + 1);
    if (block_at_y_plus_1.type == BLOCK_AIR || block_at_y_plus_1.type == BLOCK_WATER) {
        return false;
    }

    return true;
}


// Mob.c 파일의 수정된 update_mob_ai 함수
void update_mob_ai() {
    long current_time = clock();
    for (int i = 0; i < mob_count; ++i) {
        // 몹의 마지막 움직임 시간이 0.5초 이상 지났을 때만 A* 알고리즘을 실행
        if ((current_time - mobs[i].last_move_time) / (double)CLOCKS_PER_SEC >= 0.5) {

            // A* 알고리즘으로 플레이어까지의 경로를 찾기
            path_t mob_path = find_path(mobs[i].x, mobs[i].y, player.x, player.y, is_mob_movable);

            // 경로가 있고, 몹의 현재 위치가 아닌 다음 이동할 지점이 있을 경우에만 이동
            
            if (mob_path.count > 1) {
                mobs[i].x = mob_path.path[1].X;
                mobs[i].y = mob_path.path[1].Y;
            }

           
            // 마지막 움직임 시간 업데이트
            mobs[i].last_move_time = current_time;
        }
    }
}

//전투 시스템 마우스 콜백
void handle_mob_click(const bool left_click, const COORD mouse_pos)
{
    if (!left_click) return; // 왼쪽 클릭만 처리합니다.

    COORD center_m = console_c();

    for (int i = 0; i < mob_count; i++)
    {
        // 몬스터의 화면 좌표(픽셀)를 계산
        int mob_screen_x_start = center_m.X + (mobs[i].x - player.x);
        int mob_screen_y_start = center_m.Y + (mobs[i].y - player.y);

        // 마우스 클릭이 몬스터 스프라이트 영역 내에 있는지 확인
        if (mouse_pos.X >= mob_screen_x_start && mouse_pos.X < mob_screen_x_start + MOB_SPRITE_WIDTH &&
            mouse_pos.Y >= mob_screen_y_start && mouse_pos.Y < mob_screen_y_start + MOB_SPRITE_HEIGHT)
        {
            // 몬스터가 플레이어로부터 5칸 이내에 있는지 확인합니다.
            double distance = sqrt(pow(mobs[i].x - player.x, 2) + pow(mobs[i].y - player.y, 2));

            if (distance <= 5.0)
            {
                mobs[i].HP -= 5; // 몬스터 체력 5 감소
                return; // 한 몬스터만 대미지를 입히고 함수를 종료합니다.
            }
        }
    }
}

// 이 함수를 게임 초기화 함수(예: main.c의 initialize_game)에서 호출해야 합니다.
void register_mob_click_handler() {
    subscribe_mouse_click_with_pos(handle_mob_click);
}