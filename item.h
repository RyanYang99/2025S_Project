#pragma once

#include "console.h"

typedef enum {
    ITEM_NONE = 0,
    ITEM_APPLE = 401,
    ITEM_HEALTH_POTION = 402
} item_t;

const bool item_use(const item_t item);
const color_character_t item_get_texture(const item_t item, const int x, const int y);