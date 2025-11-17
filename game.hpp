#pragma once

#include "date_time.hpp"
#include "ambient_BGM_state.hpp"

class Game {
private:
    static Game *instance_;

    bool exit_{};
    date_time elapsed_since_start_{};
    BGM current_BGM_state{};

    void render(void);
    void update_BGM(void);

public:
    static Game *instance(void) noexcept;

    Game(void);
    Game(const Game &) = delete;
    Game(Game &&) = delete;

    const date_time &elapsed_since_start(void) const noexcept;
    void exit(const bool _exit) noexcept;
    
    void update(void);

    Game operator=(const Game &) = delete;
    Game operator=(const Game &&) = delete;

    ~Game(void);
};