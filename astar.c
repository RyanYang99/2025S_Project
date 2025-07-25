#include "leak.h"

// astar.c
#include "astar.h" // 새로 만든 astar.h를 포함합니다.

#include <stdio.h>   // fprintf, NULL 등을 위해
#include <stdlib.h>  // malloc, free, qsort, exit 등을 위해
#include <math.h>    // abs() 함수를 위해

// PathNode 메모리 할당 및 초기화
PathNode* Create_node(int x, int y, int g, int h, PathNode* parent) {
    PathNode* newNode = (PathNode*)malloc(sizeof(PathNode));
    if (newNode == NULL) {
        fprintf(stderr, "메모리 할당 실패 (PathNode)\n");
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

// 휴리스틱 함수 (맨해튼 거리)
int Calculate_Heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// 해당 좌표가 맵 범위 내에 있고 이동 가능한 블록인지 확인
bool is_valid_block(int x, int y, int map_size_x, int map_size_y) {
    if (x < 0 || x >= map_size_x || y < 0 || y >= map_size_y - 1) {
        return false;
    }
    block_t block_under_mob = map.ppBlocks[y + 1][x].type;
    return (block_under_mob == BLOCK_GRASS || block_under_mob == BLOCK_DIRT);
}

// 노드 비교 함수 (qsort 사용을 위해)
int Compare_nodes(const void* a, const void* b) {
    PathNode* nodeA = *(PathNode**)a;
    PathNode* nodeB = *(PathNode**)b;
    return nodeA->f - nodeB->f; // F값이 작은 노드가 우선순위 높음
}

// A* 경로 찾기 함수 (다음 한 칸 이동)
COORD find_path_next_step(int start_x, int start_y, int target_x, int target_y, int map_size_x, int map_size_y) {
    // 동적으로 closed_list_grid 할당 및 초기화
    bool** closed_list_grid = (bool**)malloc(map_size_y * sizeof(bool*));
    if (closed_list_grid == NULL) {
        fprintf(stderr, "closed_list_grid 메모리 할당 실패\n");
        return (COORD) { -1, -1 };
    }
    for (int i = 0; i < map_size_y; ++i) {
        closed_list_grid[i] = (bool*)calloc(map_size_x, sizeof(bool)); // calloc으로 false로 초기화
        if (closed_list_grid[i] == NULL) {
            fprintf(stderr, "closed_list_grid[%d] 메모리 할당 실패\n", i);
            for (int j = 0; j < i; ++j) free(closed_list_grid[j]);
            free(closed_list_grid);
            return (COORD) { -1, -1 };
        }
    }

    // A* 노드들을 저장할 동적 배열
#define MAX_NODES_FOR_ASTAR_SEARCH 200 * 200 // 예시: MAP_MAX_Y * 최대 X 사이즈
    PathNode** open_list = (PathNode**)malloc(MAX_NODES_FOR_ASTAR_SEARCH * sizeof(PathNode*));
    if (open_list == NULL) {
        fprintf(stderr, "open_list 메모리 할당 실패\n");
        for (int i = 0; i < map_size_y; ++i) free(closed_list_grid[i]);
        free(closed_list_grid);
        return (COORD) { -1, -1 };
    }
    int open_list_count = 0;

    // 할당된 모든 PathNode를 추적하여 함수 종료 시 해제
    PathNode** all_allocated_nodes = (PathNode**)malloc(MAX_NODES_FOR_ASTAR_SEARCH * sizeof(PathNode*));
    if (all_allocated_nodes == NULL) {
        fprintf(stderr, "all_allocated_nodes 메모리 할당 실패\n");
        free(open_list);
        for (int i = 0; i < map_size_y; ++i) free(closed_list_grid[i]);
        free(closed_list_grid);
        return (COORD) { -1, -1 };
    }
    int allocated_node_count = 0;

    // 시작 노드 생성 및 오픈 리스트에 추가
    PathNode* start_node = Create_node(start_x, start_y, 0, Calculate_Heuristic(start_x, start_y, target_x, target_y), NULL);
    open_list[open_list_count++] = start_node;
    all_allocated_nodes[allocated_node_count++] = start_node;

    PathNode* current_node = NULL;
    COORD next_step = { -1, -1 }; // 찾지 못했을 경우 반환할 값

    int dx[] = { 0, 0, 1, -1 }; // 상, 하, 우, 좌
    int dy[] = { -1, 1, 0, 0 };

    while (open_list_count > 0) {
        // 오픈 리스트 정렬 
        qsort(open_list, open_list_count, sizeof(PathNode*), Compare_nodes);

        // 가장 작은 F값 노드 추출
        current_node = open_list[0];
        // 추출한 노드를 오픈 리스트에서 제거 (배열 재정렬)
        for (int i = 0; i < open_list_count - 1; ++i) {
            open_list[i] = open_list[i + 1];
        }
        open_list_count--;

        // 현재 노드를 클로즈 리스트에 추가
        if (current_node->x >= 0 && current_node->x < map_size_x &&
            current_node->y >= 0 && current_node->y < map_size_y) {
            closed_list_grid[current_node->y][current_node->x] = true;
        }

        // 목표에 도달했는지 확인
        if (current_node->x == target_x && current_node->y == target_y) {
            // 경로 재구성: 시작 노드 바로 다음 노드를 찾음
            PathNode* path_ptr = current_node;
            if (path_ptr->parent == NULL) { // 이미 시작 노드에 도착했다면 (시작점 == 목표점)
                next_step.X = (SHORT)target_x; // 현재 위치가 곧 목표 위치
                next_step.Y = (SHORT)target_y;
            }
            else {
                while (path_ptr->parent != NULL && path_ptr->parent->parent != NULL) {
                    path_ptr = path_ptr->parent;
                }
                next_step.X = (SHORT)path_ptr->x;
                next_step.Y = (SHORT)path_ptr->y;
            }
            // 찾았으므로 루프 종료
            break;
        }

        // 이웃 노드 탐색
        for (int i = 0; i < 4; ++i) {
            int neighbor_x = current_node->x + dx[i];
            int neighbor_y = current_node->y + dy[i];

            // 맵 범위 내에 있고 이동 가능한 블록인지 확인
            if (!is_valid_block(neighbor_x, neighbor_y, map_size_x, map_size_y)) {
                continue;
            }

            // 클로즈 리스트에 있는지 확인
            if (neighbor_x >= 0 && neighbor_x < map_size_x &&
                neighbor_y >= 0 && neighbor_y < map_size_y &&
                closed_list_grid[neighbor_y][neighbor_x]) {
                continue;
            }

            int new_g = current_node->g + 1; // 인접 노드이므로 비용은 1

            // 오픈 리스트에 이미 있는지 확인
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
                // 오픈 리스트에 없으면 새 노드 생성 및 추가
                PathNode* neighbor_node = Create_node(
                    neighbor_x, neighbor_y, new_g,
                    Calculate_Heuristic(neighbor_x, neighbor_y, target_x, target_y),
                    current_node
                );
                if (open_list_count < MAX_NODES_FOR_ASTAR_SEARCH) { // 오픈 리스트 용량 확인
                    open_list[open_list_count++] = neighbor_node;
                    all_allocated_nodes[allocated_node_count++] = neighbor_node;
                }
                else {
                    fprintf(stderr, "오픈 리스트 용량 초과!\n");
                    free(neighbor_node); // 할당된 노드 해제
                }
            }
        }
    }

    // 할당된 모든 PathNode 메모리 해제
    for (int i = 0; i < allocated_node_count; ++i) {
        free(all_allocated_nodes[i]);
    }
    free(all_allocated_nodes);
    free(open_list); // 오픈 리스트 배열 자체 해제

    // 클로즈 리스트 그리드 메모리 해제
    for (int i = 0; i < map_size_y; ++i) {
        free(closed_list_grid[i]);
    }
    free(closed_list_grid);

    return next_step;
}