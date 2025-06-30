#include "leak.h"

#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include "map.h"
#include "console.h"

//임시 테스트용
bool has_movement = false;

static void movement(const wchar_t character)
{
    if (character == L'w')
    {
        --player.y;
        has_movement = true;
    }
    else if (character == L'a')
    {
        --player.x;
        has_movement = true;
    }
    else if (character == L's')
    {
        ++player.y;
        has_movement = true;
    }
    else if (character == L'd')
    {
        ++player.x;
        has_movement = true;
    }
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

    initialize_console(true, movement);
    
    //map.h의 map 전역 변수로 현재 사용중인 맵 접근
    map = create_map();
    generate_map();

    clear();
    while (true)
    {
        update_console();

        if (has_movement)
        {
            has_movement = false;
            clear();
        }

        render();
    }

    destroy_map();
    return EXIT_SUCCESS;
}