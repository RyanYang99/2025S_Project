#include "leak.hpp"
#include "block_control.hpp"

#include <cmath>

#include "map.hpp"
#include "tool.hpp"
#include "sound.hpp"
#include "input.hpp"
#include "player.hpp"
#include "console.hpp"
#include "inventory.hpp"
#include "boss_malakh.hpp"
#include "item_database.hpp"

int block_control_selected_x{}, block_control_selected_y{};

static bool show{};
static int draw_x{}, draw_y{};

static int cursor_flash_timer{};
static FG cursor_flash_color{ FG::white };
static bool cursor_out_of_range{}; //범위 초과 여부

//범위 체크 함수
static bool is_cursor_in_range(void) noexcept {
    return abs(block_control_selected_x - player.x) <= 5 && abs(block_control_selected_y - player.y) <= 5;
}

//마우스 클릭 시 상호작용 처리
static void handle_mouse_click(const bool left) {
    if (block_control_selected_x < 0 || block_control_selected_x >= map.size.x || block_control_selected_y < 0 || block_control_selected_y >= map.size.y)
        return;

    if (!is_cursor_in_range()) {
        //범위 밖 클릭 → 빨간색 경고
        cursor_flash_color = FG::red;
        return;
    }

    cursor_flash_color = FG::green;
    cursor_flash_timer = 50;

    //1. 현재 장착된 아이템 가져오기
    item_information_t *pItem_information{};
    player_item_t *pEquipped{};
    if (inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item) {
        pEquipped = &inventory.item[inventory.pHotbar[inventory.selected_hotbar_index].index_in_inventory];
        pItem_information = database_find_item_by_index(pEquipped->item_DB_index);
    }

    if (left) {
        //2. 현재 블록 정보 확인
        const block_info_t target_block{ map_get_block_info(block_control_selected_x, block_control_selected_y) };

        //3. 도구가 해당 블록을 부술 수 있는지 확인
        if (!tool_can_break_block(pItem_information, target_block.type))
            return;

        //4. 도구의 데미지 계산
        const int damage{ tool_get_damage_to_block(pItem_information, target_block.type) };

        //5. 데미지를 주고 파괴 여부 확인
        if (map_damage_block(block_control_selected_x, block_control_selected_y, damage)) {
            const int drop{ tool_get_drop_from_block(target_block.type) };

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
            if (map_set_block(block_control_selected_x, block_control_selected_y, static_cast<block_t>(pItem_information->index))) {

                const int boss_spawn_y{ block_control_selected_y - BOSS_SPRITE_HEIGHT },
                          boss_spawn_x{ block_control_selected_x };
                boss_initialize(boss_spawn_x, boss_spawn_y, 100, 10);
                boss_spawned = true;

                sound_play_sound_effect(BOSS_SOUND_SPAWN);
                
                inventory_decrement_item(pEquipped);
            }
        }
    } else if (pItem_information->is_placeable &&
        tool_can_place_block(block_control_selected_x, block_control_selected_y) &&
        map_set_block(block_control_selected_x, block_control_selected_y, static_cast<block_t>(pItem_information->index)))
        inventory_decrement_item(pEquipped);
}

//마우스 이동 시 최신 위치 갱신
static void handle_mouse_move(const COORD position) noexcept {
    const COORD &size{ Console::size() };

    const float screen_x = static_cast<float>(size.X) / 2.0f,
                screen_y = static_cast<float>(size.Y) / 2.0f,
                relative_mouse_x = position.X - screen_x,
                relative_mouse_y = position.Y - screen_y;

    block_control_selected_x = player.x + static_cast<int>(floorf(relative_mouse_x / TEXTURE_SIZE));
    block_control_selected_y = player.y + static_cast<int>(floorf(relative_mouse_y / TEXTURE_SIZE));
    draw_x = static_cast<int>(screen_x + (block_control_selected_x - player.x) * TEXTURE_SIZE);
    draw_y = static_cast<int>(screen_y + (block_control_selected_y - player.y) * TEXTURE_SIZE);
}

static void handle_in_console(const bool in_console) noexcept {
    show = in_console;
}

//초기화 및 해제
void block_control_initialize(void) {
    Input::subscribe_input_mouse_click(handle_mouse_click);
    Input::subscribe_input_mouse_position(handle_mouse_move);
    Input::subscribe_input_mouse_in_console(handle_in_console);
}

//가상 커서 렌더링 (모서리 스타일)
void block_control_render(void) {
    if (!show)
        return;

    cursor_out_of_range = !is_cursor_in_range();

    FG color{ FG::white };

    if (cursor_flash_timer > 0) {
        color = cursor_flash_color;
        --cursor_flash_timer;
    } else if (cursor_out_of_range)
        color = FG::red;

    const cchar character { L'■', BG::black, color };

    //각 모서리에 문자를 출력
    Console::print(character, { static_cast<SHORT>(draw_x), static_cast<SHORT>(draw_y) });
    Console::print(character, { static_cast<SHORT>(draw_x + TEXTURE_SIZE - 1), static_cast<SHORT>(draw_y) });
    Console::print(character, { static_cast<SHORT>(draw_x), static_cast<SHORT>(draw_y + TEXTURE_SIZE - 1) });
    Console::print(character, { static_cast<SHORT>(draw_x + TEXTURE_SIZE - 1), static_cast<SHORT>(draw_y + TEXTURE_SIZE - 1) });
}

void block_control_destroy(void) noexcept {
    Input::unsubscribe_input_mouse_click(handle_mouse_click);
    Input::unsubscribe_input_mouse_position(handle_mouse_move);
    Input::unsubscribe_input_mouse_in_console(handle_in_console);
}