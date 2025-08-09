#pragma once

#include <stdbool.h>

extern bool game_exit;
extern bool is_boss_spawned;
extern int selected_item_type;


void initialize_game(void);
void run_game(void);
void destroy_game(void);