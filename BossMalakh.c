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

//�������� ���� ��ũ�� ������

#define BG_BLACK BACKGROUND_T_BLACK // �������� ���� ��ũ�� ����
#define FG_WHITE FOREGROUND_T_WHITE
#define FG_YELLOW FOREGROUND_T_YELLOW
#define FG_CYAN FOREGROUND_T_CYAN // �� ����
#define FG_MAGENTA FOREGROUND_T_MAGENTA // ������ ����Ʈ ����
#define FG_RED FOREGROUND_T_RED 

#define BOSS_SPRITE_WIDTH 20
#define BOSS_SPRITE_HEIGHT 20
#define BOSS_DRAW_SCALE 3 //��������Ʈ Ȯ�� ����


static const color_tchar_t boss_malakh_sprite_data[BOSS_SPRITE_HEIGHT][BOSS_SPRITE_WIDTH] = {
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'.',BG_BLACK,FG_MAGENTA},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 0 (������ �ֻ�� ��)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{L' ',BG_BLACK,BG_BLACK},{L'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{L'\\',BG_BLACK,FG_YELLOW},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 1 (�� Ȯ��)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,FG_YELLOW},{L'/',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{'-',BG_BLACK,FG_YELLOW},{L'\\',BG_BLACK,FG_YELLOW},{L' ',BG_BLACK,FG_YELLOW},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 2 (�� �� ũ��)

    // ���̾Ƹ�� ���� ���� (���θ� ä��)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 3
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 4
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 5

    // �� �� ��ü �κ� 
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 6 (�� ���: X, V�� ������)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 7 (�� �߾�)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{'/',BG_BLACK,FG_WHITE},{L'X',BG_BLACK,FG_RED},   {L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'X',BG_BLACK,FG_RED},{L'V',BG_BLACK,FG_RED},{L'X',BG_BLACK,FG_RED},{L' ',BG_BLACK,BG_BLACK},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 8 (�� �ϴ�)
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'V',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'^',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'V',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'^',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L'V',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 9 (��: V, ^�� �̿��� �̻� ǥ��)

    // ���̾Ƹ�� �Ʒ��κ� 
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 10
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 11
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,FG_WHITE},{L'/',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 12
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 13
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 14
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L'/',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'H',BG_BLACK,FG_WHITE},{'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 15
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,FG_WHITE},{L'\\',BG_BLACK,FG_WHITE},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 16 (���̾Ƹ�� ���ϴ� ��)
    
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 17
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}, // 18
    {{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK},{L' ',BG_BLACK,BG_BLACK}}  // 19
};

//���ڿ� ��� ���� �Լ�
static void Boss_Print_String_Color_W(const wchar_t* wstr, const COORD position, unsigned short background_color, unsigned short foreground_color) {
    COORD current_pos = position;
    for (int i = 0; wstr[i] != L'\0'; ++i) {
        print_color_tchar((color_tchar_t) { wstr[i], background_color, foreground_color }, current_pos);
        current_pos.X++;
    }
}

//���� ���� ���� ���̽�Ÿ
static bool is_boss_chasing(int x, int y)
{
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y) {
        return false;
    }
    block_info_t block = get_block_info_at(x, y);

    //�̵�  ���� 
    return block.type == BLOCK_AIR || block.type == BLOCK_GRASS || block.type == BLOCK_DIRT || block.type == BLOCK_SNOW;

}


//���� �ϴÿ� �����ϱ� ���� ��ġ ã�� �Լ�
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
    //���� ����� ��ã���� ��� 
    int default_y = map.size.y / 4;

    return default_y;
}




//���� �ʱ�ȭ
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
    boss.special_attack_cooltime = 10000; //10�� ��Ÿ��
    boss.last_special_attack_time = clock();

    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 20; x++)
        {
            boss.sprite_data[y][x] = boss_malakh_sprite_data[y][x];

        }
    }

}
// ���� ������
void Boss_Render()
{
    if (boss.state == E_BOOS_STATE_DEFEATED) return;

    COORD center_m = { console.size.X / 2 , console.size.Y / 2 };

   
    int boss_screen_base_x = center_m.X + (boss.x - player.x) * BOSS_DRAW_SCALE - (BOSS_SPRITE_WIDTH * BOSS_DRAW_SCALE / 2);
    int boss_screen_base_y = center_m.Y + (boss.y - player.y) * BOSS_DRAW_SCALE - (BOSS_SPRITE_HEIGHT * BOSS_DRAW_SCALE / 2) - 30; // 30�ȼ�(10ĭ)��ŭ �� ���� �̵�

    // === ��������Ʈ ������ ===
    for (int y_offset = 0; y_offset < 20; ++y_offset) { // 20��
        for (int x_offset = 0; x_offset < 20; ++x_offset) { // 20��
            // �� ��������Ʈ �ȼ��� 3x3���� Ȯ���Ͽ� �׸�
            for (int py = 0; py < 3; ++py) {
                for (int px = 0; px < 3; ++px) {
                    COORD char_pos = { boss_screen_base_x + (x_offset * 3) + px, boss_screen_base_y + (y_offset * 3) + py };

                    // ȭ�� ���� üũ
                    if (char_pos.X >= 0 && char_pos.X < console.size.X &&
                        char_pos.Y >= 0 && char_pos.Y < console.size.Y) {

                        color_tchar_t char_to_print = boss.sprite_data[y_offset][x_offset];
                        if (char_to_print.character != L' ') { // ������ �׸��� ���� (���� ó��)
                            print_color_tchar(char_to_print, char_pos);
                        }
                    }
                }
            }
        }
    }
    // ========================

    // ���� ü�� �� ������ (��������Ʈ ���� ǥ��)
    wchar_t hp_text[50];
    swprintf(hp_text, sizeof(hp_text) / sizeof(wchar_t), L"HP: %d/%d", boss.hp, boss.Max_hp);

    // ü�� �� ��ġ ����: ��������Ʈ �߾� ��ܿ� ������
    // ��������Ʈ�� ���� �ȼ� �ʺ� (20 * 3 = 60)�� ����Ͽ� �߾� ���
    COORD hp_pos = { boss_screen_base_x + (20 * 3 / 2) - (int)(wcslen(hp_text) / 2.0), boss_screen_base_y - 1 };
    if (hp_pos.X < 0) hp_pos.X = 0;
    if (hp_pos.Y < 0) hp_pos.Y = 0; // Y ��ǥ�� 0 �̸� ����

    Boss_Print_String_Color_W(hp_text, hp_pos, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);
}


//���� AI
void Boss_Update_Ai()
{
    if (boss.state == E_BOOS_STATE_DEFEATED)return;

    long current_time = clock();
    int dist_x = abs(boss.x - player.x);
    int dist_y = abs(boss.y - player.y);
    int distance = dist_x + dist_y; //����ư �Ÿ�

    //������ ��ȯ ����
    //hp 30% ����
    if (boss.hp <= boss.Max_hp * 0.3 && boss.state < E_BOOS_STATE_PHASE3)
    {
        //3������ ����
        boss.state = E_BOOS_STATE_PHASE3;
#if _DEBUG
        printf("Boss enters Phase 3 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
        boss.atk = boss.atk * 2;  //���ݷ� 2�� ���

        boss.special_attack_cooltime = 4000; //4�� ��Ÿ��
        boss.last_special_attack_time = current_time; // ��Ÿ�� �ʱ�ȭ �߰�
    }
    //hp 60% ����
    else if (boss.hp <= boss.Max_hp * 0.6 && boss.state < E_BOOS_STATE_PHASE2)
    {
        //2������ ����
        boss.state = E_BOOS_STATE_PHASE2;
#if _DEBUG
        printf("Boss enters Phase 2 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
        boss.atk = boss.atk * 1.5; //���ݷ� 1.5�� ���
        boss.special_attack_cooltime = 6000; //6�ʸ��� �ñر� ���
        boss.last_special_attack_time = current_time; // ��Ÿ�� �ʱ�ȭ �߰�
    }
    else if (boss.hp > boss.Max_hp * 0.6 && boss.state != E_BOOS_STATE_PHASE1) // �ʱ� && 1������ ���� üũ ����
    {
        boss.state = E_BOOS_STATE_PHASE1;
#if _DEBUG
        printf("Boss is in  Phase 1 ! (Hp : %d / %d )\n", boss.hp, boss.Max_hp);
#endif
        boss.last_action_time = current_time;
    }

    //�� ����� ����

       // --- �� ����� �ൿ ���� ---
    switch (boss.state) {
    case E_BOOS_STATE_PHASE1: { // 1������: �ܼ� �߰� �� �Ϲ� ����
        if (distance <= 100) { //100ĭ ���� �� ���� 
            if ((current_time - boss.last_move_time) / (double)CLOCKS_PER_SEC >= 3.0) { // ~�ʸ��� �̵�
                
                int steps_paths = 0;
                const int max_steps = 5; //5ĭ �� �̵� 

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
            {//���̻� �̵��Ҽ� ���ų�, ��� ��ã�� ��� ���� ���� 
                break;
            }
            //����� �÷��̾�� ����� ���� �̵����� ����
            if (abs(boss.x - player.x) + abs(boss.y - player.y) <= 10) {
                break;
            }

        boss.last_move_time = current_time;
        }
        //���� 10ĭ �̳��� ���� ��� ���� 
        if (distance <= 10) {

            if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 1.0) { // 1�ʸ��� ����
                player_take_damage(boss.atk);
#if _DEBUG
                printf("Boss Phase 1 attacks player! (%d HP left)\n", player.hp);
#endif
                boss.last_action_time = current_time;
            }
    }
        
        // 1������ Ư�� ���� (��Ÿ���� �Ǹ� ���)
        if ((current_time - boss.last_special_attack_time) / (double)CLOCKS_PER_SEC * 1000 >= boss.special_attack_cooltime) {
#if _DEBUG
            printf("Boss Phase 1 uses Special Attack!\n");
#endif
            // ������ ���� ���� ���� (�÷��̾� �ֺ� 3x3 ������)
            if (distance <= 3) {
                player_take_damage(boss.atk * 1.5);
            }
            boss.last_special_attack_time = current_time;
        }
        break;
    }

    case E_BOOS_STATE_PHASE2: { // 2������: �� ���� �̵�, ���ο� ��ų �߰�
        if (distance <= 100) {
            if ((current_time - boss.last_move_time) / (double)CLOCKS_PER_SEC >= 2.0) { //2�ʸ��� �̵�
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
        if(distance <=15 ) { // 15ĭ ���� ������ ������ ����
            if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 1.0) { // 1�ʸ��� ����
                player_take_damage(boss.atk);
#if _DEBUG
                printf("Boss Phase 2 attacks player! (%d HP left)\n", player.hp);
#endif
                boss.last_action_time = current_time;
            }
        }
        // 2������ Ư�� ���� (��Ÿ���� �� ª����)
        if ((current_time - boss.last_special_attack_time) / (double)CLOCKS_PER_SEC * 1000 >= boss.special_attack_cooltime) {
#if _DEBUG
            printf("Boss Phase 2 uses Enhanced Special Attack!\n");
#endif
            // �÷��̾� �ֺ� 5x5 ���� ������ (�� ���� ����)
            if (distance <= 5) {
                player_take_damage(boss.atk * 2);
            }
            boss.last_special_attack_time = current_time;
        }
        break;
    }

    case E_BOOS_STATE_PHASE3: { // 3������: ���� ������ ����, ���ϱ� ����� ����
        if (distance <= 100) {
            if ((current_time - boss.last_move_time) / (double)CLOCKS_PER_SEC >= 1.0) { // 1�ʸ��� �̵� (�ſ� ����)
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

        // 3������� ���� ���� (��: 2�ʸ��� ���� ���� �ߵ�)
        if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 2.0) {
            int pattern_choice = rand() % 2; // 2���� ���� �� �ϳ� ����
            if (pattern_choice == 0) { // ���� 1: ���� ����
#if _DEBUG
                printf("Boss Phase 3 - Area Blast! (Player HP: %d)\n", player.hp);
#endif
                if (distance <= 8) { // ���� ����
                    player_take_damage(boss.atk * 3);
                }
            }

            boss.last_action_time = current_time;
        }
        // Ư�� ���� (��Ÿ���� �ſ� ª����)
        if ((current_time - boss.last_special_attack_time) / (double)CLOCKS_PER_SEC * 1000 >= boss.special_attack_cooltime) {
#if _DEBUG
            printf("Boss Phase 3 uses Ultimate Special Attack!\n");
#endif
            // ȭ�� ��ü �Ǵ� �ſ� ���� ���� ������
            player_take_damage(boss.atk * 5); // �ſ� ū ������
            boss.last_special_attack_time = current_time;
        }
        break;
    }

    case E_BOOS_STATE_DAMAGED: { // �ǰ� ���� (��� ����)
        if ((current_time - boss.last_action_time) / (double)CLOCKS_PER_SEC >= 0.2) { // 0.2�� ���� ��
            // �ǰ� ���� �� ���� ������ ���·� ���ư�
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



//���� ������ ������ �Լ�
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
        //todo : ���� ���ó�� -> ������ ���
    }
    else
    {
        boss.state = E_BOOS_STATE_DAMAGED;
        boss.last_action_time = clock();
    }

}
//���� �浹 ó��
void Boss_Player_Collision()
{
    if (boss.state == E_BOOS_STATE_DEFEATED)return;

    if (abs(boss.x - player.x) <= 1 && abs(boss.y - player.y) <= 1)
    {

    }
}