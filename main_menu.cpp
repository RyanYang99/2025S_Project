#define _CRT_SECURE_NO_WARNINGS

#include "leak.hpp"
#include "main_menu.hpp"

#include <time.h>
#include <conio.h>
#include <string>
#include <format>

#include "map.hpp"
#include "save.hpp"
#include "sound.hpp"
#include "date_time.hpp"

#define PRINT_SELECTION(string, y, index, background, foreground) \
    Console::print_center(std::format("{}{}{}", selection == index ? selected_left : "", string, selection == index ? selected_right : ""), y + index, background, foreground)

static const char * const selected_left = "> ",
                  * const selected_right = " <";

static date_time menu_time{};

main_menu_state main_menu(void) {
    const char *ppLogoSmall[1] = { "TerraCraft" },
               *ppLogoMedium[4] = {
                   "### ### #### ####   #    ## ####   #   ### ###",
                   " #  #   ###  ###   # #  #   ###   # #  ##   # ",
                   " #  #   # #  # #   ###  #   # #   ###  #    # ",
                   " #  ### #  # #  # #   #  ## #  # #   # #    # "
               },
               *ppLogoLarge[11] = {
                   "##### ##### ####  ####    #  ",
                   "  #   #     #   # #   #  # # ",
                   "  #   ##### ####  ####  #####",
                   "  #   #     #  #  #  #  #   #",
                   "  #   ##### #   # #   # #   #",
                   "                             ",
                   " #### ####    #   ##### #####",
                   "#     #   #  # #  #       #  ",
                   "#     ####  ##### #####   #  ",
                   "#     #  #  #   # #       #  ",
                   " #### #   # #   # #       #  "
               },
       * const pControls = "[W / S]: Select, [Space]: Enter",
       * const pNew_game = "Create a New World...",
       * const pLoad_game = "Load save",
       * const pQuit = "Quit";

    Console::clear();
    sound_play_BGM("main_menu");

    int selection = 0;
    while (true) {
        const char **ppLogo = ppLogoSmall;
        int lines = 1;
        if (Console::size().Y >= 4 + 3) {
            ppLogo = ppLogoMedium;
            lines = 4;
        }
        if (Console::size().Y >= 11 + 3) {
            ppLogo = ppLogoLarge;
            lines = 11;
        }

        const float margin = 0.25f;
        const int center_lines = 4;
        int top = (int)(Console::size().Y * margin), bottom = Console::size().Y - top;
        int offset = center_lines;
        if (top < offset) {
            top = 0;
            offset = 0;
        }
        if (bottom + center_lines - 1 >= Console::size().Y)
            bottom = Console::size().Y - center_lines;

        menu_time.set_local_time();
        Console::fill(map_get_block_texture(BLOCK_AIR, 0, 0, menu_time.hour()));

        for (int i = 0; i < lines; ++i)
            Console::print_center(ppLogo[i], i + top - offset, BG::black, FG::green);

        PRINT_SELECTION(pControls, bottom, -1, BG::black, FG::white);
        PRINT_SELECTION(pNew_game, bottom, 0, BG::black, FG::dark_green);
        PRINT_SELECTION(pLoad_game, bottom, 1, BG::black, FG::cyan);
        PRINT_SELECTION(pQuit, bottom, 2, BG::black, FG::dark_red);

        Console::update();

        if (_kbhit()) {
            switch (tolower(_getch())) {
                case 'w':
                    if (selection > 0)
                        --selection;
                    break;

                case 's':
                    if (selection < 2)
                        ++selection;
                    break;

                case ' ':
                    if (selection == 0)
                        return main_menu_state::new_game;
                    else if (selection == 1)
                        return main_menu_state::load;
                    else if (selection == 2)
                        return main_menu_state::quit;
                    break;
            }

            Console::clear();
        }
    }

    return main_menu_state::new_game;
}

bool main_menu_load_menu(void) {
    const bool * const pUsed = get_save_spots();
    const int half = (MAX_SAVE_SPOTS + 2) / 2;

    Console::clear();

    int selection = 0;
    while (true) {
        int y = Console::size().Y / 2 - half;

        Console::print_center("Load Save", y++, BG::black, FG::cyan);
        Console::print_center("[Space]: Load, [ESC]: Back", y++, BG::black, FG::cyan);

        for (int i = 0; i < MAX_SAVE_SPOTS; ++i) {
            const std::string string{ std::format("Save Slot {}: {}", i, pUsed[i] ? "Used" : "Empty") };

            PRINT_SELECTION(string.c_str(), y, i, BG::black, FG::cyan);
        }

        if (_kbhit()) {
            switch (tolower(_getch())) {
                case 'w':
                    if (selection > 0)
                        --selection;
                    break;

                case 's':
                    if (selection < MAX_SAVE_SPOTS - 1)
                        ++selection;
                    break;

                case ' ':
                    if (pUsed[selection]) {
                        load_save_index(selection);
                        return true;
                    }
                    break;

                case VK_ESCAPE:
                    return false;
            }

            Console::clear();
        }

        Console::update();
    }

    return false;
}