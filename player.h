#pragma once

#include <windows.h>
#include "console.h"

// �÷��̾� ���� ����ü
typedef struct {
    int x, y; // �� ���� ��ġ
    int hp;   // ü�� �� �߰� ����
    // ��Ÿ �÷��̾� �Ӽ�
} player_t;

// ���� �÷��̾� ��ü
extern player_t player;

// �÷��̾� �ʱ�ȭ
void player_init(int x, int y);

// �÷��̾� �̵�
void player_move(int dx, int dy);

// �÷��̾� ������
void render_player(void);