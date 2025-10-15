#include "leak.hpp"
#include "game.hpp"

#include "mob.hpp"
#include "map.hpp"
#include "save.hpp"
#include "delta_time.hpp"
#include "astar.hpp"
#include "sound.hpp"
#include "input.hpp"
#include "player.hpp"
#include "inventory.hpp"
#include "date_time.hpp"
#include "crafting_UI.hpp"
#include "boss_malakh.hpp"
#include "block_control.hpp"
#include "ambient_BGM_state.hpp"

#if _DEBUG
#include <format>

#include "console.hpp"
#endif

game *game::instance_{};

#if _DEBUG
static void render_debug_text(void) {
    const BACKGROUND_color_t background{ BACKGROUND_T_BLACK };
    const FOREGROUND_color_t foreground{ FOREGROUND_T_WHITE };

    COORD position = { 0, console_size.Y - 4 };

    int fps{ -1 };
    if (delta_time_t::delta_time > 0.0f)
        fps = static_cast<int>(1.0f / delta_time_t::delta_time);
    console_fprint_string(std::format("FPS: {}", fps).c_str(), position, background, foreground);
    ++position.Y;

    console_fprint_string(std::format("Player: ({}, {})", player.x, player.y).c_str(), position, background, foreground);
    ++position.Y;

    console_fprint_string(std::format("Mouse: ({}, {})", block_control_selected_x, block_control_selected_y).c_str(), position, background, foreground);
    ++position.Y;

    console_fprint_string(std::format("Boss Spawned: {}", boss_spawned).c_str(), position, background, foreground);
}
#endif

void game::render(void) {
    map_render();
    if (boss_spawned)
        boss_render();
    else
        mob_render();
    player_render();

    block_control_render();
    inventory_render();
    elapsed_since_start_.render();
    save_render();
    crafting_UI_render();

#if _DEBUG
    render_debug_text();
#endif
}

void game::update_BGM(void) {
    if (boss_spawned) {
        if (current_BGM_state != ambient_BGM_state_t::boss) {
            sound_play_BGM("boss");
            current_BGM_state = ambient_BGM_state_t::boss;
        }
        return;
    }
    else if (current_BGM_state == ambient_BGM_state_t::boss)
        current_BGM_state = ambient_BGM_state_t::none;

    if (elapsed_since_start_.is_night()) { //밤일때
        if (current_BGM_state != ambient_BGM_state_t::night) {
            sound_play_BGM("night");
            current_BGM_state = ambient_BGM_state_t::night;
        }
    }
    else {
        if (current_BGM_state != ambient_BGM_state_t::day) { //낮일때
            sound_play_BGM("day");
            current_BGM_state = ambient_BGM_state_t::day;
        }
    }
}

game *game::instance(void) noexcept {
    return instance_;
}

game::game(void) {
    instance_ = this;

    if (pSave_current)
        elapsed_since_start_ = pSave_current->game_time;

    map_create();
    player_initialize();
    mob_initialize();
    block_control_initialize();
    inventory_initialize();
    save_initialize();
    save_free();
}

const date_time &game::elapsed_since_start(void) const noexcept {
    return elapsed_since_start_;
}

void game::exit(const bool _exit) noexcept {
    exit_ = _exit;
}

void game::update(void) {
    while (!exit_) {
        delta_time_t::update();

        console_update();
        input::update();
        elapsed_since_start_.update();
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

game::~game(void) {
    instance_ = nullptr;

    mob_destroy();
    boss_destroy();
    astar_destroy();
    inventory_destroy();
    block_control_destroy();
    player_destroy();
    map_destroy();
}