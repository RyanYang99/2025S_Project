#include "leak.h"

#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include "map.h"
#include "input.h"
#include "player.h"
#include "console.h"

static void render(void)
{
    render_map();
    render_player();
    //debug_render_map();
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
    player_init(map.size.x / 2, map.size.y / 2);

    clear();
    while (true)
    {
        update_console();
        handle_input_event();
        render();
    }

    destroy_map();
    destroy_input_handler();
    destroy_console();
    return EXIT_SUCCESS;
}