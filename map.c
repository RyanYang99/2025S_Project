#include "leak.h"
#include "map.h"

#if _DEBUG
#include <stdio.h>
#endif

#include <time.h>
#include <stdbool.h>
#include "perlin.h"
#include "player.h"

map_t map = { 0 };

int total_offsets = 0;
int offset_callback_count = 0;
offset_changed_t *pOffset_callbacks = NULL;

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
    }
};

static void allocate_map(void)
{
    const int y_size = sizeof(block_t *) * map.size.y;

    if (!map.ppBlocks)
    {
        map.ppBlocks = malloc(y_size);
        memset(map.ppBlocks, 0, y_size);
    }
    else
        map.ppBlocks = realloc(map.ppBlocks, y_size);

    const int x_size = sizeof(block_t) * map.size.x;
    for (int y = 0; y < map.size.y; ++y)
        if (!map.ppBlocks[y])
            map.ppBlocks[y] = malloc(x_size);
        else
            map.ppBlocks[y] = realloc(map.ppBlocks[y], x_size);
}

//map.ppBlocks가 map.size에 맞게 조정되었음을 가정함
static void generate_map(const int old_width, const bool right)
{
    const int start_x = right ? old_width : 0;
    int end_x = map.size.x;
    if (!right)
        end_x -= old_width;

    for (int y = 0; y < map.size.y; ++y)
        for (int x = start_x; x < end_x; ++x)
            map.ppBlocks[y][x] = BLOCK_AIR;

    for (int x = start_x; x < end_x; ++x)
    {
        const float noise = perlin_noise(((float)x - total_offsets) * 0.01f);
        const int height = (int)(map.size.y * ((noise + 1.0f) / 2.0f));

        map.ppBlocks[map.size.y - 1][x] = BLOCK_BEDROCK;

        for (int y = map.size.y - 2; y > height + 7; --y)
            map.ppBlocks[y][x] = BLOCK_STONE;

        for (int y = height + 7; y > height; --y)
            map.ppBlocks[y][x] = BLOCK_DIRT;

        map.ppBlocks[height][x] = BLOCK_GRASS;
    }

    for (int x = start_x; x < end_x; ++x)
        //철광석을 20% 확률로 생성
        if (rand() % 100 >= 80)
        {
            //철광석을 120~129 사이에 생성
            const int iron_y = (rand() % 10) + 120;

            if (map.ppBlocks[iron_y][x] == BLOCK_STONE)
                map.ppBlocks[iron_y][x] = BLOCK_IRON_ORE;
        }
}

static void update_offset(const int offset)
{
    map.offset_x = offset;
    total_offsets += offset;

    for (int i = 0; i < offset_callback_count; ++i)
        if (pOffset_callbacks[i] != NULL)
            pOffset_callbacks[i]();
}

//width가 음수일 경우 맵을 왼쪽으로 늘림, 양수일 경우 오른쪽
static void resize_map(const int width)
{
    if (!width)
        return;

    const int old = map.size.x, absolute_width = abs(width);
    map.size.x += absolute_width;

    allocate_map();

    const bool is_negative = width < 0;
    if (is_negative)
    {
        update_offset(absolute_width);

        for (int y = 0; y < map.size.y; ++y)
            memmove(&map.ppBlocks[y][absolute_width], &map.ppBlocks[y][0], sizeof(block_t) * old);
    }

    generate_map(old, !is_negative);
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

    for (int ty = 0; ty < TEXTURE_SIZE; ++ty)
    {
        for (int tx = 0; tx < TEXTURE_SIZE; ++tx)
        {
            const COORD position = { console_position.X + (SHORT)(ltr ? tx : -tx), console_position.Y + (SHORT)(utd ? ty: -ty) };

            exit = (position.X < 0 || position.X >= console.size.X) && (position.Y < 0 || position.Y >= console.size.Y);
            if (position.X < 0 || position.X >= console.size.X || position.Y < 0 || position.Y >= console.size.Y)
                break;

            size.X = (SHORT)tx + 1;
            size.Y = (SHORT)ty + 1;

            print_color_tchar(pBlock_textures[map.ppBlocks[map_position.y][map_position.x]]
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
        resize_map(1);

    console_position.X = console_position_half.X - 1;
    console_position = render_aft_or_forward(console_position_half, console_position, false);
    if (console_position.X != -1)
        resize_map(-1);
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
            block_t block = map.ppBlocks[y][x];
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