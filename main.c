#include "leak.h"

#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include "map.h"

#include "input.h"
#include "player.h"
#include "console.h"
#include "BlockCtrl.h"
#include "Mob.h"
#include "BossMalakh.h"

static void render(void)
{
    render_map();
    render_player();
    render_virtual_cursor();
    
    Mob_render();
    Mob_Spawn_Time();
    update_mob_ai();
    Boss_Render();
    Boss_Update_Ai();

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

    int boss_start_x = map.size.x / 2 - 20;
    int boss_start_y = map.size.y / 2;
    Boss_Init(boss_start_x, boss_start_y, 1000, 10);



    BlockControl_Init();


    clear();
    while (true)
    {
        update_console();
        handle_input_event();
        render();
        
    }

    BlockControl_Destroy();
    destroy_map();
    destroy_input_handler();
    destroy_console();

    return EXIT_SUCCESS;
}