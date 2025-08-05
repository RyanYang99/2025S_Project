#pragma once

#include "Mob.h"
#include "map.h"
#include "player.h"
#include "perlin.h"
#include "inventory.h"
#include "date_time.h"

#define MAX_SAVE_SPOTS 3

#pragma pack(push, 1)
typedef struct {
    date_time_t game_time;

    int x, y, hp;

    player_item_t pInventory[INVENTORY_SIZE];
    int pHotbar_linked_index[HOTBAR_COUNT];

    int pPermuation_table[PERLIN_SIZE], map_x, map_y;
    block_info_t *pBlocks;

    int mob_count, mob_level;
    Mob *pMobs;
} save_t;
#pragma pack(pop)

extern save_t *pCurrent_save;

void initialize_save(void);
void instantiate_save(void);

void save_input(void);
void render_save_menu(void);

void load_save_index(const int index);

const LPCWSTR get_save_folder(void);

bool *get_save_spots(void);

void free_save(void);