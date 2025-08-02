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

#define GRAVITY 25.0f         // 중력 가속도

#define BG_BLACK BACKGROUND_T_BLACK 
#define FG_WHITE FOREGROUND_T_WHITE
#define FG_YELLOW FOREGROUND_T_YELLOW
#define FG_CYAN FOREGROUND_T_CYAN 
#define FG_MAGENTA FOREGROUND_T_MAGENTA /
#define FG_RED FOREGROUND_T_RED
#define FG_GREEN FOREGROUND_T_GREEN




Mob mobs[Max_Mob];

int mob_count = 0; // 몹의 개체수 확인
int mob_level = 1; // 난이도 (시간비례 증가)  

long startTime = 0; // 게임 시작 시간
long last_spawn_time = 0;// 마지막 스폰시간

// 5x5 좀비 몬스터 스프라이트 데이터 정의
const color_tchar_t zombie_sprite_data[5][5] = {
    {{L' ',BG_BLACK,BG_BLACK}, {L'.',BG_BLACK,FG_GREEN}, {L'_',BG_BLACK,FG_GREEN}, {L'.',BG_BLACK,FG_GREEN}, {L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,FG_GREEN}, {L'o',BG_BLACK,FG_RED}, {L' ',BG_BLACK,FG_GREEN}, {L'o',BG_BLACK,FG_RED}, {L' ',BG_BLACK,FG_GREEN}},
    {{L' ',BG_BLACK,FG_GREEN}, {L'\\',BG_BLACK,FG_GREEN}, {L'|',BG_BLACK,FG_GREEN}, {L'/',BG_BLACK,FG_GREEN}, {L' ',BG_BLACK,FG_GREEN}},
    {{L' ',BG_BLACK,FG_GREEN}, {L'/',BG_BLACK,FG_GREEN}, {L'M',BG_BLACK,FG_GREEN}, {L'\\',BG_BLACK,FG_GREEN}, {L' ',BG_BLACK,BG_BLACK}},
    {{L' ',BG_BLACK,BG_BLACK}, {L' ',BG_BLACK,FG_GREEN}, {L'|',BG_BLACK,FG_GREEN}, {L' ',BG_BLACK,FG_GREEN}, {L' ',BG_BLACK,BG_BLACK}}
};

COORD console_c() // 콘솔 중앙 찾기
{
    COORD center_m;
    center_m.X = console.size.X / 2;
    center_m.Y = console.size.Y / 2;
    return center_m;
}

void Mob_Spawn_Time()
{
    static float spawnTimer = 0.0f, totalElapsedTime = 0.0f;
    spawnTimer += delta_time;
    totalElapsedTime += delta_time;

    if (spawnTimer >= 5.0f) // 소환 간격 설정(초 단위)
    {
        spawnTimer = 0;

        if (mob_count < Max_Mob) //일정수 이상 몹생성 제한
        {
            MobSpawn(player.x, player.y); //플레이어 좌표를 받아 플레이어 기준으로 생성
        }
    }

    //totalElapsedTime = (prevSpawn - startTime) / CLOCKS_PER_SEC;
    long dif_prevtime = clock();
    long dif_check = startTime;
    if (((dif_prevtime - dif_check) / CLOCKS_PER_SEC >= 100) && (mob_level < 10)) // 일정시간마다 스폰되는 몹의 레벨 상승
    {
        mob_level++;
        dif_check = dif_prevtime;
    }
}

void MobSpawn(int player_x, int player_y)
{

    for (int check_x = player_x - 50; check_x <= player_x + 50; check_x++) //플레이어 주변 x값 탐색
    {
        for (int check_y = player_y - 50; check_y <= player_y + 50; check_y++) //플레이어 주변 y값 탐색
        {
            if (check_x >= 0 && check_x < map.size.x && check_y >= 0 && check_y + 1 < map.size.y) // 맵 배열 초과 확인(초과하여 탐색시 예외발생 방지용)
            {
                if (
                    (map.ppBlocks[check_y + 1][check_x].type == BLOCK_GRASS && map.ppBlocks[check_y][check_x].type == BLOCK_AIR) ||
                    (map.ppBlocks[check_y + 1][check_x].type == BLOCK_SNOW && map.ppBlocks[check_y][check_x].type == BLOCK_AIR) ||
                    (map.ppBlocks[check_y + 1][check_x].type == BLOCK_SAND && map.ppBlocks[check_y][check_x].type == BLOCK_AIR)
                    ) //풀, 눈 자체가 땅위에 생성 되므로 풀 위이기만 하면 생성 되도록 설정(눈은 여러겹 생성이므로 배경과 바로 아래 블럭이 조건에 맞는지 확인)
                {
                    if (
                        ((check_x <= player_x - 20) || (check_x >= player_x + 20)) &&
                        ((check_y <= player_y - 20) || (check_y >= player_y + 20))
                        ) // 플레이어 주변 20칸 이내로는 스폰X
                    {
                        int Spawn_r = rand() % 50; // 스폰 확률
                        if (Spawn_r == 0)
                        {
                            mobs[mob_count].x = check_x; // 탐색으로 조건이 맞는 좌표값 저장
                            mobs[mob_count].y = check_y;
                            mobs[mob_count].HP = mob_level * 10; //몹 레벨에 따라 몹 체력, 몹 공격력 증가
                            mobs[mob_count].atk = mob_level * 5;
                            mobs[mob_count].precise_x = (float)check_x; //중력 적용 좌표값 초기화
                            mobs[mob_count].precise_y = (float)check_y;
                            mobs[mob_count].velocity_y = 0;
                            mobs[mob_count].is_on_ground = false;
                            mobs[mob_count].despawn_check = clock(); //디스폰 초기화
                            mobs[mob_count].last_move_time = clock();
                            mob_count++;
                        }
                    }
                }
            }

        }
    }

}


void Mob_render() // 몹 렌더링
{
    COORD mob_center = console_c();

    for (int i = 0; i < mob_count; i++)
    {
        // 몹 스프라이트의 좌측 상단 좌표 계산
        int mob_x_start = mob_center.X + (mobs[i].x - player.x) * 3;
        int mob_y_start = mob_center.Y + (mobs[i].y - player.y) * 3;

        // 스프라이트 크기를 고려하여 화면 밖 몹은 그리지 않음
        if (mob_x_start + 4 < 0 || mob_x_start >= console.size.X ||
            mob_y_start + 4 < 0 || mob_y_start >= console.size.Y)
        {
            continue;
        }

        // 5x5 스프라이트 출력
        for (int j = 0; j < 5; ++j) {
            for (int k = 0; k < 5; ++k) {
                COORD current_pos = { mob_x_start + k, mob_y_start + j };
                if (current_pos.X >= 0 && current_pos.X < console.size.X &&
                    current_pos.Y >= 0 && current_pos.Y < console.size.Y) {
                    print_color_tchar(zombie_sprite_data[j][k], current_pos);
                }
            }
        }
    }
}

void DespawnMob()
{
    for (int i = 0; i < mob_count; )
    {
        int x = abs(mobs[i].x - player.x); // 몹거리 절댓값 체크
        int y = abs(mobs[i].y - player.y);
        if (x > 70 || y > 70) //플레이어 기준 일정거리 이상이면 몹 디스폰카운트 시작
        {
            if ((clock() - mobs[i].despawn_check) / CLOCKS_PER_SEC >= 5) //디스폰카운트 일정시간 적용시 몹 디스폰
            {
                for (int j = i; j < mob_count - 1; j++)
                {
                    mobs[j] = mobs[j + 1]; //디스폰 적용몹 배열 제거 및 밀기
                }
                mob_count--;
                continue;
            }
        }
        mobs[i].despawn_check = clock(); // 디스폰 초기화
        i++;
    }
}

void Mob_deadcheck() // 몹 사망체크
{
    for (int i = 0; i < mob_count; )
    {
        if (mobs[i].HP <= 0)
        {

            for (int j = i; j < mob_count - 1; j++)
            {
                mobs[j] = mobs[j + 1]; // hp가 0이하인 몹 배열 제거 및 밀기
            }
            mob_count--;
            continue;
        }
        i++;
    }
}

void Mob_physics()
{
    for (int i = 0; i < Max_Mob; i++)
    {
        // 땅에 있는지 확인
        if (!is_walkable(mobs[i].x, mobs[i].y + 1)) {
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

        if (new_y > mobs[i].y) {
            while (!is_walkable(mobs[i].x, new_y)) {
                new_y--;
                mobs[i].velocity_y = 0;
                mobs[i].precise_y = (float)new_y;
                mobs[i].is_on_ground = true;
            }
        }
        else if (new_y < mobs[i].y) {
            while (!is_walkable(mobs[i].x, new_y)) {
                new_y++;
                mobs[i].velocity_y = 0;
                mobs[i].precise_y = (float)new_y;
                break;
            }
        }
        mobs[i].y = new_y;

        // X축 이동
        int new_x = (int)mobs[i].precise_x;
        if (new_x != mobs[i].x) {
            // 땅에 있든 공중이든 x 이동 허용하되, 충돌만 막음
            if (is_walkable(new_x, mobs[i].y)) {
                mobs[i].x = new_x;
            }
            else {
                mobs[i].precise_x = mobs[i].x; // 충돌 시 복구
            }
        }

        // 마지막으로 정수 좌표 동기화
        mobs[i].x = (int)mobs[i].precise_x;
        mobs[i].y = (int)mobs[i].precise_y;
    }
}


static bool is_mob_movable(int x, int y) {
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y) {
        return false;
    }
    block_info_t block = get_block_info_at(x, y);

    // 몹은 땅(GRASS, DIRT, SNOW)
    return block.type == BLOCK_GRASS || block.type == BLOCK_DIRT || block.type == BLOCK_SNOW;
}

void update_mob_ai() {
    long current_time = clock();
    for (int i = 0; i < mob_count; ++i) {
        if ((current_time - mobs[i].last_move_time) / (double)CLOCKS_PER_SEC >= 0.5) {


            path_t mob_path = find_path(mobs[i].x, mobs[i].y, player.x, player.y, is_mob_movable);

            if (mob_path.count > 0) {
                // 경로가 있으면 첫 번째 단계로 이동
                mobs[i].x = mob_path.path[0].X;
                mobs[i].y = mob_path.path[0].Y;
            }

            // 메모리 해제 (경로가 더 이상 필요 없을 경우)
            // free(mob_path.path);

            mobs[i].last_move_time = current_time;
        }
    }
}