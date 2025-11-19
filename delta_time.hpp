#pragma once

class DeltaTime {
private:
    static float delta_time_;

public:
    static float delta_time(void) noexcept;
    static void update(void) noexcept;
};