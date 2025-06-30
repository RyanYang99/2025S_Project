#pragma once

#include "console.h"

#define BLOCKS 4
#define TEXTURE_SIZE 3

#define CHUNK_X 50
#define CHUNK_Y 200

typedef enum
{
    BLOCK_NONE,
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE
} block_t;

typedef struct
{
    bool extend_right, extend_left;
    POINT size;
    block_t **ppBlocks;
} map_t;

//임시
extern POINT player;

extern map_t map;
extern const color_tchar_t pBlock_textures[BLOCKS][TEXTURE_SIZE][TEXTURE_SIZE];

map_t create_map(void);
void generate_map(void);
void destroy_map(void);
void render_map(void);

#if _DEBUG
void debug_render_map(void);
#endif