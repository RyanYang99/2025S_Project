#include "leak.h"
#include "main_menu.h"

#include <time.h>
#include <conio.h>

#include "map.h"
#include "save.h"
#include "sound.h"
#include "date_time.h"
#include "formatter.h"

#define PRINT_SELECTION(string, y, index, background, foreground) \
    console_print_center("%s%s%s", y + index, background, foreground, selection == index ? selected_left : "", string, selection == index ? selected_right : "")

static const char * const selected_left = "> ",
                  * const selected_right = " <";

static void update_time(void) {
    time_t now = time(NULL);
    const struct tm *pTime_info = localtime(&now);

    date_time_elapsed_since_start.hour = pTime_info->tm_hour;
}

const main_menu_state_t main_menu(void) {
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

    console_clear();
    sound_play_BGM("main_menu");

    int selection = 0;
    while (true) {
        char **ppLogo = ppLogoSmall;
        int lines = 1;
        if (console_size.Y >= 4 + 3) {
            ppLogo = ppLogoMedium;
            lines = 4;
        }
        if (console_size.Y >= 11 + 3) {
            ppLogo = ppLogoLarge;
            lines = 11;
        }

        const float margin = 0.25f;
        const int center_lines = 4;
        int top = (int)(console_size.Y * margin), bottom = console_size.Y - top;
        int offset = center_lines;
        if (top < offset) {
            top = 0;
            offset = 0;
        }
        if (bottom + center_lines - 1 >= console_size.Y)
            bottom = console_size.Y - center_lines;

        update_time();
        console_fill(map_get_block_texture(BLOCK_AIR, 0, 0));

        for (int i = 0; i < lines; ++i)
            console_print_center(ppLogo[i], i + top - offset, BACKGROUND_T_BLACK, FOREGROUND_T_GREEN);

        PRINT_SELECTION(pControls, bottom, -1, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);
        PRINT_SELECTION(pNew_game, bottom, 0, BACKGROUND_T_BLACK, FOREGROUND_T_DARKGREEN);
        PRINT_SELECTION(pLoad_game, bottom, 1, BACKGROUND_T_BLACK, FOREGROUND_T_CYAN);
        PRINT_SELECTION(pQuit, bottom, 2, BACKGROUND_T_BLACK, FOREGROUND_T_DARKRED);

        console_update();

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
                        return MAIN_MENU_STATE_NEW_GAME;
                    else if (selection == 1)
                        return MAIN_MENU_STATE_LOAD_GAME;
                    else if (selection == 2)
                        return MAIN_MENU_STATE_QUIT;
                    break;
            }

            console_clear();
        }
    }

    return 0;
}

const bool main_menu_load_menu(void) {
    const bool * const pUsed = get_save_spots();
    const int half = (MAX_SAVE_SPOTS + 2) / 2;

    console_clear();

    int selection = 0;
    while (true) {
        int y = console_size.Y / 2 - half;

        console_print_center("Load Save", y++, BACKGROUND_T_BLACK, FOREGROUND_T_CYAN);
        console_print_center("[Space]: Load, [ESC]: Back", y++, BACKGROUND_T_BLACK, FOREGROUND_T_CYAN);

        for (int i = 0; i < MAX_SAVE_SPOTS; ++i) {
            char * const pString = format_string("Save Slot %d: %s", i, pUsed[i] ? "Used" : "Empty");
            PRINT_SELECTION(pString, y, i, BACKGROUND_T_BLACK, FOREGROUND_T_CYAN);
            free(pString);
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

            console_clear();
        }

        console_update();
    }

    return false;
}