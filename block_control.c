#include "leak.h"
#include "block_control.h"

#include <math.h>

#include "tool.h"
#include "input.h"
#include "player.h"
#include "inventory.h"
#include "boss_malakh.h"

int block_control_selected_x = 0, block_control_selected_y = 0;

static bool show = false;
static int draw_x = 0, draw_y = 0;

static int cursor_flash_timer = 0;
static FOREGROUND_color_t cursor_flash_color = FOREGROUND_T_WHITE;
static bool cursor_out_of_range = false; //범위 초과 여부

//범위 체크 함수
static const bool is_cursor_in_range(void) {
    const int dx = abs(block_control_selected_x - player.x), dy = abs(block_control_selected_y - player.y);
    return (dx <= 5 && dy <= 5);
}

//마우스 클릭 시 상호작용 처리
static void handle_mouse_click(const bool left) {
    if (block_control_selected_x < 0 || block_control_selected_x >= map.size.x || block_control_selected_y < 0 || block_control_selected_y >= map.size.y)
        return;

    if (!is_cursor_in_range()) {
        //범위 밖 클릭 → 빨간색 경고
        cursor_flash_color = FOREGROUND_T_RED;
        return;
    }

    cursor_flash_color = FOREGROUND_T_GREEN;
    cursor_flash_timer = 50;

    //1. 현재 장착된 아이템 가져오기
    item_information_t *pItem_information = NULL;
    player_item_t *pEquipped = NULL;
    if (inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item) {
        pEquipped = &inventory.item[inventory.pHotbar[inventory.selected_hotbar_index].index_in_inventory];
        pItem_information = database_find_item_by_index(pEquipped->item_DB_index);
    }

    if (left) {
        //2. 현재 블록 정보 확인
        const block_info_t target_block = map_get_block_info(block_control_selected_x, block_control_selected_y);

        //3. 도구가 해당 블록을 부술 수 있는지 확인
        if (!tool_can_break_block(pItem_information, target_block.type))
            return;

        //4. 도구의 데미지 계산
        const int damage = tool_get_damage_to_block(pItem_information, target_block.type);

        //5. 데미지를 주고 파괴 여부 확인
        if (map_damage_block(block_control_selected_x, block_control_selected_y, damage)) {
            const int drop = tool_get_drop_from_block(target_block.type);

            if (drop != -1)
                inventory_add_item(drop, 1);

            inventory_decrement_durability();
        }

        return;
    }

    if (!pItem_information ||
        !pItem_information->is_placeable ||
        pEquipped->quantity <= 0 ||
        !tool_can_place_block(block_control_selected_x, block_control_selected_y))
        return;

    if (pEquipped->item_DB_index == BLOCK_SEED_OF_MALAKH) {
        if (!boss_spawned) {
            if (map_set_block(block_control_selected_x, block_control_selected_y, pItem_information->index)) {

                const int boss_spawn_y = block_control_selected_y - BOSS_SPRITE_HEIGHT;
                const int boss_spawn_x = block_control_selected_x;
                boss_initialize(boss_spawn_x, boss_spawn_y, 100, 10);
                boss_spawned = true;

                inventory_decrement_item(pEquipped);
            }
        }
    } else if (pItem_information->is_placeable &&
        tool_can_place_block(block_control_selected_x, block_control_selected_y) &&
        map_set_block(block_control_selected_x, block_control_selected_y, pItem_information->index))
        inventory_decrement_item(pEquipped);
}

//마우스 이동 시 최신 위치 갱신
static void handle_mouse_move(const COORD position) {
    const float screen_x = (float)console_size.X / 2.0f,
                screen_y = (float)console_size.Y / 2.0f,
                relative_mouse_x = position.X - screen_x,
                relative_mouse_y = position.Y - screen_y;

    block_control_selected_x = player.x + (int)floorf(relative_mouse_x / TEXTURE_SIZE);
    block_control_selected_y = player.y + (int)floorf(relative_mouse_y / TEXTURE_SIZE);
    draw_x = (int)(screen_x + (block_control_selected_x - player.x) * TEXTURE_SIZE);
    draw_y = (int)(screen_y + (block_control_selected_y - player.y) * TEXTURE_SIZE);
}

static void handle_in_console(const bool in_console) {
    show = in_console;
}

//초기화 및 해제
void block_control_initialize(void) {
    input_subscribe_mouse_click(handle_mouse_click);
    input_subscribe_mouse_position(handle_mouse_move);
    input_subscribe_mouse_in_console(handle_in_console);
}

//가상 커서 렌더링 (모서리 스타일)
void block_control_render(void) {
    if (!show)
        return;

    cursor_out_of_range = !is_cursor_in_range();

    FOREGROUND_color_t color = FOREGROUND_T_WHITE;

    if (cursor_flash_timer > 0) {
        color = cursor_flash_color;
        --cursor_flash_timer;
    } else if (cursor_out_of_range)
        color = FOREGROUND_T_RED;

    color_character_t character = {
        .character = L'■',
        .background = BACKGROUND_T_BLACK,
        .foreground = color
    };

    //각 모서리에 문자를 출력
    console_print_color_character(character, (COORD) { (SHORT)draw_x, (SHORT)draw_y });
    console_print_color_character(character, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)draw_y });
    console_print_color_character(character, (COORD) { (SHORT)draw_x, (SHORT)(draw_y + TEXTURE_SIZE - 1) });
    console_print_color_character(character, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)(draw_y + TEXTURE_SIZE - 1) });
}

void block_control_destroy(void) {
    input_unsubscribe_mouse_click(handle_mouse_click);
    input_unsubscribe_mouse_position(handle_mouse_move);
    input_unsubscribe_mouse_in_console(handle_in_console);
}