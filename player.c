#include "player.h"
#include "console.h"

player_t player = { 0, 0, 100 }; // �ʱ� ��ġ �� ü��

void player_init(int x, int y) {
    player.x = x;
    player.y = y;
    player.hp = 100; // �ʱ� ü��
}

void player_move(int dx, int dy) {
    player.x += dx;
    player.y += dy;
    // ��� üũ �� �߰� ����
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