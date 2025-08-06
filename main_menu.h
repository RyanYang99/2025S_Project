#pragma once

#include <stdbool.h>

typedef enum {
    MAIN_MENU_STATE_NEW_GAME,
    MAIN_MENU_STATE_LOAD_GAME,
    MAIN_MENU_STATE_QUIT
} main_menu_state_t;

const main_menu_state_t main_menu(void);
bool load_menu(void);