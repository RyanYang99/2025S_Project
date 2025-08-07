#pragma once

#include <stdbool.h>

#include <windows.h> 
#include "map.h"

#define MAX_PATH_LENGTH 50 
#define MAX_NODES_FOR_ASTAR_SEARCH 2000
#define MAX_HEAP_SIZE MAX_NODES_FOR_ASTAR_SEARCH

// A* 경로 탐색의 결과를 담는 구조체
typedef struct {
    COORD path[MAX_PATH_LENGTH];
    int count;          // 경로에 포함된 좌표의 총 개수
    int current_index;  // 현재 따라가고 있는 경로의 인덱스
} path_t;

// 이동 가능 여부를 확인하는 함수 포인터 타입을 정의
typedef bool (*IsMovableFunc)(int, int);

// A* 경로 찾기 함수
path_t find_path(int start_x, int start_y, int target_x, int target_y, IsMovableFunc is_movable);

void destroy_astar(void);