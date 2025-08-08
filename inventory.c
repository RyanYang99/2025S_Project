#include "leak.h"
#include "inventory.h"

#include <conio.h>
#include "map.h"
#include "save.h"
#include "item.h"
#include "Tool.h"
#include "input.h"
#include "delta.h"
#include "console.h"
#include "formatter.h"

#define INVENTORY_BACKGROUND BACKGROUND_T_BLACK
#define INVENTORY_FOREGROUND FOREGROUND_T_WHITE
#define INVENTORY_FOREGROUND_DARK FOREGROUND_T_GRAY
#define INVENTORY_FOREGROUND_BLINK FOREGROUND_T_WHITE
#define INVENTORY_FOREGROUND_IN_HOTBAR FOREGROUND_T_GREEN

#define HOTBAR_SIZE_IN_CHARACTERS_X (TEXTURE_SIZE * HOTBAR_COUNT + HOTBAR_COUNT + 1)
#define HOTBAR_SIZE_IN_CHARACTERS_Y (TEXTURE_SIZE + 2)

static bool should_render_name = false;
static float name_render_timer = 0.0f;

inventory_t inventory = { 0 };

static const int max_selection_index = ITEMS_PER_PAGE - 1,
                 max_page_index = MAX_PAGES - 1,
                 max_hotbar_index = HOTBAR_COUNT - 1;

static bool is_inventory_open = false;
static int current_selection_index = 0,
           current_page_index = 0;

static void inventory_mouse_click(const bool left) {
    if (left || !inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item)
        return;

    player_item_t* const pItem = &inventory.item[inventory.pHotbar[inventory.selected_hotbar_index].index_in_inventory];
    if (!pItem->item_db_index)
        return;

    const item_information_t* const pItem_info = find_item_by_index(pItem->item_db_index);
    if (!pItem_info)
        return;

    if (use_item(pItem->item_db_index))
        decrement_item_from_inventory(pItem);
}


void initialize_inventory(void) {
    is_inventory_open = false;
    current_selection_index = current_page_index = 0;

    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (pCurrent_save)
            inventory.item[i] = pCurrent_save->pInventory[i];
        else
            inventory.item[i].item_db_index = inventory.item[i].quantity = inventory.item[i].durability = 0; // 0은 빈 칸을 의미
    }

    for (int i = 0; i < HOTBAR_COUNT; ++i) {
        if (pCurrent_save) {
            inventory.pHotbar[i].index_in_inventory = pCurrent_save->pHotbar_linked_index[i];

            if (inventory.pHotbar[i].index_in_inventory != -1)
                inventory.pHotbar[i].pPlayer_Item = &inventory.item[inventory.pHotbar[i].index_in_inventory];
        } else {
            inventory.pHotbar[i].index_in_inventory = -1;
            inventory.pHotbar[i].pPlayer_Item = NULL;
        }
    }

    subscribe_mouse_click(inventory_mouse_click);
}

/*
    인벤토리에 아이템을 추가하는 함수
    (이미 있는 아이템이면 개수 추가, 없으면 빈 칸에 추가)
*/
bool add_item_to_inventory(const int item_db_index, int quantity) {
    item_information_t *pItem_info = find_item_by_index(item_db_index);
    assert(pItem_info != NULL && "DB에 존재하지 않는 아이템을 추가하려고 시도했습니다!");

    while (quantity) {
        if (pItem_info->max_stack > 1) {
            for (int i = 0; i < INVENTORY_SIZE; ++i) {
                if (inventory.item[i].item_db_index == item_db_index &&
                    inventory.item[i].quantity < pItem_info->max_stack) { // items -> item (x2)
                    int freeSpace = pItem_info->max_stack - inventory.item[i].quantity; // items -> item
                    if (quantity <= freeSpace) {
                        inventory.item[i].quantity += quantity; // items -> item
                        quantity = 0;
                    } else {
                        inventory.item[i].quantity = pItem_info->max_stack; // items -> item
                        quantity -= freeSpace;
                    }

                    if (!quantity) {
                        return true;
                    }
                }
            }
        }

        int emptySlot = -1;
        for (int i = 0; i < INVENTORY_SIZE; ++i) {
            if (!inventory.item[i].item_db_index) { // items -> item
                emptySlot = i;
                break;
            }
        }

        if (emptySlot == -1) {
            printf("인벤토리가 가득 차서 아이템 [%s] %d개를 더는 얻을 수 없습니다.\n", pItem_info->name, quantity);
            return false;
        }

        inventory.item[emptySlot].item_db_index = item_db_index; // items -> item
        inventory.item[emptySlot].durability = pItem_info->base_durability; // items -> item

        if (quantity <= pItem_info->max_stack) {
            inventory.item[emptySlot].quantity = quantity; // items -> item
            quantity = 0;
        } else {
            inventory.item[emptySlot].quantity = pItem_info->max_stack; // items -> item
            quantity -= pItem_info->max_stack;
        }
    }

    return true;
}

static void remove_item_from_inventory(player_item_t * const pItem) {
    hotbar_link_t *pHotbar = &inventory.pHotbar[inventory.selected_hotbar_index];

    pItem->item_db_index = 0;
    pItem->durability = 0;
    pItem->quantity = 0;

    if (pHotbar->pPlayer_Item->item_db_index == pItem->item_db_index) {
        pHotbar->index_in_inventory = -1;
        pHotbar->pPlayer_Item = NULL;
    }
}

void decrement_item_from_inventory(player_item_t * const pItem) {
    const item_information_t *pItem_information = find_item_by_index(pItem->item_db_index);
    if (!pItem_information)
        return;

    if (pItem->quantity <= 0)
        return;

    if (!(--pItem->quantity))
        remove_item_from_inventory(pItem);

}

static void render_inventory_item(const int y,
                                  const int inventory_index,
                                  const bool selected,
                                  const bool blink) {
    const player_item_t * const pItem = &inventory.item[inventory_index];
    COORD position = {
        .Y = (SHORT)y
    };
        
    FOREGROUND_color_t foreground = INVENTORY_FOREGROUND_DARK;
    if (selected && blink)
        foreground = INVENTORY_FOREGROUND_BLINK;

    if (selected)
        position.X += (SHORT)fprint_string("> ", position, INVENTORY_BACKGROUND, foreground);

    if (pItem->item_db_index) {
        const item_information_t * const pItem_info = find_item_by_index(pItem->item_db_index);
        if (!pItem_info)
            return;

        position.X += (SHORT)fprint_string("[ %s", position, INVENTORY_BACKGROUND, foreground, pItem_info->name);

        if (pItem_info->max_stack > 1)
            position.X += (SHORT)fprint_string(" (x%d)", position, INVENTORY_BACKGROUND, foreground, pItem->quantity);

        if (pItem_info->type == ITEM_TYPE_TOOL || pItem_info->type == ITEM_TYPE_ARMOR)
            position.X += (SHORT)fprint_string(" (Durability: %d/%d)", position, INVENTORY_BACKGROUND, foreground, pItem->durability, pItem_info->base_durability);

        position.X += (SHORT)fprint_string(" ]", position, INVENTORY_BACKGROUND, foreground);

        for (int i = 0; i < max_hotbar_index; ++i)
            if (inventory.pHotbar[i].index_in_inventory == inventory_index)
                fprint_string(" [%d] ", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND_IN_HOTBAR, i + 1);


    } else
        fprint_string("[ Empty ]", position, INVENTORY_BACKGROUND, foreground);
}

void render_inventory(void) {
    static float blink_time = 0.0f;
    static bool blink = false;

    if (!is_inventory_open)
        return;

    blink_time += delta_time;
    if (blink_time >= 0.5f && blink_time < 1.0f)
        blink = true;
    else if (blink_time >= 1.0f) {
        blink = false;
        blink_time = 0.0f;
    }

    COORD position = { 0 };
    fprint_string("=== Inventory (%d / %d) ===", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND, current_page_index + 1, MAX_PAGES);

    const int start_index = current_page_index * ITEMS_PER_PAGE;
    for (int i = 0; i < ITEMS_PER_PAGE; ++i)
    {
        render_inventory_item(++position.Y, start_index + i, i == current_selection_index, blink);
    }

    ++position.Y;
    fprint_string("=== (W / S: Select, A / D: Page, 0 ~ 9: Hotbar, I: Close) ===", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND);

    const player_item_t * const pItem = &inventory.item[start_index + current_selection_index];
    if (!pItem->item_db_index)
        return;

    const item_information_t * const pItem_info = find_item_by_index(pItem->item_db_index);
    position.Y += 2;
    position.X += (SHORT)fprint_string("%s", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND, pItem_info->name);

    char *pDescription = "";
    switch (pItem_info->type) {
        case ITEM_TYPE_MATERIAL:
            pDescription = ": Used in crafting.";
            break;

        case ITEM_TYPE_MISC:
            pDescription = ": Has a special ability.";
            break;
    }

    fprint_string("%s", position, INVENTORY_BACKGROUND, FOREGROUND_T_YELLOW, pDescription);
}

static bool should_skip(const int i, item_type_t * const poItem_type) {
    if (inventory.pHotbar[i].pPlayer_Item == NULL ||
        inventory.pHotbar[i].index_in_inventory == -1 ||
        inventory.item[inventory.pHotbar[i].index_in_inventory].quantity <= 0)
        return true;

    const int index = inventory.pHotbar[i].pPlayer_Item->item_db_index;
    const item_information_t *pItem_info = find_item_by_index(index);
    if (pItem_info == NULL)
        return true;

    *poItem_type = pItem_info->type;
    return false;
}
void render_hotbar(void) {
    const int slot_width = TEXTURE_SIZE + 2;  // 테두리 포함 가로 크기
    const int slot_height = TEXTURE_SIZE + 2; // 테두리 포함 세로 크기

    COORD position = {
        .X = (SHORT)(console.size.X / 2 - (HOTBAR_COUNT * slot_width) / 2),
        .Y = (SHORT)(console.size.Y - slot_height - 1) // 화면 하단 위치 (필요에 따라 조정)
    };

    if (position.X < 0 || position.Y < 0)
        return;

    if (should_render_name && inventory.pHotbar[inventory.selected_hotbar_index].index_in_inventory != -1) {
        if (name_render_timer >= 3.0f) {
            should_render_name = false;
            name_render_timer = 0.0f;
        }
        name_render_timer += delta_time;

        const item_information_t const * pInformation = find_item_by_index(inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item->item_db_index);
        print_center("%s", position.Y - 2, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE, pInformation->name);
    }

    for (int i = 0; i < HOTBAR_COUNT; ++i) {
        bool is_selected = (inventory.selected_hotbar_index == i);

        // 테두리 색상 (선택 시 밝게, 아니면 어둡게)
        WORD border_background = is_selected ? BACKGROUND_T_WHITE : BACKGROUND_T_DARKGRAY;

        int slot_start_x = position.X + i * slot_width;
        int slot_start_y = position.Y;

        // 1. 테두리 영역 출력
        // 위, 아래 가로줄 (빈칸 문자 + 테두리 배경색)
        for (int tx = 0; tx < slot_width; ++tx) {
            print_color_tchar((color_tchar_t) { ' ', border_background, 0 }, (COORD) { (SHORT)(slot_start_x + tx), (SHORT)slot_start_y });
            print_color_tchar((color_tchar_t) { ' ', border_background, 0 }, (COORD) { (SHORT)(slot_start_x + tx), (SHORT)(slot_start_y + slot_height - 1) });
        }

        // 좌, 우 세로줄
        for (int ty = 1; ty < slot_height - 1; ++ty) {
            print_color_tchar((color_tchar_t) { ' ', border_background, 0 }, (COORD) { (SHORT)(slot_start_x), (SHORT)(slot_start_y + ty) });
            print_color_tchar((color_tchar_t) { ' ', border_background, 0 }, (COORD) { (SHORT)(slot_start_x + slot_width - 1), (SHORT)(slot_start_y + ty) });
        }

        // 2. 슬롯 내부 텍스처 출력 (기존 방식과 동일)
        for (int ty = 1; ty < slot_height - 1; ++ty) {
            for (int tx = 1; tx < slot_width - 1; ++tx) {
                color_tchar_t texture_char = { ' ', 0, 0 };

                if (inventory.pHotbar[i].pPlayer_Item != NULL) {
                    int item_index = inventory.pHotbar[i].pPlayer_Item->item_db_index;
                    item_type_t item_type = ITEM_TYPE_NONE;

                    const item_information_t* pItem_info = find_item_by_index(item_index);
                    if (pItem_info != NULL) {
                        item_type = pItem_info->type;
                    }

                    int tex_x = tx - 1;
                    int tex_y = ty - 1;

                    if (item_type == ITEM_TYPE_MATERIAL)
                        texture_char = get_block_texture(item_index, tex_x, tex_y);
                    else if (item_type == ITEM_TYPE_TOOL)
                        texture_char = get_tool_texture(item_index, tex_x, tex_y);
                    else if (item_type == ITEM_TYPE_MISC)
                        texture_char = get_item_texture(item_index, tex_x, tex_y);
                }

                print_color_tchar(texture_char, (COORD) { (SHORT)(slot_start_x + tx), (SHORT)(slot_start_y + ty) });
            }
        }
    }
}



int get_inventory_count(int item_db_index) {
    int count = 0;
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (inventory.item[i].item_db_index == item_db_index) {
            count += inventory.item[i].quantity;
        }
    }
    return count;
}

//I키 입력시 인벤토리 호출
void inventory_input(void) {
    if (!keyboard_pressed)
        return;

    const char character = (char)tolower(input_character);

    bool is_number = false;
    int number = character - '0';
    if (number > 0 && number < 10) {
        is_number = true;
        --number;
    } else if (number == 0) {
        is_number = true;
        number = 9;
    }
    
    if (character == 'i')
        is_inventory_open = !is_inventory_open;
    else if (is_number && number <= max_hotbar_index) {
        inventory.selected_hotbar_index = number;
        should_render_name = true;
        name_render_timer = 0.0f;
    }

    if (!is_inventory_open)
        return;

    if (character == 'w' && current_selection_index > 0)
        --current_selection_index;
    else if (character == 's' && current_selection_index < max_selection_index)
        ++current_selection_index;
    else if (character == 'a' && current_page_index > 0)
        --current_page_index;
    else if (character == 'd' && current_page_index < max_page_index)
        ++current_page_index;
    else if (is_number && number <= max_hotbar_index) {
        const int index = current_page_index * ITEMS_PER_PAGE + current_selection_index;
        if (!inventory.item[index].item_db_index)
            return;

        for (int i = 0; i < max_hotbar_index; ++i)
            if (inventory.pHotbar[i].index_in_inventory == index)
                return;

        inventory.pHotbar[number].index_in_inventory = index;
        inventory.pHotbar[number].pPlayer_Item = &inventory.item[index];
    }
}

void save_inventory(void) {
    if (!pCurrent_save)
        instantiate_save();

    for (int i = 0; i < INVENTORY_SIZE; ++i)
        pCurrent_save->pInventory[i] = inventory.item[i];
    
    for (int i = 0; i < HOTBAR_COUNT; ++i)
        pCurrent_save->pHotbar_linked_index[i] = inventory.pHotbar[i].index_in_inventory;
}