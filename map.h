#pragma once

#include "console.h"

#define BLOCKS 12
#define TEXTURE_SIZE 3
#define MAP_MAX_Y 200

typedef void (* offset_changed_t)(void);

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_GRASS = 101,
    BLOCK_DIRT = 102,
    BLOCK_BEDROCK = 1,
    BLOCK_STONE = 103,
    BLOCK_IRON_ORE = 104,
    BLOCK_LOG = 105,
    BLOCK_LEAF = 106,
    BLOCK_SNOW = 107,
    BLOCK_SAND = 108,
    BLOCK_WATER = 2,
    BLOCK_STAR = 3,
    BLOCK_WORKBENCH = 109,
    BLOCK_SEED_OF_MALAKH = 110
} block_t;

//블록 정보 구조체: 종류 + 체력 포함
#pragma pack(push, 1)
typedef struct {
    block_t type;
    int hp;
} block_info_t;
#pragma pack(pop)

typedef struct {
    int offset_x;
    POINT size;

    block_info_t **ppBlocks;  //2D 배열로 블록 정보
} map_t;

extern map_t map;

void map_render(void);
void map_destroy(void);

void map_create(void);

//블록 관련 함수
void map_initialize_block(block_info_t * const pBlock, const block_t type);
const block_info_t map_get_block_info(const int x, const int y);
const bool map_set_block(const int x, const int y, const block_t type);
const bool map_damage_block(const int x, const int y, const int damage);

const color_character_t map_get_block_texture(const block_t block, const int x, const int y);

const bool map_is_air_or_star(const block_t block);

void map_subscribe_offset_change(const offset_changed_t callback);
void map_unsubscribe_offset_change(const offset_changed_t callback);

void map_save(void);