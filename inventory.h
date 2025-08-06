#pragma once

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ItemDB.h"

#define ITEMS_PER_PAGE 10 //한 페이지에 보여줄 아이템 수
#define MAX_PAGES 5 //최대 페이지 수
#define INVENTORY_SIZE (ITEMS_PER_PAGE * MAX_PAGES) //총 인벤토리 칸 수

#define HOTBAR_COUNT 10

#pragma pack(push, 1)
typedef struct {
    int item_db_index; //DB를 참조하는 index
    int quantity; //현재 수량
    int durability; //현재 내구도
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

void initialize_inventory(void);

bool add_item_to_inventory(const int item_db_index, int quantity);
void decrement_item_from_inventory(player_item_t * const pItem);

void render_inventory(void);
void render_hotbar(void);
void inventory_input(void);

void save_inventory(void);