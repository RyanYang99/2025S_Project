#pragma once

#include "date_time.hpp"
#include "ambient_BGM_state.hpp"

class game {
private:
    static game *instance_;

    bool exit_{};
    date_time elapsed_since_start_{};
    ambient_BGM_state_t current_BGM_state{};

    void render(void);
    void update_BGM(void);

public:
    static game *instance(void) noexcept;

    game(void);
    game(const game &) = delete;
    game(game &&) = delete;

    const date_time &elapsed_since_start(void) const noexcept;
    void exit(const bool _exit) noexcept;
    
    void update(void);

    game operator=(const game &) = delete;
    game operator=(const game &&) = delete;

    ~game(void);
};