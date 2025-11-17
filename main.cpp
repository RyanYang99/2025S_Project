#include "leak.hpp"

#include <string>
#include <iostream>
#include <Windows.h>

#include "save.hpp"
#include "game.hpp"
#include "input.hpp"
#include "sound.hpp"
#include "console.hpp"
#include "main_menu.hpp"
#include "crafting_UI.hpp"
#include "item_database.hpp"

static bool force_old_console(void) {
    if (Console::is_new_windows_terminal()) {
        std::cout << "Attempting to launch in conhost.exe." << std::endl;

        int argc{};
        LPWSTR *argv{ CommandLineToArgvW(GetCommandLine(), &argc) };
        if (!argv)
            return false;

        STARTUPINFO startup_info {};
        startup_info.cb = sizeof(startup_info);
        PROCESS_INFORMATION process_information{};

        std::wstring argument{ L"-- " };
        argument += argv[0];

        const BOOL success{ CreateProcess(TEXT("C:\\Windows\\System32\\conhost.exe"),
                                          &argument[0],
                                          nullptr,
                                          nullptr,
                                          false,
                                          0,
                                          nullptr,
                                          nullptr,
                                          &startup_info,
                                          &process_information) };

        LocalFree(argv);
        if (success) {
            WaitForSingleObject(process_information.hProcess, INFINITE);
            CloseHandle(process_information.hProcess);
            CloseHandle(process_information.hThread);
            return true;
        }
    }

    return false;
}

int main(void) {
#if _DEBUG
    //메모리 누수 체크
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    if (force_old_console())
        return 0;

    database_initialize(false);
    crafting_UI_initialize();
    Console::initialize();
    sound_initialize();

    while (true) {
        const main_menu_state main_menu_state{ main_menu() };
        if (main_menu_state == main_menu_state::quit)
            break;
        else if (main_menu_state == main_menu_state::load) {
            if (!main_menu_load_menu())
                continue;
        } else
            save_free();

        Input::initialize();

        Game game_{};
        game_.update();

        Input::destroy();
    }

    database_destroy();
    sound_destroy();
    Console::destroy();
    return EXIT_SUCCESS;
}