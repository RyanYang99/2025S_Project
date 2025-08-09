#include "leak.h"
#include "Crafting_UI.h"

#include <stdio.h>
#include <stdbool.h>
#include "map.h"
#include "input.h"
#include "player.h"
#include "inventory.h"

#define MATERIAL_STRING_COUNT 128
#define CRAFTING_MAX_RECIPES 64
#define CRAFTING_SLOTS 20

typedef struct {
    bool requires_workbench;                 //제작대 필요
    int pIngredient_indices[CRAFTING_SLOTS], //아이템DB 인덱스 (재료)
        pIngredient_counts[CRAFTING_SLOTS];  //각 재료 필요 갯수
    int result_index,                        //결과 아이템 DB 인덱스
        result_count;                        //결과 아이템 수량
} crafting_recipe_t;

static bool is_crafting_open = false;
static crafting_recipe_t pRecipes[CRAFTING_MAX_RECIPES] = { 0 };
static int recipe_count = 0, selected_recipe_index = 0;

static void load_recipes_from_csv(const char * const pFile_name) {
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

void initialize_crafting_UI(void) {
    load_recipes_from_csv("recipes.csv");
}

static bool can_craft(const crafting_recipe_t * const pRecipe) {
    for (int i = 0; i < CRAFTING_SLOTS; ++i) {
        if (!pRecipe->pIngredient_indices[i])
            continue;

        int required = pRecipe->pIngredient_counts[i], found = 0;
        for (int j = 0; j < INVENTORY_SIZE; ++j)
            if (inventory.item[j].item_db_index == pRecipe->pIngredient_indices[i])
                found += inventory.item[j].quantity;

        if (found < required)
            return false;
    }
    return true;
}

static void craft_selected_recipe(void) {
    if (selected_recipe_index < 0 || selected_recipe_index >= recipe_count)
        return;

    const crafting_recipe_t *pRecipe = &pRecipes[selected_recipe_index];
    if (!can_craft(pRecipe))
        return;

    for (int i = 0; i < CRAFTING_SLOTS; ++i) {
        if (!pRecipe->pIngredient_indices[i])
            continue;

        int remaining = pRecipe->pIngredient_counts[i];
        for (int j = 0; j < INVENTORY_SIZE && remaining > 0; ++j) {
            player_item_t *pItem = &inventory.item[j];
            if (pItem->item_db_index == pRecipe->pIngredient_indices[i]) {
                const int to_remove = pItem->quantity < remaining ? pItem->quantity : remaining;
                pItem->quantity -= to_remove;
                remaining -= to_remove;
                if (pItem->quantity <= 0)
                    *pItem = (player_item_t){ 0 };
            }
        }
    }

    add_item_to_inventory(pRecipe->result_index, pRecipe->result_count);
}

void crafting_UI_input(void) {
    if (!keyboard_pressed)
        return;

    const char key = (char)tolower(input_character);
    if (is_crafting_open)
        switch (key) {
            case 'w':
                if (selected_recipe_index > 0)
                    --selected_recipe_index;
                break;

            case 's':
                if (selected_recipe_index < recipe_count - 1)
                    ++selected_recipe_index;
                break;

            case 'e':
                craft_selected_recipe();
                break;
        }

    if (key == 'c') {
        if (!is_crafting_open)
            selected_recipe_index = 0;

        is_crafting_open = !is_crafting_open;
    }
}

static bool is_workbench_nearby(void) {
    for (int dy = -5; dy <= 5; ++dy)
        for (int dx = -5; dx <= 5; ++dx)
            if (get_block_info_at(player.x + dx, player.y + dy).type == BLOCK_WORKBENCH)
                return true;

    return false;
}

void render_crafting_UI(void) {
    if (!is_crafting_open)
        return;
    clear();

    COORD position = {
        .X = 1,
        .Y = 1
    };
    fprint_string("[ Craftable Items ]", position, BACKGROUND_T_BLACK, FOREGROUND_T_YELLOW);
    ++position.Y;

    const BACKGROUND_color_t background = BACKGROUND_T_BLACK;
    for (int i = 0; i < recipe_count; ++i) {
        const crafting_recipe_t * const pRecipe = &pRecipes[i];
        if (pRecipe->requires_workbench && !is_workbench_nearby())
            continue;

        const item_information_t * const pItem = find_item_by_index(pRecipe->result_index);
        const bool selected = i == selected_recipe_index;
        const FOREGROUND_color_t foreground = selected ? FOREGROUND_T_WHITE : FOREGROUND_T_GRAY;

        const SHORT length = (SHORT)fprint_string(selected ? "> %s x%d " : "  %s x%d ",
                                                  position,
                                                  background,
                                                  foreground,
                                                  pItem ? pItem->name : "???",
                                                  pRecipe->result_count);

        const bool craftable = can_craft(pRecipe);
        position.X += length;
        fprint_string("[%c]", position, background, craftable ? FOREGROUND_T_GREEN : FOREGROUND_T_RED, craftable ? 'O' : 'X');
        position.X -= length;
        ++position.Y;

        // 선택된 레시피의 재료 표시
        if (!selected)
            continue;

        for (int j = 0; j < CRAFTING_SLOTS; ++j) {
            if (!pRecipe->pIngredient_indices[j])
                break;

            const item_information_t * const pIngredient_item = find_item_by_index(pRecipe->pIngredient_indices[j]);
            // 재료명 + (가지고있는수량/필요한수량) 표시
            fprint_string("    - %s (%d/%d)",
                          position,
                          BACKGROUND_T_BLACK,
                          FOREGROUND_T_CYAN,
                          pIngredient_item ? pIngredient_item->name : "???",
                          get_inventory_count(pRecipe->pIngredient_indices[j]),
                          pRecipe->pIngredient_counts[j]);
            ++position.Y;
        }
    }

    fprint_string("[W / S]: Select [E]: Craft [C]: Close", position, BACKGROUND_T_BLACK, FOREGROUND_T_GREEN);
}