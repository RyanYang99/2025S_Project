// astar.c
#include "astar.h" // ���� ���� astar.h�� �����մϴ�.

#include <stdio.h>   // fprintf, NULL ���� ����
#include <stdlib.h>  // malloc, free, qsort, exit ���� ����
#include <math.h>    // abs() �Լ��� ����

// PathNode �޸� �Ҵ� �� �ʱ�ȭ
PathNode* Create_node(int x, int y, int g, int h, PathNode* parent) {
    PathNode* newNode = (PathNode*)malloc(sizeof(PathNode));
    if (newNode == NULL) {
        fprintf(stderr, "�޸� �Ҵ� ���� (PathNode)\n");
        exit(EXIT_FAILURE);
    }
    newNode->x = x;
    newNode->y = y;
    newNode->g = g;
    newNode->h = h;
    newNode->f = g + h;
    newNode->parent = parent;
    return newNode;
}

// �޸���ƽ �Լ� (����ư �Ÿ�)
int Calculate_Heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// �ش� ��ǥ�� �� ���� ���� �ְ� �̵� ������ ������� Ȯ��
bool is_valid_block(int x, int y, int map_size_x, int map_size_y) {
    if (x < 0 || x >= map_size_x || y < 0 || y >= map_size_y - 1) {
        return false;
    }
    block_t block_under_mob = map.ppBlocks[y + 1][x].type;
    return (block_under_mob == BLOCK_GRASS || block_under_mob == BLOCK_DIRT);
}

// ��� �� �Լ� (qsort ����� ����)
int Compare_nodes(const void* a, const void* b) {
    PathNode* nodeA = *(PathNode**)a;
    PathNode* nodeB = *(PathNode**)b;
    return nodeA->f - nodeB->f; // F���� ���� ��尡 �켱���� ����
}

// A* ��� ã�� �Լ� (���� �� ĭ �̵�)
COORD find_path_next_step(int start_x, int start_y, int target_x, int target_y, int map_size_x, int map_size_y) {
    // �������� closed_list_grid �Ҵ� �� �ʱ�ȭ
    bool** closed_list_grid = (bool**)malloc(map_size_y * sizeof(bool*));
    if (closed_list_grid == NULL) {
        fprintf(stderr, "closed_list_grid �޸� �Ҵ� ����\n");
        return (COORD) { -1, -1 };
    }
    for (int i = 0; i < map_size_y; ++i) {
        closed_list_grid[i] = (bool*)calloc(map_size_x, sizeof(bool)); // calloc���� false�� �ʱ�ȭ
        if (closed_list_grid[i] == NULL) {
            fprintf(stderr, "closed_list_grid[%d] �޸� �Ҵ� ����\n", i);
            for (int j = 0; j < i; ++j) free(closed_list_grid[j]);
            free(closed_list_grid);
            return (COORD) { -1, -1 };
        }
    }

    // A* ������ ������ ���� �迭
#define MAX_NODES_FOR_ASTAR_SEARCH 200 * 200 // ����: MAP_MAX_Y * �ִ� X ������
    PathNode** open_list = (PathNode**)malloc(MAX_NODES_FOR_ASTAR_SEARCH * sizeof(PathNode*));
    if (open_list == NULL) {
        fprintf(stderr, "open_list �޸� �Ҵ� ����\n");
        for (int i = 0; i < map_size_y; ++i) free(closed_list_grid[i]);
        free(closed_list_grid);
        return (COORD) { -1, -1 };
    }
    int open_list_count = 0;

    // �Ҵ�� ��� PathNode�� �����Ͽ� �Լ� ���� �� ����
    PathNode** all_allocated_nodes = (PathNode**)malloc(MAX_NODES_FOR_ASTAR_SEARCH * sizeof(PathNode*));
    if (all_allocated_nodes == NULL) {
        fprintf(stderr, "all_allocated_nodes �޸� �Ҵ� ����\n");
        free(open_list);
        for (int i = 0; i < map_size_y; ++i) free(closed_list_grid[i]);
        free(closed_list_grid);
        return (COORD) { -1, -1 };
    }
    int allocated_node_count = 0;

    // ���� ��� ���� �� ���� ����Ʈ�� �߰�
    PathNode* start_node = Create_node(start_x, start_y, 0, Calculate_Heuristic(start_x, start_y, target_x, target_y), NULL);
    open_list[open_list_count++] = start_node;
    all_allocated_nodes[allocated_node_count++] = start_node;

    PathNode* current_node = NULL;
    COORD next_step = { -1, -1 }; // ã�� ������ ��� ��ȯ�� ��

    int dx[] = { 0, 0, 1, -1 }; // ��, ��, ��, ��
    int dy[] = { -1, 1, 0, 0 };

    while (open_list_count > 0) {
        // ���� ����Ʈ ���� 
        qsort(open_list, open_list_count, sizeof(PathNode*), Compare_nodes);

        // ���� ���� F�� ��� ����
        current_node = open_list[0];
        // ������ ��带 ���� ����Ʈ���� ���� (�迭 ������)
        for (int i = 0; i < open_list_count - 1; ++i) {
            open_list[i] = open_list[i + 1];
        }
        open_list_count--;

        // ���� ��带 Ŭ���� ����Ʈ�� �߰�
        if (current_node->x >= 0 && current_node->x < map_size_x &&
            current_node->y >= 0 && current_node->y < map_size_y) {
            closed_list_grid[current_node->y][current_node->x] = true;
        }

        // ��ǥ�� �����ߴ��� Ȯ��
        if (current_node->x == target_x && current_node->y == target_y) {
            // ��� �籸��: ���� ��� �ٷ� ���� ��带 ã��
            PathNode* path_ptr = current_node;
            if (path_ptr->parent == NULL) { // �̹� ���� ��忡 �����ߴٸ� (������ == ��ǥ��)
                next_step.X = target_x; // ���� ��ġ�� �� ��ǥ ��ġ
                next_step.Y = target_y;
            }
            else {
                while (path_ptr->parent != NULL && path_ptr->parent->parent != NULL) {
                    path_ptr = path_ptr->parent;
                }
                next_step.X = path_ptr->x;
                next_step.Y = path_ptr->y;
            }
            // ã�����Ƿ� ���� ����
            break;
        }

        // �̿� ��� Ž��
        for (int i = 0; i < 4; ++i) {
            int neighbor_x = current_node->x + dx[i];
            int neighbor_y = current_node->y + dy[i];

            // �� ���� ���� �ְ� �̵� ������ ������� Ȯ��
            if (!is_valid_block(neighbor_x, neighbor_y, map_size_x, map_size_y)) {
                continue;
            }

            // Ŭ���� ����Ʈ�� �ִ��� Ȯ��
            if (neighbor_x >= 0 && neighbor_x < map_size_x &&
                neighbor_y >= 0 && neighbor_y < map_size_y &&
                closed_list_grid[neighbor_y][neighbor_x]) {
                continue;
            }

            int new_g = current_node->g + 1; // ���� ����̹Ƿ� ����� 1

            // ���� ����Ʈ�� �̹� �ִ��� Ȯ��
            bool in_open_list = false;
            PathNode* existing_node = NULL;
            for (int j = 0; j < open_list_count; ++j) {
                if (open_list[j]->x == neighbor_x && open_list[j]->y == neighbor_y) {
                    in_open_list = true;
                    existing_node = open_list[j];
                    break;
                }
            }

            if (in_open_list) {
                if (new_g < existing_node->g) {
                    existing_node->g = new_g;
                    existing_node->f = existing_node->g + existing_node->h;
                    existing_node->parent = current_node;
                }
            }
            else {
                // ���� ����Ʈ�� ������ �� ��� ���� �� �߰�
                PathNode* neighbor_node = Create_node(
                    neighbor_x, neighbor_y, new_g,
                    Calculate_Heuristic(neighbor_x, neighbor_y, target_x, target_y),
                    current_node
                );
                if (open_list_count < MAX_NODES_FOR_ASTAR_SEARCH) { // ���� ����Ʈ �뷮 Ȯ��
                    open_list[open_list_count++] = neighbor_node;
                    all_allocated_nodes[allocated_node_count++] = neighbor_node;
                }
                else {
                    fprintf(stderr, "���� ����Ʈ �뷮 �ʰ�!\n");
                    free(neighbor_node); // �Ҵ�� ��� ����
                }
            }
        }
    }

    // �Ҵ�� ��� PathNode �޸� ����
    for (int i = 0; i < allocated_node_count; ++i) {
        free(all_allocated_nodes[i]);
    }
    free(all_allocated_nodes);
    free(open_list); // ���� ����Ʈ �迭 ��ü ����

    // Ŭ���� ����Ʈ �׸��� �޸� ����
    for (int i = 0; i < map_size_y; ++i) {
        free(closed_list_grid[i]);
    }
    free(closed_list_grid);

    return next_step;
}