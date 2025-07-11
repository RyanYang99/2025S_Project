#pragma once

#include "console.h"

#define BLOCKS 8
#define TEXTURE_SIZE 3
#define MAP_MAX_Y 200

typedef void (*offset_changed_t)(void);

typedef enum
{
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_BEDROCK, //뚫을 수 없는 블록
    BLOCK_STONE,
    BLOCK_IRON_ORE,
    BLOCK_LOG,
    BLOCK_LEAF
}
block_t;

typedef struct
{
    int offset_x;
    POINT size;
    block_t **ppBlocks;
}
map_t;

extern map_t map;
extern const color_tchar_t pBlock_textures[BLOCKS][TEXTURE_SIZE][TEXTURE_SIZE];

void create_map(void);
void destroy_map(void);
void render_map(void);

void subscribe_offset_change(const offset_changed_t callback);
void unsubscribe_offset_change(const offset_changed_t callback);

#if _DEBUG
void debug_render_map(const bool pause);
#endif