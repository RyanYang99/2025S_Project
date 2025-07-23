
#pragma once

#include <stdbool.h> // bool을 사용하기 위해 필요
#include <windows.h> // COORD를 사용하기 위해 필요 (Mob.h에서도 사용)
#include "map.h"     // map_t 구조체 및 block_t 열거형 사용을 위해 필요

// A* 알고리즘을 위한 노드 구조체
typedef struct PathNode {
    int x, y;           // 노드의 맵 좌표
    int g;              // 시작점에서 현재 노드까지의 실제 비용
    int h;              // 현재 노드에서 목표까지의 추정 비용 (휴리스틱)
    int f;              // 총 비용 (g + h)
    struct PathNode* parent; // 경로 재구성을 위한 부모 노드 포인터
} PathNode;

// 노드 비교 함수 (qsort 사용을 위해)
int Compare_nodes(const void* a, const void* b);

// PathNode 메모리 할당 및 초기화
PathNode* Create_node(int x, int y, int g, int h, PathNode* parent);

// 휴리스틱 함수 (맨해튼 거리)
int Calculate_Heuristic(int x1, int y1, int x2, int y2);

// 해당 좌표가 맵 범위 내에 있고 이동 가능한 블록인지 확인
bool is_valid_block(int x, int y, int map_size_x, int map_size_y);

// A* 경로 찾기 함수: 다음 이동할 한 칸의 좌표를 반환합니다.
// 경로를 찾지 못하면 {-1, -1}을 반환합니다.
// map_t의 크기를 사용하기 위해 map_size_x와 map_size_y를 인자로 받습니다.
COORD find_path_next_step(int start_x, int start_y, int target_x, int target_y, int map_size_x, int map_size_y);