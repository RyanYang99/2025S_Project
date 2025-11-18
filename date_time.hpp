#pragma once

class date_time {
private:
    int day_{}, hour_{ 12 }, minute_{};
    float second_{};

public:
    int day(void) const noexcept;
    int hour(void) const noexcept;
    int minute(void) const noexcept;
    float second(void) const noexcept;

    void set_local_time(void) noexcept;

    void update(void) noexcept;
    void render(void);

    bool is_night(void) const noexcept;

    void save(void) const;
};