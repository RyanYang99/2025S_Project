#pragma once

#include "map.h"
#include "ItemDB.h"
#include "console.h"
//#include "inventory.h"

const bool can_tool_break_block(const item_information_t * const pTool, const block_t block);
const int get_tool_damage_to_block(const item_information_t * const tool, const block_t block);
const int get_drop_from_block(const block_t block);

/*
player_item_t* GetEquippedItem(inventory_t* inv, int selectedSlotIndex);

bool CanPlaceBlock(int x, int y);

bool ConsumeEquippedBlockItem(inventory_t* inv, const item_database_t* db);
*/

const color_tchar_t get_tool_texture(const tool_t tool, const int x, const int y);