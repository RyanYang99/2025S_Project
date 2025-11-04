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
    const BG background{ BG::black };
    const FG foreground{ FG::white };

    const COORD &size{ console::size() };
    COORD position = { 0, static_cast<SHORT>(size.Y - 4) };

    int fps{ -1 };
    if (delta_time_t::delta_time > 0.0f)
        fps = static_cast<int>(1.0f / delta_time_t::delta_time);
    console::print(std::format("FPS: {}", fps), position, background, foreground);
    ++position.Y;

    console::print(std::format("Player: ({}, {})", player.x, player.y), position, background, foreground);
    ++position.Y;

    console::print(std::format("Mouse: ({}, {})", block_control_selected_x, block_control_selected_y), position, background, foreground);
    ++position.Y;

    console::print(std::format("Boss Spawned: {}", boss_spawned), position, background, foreground);
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
        if (current_BGM_state != BGM::boss) {
            sound_play_BGM("boss");
            current_BGM_state = BGM::boss;
        }
        return;
    }
    else if (current_BGM_state == BGM::boss)
        current_BGM_state = BGM::none;

    if (elapsed_since_start_.is_night()) { //밤일때
        if (current_BGM_state != BGM::night) {
            sound_play_BGM("night");
            current_BGM_state = BGM::night;
        }
    }
    else {
        if (current_BGM_state != BGM::day) { //낮일때
            sound_play_BGM("day");
            current_BGM_state = BGM::day;
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

        console::update();
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
    inventory_destroy();
    block_control_destroy();
    player_destroy();
    map_destroy();
}