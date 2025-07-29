#include "leak.h"

#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
#include "map.h"
#include "Mob.h"
#include "input.h"
#include "delta.h"
#include "player.h"
#include "console.h"
#include "BlockCtrl.h"
#include "global_state.h"
#include "ItemDB.h"
#include "inventory.h"

Inventory* g_inv;
ItemDB* g_db;

#if _DEBUG
static void print_debug_text(const char* pText, const int y)
{
    color_tchar_t character =
    {
        .background = BACKGROUND_T_BLACK,
        .foreground = FOREGROUND_T_WHITE,
    };
    COORD position =
    {
        .X = 0,
        .Y = (SHORT)y
    };

    for (int i = 0; i < strlen(pText); ++i)
    {
        character.character = pText[i];
        position.X = (SHORT)i;
        print_color_tchar(character, position);
    }
}

static void render_debug_text(void)
{
    char pMessage[100] = "";

    int fps = -1;
    if (delta_time > 0.0f)
        fps = (int)(1.0f / delta_time);
    sprintf_s(pMessage, 100, "FPS: %d", fps);
    print_debug_text(pMessage, 0);

    sprintf_s(pMessage, 100, "Player: (%d, %d)", player.x, player.y);
    print_debug_text(pMessage, 1);

    sprintf_s(pMessage, 100, "Mouse: (%d, %d)", selected_block_x, selected_block_y);
    print_debug_text(pMessage, 2);
}
#endif

static void render(void)
{
    render_map();
    render_player();
    render_virtual_cursor();
    Mob_render();

#if _DEBUG
    render_debug_text();
#endif
}

int main(void)
{
#if _DEBUG
    //메모리 누수 체크
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    ItemDB db; // 아이템 DB 선언
    Inventory inv;

    g_db = &db;
    g_inv = &inv;

    CallItemDB(g_db);

    initialize_console(true);
    initialize_input_handler();
    create_map();
    player_init(map.size.x / 2);
    BlockControl_Init();
    InitInventory(g_inv);


    clear();
    while (!game_exit)
    {
        update_delta_time();

        MSG msg = { 0 };
        while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        update_console();

        HandleInventoryKeyInput();

        /*player.is_moving = 0;*/
        player_update();
        Mob_Spawn_Time();
        update_mob_ai();


        render();
    }

    BlockControl_Destroy();
    destroy_map();
    destroy_input_handler();
    destroy_console();
    return EXIT_SUCCESS;
}