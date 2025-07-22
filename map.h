#pragma once

#include "console.h"

#define BLOCKS 9
#define TEXTURE_SIZE 3
#define MAP_MAX_Y 200

typedef void (*offset_changed_t)(void);

typedef enum
{
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_BEDROCK,
    BLOCK_STONE,
    BLOCK_IRON_ORE,
    BLOCK_LOG,
    BLOCK_LEAF,
    BLOCK_SNOW
} block_t;

// ✅ 블록 정보 구조체: 종류 + 체력 포함
typedef struct
{
    block_t type;
    int hp;
} block_info_t;

typedef struct
{
    int offset_x;
    POINT size;

    block_info_t** ppBlocks;  // 2D 배열로 블록 정보
} map_t;

extern map_t map;
extern const color_tchar_t pBlock_textures[BLOCKS][TEXTURE_SIZE][TEXTURE_SIZE];

void create_map(void);
void destroy_map(void);
void render_map(void);

//블록 관련 함수
int get_block_max_health(block_t type);
void initialize_block(block_info_t* block, block_t type);
bool damage_block_at(map_t* map, int x, int y, int damage);
block_info_t get_block_info_at(int x, int y);
void set_block_at(int x, int y, block_t type);



void subscribe_offset_change(const offset_changed_t callback);
void unsubscribe_offset_change(const offset_changed_t callback);

#if _DEBUG
void debug_render_map(const bool pause);
#endif
