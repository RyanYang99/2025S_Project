#include "leak.h"

#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
#include "map.h"
#include "input.h"
#include "player.h"
#include "console.h"

static void update(float delta_time)
{
    // 게임의 모든 업데이트 로직을 이곳에서 처리합니다.
    player_update(delta_time);
    // (향후 몬스터, 오브젝트 등의 업데이트도 여기에 추가)
}

static void render(void)
{
    render_map();
    render_player();
    //debug_render_map(true);
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

    // --- Delta Time 측정을 위한 변수 초기화 ---
    clock_t last_time = clock();
    float delta_time = 0.0f;

    clear();
    while (true)
    {
        // --- 1. Delta Time 계산 ---
        clock_t current_time = clock();
        delta_time = (float)(current_time - last_time) / CLOCKS_PER_SEC;
        last_time = current_time;

        // --- 2. 입력 처리 ---
        // handle_input_event()는 내부적으로 player의 is_moving 상태를 변경합니다.
        handle_input_event();
        /* player.is_moving = 0;*/
        // --- 3. 게임 상태 업데이트 ---
        // 입력 결과와 Delta Time을 바탕으로 게임 월드의 상태를 갱신합니다.
        update(delta_time);

        // --- 4. 렌더링 ---
        // 갱신된 상태를 화면에 그립니다.
        update_console(); // 콘솔 버퍼 관리
        render();         // 실제 그리기 호출
    }

    destroy_map();
    destroy_input_handler();
    destroy_console();
    return EXIT_SUCCESS;
}