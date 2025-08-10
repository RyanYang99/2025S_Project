#include "leak.h"

#include <stdio.h>
#include "save.h"
#include "game.h"
#include "input.h"
#include "sound.h"
#include "main_menu.h"
#include "crafting_UI.h"
#include "item_database.h"

static bool force_old_console(void) {
    if (console_is_new_windows_terminal()) {
        printf_s("Attempting to launch in conhost.exe.\n");

        int argc = 0;

        LPWSTR *pArgv = CommandLineToArgvW(GetCommandLine(), &argc);

        STARTUPINFO startup_info = {
            .cb = sizeof(startup_info)
        };
        PROCESS_INFORMATION process_information = { 0 };

        const int path_character = (int)wcslen(pArgv[0]) + 4, path_size = sizeof(WCHAR) * path_character;
        LPWSTR pArgument = calloc(path_size, sizeof(WCHAR));
        wcscat_s(pArgument, path_size, L"-- ");
        wcscat_s(pArgument, path_size, pArgv[0]);

        WCHAR pWorking[MAX_PATH] = { 0 };
        GetCurrentDirectory(MAX_PATH, pWorking);

        const BOOL success = CreateProcess(TEXT("C:\\Windows\\System32\\conhost.exe"),
                                           pArgument,
                                           NULL,
                                           NULL,
                                           false,
                                           0,
                                           NULL,
                                           pWorking,
                                           &startup_info,
                                           &process_information);

        LocalFree(pArgv);
        free(pArgument);
        if (success) {
            WaitForSingleObject(process_information.hProcess, INFINITE);
            CloseHandle(process_information.hProcess);
            CloseHandle(process_information.hThread);
            return true;
        }
    }

    return false;
}

int main(void)
{
#if _DEBUG
    //메모리 누수 체크
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    if (force_old_console())
        return 0;

    database_initialize(false);
    crafting_UI_initialize();
    console_initialize(true, false);
    sound_initialize();
    sound_play_menu_BGM("BGM/song18_1.wav");

    while (true) {
        const main_menu_state_t main_menu_state = main_menu();
        if (main_menu_state == MAIN_MENU_STATE_QUIT)
            break;
        else if (main_menu_state == MAIN_MENU_STATE_LOAD_GAME) {
            if (!main_menu_load_menu())
                continue;
        }
        else
            save_free();

        sound_play_BGM("BGM/fixed_roop1.wav");
        input_initialize();
        game_initialize();
        game_update();
        game_destroy();
        input_destroy();
    }

    database_destroy();
    sound_destroy();
    console_destroy();
    return EXIT_SUCCESS;
}