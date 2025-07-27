#include "leak.h"

#include "BlockCtrl.h"
#include "map.h"
#include "player.h"
#include <math.h> // floorf �Լ� ���

static COORD latestMousePos;
static float screen_cx = 0, screen_cy = 0, relative_mouse_x = 0, relative_mouse_y = 0;
static int block_x = 0, block_y = 0, draw_x = 0, draw_y = 0;

#if _DEBUG
int selected_block_x = -1, selected_block_y = -1;
#endif

// ���콺 �̵� �� �ֽ� ��ġ ����
static void BC_OnMouseMove(const COORD pos)
{
    latestMousePos = pos;

    screen_cx = (float)console.size.X / 2.0f;
    screen_cy = (float)console.size.Y / 2.0f;
    relative_mouse_x = latestMousePos.X - screen_cx;
    relative_mouse_y = latestMousePos.Y - screen_cy;

    block_x = player.x + (int)floorf(relative_mouse_x / TEXTURE_SIZE);
    block_y = player.y + (int)floorf(relative_mouse_y / TEXTURE_SIZE);
    draw_x = (int)(screen_cx + (block_x - player.x) * TEXTURE_SIZE);
    draw_y = (int)(screen_cy + (block_y - player.y) * TEXTURE_SIZE);

#if _DEBUG
    selected_block_x = block_x;
    selected_block_y = block_y;
#endif
}

// ���콺 Ŭ�� �� ��ȣ�ۿ� ó��
static void BC_OnMouseClick(const bool left)
{
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
    // �� �𼭸��� ���ڸ� ���
    color_tchar_t tl = { L'��', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t tr = { L'��', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t bl = { L'��', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t br = { L'��', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };

    print_color_tchar(tl, (COORD) { (SHORT)draw_x, (SHORT)draw_y });
    print_color_tchar(tr, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)draw_y });
    print_color_tchar(bl, (COORD) { (SHORT)draw_x, (SHORT)(draw_y + TEXTURE_SIZE - 1) });
    print_color_tchar(br, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)(draw_y + TEXTURE_SIZE - 1) });
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