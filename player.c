#include "leak.h"
#include "player.h"

#include "map.h"
#include "input.h"
#include "console.h"
#include <stdio.h> // swprintf 사용하기위해

player_t player = { 0, 0, 100 }; // 초기 위치 및 체력

static void movement(const char character) {
    if (character == 'w')
        --player.y;
    else if (character == 'a')
        --player.x;
    else if (character == 's')
        ++player.y;
    else if (character == 'd')
        ++player.x;
}

static void update_player_offset(void) {
    player.x += map.offset_x;
}

void player_init(int x, int y) {
    player.x = x;
    player.y = y;
    player.hp = 100; // 초기 체력

    subscribe_keyhit(movement);
    subscribe_offset_change(update_player_offset);
}

void player_move(int dx, int dy) {
    player.x += dx;
    player.y += dy;
    // 경계 체크 등 추가 가능
}

void render_player(void) {
    // 플레이어 콘솔 위치 (화면 중앙)
    COORD pos = { console.size.X / 2, console.size.Y / 2 };

    // 체력 문자열 준비
    wchar_t hp_str[16];
    swprintf(hp_str, 16, L"HP: %d", player.hp);

    // 체력 문자열을 플레이어 머리 위에 출력
    COORD hp_pos = pos;
    if (hp_pos.Y > 0) hp_pos.Y -= 1; // 머리 위 한 칸 위로
    for (int i = 0; hp_str[i] != L'\0'; ++i) {
        COORD char_pos = hp_pos;
        char_pos.X += (SHORT)i - (SHORT)(wcslen(hp_str) / 2); // 중앙 정렬
        print_color_tchar((color_tchar_t) { hp_str[i], BACKGROUND_T_BLACK, FOREGROUND_T_RED }, char_pos);
    }

    // 플레이어 그리기
    color_tchar_t player_char = {
        '@',
        BACKGROUND_T_BLACK,
        FOREGROUND_T_YELLOW
    };
    print_color_tchar(player_char, pos);
}