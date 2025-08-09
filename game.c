#include "leak.h"
#include "game.h"

#include "Mob.h"
#include "boss_malakh.h"
#include "map.h"
#include "save.h"
#include "astar.h"
#include "input.h"
#include "delta.h"
#include "player.h"
#include "block_control.h"
#include "inventory.h"
#include "date_time.h"
#include "Crafting_UI.h"

bool game_exit = false;

#if _DEBUG
static void render_debug_text(void) {
    const BACKGROUND_color_t background = BACKGROUND_T_BLACK;
    const FOREGROUND_color_t foreground = FOREGROUND_T_WHITE;

    COORD position = {
        .X = 0,
        .Y = console_size.Y - 4
    };

    int fps = -1;
    if (delta_time > 0.0f)
        fps = (int)(1.0f / delta_time);
    console_fprint_string("FPS: %d", position, background, foreground, fps);
    ++position.Y;

    console_fprint_string("Player: (%d, %d)", position, background, foreground, player.x, player.y);
    ++position.Y;

    console_fprint_string("Mouse: (%d, %d)", position, background, foreground, block_control_selected_x, block_control_selected_y);
    ++position.Y;

    console_fprint_string("Boss Spawned: %d", position, background, foreground, boss_spawned);
}

static void test(void) {
    inventory_add_item(109, 1);
    inventory_add_item(102, 1);
    inventory_add_item(107, 1);
    inventory_add_item(106, 1);
    inventory_add_item(103, 1);
    inventory_add_item(105, 1);
    inventory_add_item(108, 1);
}
#endif

static void render(void) {
    map_render();
    player_render();
    block_control_render();

    if (boss_spawned)
        boss_render();
    else
        mob_render();

    inventory_render();
    date_time_render();
    save_render();
    crafting_UI_render();

#if _DEBUG
    render_debug_text();
#endif
}

void game_initialize(void) {
    game_exit = boss_spawned = false;

    date_time_initialize();
    map_create();
    player_initialize();
    mob_initialize();
    block_control_initialize();
    inventory_initialize();
    save_initialize();
    save_free();

#if _DEBUG
    test();
#endif
}

void game_update(void) {
    while (!game_exit) {
        delta_time_update();

        console_update();
        input_update();
        date_time_update();
        player_update();

        if (boss_spawned)
            boss_update();
        else 
            mob_update();

        inventory_input();
        crafting_UI_input();
        save_input();

        render();
    }
}

void game_destroy(void) {
    mob_destroy();
    boss_destroy();
    astar_destroy();
    inventory_destroy();
    block_control_destroy();
    map_destroy();
}