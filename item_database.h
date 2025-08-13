#pragma once

#include <stdbool.h>

typedef enum {
    ITEM_TYPE_NONE,
    ITEM_TYPE_MATERIAL, //블록
    ITEM_TYPE_TOOL, //도구
    ITEM_TYPE_ARMOR, //갑옷
    ITEM_TYPE_MISC //소모품
} item_type_t;

typedef enum {
    TOOL_NONE,
    TOOL_WOODEN_SWORD = 201,
    TOOL_STONE_SWORD = 202,
    TOOL_IRON_SWORD = 203,
    TOOL_WOODEN_PICKAXE = 204,
    TOOL_STONE_PICKAXE = 205,
    TOOL_IRON_PICKAXE = 206,
    TOOL_WOODEN_AXE = 207,
    TOOL_STONE_AXE = 208,
    TOOL_IRON_AXE = 209,
    TOOL_WOODEN_SHOVEL = 210,
    TOOL_STONE_SHOVEL = 211,
    TOOL_IRON_SHOVEL = 212
} tool_t;

//도구 종류 상수 (tool_kind 값에 대응)
typedef enum {
    TOOL_KIND_NONE,
    TOOL_KIND_SWORD,
    TOOL_KIND_PICKAXE,
    TOOL_KIND_AXE,
    TOOL_KIND_SHOVEL
} tool_kind_t;

typedef enum {
    MATERIAL_TIER_NONE,
    MATERIAL_TIER_WOOD,
    MATERIAL_TIER_STONE,
    MATERIAL_TIRE_IRON
} material_tier_t;

typedef struct {
    int index; //아이템 인덱스
    char name[32]; //아이템 이름
    int max_stack; //아이템 최대갯수(장비는 1로 고정)
    item_type_t type; //아이템 종류
    int base_durability; //아이템 내구도

    tool_kind_t tool_kind;
    material_tier_t material_tier;
    bool is_placeable;
} item_information_t;

typedef struct {
    item_information_t *pItem_information;
    size_t count;
} item_database_t;

extern item_database_t database;

void database_initialize(const bool edit);
void database_destroy(void);

item_information_t *database_find_item_by_index(const int index);