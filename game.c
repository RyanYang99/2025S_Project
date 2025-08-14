#include "leak.h"
#include "game.h"

#include "mob.h"
#include "map.h"
#include "save.h"
#include "delta.h"
#include "astar.h"
#include "sound.h"
#include "input.h"
#include "player.h"
#include "inventory.h"
#include "date_time.h"
#include "crafting_UI.h"
#include "boss_malakh.h"
#include "block_control.h"

typedef enum {
    AMBIENT_BGM_NONE,
    AMBIENT_BGM_DAY, //낮 BGM 재생 중
    AMBIENT_BGM_NIGHT, //밤 BGM 재생 중
    AMBIENT_BGM_BOSS
} ambient_BGM_state_t;

bool game_exit = false;

static ambient_BGM_state_t current_BGM_state = AMBIENT_BGM_NONE;

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
#endif

static void render(void) {
    map_render();
    if (boss_spawned)
        boss_render();
    else
        mob_render();
    player_render();

    block_control_render();
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
    current_BGM_state = AMBIENT_BGM_NONE;

    date_time_initialize();
    map_create();
    player_initialize();
    mob_initialize();
    block_control_initialize();
    inventory_initialize();
    save_initialize();
    save_free();
}

static void update_BGM(void) {
    if (boss_spawned) {
        if (current_BGM_state != AMBIENT_BGM_BOSS) {
            sound_play_BGM("boss");
            current_BGM_state = AMBIENT_BGM_BOSS;
        }
        return;
    }
    else if (current_BGM_state == AMBIENT_BGM_BOSS)
        current_BGM_state = AMBIENT_BGM_NONE;

    if (date_time_is_night()) { //밤일때
        if (current_BGM_state != AMBIENT_BGM_NIGHT) {
            sound_play_BGM("night");
            current_BGM_state = AMBIENT_BGM_NIGHT;
        }
    }
    else {
        if (current_BGM_state != AMBIENT_BGM_DAY) { //낮일때
            sound_play_BGM("day");
            current_BGM_state = AMBIENT_BGM_DAY;
        }
    }
}

void game_update(void) {
    while (!game_exit) {
        delta_time_update();

        console_update();
        input_update();
        date_time_update();
        update_BGM();

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
    player_destroy();
    map_destroy();
}