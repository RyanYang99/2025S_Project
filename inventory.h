#pragma once

#include <stdbool.h>

#define ITEMS_PER_PAGE 10 //한 페이지에 보여줄 아이템 수
#define MAX_PAGES 5 //최대 페이지 수
#define INVENTORY_SIZE (ITEMS_PER_PAGE * MAX_PAGES) //총 인벤토리 칸 수

#define HOTBAR_COUNT 10

#pragma pack(push, 1)
typedef struct {
    int item_DB_index, //DB를 참조하는 index
        quantity, //현재 수량
        durability; //현재 내구도
} player_item_t;
#pragma pack(pop)

typedef struct {
    int index_in_inventory;
    player_item_t *pPlayer_Item;
} hotbar_link_t;

typedef struct {
    player_item_t item[INVENTORY_SIZE];

    int selected_hotbar_index; //핫바 현재 선택 된 칸
    hotbar_link_t pHotbar[HOTBAR_COUNT];
} inventory_t;

extern inventory_t inventory;

void inventory_initialize(void);
void inventory_input(void);
void inventory_render(void);
void inventory_destroy(void);

const int inventory_get_count(const int item_DB_index);
const bool inventory_add_item(const int item_DB_index, const int quantity);
void inventory_decrement_item(player_item_t * const pItem);
void inventory_decrement_durability(void);

void inventory_save(void);