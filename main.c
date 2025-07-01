#include "leak.h"

#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include "map.h"
#include "console.h"

//임시 테스트용
static int movement(void)
{
    if (!_kbhit())
        return 0;

    const char character = (char)_getch();
    if (character == 'w')
        --player.y;
    else if (character == 'a')
        --player.x;
    else if (character == 's')
        ++player.y;
    else if (character == 'd')
        ++player.x;
    else if (character == 'q')
        return 2;

    return 1;
}

static void render(void)
{
    render_map();
    //debug_render_map();
}

int main(void)
{
#if _DEBUG
    //메모리 누수 체크
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    initialize_console(true);
    
    //map.h의 map 전역 변수로 현재 사용중인 맵 접근
    map = create_map();
    generate_map();

    clear();
    while (true)
    {
        update_console();

        const int result = movement();
        if (result == 1)
            clear();
        else if (result == 2)
            break;

        render();
    }

    destroy_map();
    destroy_console();
    return EXIT_SUCCESS;
}