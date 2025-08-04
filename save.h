#pragma once

#include "map.h"
#include "player.h"
#include "perlin.h"
#include "inventory.h"
#include "date_time.h"

#pragma pack(push, 1)
typedef struct {
    char *pSave_name;
    date_time_t game_time;

    int x, y, hp;

    player_item_t pInventory[INVENTORY_SIZE];
    int pHotbar_linked_index[HOTBAR_COUNT];

    int pPermuation_table[PERLIN_SIZE];

    int map_x, map_y;
    block_info_t *pBlocks;
} save_t;
#pragma pack(pop)

extern save_t *pCurrent_save;

void instantiate_save(void);
void write_save(const char * const pPath);
save_t *load_save(const char * const pPath);