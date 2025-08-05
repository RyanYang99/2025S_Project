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


#define GRAVITY 25.0f
#define BG_BLACK BACKGROUND_T_BLACK
#define FG_WHITE FOREGROUND_T_WHITE
#define FG_YELLOW FOREGROUND_T_YELLOW
#define FG_CYAN FOREGROUND_T_CYAN
#define FG_MAGENTA FOREGROUND_T_MAGENTA
#define FG_RED FOREGROUND_T_RED
#define FG_GREEN FOREGROUND_T_GREEN
#define FG_DARKYELLOW FOREGROUND_T_DARKYELLOW
#define FG_DARKGREEN FOREGROUND_T_DARKGREEN

#define MOB_SPRITE_WIDTH 5
#define MOB_SPRITE_HEIGHT 5

#define MOB_SPD 0.2f
#define JUMP_SPD -12.0f

char mob_debug_message[MAX_MOB_DEBUG_MESSAGE_LEN] = ""; // 변수 정의


static const color_tchar_t zombie_sprite_data[MOB_SPRITE_HEIGHT][MOB_SPRITE_WIDTH] = {
    { {L' ',0,0}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L' ',0,0} },
    { {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0} },
    { {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN} },
    { {L' ',0,0}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L' ',0,0} },
    { {L' ',0,0}, {L'█', BG_BLACK, FG_GREEN}, {L' ',0,0}, {L'█', BG_BLACK, FG_GREEN}, {L' ',0,0} }
};


Mob mobs[Max_Mob];

int mob_count = 0;
int mob_level = 1;

COORD console_c()
{
    COORD center_m = { console.size.X / 2, console.size.Y / 2 };
    return center_m;
}

void Mob_Spawn_Time()
{
    static float spawnTimer = 0.0f;
    static float levelUpTimer = 0.0f;

    spawnTimer += delta_time;
    levelUpTimer += delta_time;

    if (spawnTimer >= 3.0f)
    {
        spawnTimer = 0;
        if (mob_count < Max_Mob)
        {
            MobSpawn(player.x, player.y);
        }
    }

    if (levelUpTimer >= 100.0f && mob_level < 10)
    {
        mob_level++;
        levelUpTimer = 0.0f;
    }
}


void MobSpawn(int player_x, int player_y)
{
    srand((unsigned int)time(NULL));

    int spawn_x, spawn_y;
    int max_attempts = 1000;
    int ground_y;

    for (int attempt = 0; attempt < max_attempts; attempt++)
    {
        spawn_x = player_x + (rand() % 101) - 50;
        spawn_y = player_y + (rand() % 101) - 50;

        if (spawn_x >= 0 && spawn_x < map.size.x && spawn_y >= 0 && spawn_y < map.size.y - 1)
        {
            if (abs(spawn_x - player.x) <= 20 && abs(spawn_y - player.y) <= 20)
            {
                continue;
            }

            ground_y = -1;
            for (int y = spawn_y; y < map.size.y; y++) {
                block_info_t block_at_y = get_block_info_at(spawn_x, y);
            
                if (block_at_y.type != BLOCK_AIR && block_at_y.type != BLOCK_WATER && block_at_y.type != BLOCK_LEAF) {
                    ground_y = y;
                    break;
                }
            }

            if (ground_y != -1)
            {
                mobs[mob_count].x = spawn_x;
                mobs[mob_count].y = ground_y - MOB_SPRITE_HEIGHT;
                mobs[mob_count].HP = mob_level * 10;
                mobs[mob_count].atk = mob_level * 2;
                mobs[mob_count].precise_x = (float)mobs[mob_count].x;
                mobs[mob_count].precise_y = (float)mobs[mob_count].y;
                mobs[mob_count].velocity_y = 0;
                mobs[mob_count].is_on_ground = true;

                mobs[mob_count].ai_timer = 0.0f;
                mobs[mob_count].despawn_timer = 0.0f;

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
        int screen_x = center_m.X + (mobs[i].x - player.x);
        int screen_y = center_m.Y + (mobs[i].y - player.y);

        int sprite_width = MOB_SPRITE_WIDTH;
        int sprite_height = MOB_SPRITE_HEIGHT;
        if (screen_x + sprite_width < 0 || screen_x >= console.size.X ||
            screen_y + sprite_height < 0 || screen_y >= console.size.Y)
        {
            continue;
        }

        COORD hp_pos;
        hp_pos.X = screen_x;
        hp_pos.Y = screen_y - 1;

        wchar_t hp_str[16];
        swprintf(hp_str, 16, L"HP: %d", mobs[i].HP);

        for (int j = 0; hp_str[j] != L'\0'; ++j) {
            COORD char_pos = { hp_pos.X + j, hp_pos.Y };

            color_tchar_t hp_char = { hp_str[j], BACKGROUND_T_BLACK, FOREGROUND_T_RED };
            print_color_tchar(hp_char, char_pos);
        }

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
            mobs[i].despawn_timer += delta_time;
            if (mobs[i].despawn_timer >= 5.0f)
            {
                for (int j = i; j < mob_count - 1; j++)
                {
                    mobs[j] = mobs[j + 1];
                }
                mob_count--;
                continue;
            }
        }
        else {
            mobs[i].despawn_timer = 0.0f;
        }
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
        int block_below_y = mobs[i].y + MOB_SPRITE_HEIGHT;

        bool is_standing_on_ground = false;

        for (int x_offset = 0; x_offset < MOB_SPRITE_WIDTH; ++x_offset) {
            block_info_t block_below = get_block_info_at(mobs[i].x + x_offset, block_below_y);

            if (block_below.type != BLOCK_AIR && block_below.type != BLOCK_WATER && block_below.type != BLOCK_LEAF) {
                is_standing_on_ground = true;
                break;
            }
        }

        if (is_standing_on_ground) {
            mobs[i].is_on_ground = true;
            if (mobs[i].velocity_y > 0) {
                mobs[i].velocity_y = 0;
            }
            mobs[i].y = block_below_y - MOB_SPRITE_HEIGHT;
            mobs[i].precise_y = (float)mobs[i].y;

        }
        else {
            mobs[i].is_on_ground = false;
        }

        if (!mobs[i].is_on_ground) {
            mobs[i].velocity_y += GRAVITY * delta_time;
        }

        // 💡 몬스터의 x축 이동 충돌 감지 로직 추가
        float new_precise_x = mobs[i].precise_x + mobs[i].velocity_x * delta_time;
        int new_x = (int)new_precise_x;

        bool can_move_horizontally = true;
        for (int y_offset = 0; y_offset < MOB_SPRITE_HEIGHT; ++y_offset) {
            block_info_t block_at_new_pos = get_block_info_at(new_x, mobs[i].y + y_offset);
            if (block_at_new_pos.type != BLOCK_AIR && block_at_new_pos.type != BLOCK_WATER && block_at_new_pos.type != BLOCK_LEAF) {
                can_move_horizontally = false;
                break;
            }
        }

        if (can_move_horizontally) {
            mobs[i].precise_x = new_precise_x;
        }
        else {
            mobs[i].velocity_x = 0; // 벽에 부딪히면 속도를 0으로 설정
        }

        mobs[i].precise_y += mobs[i].velocity_y * delta_time;

        mobs[i].x = (int)mobs[i].precise_x;
        mobs[i].y = (int)mobs[i].precise_y;
    }
}


static bool is_mob_movable(int x, int y) {
    // 1. 맵 경계 확인
    if (x < 0 || x + MOB_SPRITE_WIDTH > map.size.x || y < 0 || y + MOB_SPRITE_HEIGHT > map.size.y) {
        return false;
    }

    
    //for (int x_offset = 0; x_offset < MOB_SPRITE_WIDTH; ++x_offset) {
    //    block_info_t block_under_foot = get_block_info_at(x + x_offset, y - 1);
    //    if (block_under_foot.type == BLOCK_AIR || block_under_foot.type == BLOCK_WATER || block_under_foot.type == BLOCK_LEAF) {
    //        return false; // 발 아래가 단단한 블록이 아니면 이동 불가능
    //    }
    //}
    // 
    //    for (int x_offset = 0; x_offset < MOB_SPRITE_WIDTH; ++x_offset) {
    //        block_info_t block_in_body = get_block_info_at(x + x_offset, y + y_offset);

    //        // 몸체가 단단한 블록과 겹치면 이동 불가능
    //        if (block_in_body.type != BLOCK_AIR && block_in_body.type != BLOCK_WATER && block_in_body.type != BLOCK_LEAF) {
    //            return false;
    //        }
    //    }
    //}

    return true;
}


void update_mob_ai() {
    for (int i = 0; i < mob_count; ++i) {
        if (mobs[i].ai_timer >= MOB_SPD) {
            mobs[i].ai_timer = 0.0f;

            bool is_movable_start_pos = is_mob_movable(mobs[i].x, mobs[i].y);

            path_t mob_path = find_path(mobs[i].x, mobs[i].y, player.x, player.y, is_mob_movable);

           
            snprintf(mob_debug_message, MAX_MOB_DEBUG_MESSAGE_LEN,
                "Mob %d - Pos: (%d, %d), Movable: %s, Path count: %d",
                i, mobs[i].x, mobs[i].y, is_movable_start_pos ? "True" : "False", mob_path.count);

            if (mob_path.count > 1) {
                int next_x = mob_path.path[1].X;
                int next_y = mob_path.path[1].Y;

                if (next_x < mobs[i].x) {
                    mobs[i].velocity_x = -1.0f;
                }
                else if (next_x > mobs[i].x) {
                    mobs[i].velocity_x = 1.0f;
                }
                else {
                    mobs[i].velocity_x = 0;
                }

                if (next_y < mobs[i].y && mobs[i].is_on_ground) {
                    mobs[i].velocity_y = JUMP_SPD;
                }
            }
            else {
                mobs[i].velocity_x = 0;
            }
        }
        else {
            mobs[i].ai_timer += delta_time;
        }
    }
}


void handle_mob_click(const bool left_click, const COORD mouse_pos)
{
    if (!left_click) return;

    COORD center_m = console_c();

    for (int i = 0; i < mob_count; i++)
    {
        int mob_screen_x_start = center_m.X + (mobs[i].x - player.x);
        int mob_screen_y_start = center_m.Y + (mobs[i].y - player.y);

        if (mouse_pos.X >= mob_screen_x_start && mouse_pos.X < mob_screen_x_start + MOB_SPRITE_WIDTH &&
            mouse_pos.Y >= mob_screen_y_start && mouse_pos.Y < mob_screen_y_start + MOB_SPRITE_HEIGHT)
        {
            double distance = sqrt(pow(mobs[i].x - player.x, 2) + pow(mobs[i].y - player.y, 2));

            if (distance <= 5.0)
            {
                mobs[i].HP -= 5;
                return;
            }
        }
    }
}

void register_mob_click_handler() {
    subscribe_mouse_click_with_pos(handle_mob_click);
}

void mob_init() {
    mob_count = 0;
}

//몬스터 업데이트 
void mob_update()
{
    update_mob_ai();

    Mob_physics();
 
    Mob_Spawn_Time();
    Mob_deadcheck();
    DespawnMob();

    Mob_render();
}