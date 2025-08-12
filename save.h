#pragma once

#include "mob.h"
#include "map.h"
#include "perlin.h"
#include "inventory.h"
#include "date_time.h"

#define MAX_SAVE_SPOTS 3

#pragma pack(push, 1)
typedef struct {
    date_time_t game_time;

    int x, y, HP, max_HP;

    player_item_t pInventory[INVENTORY_SIZE];
    int pHotbar_linked_index[HOTBAR_COUNT];

    int pPermuation_table[PERLIN_SIZE], map_x, map_y;
    block_info_t *pBlocks;

    int mob_count, mob_level;
    mob_t *pMobs;
} save_t;
#pragma pack(pop)

extern save_t *pSave_current;

void save_initialize(void);
void save_input(void);
void save_render(void);

void save_instantiate(void);
const LPCWSTR get_save_folder(void);
const bool * const get_save_spots(void);
void load_save_index(const int index);

void save_free(void);