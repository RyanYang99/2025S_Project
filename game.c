#include "leak.h"
#include "game.h"

#include "Mob.h"
#include "BossMalakh.h"
#include "map.h"
#include "save.h"
#include "astar.h"
#include "input.h"
#include "delta.h"
#include "player.h"
#include "BlockCtrl.h"
#include "inventory.h"
#include "date_time.h"
#include "sound.h"
#include "Crafting_UI.h"

typedef enum {
    AMBIENT_BGM_NONE,  // BGM이 없거나, 보스전 등 다른 BGM이 재생 중인 상태
    AMBIENT_BGM_DAY,   // 낮 BGM 재생 중
    AMBIENT_BGM_NIGHT  // 밤 BGM 재생 중
} ambient_bgm_state_t;



bool game_exit = false;
bool is_boss_spawned;

static ambient_bgm_state_t current_bgm_state = AMBIENT_BGM_DAY;

#if _DEBUG
static void render_debug_text(void) {
    const BACKGROUND_color_t background = BACKGROUND_T_BLACK;
    const FOREGROUND_color_t foreground = FOREGROUND_T_WHITE;

    COORD position = {
        .X = 0,
        .Y = console.size.Y - 5
    };

    int fps = -1;
    if (delta_time > 0.0f)
        fps = (int)(1.0f / delta_time);
    fprint_string("FPS: %d", position, background, foreground, fps);
    ++position.Y;

    fprint_string("Player: (%d, %d)", position, background, foreground, player.x, player.y);
    ++position.Y;

    fprint_string("Mouse: (%d, %d)", position, background, foreground, selected_block_x, selected_block_y);
    ++position.Y;

    fprint_string("Boss Spawned: %s", position, background, foreground, is_boss_spawned ? "True" : "False");
    ++position.Y;


}
#endif

static void update_ambient_bgm() {
    if (is_boss_spawned) {
        current_bgm_state = AMBIENT_BGM_NONE;
        return;
    }

    if (is_night_time()) { // 밤일때
        if (current_bgm_state != AMBIENT_BGM_NIGHT) {
            Sound_playBGM("BGM/Night/NightBGM.wav");
            current_bgm_state = AMBIENT_BGM_NIGHT;
        }
    }
    else { // 낮일때
        if (current_bgm_state != AMBIENT_BGM_DAY) {
            Sound_playBGM("BGM/Day/fixed_roop1.wav");
            current_bgm_state = AMBIENT_BGM_DAY;
        }
    }
}


static void render(void) {
    render_map();
    render_player();
    render_virtual_cursor();
    if (is_boss_spawned) { Boss_Render(); }
    else { Mob_render(); }
    render_inventory();
    render_hotbar();
    render_time();
    render_save_menu();
    render_crafting_UI();

#if _DEBUG
    render_debug_text();
#endif
}

void test_create_Bossitem()
{
    add_item_to_inventory(109, 1);
    add_item_to_inventory(110, 1);
    add_item_to_inventory(203, 1);
    add_item_to_inventory(206, 1);
}

void initialize_game(void) {
    game_exit = false;
    is_boss_spawned = false;
    initialize_date_time();
    create_map();
    player_init();
    mob_init();
    initialize_block_control();
    initialize_inventory();
    initialize_save();
    free_save();

    test_create_Bossitem();

    add_item_to_inventory(110, 1);
}


void run_game(void) {

    while (!game_exit) {
        update_delta_time();

        update_console();
        update_input();
        update_date_time();

        update_ambient_bgm();

        player_update();
        inventory_input();
        crafting_UI_input();
        save_input();

        if (is_boss_spawned) {
            Boss_update();
        }
        else
        {
            mob_spawn_manager();
            mob_update();
        }

        render();
    }
}

void destroy_game(void) {
    destroy_mob();
    destroy_Boss();
    destroy_astar();
    destroy_block_control();
    destroy_map();
}