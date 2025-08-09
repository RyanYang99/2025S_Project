#include "leak.h"
#include "save.h"
#include "game.h"
#include "input.h"
#include "ItemDB.h"
#include "main_menu.h"
#include "Crafting_UI.h"
#include "sound.h"

static bool force_old_console(void) {
    if (is_new_console()) {
        printf_s("Attempting to launch in conhost.exe.\n");

        int argc = 0;

        LPWSTR* pArgv = CommandLineToArgvW(GetCommandLine(), &argc);

        STARTUPINFO startup_info = {
            .cb = sizeof(startup_info)
        };
        PROCESS_INFORMATION process_information = { 0 };

        const int path_character = (int)wcslen(pArgv[0]) + 4, path_size = sizeof(WCHAR) * path_character;
        LPWSTR pArgument = calloc(path_size, sizeof(WCHAR));
        wcscat_s(pArgument, path_size, L"-- ");
        wcscat_s(pArgument, path_size, pArgv[0]);

        const BOOL success = CreateProcess(TEXT("C:\\Windows\\System32\\conhost.exe"),

            pArgument,
            NULL,
            NULL,
            false,
            0,
            NULL,
            NULL,
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

    call_database(false);
    initialize_crafting_UI();
    initialize_console(true, false);
    Sound_init();
    Sound_playMenuBGM("BGM/song18_1.wav");


    while (true) {
        const main_menu_state_t main_menu_state = main_menu();
        if (main_menu_state == MAIN_MENU_STATE_QUIT)
            break;
        else if (main_menu_state == MAIN_MENU_STATE_LOAD_GAME) {
            if (!load_menu())
                continue;
        }
        else
            free_save();

        Sound_playBGM("BGM/fixed_roop1.wav");
        initialize_input_handler();
        initialize_game();
        run_game();
        destroy_game();
        destroy_input_handler();
    }

    destroy_database();
    Sound_shutdown();
    destroy_console();
    return EXIT_SUCCESS;
}