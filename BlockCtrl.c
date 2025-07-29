#include "leak.h"

#include "BlockCtrl.h"
#include "map.h"
#include "player.h"
#include <math.h> // floorf 함수 사용
#include "ItemDB.h"
#include "Tool.h"
#include "inventory.h"

static COORD latestMousePos;
static float screen_cx = 0, screen_cy = 0, relative_mouse_x = 0, relative_mouse_y = 0;
static int block_x = 0, block_y = 0, draw_x = 0, draw_y = 0;

#if _DEBUG
int selected_block_x = -1, selected_block_y = -1;
#endif

// 마우스 이동 시 최신 위치 갱신
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

// 마우스 클릭 시 상호작용 처리
static void BC_OnMouseClick(const bool left)
{
    if (block_x < 0 || block_x >= map.size.x || block_y < 0 || block_y >= map.size.y)
        return;

    if (left)
    {
        // 1. 현재 장착된 아이템 가져오기
        Player_Item* equipped = GetEquippedItem(&g_inv, g_inv.selectedIndex);
        const Item_Info* info = NULL;
        if (equipped)
        {
            info = FindItemByIndex(&g_db, equipped->Item_Index);
        }

        // 2. 현재 블록 정보 확인
        block_info_t targetBlock = get_block_info_at(block_x, block_y);

        // 3. 도구가 해당 블록을 부술 수 있는지 확인
        if (!CanToolBreakBlock(info, targetBlock.type)) return;

        // 4. 도구의 데미지 계산
        //int damage = GetToolDamageToBlock(info, targetBlock.type);

        // 5. 데미지를 주고 파괴 여부 확인
        //bool destroyed = damage_block_at(&map, block_x, block_y, damage);

        /*
        if (destroyed)
        {
            int dropItemIndex = GetDropItemFromBlockType(targetBlock.type);

            if (dropItemIndex != -1)
            {
                AddItemToInventory(g_inv, g_db, dropItemIndex, 1);
            }
        }
        */
    }
    else
    {


        // 1. 인벤토리에서 선택된 아이템 가져오기
        Player_Item* equipped = GetEquippedItem(&g_inv, g_inv.selectedIndex);
        if (!equipped || equipped->quantity <= 0) return;

        // 2. 아이템 정보 가져오기
        const Item_Info* info = FindItemByIndex(&g_db, equipped->Item_Index);
        if (!info || !info->isplaceable) return;  // 설치 불가능한 아이템은 무시

        // 3. 설치 가능한 위치인지 확인
        if (!CanPlaceBlock(block_x, block_y)) return;

        // 4. 아이템 인덱스로 설치할 블록 타입 추출
        block_t blockType = GetBlockTypeFromItem(&g_db, equipped->Item_Index);
        if (blockType == BLOCK_AIR) return;

        // 5. 설치 시도
        bool success = set_block_at(block_x, block_y, blockType);
        if (!success) return;

        // 6. 수량 차감
        ConsumeEquippedBlockItem(&g_inv, &g_db);
    }
}

// 가상 커서 렌더링 (모서리 스타일)
void render_virtual_cursor(void)
{
    // 각 모서리에 문자를 출력
    color_tchar_t tl = { L'┌', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t tr = { L'┐', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t bl = { L'└', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t br = { L'┘', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };

    print_color_tchar(tl, (COORD) { (SHORT)draw_x, (SHORT)draw_y });
    print_color_tchar(tr, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)draw_y });
    print_color_tchar(bl, (COORD) { (SHORT)draw_x, (SHORT)(draw_y + TEXTURE_SIZE - 1) });
    print_color_tchar(br, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)(draw_y + TEXTURE_SIZE - 1) });
}

// 초기화 및 해제
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