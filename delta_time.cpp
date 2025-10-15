#include "leak.hpp"
#include "delta_time.hpp"

#include <ctime>

float delta_time_t::delta_time{};

void delta_time_t::update(void) noexcept {
    static clock_t last_time{};
    const clock_t current_time{ clock() };

    if (!last_time)
        last_time = current_time;
    else {
        delta_time = static_cast<float>(current_time - last_time) / CLOCKS_PER_SEC;
        last_time = current_time;
    }
}