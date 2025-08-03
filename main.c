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

#if _DEBUG
static void render_debug_text(void)
{
    const BACKGROUND_color_t background = BACKGROUND_T_BLACK;
    const FOREGROUND_color_t foreground = FOREGROUND_T_WHITE;

    COORD position = { 0, console.size.Y - 3 };

    int fps = -1;
    if (delta_time > 0.0f)
        fps = (int)(1.0f / delta_time);
    fprint_string("FPS: %d", position, background, foreground, fps);
    ++position.Y;

    fprint_string("Player: (%d, %d)", position, background, foreground, player.x, player.y);
    ++position.Y;

    fprint_string("Mouse: (%d, %d)", position, background, foreground, selected_block_x, selected_block_y);
}
#endif

static void render(void)
{
    render_map();
    render_player();
    render_virtual_cursor();
    Mob_render();
    render_inventory();
    render_hotbar();

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

    call_database(false);
    initialize_console(true, false);
    initialize_input_handler();

    register_mob_click_handler();

    create_map();
    player_init(map.size.x / 2);
    initialize_block_control();
    initialize_inventory();

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
        update_input();

        player_update();
        inventory_input();
        Mob_Spawn_Time();
        update_mob_ai();

        render();
    }

    destroy_block_control();
    destroy_database();
    destroy_map();
    destroy_input_handler();
    destroy_console();
    return EXIT_SUCCESS;
}