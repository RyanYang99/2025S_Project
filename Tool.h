#pragma once

#include "map.h"
#include "console.h"
#include "inventory.h"
#include "item_database.h"

const bool tool_can_break_block(const item_information_t * const pTool, const block_t block);
const int tool_get_damage_to_block(const item_information_t * const tool, const block_t block);
const int tool_get_drop_from_block(const block_t block);

const bool tool_can_place_block(const int x, const int y);

const color_character_t tool_get_texture(const tool_t tool, const int x, const int y);
const color_character_t tool_get_swing_texture(const tool_t tool, const int x, const int y);