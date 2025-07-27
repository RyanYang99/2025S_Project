#include "BossMalakh.h"
#include "astar.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


extern map_t map;
extern player_t player;
extern console_t console;
extern block_info_t get_block_info_at(int x, int y);

BossMalakh boss;

//가독성을 위해 매크로 재정의

#define BG_BLACK BACKGROUND_T_BLACK // 가독성을 위해 매크로 정의
#define FG_WHITE FOREGROUND_T_WHITE
#define FG_YELLOW FOREGROUND_T_YELLOW
#define FG_CYAN FOREGROUND_T_CYAN // 눈 색상
#define FG_MAGENTA FOREGROUND_T_MAGENTA // 엔젤링 포인트 색상
#define FG_RED FOREGROUND_T_RED 

#define BOSS_SPRITE_WIDTH 20
#define BOSS_SPRITE_HEIGHT 20
#define BOSS_DRAW_SCALE 3 //스프라이트 확대 비율


static const color_tchar_t boss_malakh_sprite_data[BOSS_SPRITE_HEIGHT][BOSS_SPRITE_WIDTH] = {
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'.',BG_BLACK,FG_MAGENTA},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 0 (엔젤링 최상단 점)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{L' ',BG_BLACK,BG_BLACK},{L'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{L'\\',BG_BLACK,FG_YELLOW},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 1 (링 확장)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,FG_YELLOW},{L'/',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{L'\\',BG_BLACK,FG_YELLOW},{L' ',BG_BLACK,FG_YELLOW},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 2 (링 더 크게)

    // 다이아몬드 형태 시작 (내부를 채움)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 3
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 4
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 5

    // 눈 및 몸체 부분 
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 6 (눈 상단: X, V로 무섭게)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 7 (눈 중앙)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{'/',BG_BLACK,FG_WHITE},{L'X',BG_BLACK,FG_RED},   {L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 8 (눈 하단)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'V',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'^',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'V',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'^',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'V',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 9 (입: V, ^를 이용한 이빨 표현)

    // 다이아몬드 아랫부분 
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 10
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 11
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,FG_WHITE},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 12
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 13
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 14
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 15
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 16 (다이아몬드 최하단 점)
    
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 17
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 18
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}  // 19
};

//문자열 출력 도움 함수
static void Boss_Print_String_Color_W(const wchar_t* wstr, const COORD position, unsigned short background_color, unsigned short foreground_color) {
    COORD current_pos = position;
    for (int i = 0; wstr[i] != L'\0'; ++i) {
        print_color_tchar((color_tchar_t) { wstr[i], background_color, foreground_color }, current_pos);
        current_pos.X++;
    }
}

//보스 전용 추적 에이스타
static bool is_boss_chasing(int x, int y)
{
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y) {
        return false;
    }
    block_info_t block = get_block_info_at(x, y);

    //이동  설정 
    return block.type == BLOCK_AIR || block.type == BLOCK_GRASS || block.type == BLOCK_DIRT || block.type == BLOCK_SNOW;

}


//보스 하늘에 생성하기 위해 위치 찾는 함수
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
    //공기 블록을 못찾았을 경우 
    int default_y = map.size.y / 4;

    return default_y;
}




//보스 초기화
void Boss_Init(int start_x, int start_y, int init_hp, int attack_power)
{
    /*boss.x = start_x;
    boss.y = find_airpos_for_boss(map.size.x / 2);*/

    boss.x = map.size.x / 2;
    boss.y = map.size.y / 4;


    boss.hp = init_hp;
    boss.Max_hp = init_hp;
    boss.atk = attack_power;

    boss.state = E_BOOS_STATE_PHASE1;
    boss.last_action_time = clock();
    boss.last_move_time = clock();
    boss.special_attack_cooltime = 10000; //10초 쿨타임
    boss.last_special_attack_time = clock();

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
    int boss_screen_base_y = center_m.Y + (boss.y - player.y) * BOSS_DRAW_SCALE - (BOSS_SPRITE_HEIGHT * BOSS_DRAW_SCALE / 2) - 30; // 30픽셀(10칸)만큼 더 위로 이동

    // === 스프라이트 렌더링 ===
    for (int y_offset = 0; y_offset < 20; ++y_offset) { // 20행
        for (int x_offset = 0; x_offset < 20; ++x_offset) { // 20열
            // 각 스프라이트 픽셀을 3x3으로 확장하여 그림
            for (int py = 0; py < 3; ++py) {
                for (int px = 0; px < 3; ++px) {
                    COORD char_pos = { boss_screen_base_x + (x_offset * 3) + px, boss_screen_base_y + (y_offset * 3) + py };

                    // 화면 범위 체크
                    if (char_pos.X >= 0 && char_pos.X < console.size.X &&
                        char_pos.Y >= 0 && char_pos.Y < console.size.Y) {

                        color_tchar_t char_to_print = boss.sprite_data[y_offset][x_offset];
                        if (char_to_print.character != L' ') { // 공백은 그리지 않음 (투명 처리)
                            print_color_tchar(char_to_print, char_pos);
                        }
                    }
                }
            }
        }
    }
    // ========================

    // 보스 체력 바 렌더링 (스프라이트 위에 표시)
    wchar_t hp_text[50];
    swprintf(hp_text, sizeof(hp_text) / sizeof(wchar_t), L"HP: %d/%d", boss.hp, boss.Max_hp);

    // 체력 바 위치 조정: 스프라이트 중앙 상단에 오도록
    // 스프라이트의 실제 픽셀 너비 (20 * 3 = 60)를 고려하여 중앙 계산
    COORD hp_pos = { boss_screen_base_x + (20 * 3 / 2) - (int)(wcslen(hp_text) / 2.0), boss_screen_base_y - 1 };
    if (hp_pos.X < 0) hp_pos.X = 0;
    if (hp_pos.Y < 0) hp_pos.Y = 0; // Y 좌표도 0 미만 방지

    Boss_Print_String_Color_W(hp_text, hp_pos, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);
}


//보스 AI
void Boss_Update_Ai()
{
    if (boss.state == E_BOOS_STATE_DEFEATED)return;

    long current_time = clock();
    int dist_x = abs(boss.x - player.x);
    int dist_y = abs(boss.y - player.y);
    int distance = dist_x + dist_y; //맨해튼 거리

    //페이즈 전환 로직
    //hp 30% 이하
    if (boss.hp <= boss.Max_hp * 0.3 && boss.state < E_BOOS_STATE_PHASE3)
    {
        //3페이즈 변경
        boss.state = E_BOOS_STATE_PHASE3;
#if _DEBUG
        printf("Boss enters Phase 3 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
        boss.atk = boss.atk * 2;  //공격력 2배 상승

        boss.special_attack_cooltime = 4000; //4초 쿨타임
        boss.last_special_attack_time = current_time; // 쿨타임 초기화 추가
    }
    //hp 60% 이하
    else if (boss.hp <= boss.Max_hp * 0.6 && boss.state < E_BOOS_STATE_PHASE2)
    {
        //2페이즈 돌입
        boss.state = E_BOOS_STATE_PHASE2;
#if _DEBUG
        printf("Boss enters Phase 2 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
        boss.atk = boss.atk * 1.5; //공격력 1.5배 상승
        boss.special_attack_cooltime = 6000; //6초마다 궁극기 사용
        boss.last_special_attack_time = current_time; // 쿨타임 초기화 추가
    }
    else if (boss.hp > boss.Max_hp * 0.6 && boss.state != E_BOOS_STATE_PHASE1) // 초기 && 1페이즈 상태 체크 수정
    {
        boss.state = E_BOOS_STATE_PHASE1;
#if _DEBUG
        printf("Boss is in  Phase 1 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
    }

    //각 페이즈별 패턴

       // --- 각 페이즈별 행동 로직 ---
    switch (boss.state) {
    case E_BOOS_STATE_PHASE1: { // 1페이즈: 단순 추격 및 일반 공격
        if (distance <= 100) { //100칸 범위 내 추적 
            if ((current_time - boss.last_move_time) / (double)CLOCKS_PER_SEC >= 3.0) { // ~초마다 이동
                
                int steps_paths = 0;
                const int max_steps = 5; //5칸 씩 이동 

                while (steps_paths < max_steps)
                {
                    COORD next_pos = find_path_next_step(boss.x, boss.y, player.x, player.y, map.size.x, map.size.y);
                    if (next_pos.X != -1 && next_pos.Y != -1 && (boss.x != next_pos.X || boss.y != next_pos.Y))
                    {
                        boss.x = next_pos.X;
                        boss.y = next_pos.Y;
                        steps_paths++;
                    }

                 }
            }
            else
            {//더이상 이동할수 없거나, 경로 못찾을 경우 루프 종료 
                break;
            }
            //충분히 플레이어와 가까워 지면 이동하지 않음
            if (abs(boss.x - player.x) + abs(boss.y - player.y) <= 10) {
                break;
            }

        boss.last_move_time = current_time;
        }
        //범위 10칸 이내에 있을 경우 공격 
        if (distance <= 10) {

            if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 1.0) { // 1초마다 공격
                player_take_damage(boss.atk);
#if _DEBUG
                printf("Boss Phase 1 attacks player! (%d HP left)\n", player.hp);
#endif
                boss.last_action_time = current_time;
            }
    }
        
        // 1페이즈 특수 공격 (쿨타임이 되면 사용)
        if ((current_time - boss.last_special_attack_time) / (double)CLOCKS_PER_SEC * 1000 >= boss.special_attack_cooltime) {
#if _DEBUG
            printf("Boss Phase 1 uses Special Attack!\n");
#endif
            // 간단한 장판 공격 예시 (플레이어 주변 3x3 데미지)
            if (distance <= 3) {
                player_take_damage(boss.atk * 1.5);
            }
            boss.last_special_attack_time = current_time;
        }
        break;
    }

    case E_BOOS_STATE_PHASE2: { // 2페이즈: 더 빠른 이동, 새로운 스킬 추가
        if (distance <= 100) {
            if ((current_time - boss.last_move_time) / (double)CLOCKS_PER_SEC >= 2.0) { //2초마다 이동
                int steps_taken = 0;
                const int max_steps = 5;

                while (steps_taken < max_steps) {
                    COORD next_pos = find_path_next_step(boss.x, boss.y, player.x, player.y, map.size.x, map.size.y);
                    if (next_pos.X != -1 && next_pos.Y != -1 && (boss.x != next_pos.X || boss.y != next_pos.Y)) {
                        boss.x = next_pos.X;
                        boss.y = next_pos.Y;
                        steps_taken++;
                    }
                    else {
                        break;
                    }
                    if (abs(boss.x - player.x) + abs(boss.y - player.y) <= 1) {
                        break;
                    }
                }
                boss.last_move_time = current_time;
            }
        }
        if(distance <=15 ) { // 15칸 내에 가까이 있으면 공격
            if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 1.0) { // 1초마다 공격
                player_take_damage(boss.atk);
#if _DEBUG
                printf("Boss Phase 2 attacks player! (%d HP left)\n", player.hp);
#endif
                boss.last_action_time = current_time;
            }
        }
        // 2페이즈 특수 공격 (쿨타임이 더 짧아짐)
        if ((current_time - boss.last_special_attack_time) / (double)CLOCKS_PER_SEC * 1000 >= boss.special_attack_cooltime) {
#if _DEBUG
            printf("Boss Phase 2 uses Enhanced Special Attack!\n");
#endif
            // 플레이어 주변 5x5 범위 데미지 (더 넓은 범위)
            if (distance <= 5) {
                player_take_damage(boss.atk * 2);
            }
            boss.last_special_attack_time = current_time;
        }
        break;
    }

    case E_BOOS_STATE_PHASE3: { // 3페이즈: 가장 강력한 공격, 피하기 어려운 패턴
        if (distance <= 100) {
            if ((current_time - boss.last_move_time) / (double)CLOCKS_PER_SEC >= 1.0) { // 1초마다 이동 (매우 빠름)
                int steps_taken = 0;
                const int max_steps = 5;

                while (steps_taken < max_steps) {
                    COORD next_pos = find_path_next_step(boss.x, boss.y, player.x, player.y, map.size.x, map.size.y);
                    if (next_pos.X != -1 && next_pos.Y != -1 && (boss.x != next_pos.X || boss.y != next_pos.Y)) {
                        boss.x = next_pos.X;
                        boss.y = next_pos.Y;
                        steps_taken++;
                    }
                    else {
                        break;
                    }
                    if (abs(boss.x - player.x) + abs(boss.y - player.y) <= 1) {
                        break;
                    }
                }
                boss.last_move_time = current_time;
            }
        }

        // 3페이즈는 복합 패턴 (예: 2초마다 랜덤 패턴 발동)
        if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 2.0) {
            int pattern_choice = rand() % 2; // 2가지 패턴 중 하나 선택
            if (pattern_choice == 0) { // 패턴 1: 광역 폭발
#if _DEBUG
                printf("Boss Phase 3 - Area Blast! (Player HP: %d)\n", player.hp);
#endif
                if (distance <= 8) { // 넓은 범위
                    player_take_damage(boss.atk * 3);
                }
            }

            boss.last_action_time = current_time;
        }
        // 특수 공격 (쿨타임이 매우 짧아짐)
        if ((current_time - boss.last_special_attack_time) / (double)CLOCKS_PER_SEC * 1000 >= boss.special_attack_cooltime) {
#if _DEBUG
            printf("Boss Phase 3 uses Ultimate Special Attack!\n");
#endif
            // 화면 전체 또는 매우 넓은 범위 데미지
            player_take_damage(boss.atk * 5); // 매우 큰 데미지
            boss.last_special_attack_time = current_time;
        }
        break;
    }

    case E_BOOS_STATE_DAMAGED: { // 피격 상태 (잠시 경직)
        if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 0.2) { // 0.2초 경직 후
            // 피격 해제 후 현재 페이즈 상태로 돌아감
            if (boss.hp > boss.Max_hp * 0.6) boss.state = E_BOOS_STATE_PHASE1;
            else if (boss.hp > boss.Max_hp * 0.3) boss.state = E_BOOS_STATE_PHASE2;
            else boss.state = E_BOOS_STATE_PHASE3;
        }
        break;
    }

    case E_BOOS_STATE_DEFEATED:
        break;

    default:
        // printf("Warning: Boss in unhandled state (%d)\n", boss.state);
        break;
    }
}



//보스 데미지 입히는 함수
void Boss_Take_Damage(int damage)
{
    if (boss.state == E_BOOS_STATE_DEFEATED)return;

    boss.hp -= damage;
#if _DEBUG
    printf("Boss %d damaged. current HP : %d\n ", damage, boss.hp);
#endif

    if (boss.hp <= 0)
    {
        boss.hp = 0;
        boss.state = E_BOOS_STATE_DEFEATED;
        printf("Boss Malakh Defeated . . . \n");
        //todo : 보스 사망처리 -> 아이템 드랍
    }
    else
    {
        boss.state = E_BOOS_STATE_DAMAGED;
        boss.last_action_time = clock();
    }

}
//보스 충돌 처리
void Boss_Player_Collision()
{
    if (boss.state == E_BOOS_STATE_DEFEATED)return;

    if (abs(boss.x - player.x) <= 1 && abs(boss.y - player.y) <= 1)
    {

    }
}