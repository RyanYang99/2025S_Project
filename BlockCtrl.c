#include "leak.h"

#include "BlockCtrl.h"
#include "map.h"
#include "player.h"
#include <math.h> // floorf 함수 사용

static COORD latestMousePos;
static float screen_cx = 0, screen_cy = 0, relative_mouse_x = 0, relative_mouse_y = 0;
static int block_x = 0, block_y = 0, draw_x = 0, draw_y = 0;

#if _DEBUG
int selected_block_x = -1, selected_block_y = -1;
#endif

// 마우스 이동 시 최신 위치 갱신
static void BC_OnMouseMove(const COORD pos)
{
    latestMousePos = pos;

    screen_cx = (float)console.size.X / 2.0f;
    screen_cy = (float)console.size.Y / 2.0f;
    relative_mouse_x = latestMousePos.X - screen_cx;
    relative_mouse_y = latestMousePos.Y - screen_cy;

    block_x = player.x + (int)floorf(relative_mouse_x / TEXTURE_SIZE);
    block_y = player.y + (int)floorf(relative_mouse_y / TEXTURE_SIZE);
    draw_x = (int)(screen_cx + (block_x - player.x) * TEXTURE_SIZE);
    draw_y = (int)(screen_cy + (block_y - player.y) * TEXTURE_SIZE);

#if _DEBUG
    selected_block_x = block_x;
    selected_block_y = block_y;
#endif
}

// 마우스 클릭 시 상호작용 처리
static void BC_OnMouseClick(const bool left)
{
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
    // 각 모서리에 문자를 출력
    color_tchar_t tl = {L'┌', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t tr = {L'┐', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t bl = {L'└', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };
    color_tchar_t br = {L'┘', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE };

    print_color_tchar(tl, (COORD) { (SHORT)draw_x, (SHORT)draw_y });
    print_color_tchar(tr, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)draw_y });
    print_color_tchar(bl, (COORD) { (SHORT)draw_x, (SHORT)(draw_y + TEXTURE_SIZE - 1) });
    print_color_tchar(br, (COORD) { (SHORT)(draw_x + TEXTURE_SIZE - 1), (SHORT)(draw_y + TEXTURE_SIZE - 1) });
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