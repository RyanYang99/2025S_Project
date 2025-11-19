#include "leak.hpp"
#include "delta_time.hpp"

#include <ctime>

float DeltaTime::delta_time_{};

float DeltaTime::delta_time(void) noexcept {
    return delta_time_;
}

void DeltaTime::update(void) noexcept {
    static clock_t last_time{};
    const clock_t current_time{ clock() };

    if (!last_time)
        last_time = current_time;
    else {
        delta_time_ = static_cast<float>(current_time - last_time) / CLOCKS_PER_SEC;
        last_time = current_time;
    }
}