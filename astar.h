
#pragma once

#include <stdbool.h> // bool�� ����ϱ� ���� �ʿ�
#include <windows.h> // COORD�� ����ϱ� ���� �ʿ� (Mob.h������ ���)
#include "map.h"     // map_t ����ü �� block_t ������ ����� ���� �ʿ�

// A* �˰����� ���� ��� ����ü
typedef struct PathNode {
    int x, y;           // ����� �� ��ǥ
    int g;              // ���������� ���� �������� ���� ���
    int h;              // ���� ��忡�� ��ǥ������ ���� ��� (�޸���ƽ)
    int f;              // �� ��� (g + h)
    struct PathNode* parent; // ��� �籸���� ���� �θ� ��� ������
} PathNode;

// ��� �� �Լ� (qsort ����� ����)
int Compare_nodes(const void* a, const void* b);

// PathNode �޸� �Ҵ� �� �ʱ�ȭ
PathNode* Create_node(int x, int y, int g, int h, PathNode* parent);

// �޸���ƽ �Լ� (����ư �Ÿ�)
int Calculate_Heuristic(int x1, int y1, int x2, int y2);

// �ش� ��ǥ�� �� ���� ���� �ְ� �̵� ������ ������� Ȯ��
bool is_valid_block(int x, int y, int map_size_x, int map_size_y);

// A* ��� ã�� �Լ�: ���� �̵��� �� ĭ�� ��ǥ�� ��ȯ�մϴ�.
// ��θ� ã�� ���ϸ� {-1, -1}�� ��ȯ�մϴ�.
// map_t�� ũ�⸦ ����ϱ� ���� map_size_x�� map_size_y�� ���ڷ� �޽��ϴ�.
COORD find_path_next_step(int start_x, int start_y, int target_x, int target_y, int map_size_x, int map_size_y);