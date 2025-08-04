#include "leak.h"
#include "main_menu.h"

#include <conio.h>
#include "map.h"
#include "formatter.h"

#define PRINT_SELECTION(string, index, background, foreground) \
    print_center("%s%s%s", bottom + index, background, foreground, selection == index ? selected_left : "", string, selection == index ? selected_right : "")

static void print_center(const char * const string,
                         const int y,
                         const BACKGROUND_color_t background,
                         const FOREGROUND_color_t foreground,
                         ...) {
    COORD position = {
        .X = console.size.X / 2,
        .Y = (SHORT)y
    };
    if (position.X < 0)
        return;

    va_list args = { 0 };
    va_start(args, foreground);

    char* const pFormatted = format_string_v(string, args);
    va_end(args);

    position.X -= (SHORT)strlen(pFormatted) / 2;
    fprint_string(pFormatted, position, background, foreground);
    free(pFormatted);
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
       * const pNew_game = "Create a New World...",
       * const pLoad_game = "Load save",
       * const pQuit = "Quit",
       * const selected_left = "> ",
       * const selected_right = " <";

    clear();

    int selection = 0;
    while (true) {
        char **ppLogo = ppLogoSmall;
        int lines = 1;
        if (console.size.Y >= 4 + 3) {
            ppLogo = ppLogoMedium;
            lines = 4;
        }
        if (console.size.Y >= 11 + 3) {
            ppLogo = ppLogoLarge;
            lines = 11;
        }

        const float margin = 0.25f;
        int top = (int)(console.size.Y * margin), bottom = console.size.Y - top;
        int offset = 3;
        if (top < offset) {
            top = 0;
            offset = 0;
        }
        if (bottom + 2 >= console.size.Y)
            bottom = console.size.Y - 3;

        //TODO: 하늘 추가

        for (int i = 0; i < lines; ++i)
            print_center(ppLogo[i], i + top - offset, BACKGROUND_T_BLACK, FOREGROUND_T_GREEN);

        PRINT_SELECTION(pNew_game, 0, BACKGROUND_T_BLACK, FOREGROUND_T_DARKGREEN);
        PRINT_SELECTION(pLoad_game, 1, BACKGROUND_T_BLACK, FOREGROUND_T_CYAN);
        PRINT_SELECTION(pQuit, 2, BACKGROUND_T_BLACK, FOREGROUND_T_DARKRED);

        update_console();

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

            clear();
        }
    }

    return 0;
}