#include "leak.h"

#if _DEBUG
#include <stdio.h>
#endif

#include <stdbool.h>
#include "map.h"

POINT player = { CHUNK_X / 2, 8 };
map_t map = { 0 };

const color_tchar_t pBlock_textures[BLOCKS][TEXTURE_SIZE][TEXTURE_SIZE] =
{
    {
        {
            { '.' , FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }
        },
        {
            { ' ' , FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }
        },
        {
            { ' ' , FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }, { ' ', FOREGROUND_T_WHITE, BACKGROUND_T_BLUE }
        }
    },
    {
        {
            { ':' , FOREGROUND_T_GREEN, BACKGROUND_T_BLACK }, { ':', FOREGROUND_T_GREEN, BACKGROUND_T_BLACK }, { ':', FOREGROUND_T_GREEN, BACKGROUND_T_BLACK }
        },
        {
            { '|' , FOREGROUND_T_GREEN, BACKGROUND_T_BLACK }, { '|', FOREGROUND_T_GREEN, BACKGROUND_T_BLACK }, { '|', FOREGROUND_T_GREEN, BACKGROUND_T_BLACK }
        },
        {
            { '=' , FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }, { '=', FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }, { '=', FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }
        }
    },
    {
        {
            { '=' , FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }, { '=', FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }, { '=', FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }
        },
        {
            { '=' , FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }, { '=', FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }, { '=', FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }
        },
        {
            { '=' , FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }, { '=', FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }, { '=', FOREGROUND_T_DARKYELLOW, BACKGROUND_T_BLACK }
        }
    },
    {
        {
            { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }, { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }, { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }
        },
        {
            { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }, { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }, { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }
        },
        {
            { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }, { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }, { 'E' , FOREGROUND_T_GRAY, BACKGROUND_T_BLACK }
        }
    }
};

map_t create_map(void)
{
    map_t new_map = { 0 };
    new_map.size.x = CHUNK_X;
    new_map.size.y = CHUNK_Y;

    new_map.ppBlocks = malloc(sizeof(block_t *) * new_map.size.y);
    for (int y = 0; y < new_map.size.y; ++y)
        new_map.ppBlocks[y] = malloc(sizeof(block_t) * new_map.size.x);

    return new_map;
}

void generate_map(void)
{
    for (int y = 0; y < map.size.y; ++y)
        for (int x = 0; x < map.size.x; ++x)
        {
            block_t block = BLOCK_AIR;
            if (y > 9)
                block = BLOCK_GRASS;
            if (y > 10)
                block = BLOCK_DIRT;

            if (y == 0 || y == map.size.y - 1 || x == 0 || x == map.size.x - 1)
                block = BLOCK_STONE;

            map.ppBlocks[y][x] = block;
        }
}

void destroy_map(void)
{
    for (int y = 0; y < map.size.y; ++y)
        free(map.ppBlocks[y]);

    free(map.ppBlocks);
}

static COORD render_block(const POINT map_position, const COORD console_position, const bool ltr)
{
    bool exit = false;
    COORD size = { 0 };

    for (int ty = 0; ty < TEXTURE_SIZE; ++ty)
    {
        for (int tx = ltr ? 0 : TEXTURE_SIZE; ltr ? (tx < TEXTURE_SIZE) : (tx > 0); ltr ? ++tx : --tx)
        {
            const COORD position = { console_position.X + (SHORT)(ltr ? tx : -(TEXTURE_SIZE - tx)), console_position.Y + (SHORT)ty };

            exit = (position.X < 0 || position.X >= console.size.X) && position.Y >= console.size.Y;
            if (position.X < 0 || position.X >= console.size.X || position.Y >= console.size.Y)
                break;

            size.X = (SHORT)(ltr ? tx + 1 : TEXTURE_SIZE - tx + 1);
            size.Y = (SHORT)ty + 1;

            print_color_tchar(pBlock_textures[map.ppBlocks[map_position.y][map_position.x] - 1][ty][ltr ? tx : (tx - 1)], position);
        }

        if (exit)
            break;
    }

    return size;
}

void render_map(void)
{
    const COORD console_position_half = { console.size.X / 2, console.size.Y / 2 };
    COORD console_position = console_position_half, texture_size = { 0 };

    for (int x = player.x; x < map.size.x; ++x)
    {
        for (int y = player.y; y < map.size.y; ++y)
        {
            texture_size = render_block((POINT){ x, y }, console_position, true);
            console_position.Y += texture_size.Y;

            if (console_position.Y >= console.size.Y)
                break;
        }

        console_position.Y = console_position_half.Y - texture_size.Y;
        for (int y = player.y - 1; y >= 0; --y)
        {
            texture_size = render_block((POINT){ x, y }, console_position, true);
            console_position.Y -= texture_size.Y;
            
            if (console_position.Y < 0)
                break;
        }

        console_position.X += texture_size.X;
        console_position.Y = console_position_half.Y;
        if (console_position.X >= console.size.X)
            break;
    }

    console_position.X = console_position_half.X - 1;

    for (int x = player.x - 1; x >= 0; --x)
    {
        for (int y = player.y; y < map.size.y; ++y)
        {
            texture_size = render_block((POINT) { x, y }, console_position, false);
            console_position.Y += texture_size.Y;

            if (console_position.Y >= console.size.Y)
                break;
        }

        console_position.Y = console_position_half.Y - texture_size.Y;
        for (int y = player.y - 1; y >= 0; --y)
        {
            texture_size = render_block((POINT) { x, y }, console_position, false);
            console_position.Y -= texture_size.Y;

            if (console_position.Y < 0)
                break;
        }

        console_position.X -= texture_size.X;
        console_position.Y = console_position_half.Y;
        if (console_position.X < 0)
            break;
    }
}

#if _DEBUG
void debug_render_map(void)
{
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
            }

            putchar(character);
        }

        putchar('\n');
    }

    system("pause");
}
#endif