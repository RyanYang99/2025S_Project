#include "leak.h"
#include "game.h"

#include "Mob.h"
#include "map.h"
#include "save.h"
#include "input.h"
#include "delta.h"
#include "player.h"
#include "BlockCtrl.h"
#include "inventory.h"
#include "date_time.h"
#include "global_state.h"

#if _DEBUG
static void render_debug_text(void) {
    const BACKGROUND_color_t background = BACKGROUND_T_BLACK;
    const FOREGROUND_color_t foreground = FOREGROUND_T_WHITE;

    COORD position = {
        .X = 0,
        .Y = console.size.Y - 3
    };

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

static void render(void) {
    render_map();
    render_player();
    render_virtual_cursor();
    Mob_render();
    render_inventory();
    render_hotbar();
    render_time();

#if _DEBUG
    render_debug_text();
#endif
}

void initialize_game(void) {
	pCurrent_save = load_save("test.bin");

    initialize_date_time();
    initialize_input_handler();
    create_map();
    player_init();
    initialize_block_control();
    initialize_inventory();
}

void run_game(void) {
    while (!game_exit) {
        update_delta_time();

        MSG msg = { 0 };
        while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        update_console();
        update_input();

        update_date_time();
        player_update();
        inventory_input();
        Mob_Spawn_Time();
        update_mob_ai();

        render();
    }
}

void destroy_game(void) {
    save_date_time();
    save_player();
    save_inventory();
    save_map();

    destroy_block_control();
    destroy_input_handler();
    destroy_map();

	write_save("test.bin");
}