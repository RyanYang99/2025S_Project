#include "leak.h"
#include "item.h"

#include "map.h"
#include "player.h"

const color_tchar_t pHealth_potion[TEXTURE_SIZE][TEXTURE_SIZE] = {
    { { ' ', BACKGROUND_T_TRANSPARENT, 0 }, { ' ', BACKGROUND_T_WHITE, 0 },{ ' ', BACKGROUND_T_TRANSPARENT, 0 } },
    { { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 } },
    { { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0} },
}, pApple[TEXTURE_SIZE][TEXTURE_SIZE] = {
    { { ' ', BACKGROUND_T_TRANSPARENT, 0 }, { ' ', BACKGROUND_T_GREEN, 0 },{ ' ', BACKGROUND_T_TRANSPARENT, 0 } },
    { { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 } },
    { { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0 }, { ' ', BACKGROUND_T_RED, 0} },
};

const bool use_item(const item_t item) {
    switch (item) {
        if (player.hp >= player.max_hp)
            return false;

        case ITEM_HEALTH_POTION:

            add_health_to_player(100);
			return true;

        case ITEM_APPLE:

            add_health_to_player(30);
            return true;
    }

	return false;
}

const color_tchar_t get_item_texture(const item_t item, const int x, const int y) {
    switch (item) {
        case ITEM_HEALTH_POTION:
            return pHealth_potion[y][x];
        case ITEM_APPLE:
            return pApple[y][x];
    }

    return (color_tchar_t){ 0 };
}