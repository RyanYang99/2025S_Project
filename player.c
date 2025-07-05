#include "player.h"
#include "console.h"

player_t player = { 0, 0, 100 }; // 초기 위치 및 체력

void player_init(int x, int y) {
    player.x = x;
    player.y = y;
    player.hp = 100; // 초기 체력
}

void player_move(int dx, int dy) {
    player.x += dx;
    player.y += dy;
    // 경계 체크 등 추가 가능
}

void render_player(void) {
    COORD pos = { console.size.X / 2, console.size.Y / 2 };
    color_tchar_t player_char = {
        '@',
        BACKGROUND_T_BLACK,
        FOREGROUND_T_YELLOW
    };
    print_color_tchar(player_char, pos);
}