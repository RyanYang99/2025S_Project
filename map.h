#pragma once

#include "console.h"

#define BLOCKS 11
#define TEXTURE_SIZE 3
#define MAP_MAX_Y 200

typedef void (*offset_changed_t)(void);

typedef enum
{
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
    BLOCK_STAR = 3
} block_t;

// ✅ 블록 정보 구조체: 종류 + 체력 포함
#pragma pack(push, 1)
typedef struct
{
    block_t type;
    int hp;
} block_info_t;
#pragma pack(pop)

typedef struct
{
    int offset_x;
    POINT size;

    block_info_t** ppBlocks;  // 2D 배열로 블록 정보
} map_t;

extern map_t map;

void create_map(void);
void destroy_map(void);
void render_map(void);

//블록 관련 함수
void initialize_block(block_info_t* block, block_t type);
bool damage_block_at(map_t* map, int x, int y, int damage);
block_info_t get_block_info_at(int x, int y);
bool set_block_at(int x, int y, block_t type);
color_tchar_t get_block_texture(const block_t block, const int x, const int y);
const bool is_air_or_star(const block_t block);

void subscribe_offset_change(const offset_changed_t callback);
void unsubscribe_offset_change(const offset_changed_t callback);

void save_map(void);