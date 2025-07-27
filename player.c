#include "leak.h"
#include "player.h"

#include "map.h"
#include "input.h"
#include "delta.h"
#include "console.h"
#include "global_state.h"
#include <stdio.h> // swprintf 사용하기위해
#include <wchar.h>
#include <time.h>
#include <conio.h>

player_t player = { 0, 0, 1000, 0, 0.0f, 0 }; // 초기 위치 및 체력, is_walking 상태 추가 (0: 정지, 1: 걷기)

#define PLAYER_SPRITE_WIDTH 5
#define PLAYER_SPRITE_HEIGHT 5
#define ANIMATION_SPEED 4.0f // 1초에 4번 프레임 변경 (속도 조절 가능)

// 각 픽셀을 표현할 구조체
typedef struct {
    wchar_t character; // 표시할 유니코드 문자
    int background_color; // 배경색
    int foreground_color; // 전경색
} PlayerSpritePixel;

// 2개의 애니메이션 프레임

// 프레임 1: 기본 서 있는 자세
static const PlayerSpritePixel player_sprite_stand[PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    // {문자, 배경색, 전경색}
    { {L' ',0,0}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} }, // 머리 (위:머리카락, 아래:피부)
    { {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_CYAN}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_CYAN}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_CYAN} }, // 몸통과 팔
    { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L' ',0,0} }, // 허리, 바지
    { {L' ',0,0}, {L'▌', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0}, {L'▐', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0} }, // 다리
    { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0} }  // 신발
};

// 걷기 애니메이션 (2 프레임)
static const PlayerSpritePixel player_sprite_walk[2][PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    // 프레임 0
    {
        { {L' ',0,0}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} },
        { {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_CYAN}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_CYAN}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L' '}},
        { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L' ',0,0} },
        { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0} },
        { {L' ',0,0}, {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_DARKGRAY}, {L' ',0,0}, {L' ',0,0} }
    },
    // 프레임 1
    {
        { {L' ',0,0}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▀', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} },
        { {L' '}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_CYAN}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_CYAN} },
        { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L' ',0,0} },
        { {L' ',0,0}, {L' ',0,0}, {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0} },
        { {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_DARKGRAY}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0} }
    }
};


static void movement(void) {
    if (!_kbhit())
        return;

    const char character = (char)_getch();

    if (character == 'q')
        game_exit = true;

    if (character == 'w' || character == 'a' || character == 's' || character == 'd') {
        // 새로운 위치 계산
        int new_x = player.x;
        int new_y = player.y;

        if (character == 'w') new_y--;
        else if (character == 'a') new_x--;
        else if (character == 's') new_y++;
        else if (character == 'd') new_x++;

        // 충돌 체크 - 공기 블록(BLOCK_AIR)인 경우에만 이동
        if (is_walkable(new_x, new_y)) {
            player.x = new_x;
            player.y = new_y;
            player.is_moving = 1; // 실제로 이동했을 때만 애니메이션 활성화
        }
        // 고체 블록에 충돌한 경우 이동하지 않음
    }
}

static void update_player_offset(void) {
    player.x += map.offset_x;
}

// *** 새로운 업데이트 함수 ***
void player_update(void) {
    movement();

    if (player.is_moving) {
        // Delta Time을 타이머에 누적
        player.animation_timer += delta_time;

        // 프레임 변경 시간이 되었는지 확인
        if (player.animation_timer >= 1.0f / ANIMATION_SPEED) {
            player.animation_timer = 0.0f; // 타이머 초기화
            player.current_frame = (player.current_frame + 1) % 2; // 다음 프레임으로 (0 -> 1 -> 0 ...)
        }
    }
    else {
        // 움직이지 않으면 정지 자세로 초기화
        player.current_frame = 0;
        player.animation_timer = 0.0f;
    }

    // 다음 프레임에서는 다시 정지 상태로 시작 (입력이 없으면 is_moving = 0 유지)
     player.is_moving = 0;// 추후에 수정할것 true/false
}


void player_init(int x) {
    player.x = x;
    player.y = find_ground_pos(x);
    if (player.y - 1 >= 0)
        --player.y; // 가능할 시 찾은 블록 위로 설정

    player.hp = 1000; // 초기 체력

    //subscribe_keyhit(movement);
    subscribe_offset_change(update_player_offset);
}

// 충돌 감지 함수 구현
bool is_walkable(int x, int y) {
    // 맵 경계 체크
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y) {
        return false;
    }

    // 블록 정보 가져오기 (map.c의 함수 사용)
    block_info_t block = get_block_info_at(x, y);

    // 블록 타입에 따른 이동 가능 여부 판단
    switch (block.type) {
        case BLOCK_AIR:
        case BLOCK_LEAF:
        case BLOCK_WATER:
            return true;
    }

    return false;
}

void player_move(int dx, int dy) {
    int new_x = player.x + dx;
    int new_y = player.y + dy;

    // 충돌 체크 후 이동
    if (is_walkable(new_x, new_y)) {
        player.x = new_x;
        player.y = new_y;
        player.is_moving = 1;
    }
}

void render_player(void) {
    // 플레이어의 중심이 될 콘솔 위치 (화면 중앙)
    COORD center_pos = { console.size.X / 2, console.size.Y / 2 };

    // 플레이어의 'is_walking' 상태와 시간에 따라 어떤 프레임을 그릴지 결정
    // (여기서는 간단히 is_walking 값으로만 구분)
    // 현재 상태에 맞는 스프라이트 포인터 선택
    const PlayerSpritePixel(*current_sprite_ptr)[PLAYER_SPRITE_WIDTH];
    if (player.is_moving) {
        current_sprite_ptr = player_sprite_walk[player.current_frame];
    }
    else {
        current_sprite_ptr = player_sprite_stand;
    }

    for (int y = 0; y < PLAYER_SPRITE_HEIGHT; ++y) {
        for (int x = 0; x < PLAYER_SPRITE_WIDTH; ++x) {
            PlayerSpritePixel pixel = current_sprite_ptr[y][x];

            if (pixel.character == L' ') continue;

            COORD draw_pos;
            draw_pos.X = (SHORT)(center_pos.X + x - (PLAYER_SPRITE_WIDTH / 2));
            draw_pos.Y = (SHORT)(center_pos.Y + y - (PLAYER_SPRITE_HEIGHT / 2));

            if (draw_pos.X >= 0 && draw_pos.X < console.size.X &&
                draw_pos.Y >= 0 && draw_pos.Y < console.size.Y) {

                // 캐릭터 '픽셀'의 전경색과 배경색을 동시에 사용
                color_tchar_t tchar;
                tchar.character = pixel.character;

                tchar.background = (pixel.character == L'▀') ? pixel.foreground_color : BACKGROUND_T_BLACK;
                tchar.foreground = (pixel.character == L'▀') ? pixel.background_color : pixel.foreground_color;

                print_color_tchar(tchar, draw_pos);
            }
        }
    }

    // 체력(HP) 바 렌더링
    wchar_t hp_str[16];
    swprintf(hp_str, 16, L"HP: %d", player.hp);

    // 체력 문자열을 플레이어 스프라이트 머리 위에 출력
    COORD hp_pos;
    // 새로운 스프라이트의 최상단 좌표를 기준으로 Y 위치 조정
    hp_pos.Y = center_pos.Y - (PLAYER_SPRITE_HEIGHT / 2) - 1;
    if (hp_pos.Y < 0) hp_pos.Y = 0; // 화면 위로 벗어나지 않도록 처리

    // HP 문자열을 중앙에 정렬하여 출력
    for (int i = 0; hp_str[i] != L'\0'; ++i) {
        COORD char_pos = hp_pos;
        char_pos.X = center_pos.X + (SHORT)i - (SHORT)(wcslen(hp_str) / 2);
        if (char_pos.X >= 0 && char_pos.X < console.size.X) {
            print_color_tchar((color_tchar_t) { hp_str[i], BACKGROUND_T_BLACK, FOREGROUND_T_RED }, char_pos);
        }
    }
}

//특정 x좌표에서 가장 높은 지상 의 Y좌표를 찾아 반환
int find_ground_pos(int x)
{
    for (int y = 0; y < map.size.y; ++y)
        if (!is_walkable(x, y))
            return y; //걷지 못하는 블록을 찾으면 y좌표 반환

    return map.size.y / 2; //블록을 찾지 못하면 맵 높이의 절반 반환;
}