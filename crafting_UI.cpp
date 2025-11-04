#define _CRT_SECURE_NO_WARNINGS

#include "leak.hpp"
#include "crafting_UI.hpp"

#include "direction.hpp"
#include "map.hpp"
#include "input.hpp"
#include "player.hpp"
#include "inventory.hpp"
#include "item_database.hpp"
#include "formatter.hpp"

#define MATERIAL_STRING_COUNT 128
#define CRAFTING_MAX_RECIPES 64
#define CRAFTING_SLOTS 20

typedef struct {
    bool requires_workbench; //제작대 필요
    int pIngredient_indices[CRAFTING_SLOTS], //아이템DB 인덱스 (재료)
        pIngredient_counts[CRAFTING_SLOTS]; //각 재료 필요 갯수
    int result_index, //결과 아이템 DB 인덱스
        result_count; //결과 아이템 수량
} crafting_recipe_t;

static bool is_crafting_open = false;
static crafting_recipe_t pRecipes[CRAFTING_MAX_RECIPES] = { 0 },
                        *ppOnly_without_workbench[CRAFTING_MAX_RECIPES] = { 0 };
static int recipe_count = 0, without_workbench_count = 0,selected_recipe_index = 0;

static void load_recipes(const char * const pFile_name) {
    FILE *pFile = fopen(pFile_name, "r");
    if (!pFile)
        return;

    char line[256] = { 0 };
    //헤더 무시
    fgets(line, sizeof(line), pFile);

    while (fgets(line, sizeof(line), pFile)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (recipe_count >= CRAFTING_MAX_RECIPES)
            break;

        crafting_recipe_t * const pRecipe = &pRecipes[recipe_count];

        int material_count = 0, requires_workbench = 0;
        char material_str[MATERIAL_STRING_COUNT] = { 0 };

        if (sscanf_s(line,
                     "%d,%*[^,],%d,%d,%d,%127s",
                     &pRecipe->result_index,
                     &pRecipe->result_count,
                     &requires_workbench,
                     &material_count,
                     material_str,
                     MATERIAL_STRING_COUNT) != 5) {
            continue;
        }
        pRecipe->requires_workbench = requires_workbench;
        if (!requires_workbench)
            ppOnly_without_workbench[without_workbench_count++] = pRecipe;

        char *pNext_token = NULL, *pToken = strtok_s(material_str, "@", &pNext_token);
        for (int i = 0; i < material_count && pToken && i < CRAFTING_SLOTS; ++i) {
            sscanf_s(pToken,
                     "%d:%d",
                     &pRecipe->pIngredient_indices[i],
                     &pRecipe->pIngredient_counts[i]);
            pToken = strtok_s(NULL, "@", &pNext_token);
        }

        ++recipe_count;
    }

    fclose(pFile);
}

void crafting_UI_initialize(void) {
    load_recipes("recipes.csv");
}

static const bool can_craft(const crafting_recipe_t * const pRecipe) {
    for (int i = 0; i < CRAFTING_SLOTS; ++i) {
        if (!pRecipe->pIngredient_indices[i])
            continue;

        int required = pRecipe->pIngredient_counts[i], found = 0;
        for (int j = 0; j < INVENTORY_SIZE; ++j)
            if (inventory.item[j].item_DB_index == pRecipe->pIngredient_indices[i])
                found += inventory.item[j].quantity;

        if (found < required)
            return false;
    }
    return true;
}

static const bool is_workbench_nearby(void) {
    for (int dy = -5; dy <= 5; ++dy)
        for (int dx = -5; dx <= 5; ++dx)
            if (map_get_block_info(player.x + dx, player.y + dy).type == BLOCK_WORKBENCH)
                return true;

    return false;
}

static void craft_selected_recipe(void) {
    if (selected_recipe_index < 0 || selected_recipe_index >= recipe_count)
        return;

    const crafting_recipe_t *pRecipe = is_workbench_nearby() ? &pRecipes[selected_recipe_index] : ppOnly_without_workbench[selected_recipe_index];
    if (!can_craft(pRecipe))
        return;

    for (int i = 0; i < CRAFTING_SLOTS; ++i) {
        if (!pRecipe->pIngredient_indices[i])
            continue;

        int remaining = pRecipe->pIngredient_counts[i];
        for (int j = 0; j < INVENTORY_SIZE && remaining > 0; ++j) {
            player_item_t *pItem = &inventory.item[j];
            if (pItem->item_DB_index == pRecipe->pIngredient_indices[i]) {
                const int to_remove = pItem->quantity < remaining ? pItem->quantity : remaining;
                pItem->quantity -= to_remove;
                remaining -= to_remove;
                if (pItem->quantity <= 0)
                    *pItem = {};
            }
        }
    }

    inventory_add_item(pRecipe->result_index, pRecipe->result_count);
}

void crafting_UI_input(void) {
    if (!input::keyboard_pressed())
        return;

    const char lower = (char)tolower(input::input_character()), input_special_character{ input::input_special_character() };
    if (input_special_character == static_cast<char>(direction::up) && selected_recipe_index > 0)
        --selected_recipe_index;
    else if (input_special_character == static_cast<char>(direction::down) && selected_recipe_index < (is_workbench_nearby() ? recipe_count - 1 : without_workbench_count - 1))
        ++selected_recipe_index;
    else if (lower == 'e')
        craft_selected_recipe();
    else if (lower == 'c') {
        if (!is_crafting_open)
            selected_recipe_index = 0;

        is_crafting_open = !is_crafting_open;
    }
}

void crafting_UI_render(void) {
    if (!is_crafting_open)
        return;
    console::clear();

    COORD position = {
        .X = 1,
        .Y = 1
    };
    console::print("[ Craftable Items ]", position, BG::black, FG::yellow);
    ++position.Y;

    const BG background = BG::black;
    const bool workbench = is_workbench_nearby();

    for (int i = 0; i < (workbench ? recipe_count : without_workbench_count); ++i) {
        const crafting_recipe_t * const pRecipe = workbench ? &pRecipes[i] : ppOnly_without_workbench[i];
        if (!pRecipe)
            break;

        const item_information_t * const pItem = database_find_item_by_index(pRecipe->result_index);
        const bool selected = i == selected_recipe_index;
        const FG foreground = selected ? FG::white : FG::gray;

        const SHORT length{ static_cast<SHORT>(console::print(formatter::vformat(selected ? "> {} x{} " : "  {} x{} ",
                                                                                 pItem ? pItem->name : "???",
                                                                                 pRecipe->result_count),
                                                              position, background, foreground)) };
        
        const bool craftable = can_craft(pRecipe);
        position.X += length;
        console::print(std::format("[{}]", craftable ? 'O' : 'X'), position, background, craftable ? FG::green : FG::red);
        position.X -= length;
        ++position.Y;

        //선택된 레시피의 재료 표시
        if (!selected)
            continue;

        for (int j = 0; j < CRAFTING_SLOTS; ++j) {
            if (!pRecipe->pIngredient_indices[j])
                break;

            const item_information_t * const pIngredient_item = database_find_item_by_index(pRecipe->pIngredient_indices[j]);
            //재료명 + (가지고있는수량/필요한수량) 표시
            console::print(std::format("    - {} ({} / {})",
                                       pIngredient_item ? pIngredient_item->name : "???",
                                       inventory_get_count(pRecipe->pIngredient_indices[j]),
                                       pRecipe->pIngredient_counts[j]),
                           position,
                           BG::black,
                           FG::cyan);
            ++position.Y;
        }
    }

    console::print("[Up / Down]: Select [E]: Craft [C]: Close", position, BG::black, FG::green);
}