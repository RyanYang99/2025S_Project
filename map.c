﻿#include "leak.h"
#include "map.h"

#if _DEBUG
#include <stdio.h>
#endif

#include <time.h>
#include <math.h>
#include <stdbool.h>
#include "perlin.h"
#include "player.h"

typedef enum
{
    BIOME_PLAINS = 0,
    BIOME_SNOWY_MOUNTAINS = 1
}
biome_t;

map_t map = { 0 };

int total_offsets = 0;
int offset_callback_count = 0;
offset_changed_t* pOffset_callbacks = NULL;

const color_tchar_t pBlock_textures[BLOCKS][TEXTURE_SIZE][TEXTURE_SIZE] =
{
    {
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_BLUE }
        }
    },
    {
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }
        }
    },
    {
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ', FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }
        }
    },
    {
        {
            { '#' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { '#' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { '#' , BACKGROUND_T_GRAY, BACKGROUND_T_BLACK }
        },
        {
            { '#' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { '#' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { '#' , BACKGROUND_T_GRAY, BACKGROUND_T_BLACK }
        },
        {
            { '#' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { '#' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { '#' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }
        }
    },
    {
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { ' ' , BACKGROUND_T_GRAY, BACKGROUND_T_BLACK }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { ' ' , BACKGROUND_T_GRAY, BACKGROUND_T_BLACK }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_GRAY }
        }
    },
    {
        {
            { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }, { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }, { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }
        },
        {
            { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }, { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }, { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }
        },
        {
            { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }, { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }, { '.' , FOREGROUND_T_DARKGRAY, BACKGROUND_T_GRAY }
        }
    },
    {
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKRED }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKRED }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKRED }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_DARKYELLOW }
        }
    },
    {
        {
            { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }
        },
        {
            { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }
        },
        {
            { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }, { '.' , FOREGROUND_T_BLACK, BACKGROUND_T_GREEN }
        }
    },
    {
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }
        },
        {
            { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }, { ' ' , FOREGROUND_T_BLACK, BACKGROUND_T_WHITE }
        }
    }
};

// 블록 기본 체력 설정
static int get_block_max_health(block_t type)
{
    switch (type) //주먹=3데미지? 나무 곡=10 돌곡=17 철곡=30
    {
    case BLOCK_AIR:         return 0;
    case BLOCK_DIRT:        return 5;
    case BLOCK_STONE:       return 20;
    case BLOCK_IRON_ORE:    return 30;
    case BLOCK_GRASS:       return 3;
    case BLOCK_LOG:         return 9;
    case BLOCK_LEAF:        return 3;
    case BLOCK_SNOW:        return 3;
    case BLOCK_BEDROCK:     return -1;
    default:                return 1;
    }
}


block_info_t get_block_info_at(int x, int y)
{
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y)
    {
        block_info_t air = { BLOCK_AIR, 0 };
        return air; // 범위 밖이면 공기 반환
    }
    return map.ppBlocks[y][x];
}

void set_block_at(int x, int y, block_t type)
{
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y)
        return;

    initialize_block(&map.ppBlocks[y][x], type); // 타입 설정 및 체력 초기화
}


bool damage_block_at(map_t* map, int x, int y, int damage)
{
    if (x < 0 || x >= map->size.x || y < 0 || y >= map->size.y)
        return false;

    if (map->ppBlocks[y][x].type == BLOCK_AIR)
        return false;

    map->ppBlocks[y][x].hp -= damage;

    if (map->ppBlocks[y][x].hp <= 0)
    {
        initialize_block(&map->ppBlocks[y][x], BLOCK_AIR);
        return true;
    }

    return false;
}




static void allocate_map(void)
{
    if (!map.ppBlocks)
    {
        const int y_size = sizeof(block_info_t*) * map.size.y;
        map.ppBlocks = malloc(y_size);
        memset(map.ppBlocks, 0, y_size);
    }

    const int x_size = sizeof(block_info_t) * map.size.x;
    for (int y = 0; y < map.size.y; ++y)
        if (!map.ppBlocks[y])
            map.ppBlocks[y] = malloc(x_size);
        else
            map.ppBlocks[y] = realloc(map.ppBlocks[y], x_size);
}

// 블록 초기화 함수
static void initialize_block(block_info_t* block, block_t type)
{
    block->type = type;
    block->hp = get_block_max_health(type);
}

static int find_top(const int x)
{
    if (x < 0 || x >= map.size.x)
        return -1;

    for (int y = 0; y < map.size.y; ++y)
        if (map.ppBlocks[y][x].type != BLOCK_AIR)
            return y;

    return -1;
}

static void generate_strip(const int x, const biome_t biome, const bool override_height, const int height_to_use)
{
    for (int y = 0; y < map.size.y; ++y)
        initialize_block(&map.ppBlocks[y][x], BLOCK_AIR);

    int height = height_to_use;
    if (!override_height)
    {
        float f1 = 0.003f, f2 = (f1 + 0.0005f), a = 0.5f;
        if (biome == BIOME_SNOWY_MOUNTAINS)
        {
            f1 = 0.006f;
            a = 0.7f;
        }

        const float px = (float)x - total_offsets,
            noise = perlin_noise(px * f1) +
            perlin_noise(px * f2) * a;
        height = (int)(map.size.y * ((noise + 1.0f) / 2.0f));
    }

    const int dirt_height = 7, snow_height = 3;

    initialize_block(&map.ppBlocks[map.size.y - 1][x], BLOCK_BEDROCK);

    if (height == map.size.y)
        return;

    for (int y = map.size.y - 2; y > height + dirt_height; --y)
        initialize_block(&map.ppBlocks[y][x], BLOCK_STONE);

    for (int y = height + dirt_height; y > height; --y)
        initialize_block(&map.ppBlocks[y][x], BLOCK_DIRT);

    if (biome == BIOME_PLAINS)
        initialize_block(&map.ppBlocks[height][x], BLOCK_GRASS);
    else if (biome == BIOME_SNOWY_MOUNTAINS)
        for (int y = height + snow_height; y >= height; --y)
            initialize_block(&map.ppBlocks[y][x], BLOCK_SNOW);

    //철광석을 20% 확률로 생성
    if (rand() % 100 >= 80)
    {
        //철광석을 120~179 사이에 생성
        const int iron_y = (rand() % 60) + 120;

        if (map.ppBlocks[iron_y][x].type == BLOCK_STONE)
            initialize_block(&map.ppBlocks[iron_y][x], BLOCK_IRON_ORE);
    }
}

//map.ppBlocks가 map.size에 맞게 조정되었음을 가정함
static biome_t generate_map(const int old_width, const bool right)
{
    const int start_x = right ? old_width : 0, difference = map.size.x - old_width;
    int end_x = map.size.x;
    if (!right)
        end_x -= old_width;

    const int biome_random = rand() % 100 + 1;
    biome_t biome = BIOME_PLAINS;
    //25%
    if (biome_random > 50)
        biome = BIOME_SNOWY_MOUNTAINS;

    for (int x = start_x; x < end_x; ++x)
        generate_strip(x, biome, false, 0);

    const int blend_width = 10,
        target_1 = find_top(right ? start_x - 1 : difference),
        target_2 = find_top(right ? start_x + blend_width : end_x - blend_width);
    if (target_1 != -1 && target_2 != -1)
        for (int x = right ? start_x : end_x - blend_width, i = 0; x <= (right ? start_x + blend_width : end_x); ++x, ++i)
            generate_strip(x,
                biome,
                true,
                (int)round(lerp((float)(right ? target_1 : target_2), (float)(right ? target_2 : target_1), (float)i / blend_width)));

    return biome;
}


static void update_offset(const int offset)
{
    map.offset_x = offset;
    total_offsets += offset;

    for (int i = 0; i < offset_callback_count; ++i)
        if (pOffset_callbacks[i] != NULL)
            pOffset_callbacks[i]();
}

static const int find_grass(const int x)
{
    for (int y = 0; y < map.size.y; ++y)
        if (map.ppBlocks[y][x].type == BLOCK_GRASS)
            return y;

    return -1;
}

static void place_leaves(const int x, const int width_to_sides, const int lower, const int upper, const bool right)
{
    for (int tx = x + (right ? 1 : -1); right ? (tx <= x + width_to_sides) : (tx >= x - width_to_sides); right ? ++tx : --tx)
        for (int ty = upper; ty <= lower; ++ty)
            if (map.ppBlocks[ty][tx].type == BLOCK_AIR)
                initialize_block(&map.ppBlocks[ty][tx], BLOCK_LEAF);
}

static void place_tree(const int x, const int y, const int width_to_sides, const int height)
{
    for (int ty = y; ty >= y - height; --ty)
        initialize_block(&map.ppBlocks[ty][x], BLOCK_LOG);

    const int leaves_lower = y - (height / 2), leaves_upper = y - height;
    place_leaves(x, width_to_sides, leaves_lower, leaves_upper, true);
    place_leaves(x, width_to_sides, leaves_lower, leaves_upper, false);

    const int half = (int)round(width_to_sides / 2), top = leaves_upper - 1;
    for (int tx = x - half; tx <= x + half; ++tx)
    {
        if (map.ppBlocks[top][tx].type == BLOCK_AIR)
            initialize_block(&map.ppBlocks[top][tx], BLOCK_LEAF);
    }
}

static void generate_trees(const int start, const int end)
{
    const int minimum_tree_width_side = 2,
        variable_tree_width_side = 2,
        maximum_tree_width_side = minimum_tree_width_side + variable_tree_width_side,
        minimum_tree_height = 3,
        variable_tree_height = 3,
        maximum_tree_height = minimum_tree_height + variable_tree_height;

    for (int x = start + maximum_tree_width_side; x <= end - maximum_tree_width_side; ++x)
    {
        if (rand() % 100 < 90)
            continue;

        const int grass_y = find_grass(x);
        if (grass_y - maximum_tree_height - 1 < 0) //-1 = 잎 계산
            continue;

        const int tree_width_side = rand() % (variable_tree_width_side + 1) + minimum_tree_width_side,
            tree_height = rand() % (variable_tree_height + 1) + minimum_tree_height;

        place_tree(x, grass_y - 1, tree_width_side, tree_height);
        x += tree_width_side + 1;
    }
}

//width가 음수일 경우 맵을 왼쪽으로 늘림, 양수일 경우 오른쪽
static void resize_map(const bool right)
{
    const int chunk = 100, old = map.size.x;
    map.size.x += chunk;

    allocate_map();

    if (!right)
    {
        update_offset(chunk);

        for (int y = 0; y < map.size.y; ++y)
            memmove(&map.ppBlocks[y][chunk], &map.ppBlocks[y][0], sizeof(block_info_t) * old);
    }

    const biome_t biome = generate_map(old, right);

    if (biome == BIOME_PLAINS)
        generate_trees(right ? old : 0, right ? map.size.x - 1 : chunk);
}

void create_map(void)
{
    fill_table((int)time(NULL));

    map.size.y = MAP_MAX_Y;
    player.y = map.size.y / 2;

    const int x_size = 10;
    resize_map(x_size);
    player.x = x_size / 2;
}

void destroy_map(void)
{
    for (int y = 0; y < map.size.y; ++y)
        free(map.ppBlocks[y]);

    free(map.ppBlocks);
    map.ppBlocks = NULL;

    free(pOffset_callbacks);
    pOffset_callbacks = NULL;
    offset_callback_count = 0;
}


static COORD render_block(const POINT map_position, const COORD console_position, const bool ltr, const bool utd)
{

    bool exit = false;
    COORD size = { 0 };

    if (map_position.x < 0 || map_position.x >= map.size.x ||
        map_position.y < 0 || map_position.y >= map.size.y)
        return size; // 또는 적절한 예외 처리


    for (int ty = 0; ty < TEXTURE_SIZE; ++ty)
    {
        for (int tx = 0; tx < TEXTURE_SIZE; ++tx)
        {
            const COORD position = { console_position.X + (SHORT)(ltr ? tx : -tx), console_position.Y + (SHORT)(utd ? ty : -ty) };

            exit = (position.X < 0 || position.X >= console.size.X) && (position.Y < 0 || position.Y >= console.size.Y);
            if (position.X < 0 || position.X >= console.size.X || position.Y < 0 || position.Y >= console.size.Y)
                break;

            size.X = (SHORT)tx + 1;
            size.Y = (SHORT)ty + 1;

            print_color_tchar(pBlock_textures[map.ppBlocks[map_position.y][map_position.x].type]
                [utd ? ty : (TEXTURE_SIZE - ty - 1)][ltr ? tx : (TEXTURE_SIZE - tx - 1)],
                position);
        }

        if (exit)
            break;
    }

    return size;
}

static COORD render_aft_or_forward(const COORD console_position_half, COORD console_position, const bool forward)
{
    COORD texture_size = { 0 };

    for (int x = forward ? player.x : (player.x - 1); forward ? (x < map.size.x) : x >= 0; forward ? ++x : --x)
    {
        for (int y = player.y; y < map.size.y; ++y)
        {
            texture_size = render_block((POINT) { x, y }, console_position, forward, true);
            console_position.Y += texture_size.Y;

            if (console_position.Y >= console.size.Y)
                break;
        }

        console_position.Y = console_position_half.Y - 1;
        for (int y = player.y - 1; y >= 0; --y)
        {
            texture_size = render_block((POINT) { x, y }, console_position, forward, false);
            console_position.Y -= texture_size.Y;

            if (console_position.Y < 0)
                break;
        }

        console_position.X += forward ? texture_size.X : -texture_size.X;
        console_position.Y = console_position_half.Y;
        if ((forward && console_position.X >= console.size.X) || (!forward && console_position.X < 0))
            break;
    }

    return console_position;
}

void render_map(void)
{
    const COORD console_position_half = { console.size.X / 2, console.size.Y / 2 };

    COORD console_position = render_aft_or_forward(console_position_half, console_position_half, true);
    if (console_position.X != console.size.X)
        resize_map(true);

    console_position.X = console_position_half.X - 1;
    console_position = render_aft_or_forward(console_position_half, console_position, false);
    if (console_position.X != -1)
        resize_map(false);
}

void subscribe_offset_change(const offset_changed_t callback)
{
    if (!pOffset_callbacks)
        pOffset_callbacks = malloc(sizeof(offset_changed_t));
    else
        pOffset_callbacks = realloc(pOffset_callbacks, sizeof(offset_changed_t) * (offset_callback_count + 1));

    pOffset_callbacks[offset_callback_count++] = callback;
}

void unsubscribe_offset_change(const offset_changed_t callback)
{
    if (!pOffset_callbacks)
        return;

    for (int i = 0; i < offset_callback_count; ++i)
        if (pOffset_callbacks[i] == callback)
        {
            pOffset_callbacks[i] = NULL;
            --offset_callback_count;
        }
}

#if _DEBUG
void debug_render_map(const bool pause)
{
    clear();

    for (int y = 0; y < map.size.y; ++y)
    {
        for (int x = 0; x < map.size.x; ++x)
        {
            block_t block = map.ppBlocks[y][x].type;
            char character = ' ';

            switch (block)
            {
            case BLOCK_AIR:
                character = 'A';
                break;

            case BLOCK_GRASS:
                character = 'G';
                break;

            case BLOCK_DIRT:
                character = 'D';
                break;

            case BLOCK_STONE:
                character = 'S';
                break;

            case BLOCK_BEDROCK:
                character = 'B';
                break;

            case BLOCK_IRON_ORE:
                character = 'I';
                break;
            }

            putchar(character);
        }

        putchar('\n');
    }

    if (pause)
        system("pause");
}
#endif