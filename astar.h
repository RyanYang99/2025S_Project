#pragma once

#include "map.h"
#include <stdbool.h>
#include <windows.h> 


#define MAX_PATH_LENGTH 50 
#define MAX_NODES_FOR_ASTAR_SEARCH 2000
#define MAX_HEAP_SIZE MAX_NODES_FOR_ASTAR_SEARCH


// A* 경로 탐색의 결과를 담는 구조체
typedef struct {
    COORD path[MAX_PATH_LENGTH];
    int count;          // 경로에 포함된 좌표의 총 개수
    int current_index;  // 현재 따라가고 있는 경로의 인덱스
} path_t;


// A* 알고리즘을 위한 노드 구조체
typedef struct PathNode {
    int x, y;
    int g;
    int h;
    int f;
    struct PathNode* parent;
} PathNode;

// PathNode 메모리 할당 및 초기화
PathNode* Create_node(int x, int y, int g, int h, PathNode* parent);

// 휴리스틱 함수 (맨해튼 거리)
int Calculate_Heuristic(int x1, int y1, int x2, int y2);

// 이동 가능 여부를 확인하는 함수 포인터 타입을 정의
typedef bool (*IsMovableFunc)(int, int);

// A* 경로 찾기 함수
path_t find_path(int start_x, int start_y, int target_x, int target_y, IsMovableFunc is_movable);

void heap_push(PathNode** heap, int* heap_size, PathNode* node, int** open_list_index);
PathNode* heap_pop(PathNode** heap, int* heap_size, int** open_list_index);
void heapify_down(PathNode** heap, int heap_size, int index, int** open_list_index);
void heapify_up(PathNode** heap, int heap_size, int index, int** open_list_index);
void swap_nodes(PathNode** a, PathNode** b, int** open_list_index);