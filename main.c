#include "leak.h"

#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
#include "map.h"
#include "Mob.h"
#include "input.h"
#include "player.h"
#include "console.h"
#include "BlockCtrl.h"

static void update(float delta_time)
{
    player_update(delta_time);
}

static void render(void)
{
    render_map();
    render_player();
    render_virtual_cursor();
    Mob_render();
}

int main(void)
{
#if _DEBUG
    //메모리 누수 체크
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    initialize_console(true);
    initialize_input_handler();
    create_map();
    player_init(map.size.x / 2);
    BlockControl_Init();

    // Delta Time 측정을 위한 변수 초기화 
    clock_t last_time = clock();
    float delta_time = 0.0f;

    clear();
    while (true)
    {
        // Delta Time 계산
        clock_t current_time = clock();
        delta_time = (float)(current_time - last_time) / CLOCKS_PER_SEC;
        last_time = current_time;

        update_console();
        handle_input_event();
        render();
        /*player.is_moving = 0;*/
        update(delta_time);
        Mob_Spawn_Time();
        update_mob_ai();

        MSG msg = { 0 };
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    BlockControl_Destroy();
    destroy_map();
    destroy_input_handler();
    destroy_console();
    return EXIT_SUCCESS;
}