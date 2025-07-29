#pragma once

#include "inventory.h"


bool CanToolBreakBlock(const Item_Info* tool, int blockType);

int GetToolDamageToBlock(const Item_Info* tool, int blockType);

Player_Item* GetEquippedItem(Inventory* inv, int selectedSlotIndex);

int GetDropItemFromBlockType(block_t blockType);

bool CanPlaceBlock(int x, int y);

block_t GetBlockTypeFromItem(const ItemDB* db, int item_index);

bool ConsumeEquippedBlockItem(Inventory* inv, const ItemDB* db);
