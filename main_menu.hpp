#pragma once

enum class main_menu_state {
    new_game,
    load,
    quit
};

main_menu_state main_menu(void);
bool main_menu_load_menu(void);