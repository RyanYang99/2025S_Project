#pragma once

namespace astar {
    enum class direction {
        none,
        up,
        right,
        down,
        left
    };

    //이동 가능 여부를 확인하는 함수 포인터 타입을 정의
    typedef bool (*is_movable)(const int x, const int y);

    direction find_next_direction(const int start_x,
                                  const int start_y,
                                  const int target_x,
                                  const int target_y,
                                  const is_movable is_movable_) noexcept;
}