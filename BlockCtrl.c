#include "leak.h"

#include "BlockCtrl.h"
#include "map.h"
#include "player.h"
#include <math.h> // floor 함수 사용

#define MOUSE_X_OFFSET 0
#define MOUSE_Y_OFFSET 0  

static COORD latestMousePos;

// 마우스 이동 시 최신 위치 갱신
static void BC_OnMouseMove(const COORD pos)
{
    latestMousePos = pos;
}

// 마우스 클릭 시 상호작용 처리
static void BC_OnMouseClick(const bool left)
{
    int screen_cx = console.size.X / 2;
    int screen_cy = console.size.Y / 2;

    int block_width = 3;
    int block_height = 3;

    int relative_mouse_x = latestMousePos.X - screen_cx + MOUSE_X_OFFSET;
    int relative_mouse_y = latestMousePos.Y - screen_cy + MOUSE_Y_OFFSET;

    int block_x = player.x + (int)floor(relative_mouse_x / (float)block_width);
    int block_y = player.y + (int)floor(relative_mouse_y / (float)block_height);

    if (block_x < 0 || block_x >= map.size.x || block_y < 0 || block_y >= map.size.y)
        return;

    if (left)
    {
        int damage = 3;  // 예: 손 데미지 (나중에 도구별로 바꿀 예정)

        bool destroyed = damage_block_at(&map, block_x, block_y, damage);

        if (destroyed)
        {
            // 블록이 부서졌을 때 추가 처리 있으면 여기서
        }
    }
    else
    {
        if (block_x == player.x && block_y == player.y)
            return;  // 1. 플레이어 위치에는 설치 금지

        block_info_t target = get_block_info_at(block_x, block_y);
        if (target.type != BLOCK_AIR)
            return;  // 2. 이미 블록이 있는 곳에는 설치 금지

        block_t held_block = BLOCK_DIRT; // 4. 나중에 인벤토리에서 선택한 블록으로 대체
        if (held_block == BLOCK_AIR) return; // 공기 블록은 설치 불가

        // 5. 도구가 들려 있다면 설치 불가 (가정: BLOCK_TOOL_*로 구분하거나 enum 값 등)
        //if (held_block >= TOOL_BEGIN && held_block <= TOOL_END) return;

        // 6. 맵 경계 체크는 get_block_info_at / set_block_at에서 이미 처리
        set_block_at(block_x, block_y, held_block); // 3. 자동으로 체력 초기화됨

        
    }
}


// 가상 커서 렌더링 (모서리 스타일)
void render_virtual_cursor(void)
{
    int screen_cx = console.size.X / 2;
    int screen_cy = console.size.Y / 2;

    int block_width = 3;
    int block_height = 3;

    int relative_mouse_x = latestMousePos.X - screen_cx + MOUSE_X_OFFSET;
    int relative_mouse_y = latestMousePos.Y - screen_cy + MOUSE_Y_OFFSET;

    int block_x = player.x + (int)floor(relative_mouse_x / (float)block_width);
    int block_y = player.y + (int)floor(relative_mouse_y / (float)block_height);

    int draw_x = screen_cx + (block_x - player.x) * block_width;
    int draw_y = screen_cy + (block_y - player.y) * block_height;

    // 각 모서리에 문자를 출력
    color_tchar_t tl = {L'┌', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t tr = {L'┐', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t bl = {L'└', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t br = {L'┘', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };

    print_color_tchar(tl, (COORD) { draw_x, draw_y });
    print_color_tchar(tr, (COORD) { draw_x + block_width - 1, draw_y });
    print_color_tchar(bl, (COORD) { draw_x, draw_y + block_height - 1 });
    print_color_tchar(br, (COORD) { draw_x + block_width - 1, draw_y + block_height - 1 });
}

// 초기화 및 해제
void BlockControl_Init(void)
{
    subscribe_mouse_position(BC_OnMouseMove);
    subscribe_mouse_click(BC_OnMouseClick);
}

void BlockControl_Destroy(void)
{
    unsubscribe_mouse_click(BC_OnMouseClick);
    unsubscribe_mouse_position(BC_OnMouseMove);
}





