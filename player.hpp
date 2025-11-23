#pragma once

#include <Windows.h>

// 기존에 필요하던 헤더들
#include "map.hpp"
#include "tool.hpp"
#include "save.hpp"
#include "input.hpp"
#include "delta_time.hpp"
#include "sound.hpp"
#include "console.hpp"
#include "formatter.hpp"

#define MAX_DAMAGE_TEXTS 10

class Player {
public:
    Player();
    ~Player();

    void initialize();
    void update();
    void render();
    void destroy();

    void take_damage(int damage);
    void add_health(int additional_health);
    void save();

    int x() const { return m_x; }
    int y() const { return m_y; }
    int hp() const { return m_HP; }
    int max_hp() const { return m_max_HP; }

    void set_position(int x, int y) { m_x = x; m_y = y; m_precise_y = static_cast<float>(y); }

    static int find_ground_position(int x);

private:
    struct DamageText {
        int   damage_value{};
        float precise_y{};
        float timer{};
        bool  active{};
    };

    int m_x{}, m_y{};
    int m_max_HP{}, m_HP{}, m_attack_power{};

    int   m_is_moving{};
    float m_animation_timer{};
    int   m_current_frame{};
    int   m_facing_direction{ 1 };
    bool  m_is_swinging{};
    float m_swing_timer{};

    float m_precise_y{};
    float m_velocity_y{};
    bool  m_is_on_ground{};

    float m_move_cool_down_timer{};

    DamageText m_damage_texts[MAX_DAMAGE_TEXTS]{};

private:
    void movement();
    void update_damage_texts();
    void render_damage_texts();
    void swing_tool();
    void on_mouse_click(bool is_left_click);
    void on_map_offset_change();

    static bool is_walkable(int x, int y);

    static void mouse_click_trampoline(bool is_left_click);
    static void map_offset_trampoline();

    static Player* s_instance;
};

extern Player g_player;