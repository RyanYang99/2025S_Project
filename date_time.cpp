#include "leak.hpp"
#include "date_time.hpp"

#include <ctime>
#include <string>
#include <format>

#include "save.hpp"
#include "delta_time.hpp"
#include "console.hpp"

int date_time::day(void) const noexcept {
    return day_;
}

int date_time::hour(void) const noexcept {
    return hour_;
}

int date_time::minute(void) const noexcept {
    return minute_;
}

float date_time::second(void) const noexcept {
    return second_;
}

void date_time::set_local_time(void) noexcept {
    const time_t now{ time(nullptr) };
    tm time_information{};
    localtime_s(&time_information, &now);

    hour_ = time_information.tm_hour;
}

void date_time::update(void) noexcept {
    second_ += delta_time_t::delta_time * (86400.0f / 1200.0f); //1일당 게임 초 / 1일당 실제 초
    //date_time_elapsed_since_start.second += delta_time_t::delta_time * (86400.0f / 60.0f);

    if (second_ >= 60.0f) {
        const int minutes = static_cast<int>(second_ / 60.0f);

        minute_ += minutes;
        second_ -= minutes * 60.0f;
    }

    if (minute_ >= 60) {
        hour_ += minute_ / 60;
        minute_ %= 60;
    }

    if (hour_ >= 24) {
        day_ += hour_ / 24;
        hour_ %= 24;
    }
}

void date_time::render(void) {
    static float blink{};
    static char blink_character{ ' ' };

    blink += delta_time_t::delta_time;
    if (blink >= 2.0f) {
        blink = 0.0f;
        blink_character = ' ';
    } else if (blink >= 1.0f)
        blink_character = ':';

    const std::string sDay{ std::format("Day {}", day_) },
                      time{ std::format("{}{}{}", hour_, blink_character, minute_) };

    COORD position = { static_cast<SHORT>(console_size.X - sDay.length()), 0 };
    console_fprint_string(sDay.c_str(), position, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);

    position.X = static_cast<SHORT>(console_size.X - time.length());
    ++position.Y;
    console_fprint_string(time.c_str(), position, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);
}

bool date_time::is_night(void) const noexcept {
    return hour_ >= 21 || hour_ <= 6;
}

void date_time::save(void) const noexcept {
    if (!pSave_current)
        save_instantiate();

    pSave_current->game_time = *this;
}