#include "leak.h"

#include "BlockCtrl.h"
#include "map.h"
#include "player.h"
#include <math.h> // floor �Լ� ���

#define MOUSE_X_OFFSET 0
#define MOUSE_Y_OFFSET 0  

static COORD latestMousePos;

// ���콺 �̵� �� �ֽ� ��ġ ����
static void BC_OnMouseMove(const COORD pos)
{
    latestMousePos = pos;
}

// ���콺 Ŭ�� �� ��ȣ�ۿ� ó��
static void BC_OnMouseClick(const bool left)
{
    int screen_cx = console.size.X / 2;
    int screen_cy = console.size.Y / 2;

    int block_width = 3;
    int block_height = 3;

    int relative_mouse_x = latestMousePos.X - screen_cx + MOUSE_X_OFFSET;
    int relative_mouse_y = latestMousePos.Y - screen_cy + MOUSE_Y_OFFSET;

    int block_x = player.x + (int)floor(relative_mouse_x / (float)block_width);
    int block_y = player.y + (int)floor(relative_mouse_y / (float)block_height);

    if (block_x < 0 || block_x >= map.size.x || block_y < 0 || block_y >= map.size.y)
        return;

    if (left)
    {
        int damage = 3;  // ��: �� ������ (���߿� �������� �ٲ� ����)

        bool destroyed = damage_block_at(&map, block_x, block_y, damage);

        if (destroyed)
        {
            // ����� �μ����� �� �߰� ó�� ������ ���⼭
        }
    }
    else
    {
        if (block_x == player.x && block_y == player.y)
            return;  // 1. �÷��̾� ��ġ���� ��ġ ����

        block_info_t target = get_block_info_at(block_x, block_y);
        if (target.type != BLOCK_AIR)
            return;  // 2. �̹� ����� �ִ� ������ ��ġ ����

        block_t held_block = BLOCK_DIRT; // 4. ���߿� �κ��丮���� ������ ������� ��ü
        if (held_block == BLOCK_AIR) return; // ���� ����� ��ġ �Ұ�

        // 5. ������ ��� �ִٸ� ��ġ �Ұ� (����: BLOCK_TOOL_*�� �����ϰų� enum �� ��)
        //if (held_block >= TOOL_BEGIN && held_block <= TOOL_END) return;

        // 6. �� ��� üũ�� get_block_info_at / set_block_at���� �̹� ó��
        set_block_at(block_x, block_y, held_block); // 3. �ڵ����� ü�� �ʱ�ȭ��

        
    }
}


// ���� Ŀ�� ������ (�𼭸� ��Ÿ��)
void render_virtual_cursor(void)
{
    int screen_cx = console.size.X / 2;
    int screen_cy = console.size.Y / 2;

    int block_width = 3;
    int block_height = 3;

    int relative_mouse_x = latestMousePos.X - screen_cx + MOUSE_X_OFFSET;
    int relative_mouse_y = latestMousePos.Y - screen_cy + MOUSE_Y_OFFSET;

    int block_x = player.x + (int)floor(relative_mouse_x / (float)block_width);
    int block_y = player.y + (int)floor(relative_mouse_y / (float)block_height);

    int draw_x = screen_cx + (block_x - player.x) * block_width;
    int draw_y = screen_cy + (block_y - player.y) * block_height;

    // �� �𼭸��� ���ڸ� ���
    color_tchar_t tl = {L'��', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t tr = {L'��', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t bl = {L'��', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t br = {L'��', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };

    print_color_tchar(tl, (COORD) { draw_x, draw_y });
    print_color_tchar(tr, (COORD) { draw_x + block_width - 1, draw_y });
    print_color_tchar(bl, (COORD) { draw_x, draw_y + block_height - 1 });
    print_color_tchar(br, (COORD) { draw_x + block_width - 1, draw_y + block_height - 1 });
}

// �ʱ�ȭ �� ����
void BlockControl_Init(void)
{
    subscribe_mouse_position(BC_OnMouseMove);
    subscribe_mouse_click(BC_OnMouseClick);
}

void BlockControl_Destroy(void)
{
    unsubscribe_mouse_click(BC_OnMouseClick);
    unsubscribe_mouse_position(BC_OnMouseMove);
}





