#include "leak.h"
#include "player.h"

#include "map.h"
#include "input.h"
#include "console.h"
#include <stdio.h> // swprintf ����ϱ�����

player_t player = { 0, 0, 100 }; // �ʱ� ��ġ �� ü��

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
    player.hp = 100; // �ʱ� ü��

    subscribe_keyhit(movement);
    subscribe_offset_change(update_player_offset);
}

void player_move(int dx, int dy) {
    player.x += dx;
    player.y += dy;
    // ��� üũ �� �߰� ����
}

void render_player(void) {
    // �÷��̾� �ܼ� ��ġ (ȭ�� �߾�)
    COORD pos = { console.size.X / 2, console.size.Y / 2 };

    // ü�� ���ڿ� �غ�
    wchar_t hp_str[16];
    swprintf(hp_str, 16, L"HP: %d", player.hp);

    // ü�� ���ڿ��� �÷��̾� �Ӹ� ���� ���
    COORD hp_pos = pos;
    if (hp_pos.Y > 0) hp_pos.Y -= 1; // �Ӹ� �� �� ĭ ����
    for (int i = 0; hp_str[i] != L'\0'; ++i) {
        COORD char_pos = hp_pos;
        char_pos.X += (SHORT)i - (SHORT)(wcslen(hp_str) / 2); // �߾� ����
        print_color_tchar((color_tchar_t) { hp_str[i], BACKGROUND_T_BLACK, FOREGROUND_T_RED }, char_pos);
    }

    // �÷��̾� �׸���
    color_tchar_t player_char = {
        '@',
        BACKGROUND_T_BLACK,
        FOREGROUND_T_YELLOW
    };
    print_color_tchar(player_char, pos);
}