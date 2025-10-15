#pragma once

class delta_time_t {
public:
    static float delta_time;

    static void update(void) noexcept;
};