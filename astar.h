#pragma once

#include <stdbool.h>

typedef enum {
    DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_RIGHT,
    DIRECTION_DOWN,
    DIRECTION_LEFT
} direction_t;

// 이동 가능 여부를 확인하는 함수 포인터 타입을 정의
typedef bool (*is_movable_t)(const int x, const int y);

const direction_t find_next_direction(const int start_x,
                                      const int start_y,
                                      const int target_x,
                                      const int target_y,
                                      const is_movable_t is_movable);

void destroy_astar(void);