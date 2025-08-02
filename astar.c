#include "astar.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_NODES_FOR_ASTAR_SEARCH 2000
#define MAX_HEAP_SIZE MAX_NODES_FOR_ASTAR_SEARCH

#define MAP_WIDTH 200
#define MAP_HEIGHT 400

// 힙 관련 도우미 함수들
void heap_push(PathNode** heap, int* heap_size, PathNode* node);
PathNode* heap_pop(PathNode** heap, int* heap_size);
void heapify_down(PathNode** heap, int heap_size, int index);
void heapify_up(PathNode** heap, int heap_size, int index);
void swap_nodes(PathNode** a, PathNode** b);

extern map_t map;

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

int Calculate_Heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// 우선순위 큐(이진 힙)를 사용하여 A* 경로 찾기 함수를 개선
path_t find_path(int start_x, int start_y, int target_x, int target_y, IsMovableFunc is_movable) {
    // 메모리 관리를 위한 동적 배열
    PathNode* all_allocated_nodes[MAX_NODES_FOR_ASTAR_SEARCH * 2];
    int allocated_node_count = 0;

    // 개방 목록을 힙으로 구현 (배열 기반)
    PathNode* open_list_heap[MAX_HEAP_SIZE];
    int open_list_size = 0;

    // 폐쇄 목록과 개방 목록을 2차원 배열로 관리하여 빠른 탐색 가능
    bool close_list_visited[MAP_WIDTH][MAP_HEIGHT] = { false };
    bool in_open_list_check[MAP_WIDTH][MAP_HEIGHT] = { false };
    int open_list_index[MAP_WIDTH][MAP_HEIGHT]; // 힙 배열 내 노드의 인덱스를 저장

    PathNode* start_node = Create_node(start_x, start_y, 0, Calculate_Heuristic(start_x, start_y, target_x, target_y), NULL);

    // 힙에 시작 노드 추가
    heap_push(open_list_heap, &open_list_size, start_node);
    in_open_list_check[start_x][start_y] = true;
    open_list_index[start_x][start_y] = 0;
    all_allocated_nodes[allocated_node_count++] = start_node;

    path_t result_path = { .count = 0, .current_index = 0 };

    while (open_list_size > 0 && allocated_node_count < MAX_NODES_FOR_ASTAR_SEARCH * 2) {
        // 힙에서 가장 작은 f-score를 가진 노드 추출 (O(logN))
        PathNode* current_node = heap_pop(open_list_heap, &open_list_size);
        in_open_list_check[current_node->x][current_node->y] = false; // 힙에서 제거했으므로 false로

        // 목표 지점에 도달했는지 확인
        if (current_node->x == target_x && current_node->y == target_y) {
            PathNode* temp = current_node;
            int path_len = 0;
            while (temp != NULL && path_len < MAX_PATH_LENGTH) {
                result_path.path[path_len].X = temp->x;
                result_path.path[path_len].Y = temp->y;
                path_len++;
                temp = temp->parent;
            }

            result_path.count = path_len;

            // 경로를 역순으로 뒤집기
            for (int i = 0; i < result_path.count / 2; ++i) {
                COORD temp_coord = result_path.path[i];
                result_path.path[i] = result_path.path[result_path.count - 1 - i];
                result_path.path[result_path.count - 1 - i] = temp_coord;
            }

            // 할당된 모든 노드 메모리 해제
            for (int i = 0; i < allocated_node_count; ++i) {
                free(all_allocated_nodes[i]);
            }
            return result_path;
        }

        // 현재 노드를 폐쇄 목록에 추가 (O(1))
        close_list_visited[current_node->x][current_node->y] = true;

        // 인접 노드 탐색 (8방향)
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;

                int neighbor_x = current_node->x + dx;
                int neighbor_y = current_node->y + dy;

                if (!is_movable(neighbor_x, neighbor_y)) continue;

                // 폐쇄 목록에 이미 있다면 건너뛰기 (O(1))
                if (close_list_visited[neighbor_x][neighbor_y]) continue;

                int new_g = current_node->g + 1; // 대각선 이동 비용을 다르게 하려면 수정 필요

                // 이웃 노드가 이미 open_list에 있는지 O(1)로 확인
                if (in_open_list_check[neighbor_x][neighbor_y]) {
                    PathNode* existing_node = open_list_heap[open_list_index[neighbor_x][neighbor_y]];
                    if (new_g < existing_node->g) { // 더 나은 경로일 경우 업데이트
                        existing_node->g = new_g;
                        existing_node->f = existing_node->g + existing_node->h;
                        existing_node->parent = current_node;
                        heapify_up(open_list_heap, open_list_size, open_list_index[neighbor_x][neighbor_y]); // 힙 구조 재정렬
                    }
                }
                else { // 열린 목록에 없다면 새로 추가
                    PathNode* neighbor_node = Create_node(
                        neighbor_x, neighbor_y, new_g,
                        Calculate_Heuristic(neighbor_x, neighbor_y, target_x, target_y),
                        current_node
                    );
                    if (open_list_size < MAX_HEAP_SIZE) {
                        heap_push(open_list_heap, &open_list_size, neighbor_node);
                        in_open_list_check[neighbor_x][neighbor_y] = true;
                        open_list_index[neighbor_x][neighbor_y] = open_list_size - 1;
                        all_allocated_nodes[allocated_node_count++] = neighbor_node;
                    }
                    else {
                        fprintf(stderr, "오픈 리스트 용량 초과!\n");
                        free(neighbor_node);
                        break;
                    }
                }
            }
        }
    }

    // 경로를 찾지 못한 경우 할당된 메모리 해제
    for (int i = 0; i < allocated_node_count; ++i) {
        free(all_allocated_nodes[i]);
    }

    return result_path; // count가 0인 경로 반환
}

// 힙 관련 도우미 함수 구현
void swap_nodes(PathNode** a, PathNode** b) {
    PathNode* temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_up(PathNode** heap, int heap_size, int index) {
    while (index > 0) {
        int parent_index = (index - 1) / 2;
        if (heap[index]->f < heap[parent_index]->f) {
            swap_nodes(&heap[index], &heap[parent_index]);
            index = parent_index;
        }
        else {
            break;
        }
    }
}

void heapify_down(PathNode** heap, int heap_size, int index) {
    int min_index = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < heap_size && heap[left_child]->f < heap[min_index]->f) {
        min_index = left_child;
    }
    if (right_child < heap_size && heap[right_child]->f < heap[min_index]->f) {
        min_index = right_child;
    }

    if (min_index != index) {
        swap_nodes(&heap[index], &heap[min_index]);
        heapify_down(heap, heap_size, min_index);
    }
}

void heap_push(PathNode** heap, int* heap_size, PathNode* node) {
    if (*heap_size < MAX_HEAP_SIZE) {
        heap[*heap_size] = node;
        (*heap_size)++;
        heapify_up(heap, *heap_size, *heap_size - 1);
    }
}

PathNode* heap_pop(PathNode** heap, int* heap_size) {
    if (*heap_size <= 0) return NULL;
    PathNode* min_node = heap[0];
    (*heap_size)--;
    if (*heap_size > 0) {
        heap[0] = heap[*heap_size];
        heapify_down(heap, *heap_size, 0);
    }
    return min_node;
}