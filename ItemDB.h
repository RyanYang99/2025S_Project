#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <locale.h> 
#include <stdbool.h>
#include <windows.h>

typedef enum {
    ITEM_TYPE_NONE,
    ITEM_TYPE_MATERIAL, //블록
    ITEM_TYPE_TOOL, //도구
    ITEM_TYPE_ARMOR, //갑옷
    ITEM_TYPE_MISC //소모품
} item_type_t;

typedef enum {
	TOOL_NONE,
	TOOL_WOODEN_PICKAXE = 201
} tool_t;

// 도구 종류 상수 (tool_kind 값에 대응)
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
	MATERIAL_TIER_STONE
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

void initialize_database(void);
void call_database(const bool edit);

item_information_t *find_item_by_index(const int index);

void destroy_database(void);