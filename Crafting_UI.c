#define _CRT_SECURE_NO_WARNINGS

#include "Crafting_UI.h"
#include <stdio.h>
#include <string.h>
#include "formatter.h"
#include "input.h"
#include "console.h"
#include "map.h"
#include <conio.h>
#include "player.h"
#include "map.h"




crafting_ui_t crafting_ui = { 0 };

static bool is_workbench_nearby(const player_t *pPlayer) {
    for (int dy = -5; dy <= 5; ++dy) {
        for (int dx = -5; dx <= 5; ++dx) {
            const int x = pPlayer->x + dx;
            const int y = pPlayer->y + dy;
            if (get_block_info_at(x, y).type == BLOCK_WORKBENCH)
                return true;
        }
    }
    return false;
}

static void load_recipes_from_csv(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;

    char line[256];

    // 헤더 무시
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (crafting_ui.recipe_count >= CRAFTING_MAX_RECIPES) break;

        crafting_recipe_t* recipe = &crafting_ui.recipes[crafting_ui.recipe_count];
        memset(recipe, 0, sizeof(*recipe));

        int material_count;
        char material_str[128] = { 0 };

        if (sscanf(line, "%d,%*[^,],%d,%d,%127s",
            &recipe->result_index,
            &recipe->result_count,
            &material_count,
            material_str) != 4) {
            continue;
        }

        char* token = strtok(material_str, "@");
        for (int i = 0; i < material_count && token && i < CRAFTING_SLOTS; ++i) {
            sscanf(token, "%d:%d",
                &recipe->ingredient_indices[i],
                &recipe->ingredient_counts[i]);
            token = strtok(NULL, "@");
        }

        crafting_ui.recipe_count++;
    }

    fclose(file);
}


static bool can_craft(const crafting_recipe_t* recipe) {
    for (int i = 0; i < CRAFTING_SLOTS; ++i) {
        if (recipe->ingredient_indices[i] == 0) continue;
        int required = recipe->ingredient_counts[i];
        int found = 0;
        for (int j = 0; j < INVENTORY_SIZE; ++j) {
            if (inventory.item[j].item_db_index == recipe->ingredient_indices[i]) {
                found += inventory.item[j].quantity;
            }
        }
        if (found < required) return false;
    }
    return true;
}

static void craft_selected_recipe(void) {
    if (crafting_ui.selected_recipe_index < 0 || crafting_ui.selected_recipe_index >= crafting_ui.recipe_count)
        return;

    const crafting_recipe_t* recipe = &crafting_ui.recipes[crafting_ui.selected_recipe_index];
    if (!can_craft(recipe)) return;

    for (int i = 0; i < CRAFTING_SLOTS; ++i) {
        if (recipe->ingredient_indices[i] == 0) continue;

        int remaining = recipe->ingredient_counts[i];
        for (int j = 0; j < INVENTORY_SIZE && remaining > 0; ++j) {
            player_item_t* item = &inventory.item[j];
            if (item->item_db_index == recipe->ingredient_indices[i]) {
                int to_remove = (item->quantity < remaining) ? item->quantity : remaining;
                item->quantity -= to_remove;
                remaining -= to_remove;
                if (item->quantity <= 0)
                    *item = (player_item_t){ 0 };
            }
        }
    }

    add_item_to_inventory(recipe->result_index, recipe->result_count);
}

void initialize_crafting_ui(void) {
    crafting_ui.recipe_count = 0;
    crafting_ui.selected_recipe_index = 0;
    crafting_ui.is_open = false;
}

void update_crafting_ui(void) {
    while (crafting_ui.is_open) {
        render_crafting_ui();

        if (_kbhit()) {
            char key = (char)_getch();
            switch (key) {
            case 'w':
                if (crafting_ui.selected_recipe_index > 0)
                    crafting_ui.selected_recipe_index--;
                break;
            case 's':
                if (crafting_ui.selected_recipe_index < crafting_ui.recipe_count - 1)
                    crafting_ui.selected_recipe_index++;
                break;
            case 'e':
                craft_selected_recipe();
                break;
            case 'c':
                crafting_ui.is_open = false;
                break;
            }
        }
    }
}

void render_crafting_ui(void) {
    clear();
    COORD pos = { .X = 2, .Y = 1 };
    fprint_string("[ 제 작  가 능 한  아 이 템  목 록  ]", pos, BACKGROUND_T_BLACK, FOREGROUND_T_YELLOW);
    pos.Y += 1;

    for (int i = 0; i < crafting_ui.recipe_count; ++i) {
        const crafting_recipe_t* recipe = &crafting_ui.recipes[i];
        const item_information_t* item = find_item_by_index(recipe->result_index);
        bool selected = (i == crafting_ui.selected_recipe_index);
        bool craftable = can_craft(recipe);

        BACKGROUND_color_t bg = BACKGROUND_T_BLACK;
        FOREGROUND_color_t fg = selected ? FOREGROUND_T_WHITE : FOREGROUND_T_GRAY;

        fprint_string(selected ? "> %s x%d %s" : "  %s x%d %s", pos, bg, fg, item ? item->name : "???", recipe->result_count, craftable ? "O" : "X");

        pos.Y++;

        // 선택된 레시피의 재료 표시
        if (selected) {
            for (int j = 0; j < CRAFTING_SLOTS; ++j) {
                if (recipe->ingredient_indices[j] == 0) break;

                const item_information_t* ingredient_item = find_item_by_index(recipe->ingredient_indices[j]);
                int have_count = get_inventory_count(recipe->ingredient_indices[j]);  // 인벤토리에서 가지고 있는 수량 조회
                int need_count = recipe->ingredient_counts[j];

                // 재료명 + (가지고있는수량/필요한수량) 표시
                fprint_string("    - %s (%d/%d)",
                    pos,
                    BACKGROUND_T_BLACK,
                    FOREGROUND_T_CYAN,
                    ingredient_item ? ingredient_item->name : "???",
                    have_count,
                    need_count);
                pos.Y++;
            }
        }
    }

    
    fprint_string(" [W/S] 선 택 이 동  [E] 제 작 [C] 닫 기 ", pos, BACKGROUND_T_BLACK, FOREGROUND_T_GREEN);
    update_console();
}

void ShowCraftingUI(player_t *pPlayer) {
    initialize_crafting_ui();
    load_recipes_from_csv("base_recipes.csv");
    if (is_workbench_nearby(pPlayer)) {
        load_recipes_from_csv("workbench_recipes.csv");
    }
    crafting_ui.is_open = true;
    update_crafting_ui();
}

void Crafting_UI_input()
{
    if (keyboard_pressed) {
        if (input_character == 'C' || input_character == 'c') {
            ShowCraftingUI(&player);  // 제작 UI 호출
        }
    }
}