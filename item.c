#include "leak.h"
#include "item.h"

#include "map.h"
#include "player.h"

static const color_character_t pHealth_potion[TEXTURE_SIZE][TEXTURE_SIZE] = {
    { { ' ', BACKGROUND_T_TRANSPARENT, 0 }, { ' ', BACKGROUND_T_WHITE, 0 }, { ' ', BACKGROUND_T_TRANSPARENT, 0 } },
    { { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 } },
    { { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 } },
}, pApple[TEXTURE_SIZE][TEXTURE_SIZE] = {
    { { ' ', BACKGROUND_T_TRANSPARENT, 0 }, { ' ', BACKGROUND_T_GREEN, 0 }, { ' ', BACKGROUND_T_TRANSPARENT, 0 } },
    { { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 } },
    { { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 } },
};

const bool item_use(const item_t item) {
    switch (item) {
        case ITEM_HEALTH_POTION:
            if (player.HP >= player.max_HP)
                return false;
            else
                player_add_health(100);
                return true;

        case ITEM_APPLE:
            if (player.HP >= player.max_HP)
                return false;
            else
                player_add_health(30);
                return true;
    }

    return false;
}

const color_character_t item_get_texture(const item_t item, const int x, const int y) {
    switch (item) {
        case ITEM_HEALTH_POTION:
            return pHealth_potion[y][x];
        case ITEM_APPLE:
            return pApple[y][x];
    }

    return (color_character_t){ 0 };
}