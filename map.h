#pragma once

#include "console.h"

#define BLOCKS 4
#define TEXTURE_SIZE 3
#define MAP_MAX_Y 200

typedef void (*offset_changed_callback_t)(void);

typedef enum
{
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE
} block_t;

typedef struct
{
    int offset_x;
    POINT size;
    block_t** ppBlocks;

    int offset_callback_count;
    offset_changed_callback_t* pOffset_callbacks;
} map_t;

//임시 테스트 용
//extern POINT player;

extern map_t map;
extern const color_tchar_t pBlock_textures[BLOCKS][TEXTURE_SIZE][TEXTURE_SIZE];

void create_map(void);
void destroy_map(void);
void render_map(void);

void subscribe_to_offset_change(const offset_changed_callback_t callback);
void unsubscribe_from_offset_change(const offset_changed_callback_t callback);

#if _DEBUG
void debug_render_map(void);
void resize(const int width);
#endif