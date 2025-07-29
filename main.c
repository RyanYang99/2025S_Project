#include "leak.h"

#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
#include "map.h"
#include "Mob.h"
#include "input.h"
#include "delta.h"
#include "player.h"
#include "console.h"
#include "BlockCtrl.h"
#include "global_state.h"

#if _DEBUG
static void print_debug_text(const char *pText, const int y)
{
    color_tchar_t character =
    {
        .background = BACKGROUND_T_BLACK,
        .foreground = FOREGROUND_T_WHITE,
    };
    COORD position =
    {
        .X = 0,
        .Y = (SHORT)y
    };

    for (int i = 0; i < strlen(pText); ++i)
    {
        character.character = pText[i];
        position.X = (SHORT)i;
        print_color_tchar(character, position);
    }
}

static void render_debug_text(void)
{
    char pMessage[100] = "";

    int fps = -1;
    if (delta_time > 0.0f)
        fps = (int)(1.0f / delta_time);
    sprintf_s(pMessage, 100, "FPS: %d", fps);
    print_debug_text(pMessage, 0);

    sprintf_s(pMessage, 100, "Player: (%d, %d)", player.x, player.y);
    print_debug_text(pMessage, 1);

    sprintf_s(pMessage, 100, "Mouse: (%d, %d)", selected_block_x, selected_block_y);
    print_debug_text(pMessage, 2);
}
#endif

static void render(void)
{
    render_map();
    render_player();
    render_virtual_cursor();
    Mob_render();

#if _DEBUG
    render_debug_text();
#endif
}

int main(void)
{
#if _DEBUG
    //메모리 누수 체크
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    if (is_new_console())
    {
        printf_s("Attempting to launch in conhost.exe.\n");

        int argc = 0;
        LPWSTR *pArgv = CommandLineToArgvW(GetCommandLine(), &argc);

        STARTUPINFO startup_info =
        {
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
        if (success)
        {
            WaitForSingleObject(process_information.hProcess, INFINITE);
            CloseHandle(process_information.hProcess);
            CloseHandle(process_information.hThread);
            return 0;
        }
    }

    initialize_console(true);
    initialize_input_handler();
    create_map();
    player_init(map.size.x / 2);
    BlockControl_Init();

    clear();
    while (!game_exit)
    {
        update_delta_time();

        MSG msg = { 0 };
        while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        update_console();
        
        /*player.is_moving = 0;*/
        player_update();
        Mob_Spawn_Time();
        update_mob_ai();

        render();
    }

    BlockControl_Destroy();
    destroy_map();
    destroy_input_handler();
    destroy_console();
    return EXIT_SUCCESS;
}