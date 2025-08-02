#pragma once

#include "map.h" // 맵 관련 정의를 사용하기 위해 포함
#include <stdbool.h>
#include <windows.h> // COORD를 사용하기 위해 필요


#define MAX_PATH_LENGTH 50 


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

// 이동 가능 여부를 확인하는 함수 포인터 타입을 정의합니다.
typedef bool (*IsMovableFunc)(int, int);

// A* 경로 찾기 함수: 이제 이동 가능 여부 함수(is_movable)를 인수로 받습니다.
path_t find_path(int start_x, int start_y, int target_x, int target_y, IsMovableFunc is_movable);

