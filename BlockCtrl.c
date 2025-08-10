#include "leak.h"
#include "BlockCtrl.h"

#include <math.h> //floorf 함수 사용
#include "map.h"
#include "Tool.h"
#include "player.h"
#include "ItemDB.h"
#include "inventory.h"

int selected_block_x = 0, selected_block_y = 0;

#include "BossMalakh.h"
#include "game.h"


static bool show = false;
static COORD latestMousePos = { 0 };
static float screen_cx = 0, screen_cy = 0, relative_mouse_x = 0, relative_mouse_y = 0;
static int draw_x = 0, draw_y = 0;

static int cursor_flash_timer = 0;
static FOREGROUND_color_t cursor_flash_color = FOREGROUND_T_WHITE;
static bool cursor_out_of_range = false; // 범위 초과 여부

// 범위 체크 함수
static bool is_cursor_in_range(void) {
    int dx = abs(selected_block_x - player.x);
    int dy = abs(selected_block_y - player.y);
    return (dx <= 5 && dy <= 5);
}

//#if _DEBUG
//int selected_block_x = -1, selected_block_y = -1;
//#endif


//마우스 클릭 시 상호작용 처리
static void handle_mouse_click(const bool left) {
    if (selected_block_x < 0 || selected_block_x >= map.size.x || selected_block_y < 0 || selected_block_y >= map.size.y)
        return;

    if (!is_cursor_in_range()) {
        // 범위 밖 클릭 → 빨간색 경고 
        cursor_flash_color = FOREGROUND_T_RED;
        return;
    }

    cursor_flash_color = FOREGROUND_T_GREEN;
    cursor_flash_timer = 50;
    

    //1. 현재 장착된 아이템 가져오기
    item_information_t* pItem_information = NULL;
    player_item_t* pEquipped = NULL;
    if (inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item)
    {
        pEquipped = &inventory.item[inventory.pHotbar[inventory.selected_hotbar_index].index_in_inventory];
        pItem_information = find_item_by_index(pEquipped->item_db_index);


    if (left) {
        //2. 현재 블록 정보 확인
        const block_info_t target_block = get_block_info_at(selected_block_x, selected_block_y);

            //3. 도구가 해당 블록을 부술 수 있는지 확인
            if (!can_tool_break_block(pItem_information, target_block.type))
                return;

            //4. 도구의 데미지 계산
            const int damage = get_tool_damage_to_block(pItem_information, target_block.type);

        //5. 데미지를 주고 파괴 여부 확인
        if (damage_block_at(&map, selected_block_x, selected_block_y, damage)) {
            const int drop = get_drop_from_block(target_block.type);

                if (drop != -1)
                    add_item_to_inventory(drop, 1);
            }
        }
        else {

            if (!pItem_information ||
                !pItem_information->is_placeable ||
                pEquipped->quantity <= 0 ||
                !can_place_block(selected_block_x, selected_block_y))
                return;

            if (pEquipped->item_db_index == BLOCK_SEED_OF_MALAKH) {
                if (!is_boss_spawned) {
                    if (set_block_at(selected_block_x, selected_block_y, pItem_information->index)) {

                        const int boss_spawn_y = selected_block_y - BOSS_SPRITE_HEIGHT;
                        const int boss_spawn_x = selected_block_x;
                        Boss_Init(boss_spawn_x, boss_spawn_y, 100, 10);
                        is_boss_spawned = true;

                        decrement_item_from_inventory(pEquipped);
                    }
                }
            }

            else if (pItem_information->is_placeable && can_place_block(selected_block_x, selected_block_y)) {
                if (set_block_at(selected_block_x, selected_block_y, pItem_information->index)) {
                    decrement_item_from_inventory(pEquipped);
                }
            }
        }
    }
}

//마우스 이동 시 최신 위치 갱신
static void handle_mouse_move(const COORD pos) {
    latestMousePos = pos;

    screen_cx = (float)console.size.X / 2.0f;
    screen_cy = (float)console.size.Y / 2.0f;
    relative_mouse_x = latestMousePos.X - screen_cx;
    relative_mouse_y = latestMousePos.Y - screen_cy;

    selected_block_x = player.x + (int)floorf(relative_mouse_x / TEXTURE_SIZE);
    selected_block_y = player.y + (int)floorf(relative_mouse_y / TEXTURE_SIZE);
    draw_x = (int)(screen_cx + (selected_block_x - player.x) * TEXTURE_SIZE);
    draw_y = (int)(screen_cy + (selected_block_y - player.y) * TEXTURE_SIZE);


#if _DEBUG
    selected_block_x = selected_block_x;
    selected_block_y = selected_block_y;
#endif
    selected_block_x = player.x + (int)floorf(relative_mouse_x / TEXTURE_SIZE);
    selected_block_y = player.y + (int)floorf(relative_mouse_y / TEXTURE_SIZE);
    draw_x = (int)(screen_cx + (selected_block_x - player.x) * TEXTURE_SIZE);
    draw_y = (int)(screen_cy + (selected_block_y - player.y) * TEXTURE_SIZE);
}

static void handle_in_console(const bool in_console) {
    show = in_console;
}

//초기화 및 해제
void initialize_block_control(void) {
    subscribe_mouse_click(handle_mouse_click);
    subscribe_mouse_position(handle_mouse_move);
    subscribe_mouse_in_console(handle_in_console);
}

//가상 커서 렌더링 (모서리 스타일)
void render_virtual_cursor(void) {
    if (!show)
        return;

    cursor_out_of_range = !is_cursor_in_range();

    FOREGROUND_color_t color = FOREGROUND_T_WHITE;

    if (cursor_flash_timer > 0) {
        color = cursor_flash_color;
        cursor_flash_timer--;
    }
    else if (cursor_out_of_range) {
        color = FOREGROUND_T_RED;
    }

    color_tchar_t character = {
        .character = L'■',
        .background = BACKGROUND_T_BLACK,
        .foreground = color
    };

    //각 모서리에 문자를 출력
    print_color_tchar(character, (COORD) { (SHORT)draw_x, (SHORT)draw_y });

    print_color_tchar(character, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)draw_y });

    print_color_tchar(character, (COORD) { (SHORT)draw_x, (SHORT)(draw_y + TEXTURE_SIZE - 1) });

    print_color_tchar(character, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)(draw_y + TEXTURE_SIZE - 1) });
}

void destroy_block_control(void) {
    unsubscribe_mouse_click(handle_mouse_click);
    unsubscribe_mouse_position(handle_mouse_move);
    unsubscribe_mouse_in_console(handle_in_console);
}