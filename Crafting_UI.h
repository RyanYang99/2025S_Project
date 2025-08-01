#pragma once

#include <stdbool.h>
#include "ItemDB.h"
#include "inventory.h"
#include "console.h"

#define CRAFTING_MAX_RECIPES 16
#define CRAFTING_SLOTS 3

typedef struct {
    int ingredient_indices[CRAFTING_SLOTS]; // ������DB �ε��� (���)
    int ingredient_counts[CRAFTING_SLOTS];  // �� ��� �ʿ� ����
    int result_index;                       // ��� ������ DB �ε���
    int result_count;                       // ��� ������ ����
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
