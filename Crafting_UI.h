#pragma once

#include <stdbool.h>
#include "ItemDB.h"
#include "inventory.h"
#include "console.h"
#include "player.h"

#define CRAFTING_MAX_RECIPES 32
#define CRAFTING_SLOTS 10

typedef struct {
    int ingredient_indices[CRAFTING_SLOTS]; // 아이템DB 인덱스 (재료)
    int ingredient_counts[CRAFTING_SLOTS];  // 각 재료 필요 갯수
    int result_index;                       // 결과 아이템 DB 인덱스
    int result_count;                       // 결과 아이템 수량
} crafting_recipe_t;

typedef struct {
    crafting_recipe_t recipes[CRAFTING_MAX_RECIPES];
    int recipe_count;

    int selected_recipe_index;

    bool is_open;
} crafting_ui_t;

extern crafting_ui_t crafting_ui;

void initialize_crafting_ui(void);
void update_crafting_ui(void);
void render_crafting_ui(void);

void ShowCraftingUI(player_t* pPlayer);
void Crafting_UI_input();