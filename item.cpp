#include "leak.hpp"
#include "item.hpp"

#include "map.hpp"
#include "player.hpp"

static const color_character_t pHealth_potion[TEXTURE_SIZE][TEXTURE_SIZE] = {
    { { ' ', BG::transparent, 0 }, { ' ', BG::white, 0 }, { ' ', BG::transparent, 0 } },
    { { ' ', BG::red, 0 }, { ' ', BG::red, 0 }, { ' ', BG::red, 0 } },
    { { ' ', BG::red, 0 }, { ' ', BG::red, 0 }, { ' ', BG::red, 0 } },
}, pApple[TEXTURE_SIZE][TEXTURE_SIZE] = {
    { { ' ', BG::transparent, 0 }, { ' ', BG::green, 0 }, { ' ', BG::transparent, 0 } },
    { { ' ', BG::red, 0 }, { ' ', BG::red, 0 }, { ' ', BG::red, 0 } },
    { { ' ', BG::red, 0 }, { ' ', BG::red, 0 }, { ' ', BG::red, 0 } },
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

    return {};
}