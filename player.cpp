#include "leak.hpp"
#include "player.hpp"

#include <string>
#include <format>

#define PLAYER_SPRITE_WIDTH 5
#define PLAYER_SPRITE_HEIGHT 5
#define ANIMATION_SPEED 10.0f

#define GRAVITY 50.0f
#define JUMP_STRENGTH -12.0f

#define HORIZONTAL_MOVE_COOLDOWN 0.08f

#define HP_BAR_WIDTH 20

Player g_player;

Player* Player::s_instance = nullptr;

static const cchar pPlayer_sprite_stand[PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    { { ' ', 0, 0 }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { ' ', 0, 0 } },
    { { L'█', BG::dark_yellow, FG::dark_yellow }, {L'█', BG::dark_yellow, FG::white}, {L'▀', BG::white, FG::dark_yellow}, {L'█', BG::dark_yellow, FG::white}, { L'█', BG::dark_yellow, FG::dark_yellow } },
    { { L'█', BG::dark_yellow, FG::dark_yellow }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { L'█', BG::dark_yellow, FG::dark_yellow } },
    { { ' ', 0, 0 }, { L'▓', BG::black, FG::gray }, { ' ', 0, 0 }, { L'▓', BG::black, FG::gray }, { ' ', 0, 0 } },
    { { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { ' ', 0, 0 } }
},
pPlayer_sprite_walk[2][PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    {
        { { ' ', 0, 0 }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { ' ', 0, 0 } },
        { { L'█', BG::dark_yellow, FG::dark_yellow }, {L'█', BG::dark_yellow, FG::white}, {L'▀', BG::white, FG::dark_yellow}, {L'█', BG::dark_yellow, FG::white}, { ' ' , 0, 0 } },
        { { ' ', 0, 0 }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { L'▓', BG::black, FG::gray }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 } }
    },
    {
        { { ' ', 0, 0 }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { ' ', 0, 0 } },
        { {L' '}, {L'█', BG::dark_yellow, FG::white}, {L'▀', BG::white, FG::dark_yellow}, {L'█', BG::dark_yellow, FG::white}, { L'█', BG::dark_yellow, FG::dark_yellow } },
        { { ' ', 0, 0 }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'▓', BG::black, FG::gray }, { ' ', 0, 0 } },
        { { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } }
    }
},
pPlayer_sprite_stand_armed[PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    { { ' ', 0, 0 }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { ' ', 0, 0 } },
    { { L'█', BG::dark_yellow, FG::dark_yellow }, {L'█', BG::dark_yellow, FG::white}, {L'▀', BG::white, FG::dark_yellow}, {L'█', BG::dark_yellow, FG::white}, { L'█', BG::dark_yellow, FG::dark_yellow } },
    { { L'█', BG::dark_yellow, FG::dark_yellow }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { ' ', 0, 0 } },
    { { ' ', 0, 0 }, { L'▓', BG::black, FG::gray }, { ' ', 0, 0 }, { L'▓', BG::black, FG::gray }, { ' ', 0, 0 } },
    { { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { ' ', 0, 0 } }
},
pPlayer_sprite_walk_armed[2][PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    {
        { { ' ', 0, 0 }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, {L'█', BG::dark_yellow, FG::white}, {L'▀', BG::white, FG::dark_yellow}, {L'█', BG::dark_yellow, FG::white}, { L'█', BG::dark_yellow, FG::dark_yellow } },
        { { ' ', 0, 0 }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { L'▓', BG::black, FG::gray }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 } }
    },
    {
        { { ' ', 0, 0 }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { L'▄', BG::dark_yellow, FG::dark_yellow }, { ' ', 0, 0 } },
        { { L'█', BG::dark_yellow, FG::dark_yellow }, {L'█', BG::dark_yellow, FG::white}, {L'▀', BG::white, FG::dark_yellow}, {L'█', BG::dark_yellow, FG::white}, { L'█', BG::dark_yellow, FG::dark_yellow } },
        { { ' ', 0, 0 }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { L'█', BG::black, FG::white }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'▓', BG::black, FG::gray }, { ' ', 0, 0 } },
        { { L'█', BG::black, FG::black }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } }
    }
};

bool Player::is_walkable(int x, int y) {
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y)
        return false;

    switch (map_get_block_info(x, y).type) {
    case BLOCK_AIR:
    case BLOCK_LOG:
    case BLOCK_LEAF:
    case BLOCK_WATER:
    case BLOCK_STAR:
        return true;
    }
    return false;
}

int Player::find_ground_position(int x) {
    for (int y = 0; y < map.size.y; ++y)
        if (!is_walkable(x, y))
            return y;
    return map.size.y / 2;
}

void Player::mouse_click_trampoline(bool is_left_click) {
    if (s_instance)
        s_instance->on_mouse_click(is_left_click);
}

void Player::map_offset_trampoline() {
    if (s_instance)
        s_instance->on_map_offset_change();
}

Player::Player() {
    s_instance = this;
}

Player::~Player() {
    if (s_instance == this)
        s_instance = nullptr;
}

void Player::initialize() {
    if (pSave_current) {
        m_x = pSave_current->x;
        m_y = pSave_current->y;
        m_HP = pSave_current->HP;
        m_max_HP = pSave_current->max_HP;
    }
    else {
        m_x = map.size.x / 2;
        m_y = find_ground_position(m_x);
        if (m_y - 1 >= 0)
            --m_y;
        m_HP = m_max_HP = 1000;
    }

    m_attack_power = 10;

    m_precise_y = static_cast<float>(m_y);
    m_velocity_y = 0.0f;
    m_is_on_ground = false;

    m_is_moving = 0;
    m_current_frame = 0;
    m_animation_timer = 0.0f;
    m_is_swinging = false;
    m_swing_timer = 0.0f;
    m_move_cool_down_timer = 0.0f;
    m_facing_direction = 1;

    Input::subscribe_input_mouse_click(mouse_click_trampoline);
    map_subscribe_offset_change(map_offset_trampoline);
}

void Player::update() {
    movement();
    update_damage_texts();

    const float delta = DeltaTime::delta_time();

    if (m_is_swinging) {
        m_swing_timer -= delta;
        if (m_swing_timer <= 0.0f)
            m_is_swinging = false;
    }

    if (!is_walkable(m_x, m_y + 1)) {
        m_is_on_ground = true;
        if (m_velocity_y > 0)
            m_velocity_y = 0;
    }
    else {
        m_is_on_ground = false;
    }

    if (!m_is_on_ground)
        m_velocity_y += GRAVITY * delta;

    m_precise_y += m_velocity_y * delta;

    int new_y = static_cast<int>(m_precise_y);

    if (new_y > m_y) {
        while (!is_walkable(m_x, new_y)) {
            --new_y;
            m_velocity_y = 0;
            m_precise_y = static_cast<float>(new_y);
            m_is_on_ground = true;
        }
    }
    else if (new_y < m_y) {
        while (!is_walkable(m_x, new_y)) {
            ++new_y;
            m_velocity_y = 0;
            m_precise_y = static_cast<float>(new_y);
        }
    }
    m_y = new_y;

    if (m_is_moving) {
        m_animation_timer += delta;
        if (m_animation_timer >= 1.0f / ANIMATION_SPEED) {
            m_animation_timer = 0.0f;
            m_current_frame = (m_current_frame + 1) % 2;
        }
    }
    else {
        m_current_frame = 0;
        m_animation_timer = 0.0f;
    }
}

void Player::render() {
    const COORD center_position{
        .X = static_cast<SHORT>(Console::size().X / 2),
        .Y = static_cast<SHORT>(Console::size().Y / 2)
    };

    const player_item_t* pEquipped_item = inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item;
    const item_information_t* pToolInfo = nullptr;
    bool is_tool_equipped = false;

    if (pEquipped_item && pEquipped_item->quantity > 0) {
        pToolInfo = database_find_item_by_index(pEquipped_item->item_DB_index);
        if (pToolInfo && pToolInfo->type == ITEM_TYPE_TOOL)
            is_tool_equipped = true;
    }

    const cchar(*pCurrent_sprite)[PLAYER_SPRITE_WIDTH];
    if (is_tool_equipped)
        pCurrent_sprite = m_is_moving ? pPlayer_sprite_walk_armed[m_current_frame] : pPlayer_sprite_stand_armed;
    else
        pCurrent_sprite = m_is_moving ? pPlayer_sprite_walk[m_current_frame] : pPlayer_sprite_stand;

    for (int y = 0; y < PLAYER_SPRITE_HEIGHT; ++y)
        for (int x = 0; x < PLAYER_SPRITE_WIDTH; ++x) {
            cchar pixel = pCurrent_sprite[y][(m_facing_direction == 1) ? x : (PLAYER_SPRITE_WIDTH - 1 - x)];

            if (pixel.character == ' ' && pixel.background == BG::black)
                continue;

            const COORD draw_position{
                .X = static_cast<SHORT>(center_position.X + x - (PLAYER_SPRITE_WIDTH / 2)),
                .Y = static_cast<SHORT>(center_position.Y + y - (PLAYER_SPRITE_HEIGHT / 2))
            };

            if (draw_position.X >= 0 && draw_position.X < Console::size().X &&
                draw_position.Y >= 0 && draw_position.Y < Console::size().Y)
                Console::print(pixel, draw_position);
        }

    if (is_tool_equipped) {
        int tool_hand_offset_x = 0, tool_hand_offset_y = 0;

        if (m_is_swinging) {
            tool_hand_offset_x = (m_facing_direction == 1) ? 6 : -TEXTURE_SIZE - 1;
            tool_hand_offset_y = 0;
        }
        else {
            tool_hand_offset_x = (m_facing_direction == 1) ? 4 : -TEXTURE_SIZE + 1;
            tool_hand_offset_y = -1;
        }

        for (int y = 0; y < TEXTURE_SIZE; ++y)
            for (int x = 0; x < TEXTURE_SIZE; ++x) {
                int source_x = (m_facing_direction == 1) ? x : (TEXTURE_SIZE - 1 - x);
                cchar tool_pixel{};
                if (m_is_swinging)
                    tool_pixel = tool_get_swing_texture((tool_t)pToolInfo->index, source_x, y);
                else
                    tool_pixel = tool_get_texture((tool_t)pToolInfo->index, source_x, y);

                if (tool_pixel.foreground == FG::transparent)
                    continue;

                COORD draw_pos{
                    static_cast<SHORT>(center_position.X + (tool_hand_offset_x - PLAYER_SPRITE_WIDTH / 2) + x),
                    static_cast<SHORT>(center_position.Y + (tool_hand_offset_y - PLAYER_SPRITE_HEIGHT / 2) + y)
                };

                if (draw_pos.X >= 0 && draw_pos.X < Console::size().X &&
                    draw_pos.Y >= 0 && draw_pos.Y < Console::size().Y)
                    Console::print(tool_pixel, draw_pos);
            }
    }

    render_damage_texts();

    const int bar_width = HP_BAR_WIDTH;
    const int current_HP = m_HP < 0 ? 0 : m_HP;
    const int max_HP = m_max_HP > 0 ? m_max_HP : 1;
    const int filled = (current_HP * bar_width) / max_HP;
    const int empty = bar_width - filled;

    COORD position{
        .X = static_cast<SHORT>(Console::size().X - (bar_width + 22)),
        .Y = 2
    };

    Console::print("[", position, BG::white, FG::transparent);
    position.X += 1;

    for (int i = 0; i < filled; ++i) {
        Console::print(" ", position, BG::red, FG::black);
        position.X += 1;
    }

    for (int i = 0; i < empty; ++i) {
        Console::print(" ", position, BG::dark_gray, FG::black);
        position.X += 1;
    }

    Console::print("]", position, BG::white, FG::transparent);
    position.X += 2;

    Console::print(std::format("HP: {} / {}", current_HP, max_HP), position, BG::black, FG::yellow);
}

void Player::destroy() {
    Input::unsubscribe_input_mouse_click(mouse_click_trampoline);
    map_unsubscribe_offset_change(map_offset_trampoline);
}

void Player::take_damage(int damage) {
    m_HP -= damage;
    if (m_HP < 0)
        m_HP = 0;

    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i)
        if (!m_damage_texts[i].active) {
            m_damage_texts[i].active = true;
            m_damage_texts[i].damage_value = damage;
            m_damage_texts[i].precise_y = static_cast<float>(m_y);
            m_damage_texts[i].timer = 1.5f;
            break;
        }

    sound_play_sound_effect(PLAYER_SOUND_HURT);
}

void Player::add_health(int additional_health) {
    m_HP += additional_health;
    if (m_HP > m_max_HP)
        m_HP = m_max_HP;
}

void Player::save() {
    if (!pSave_current)
        save_instantiate();

    pSave_current->x = m_x;
    pSave_current->y = m_y;
    pSave_current->HP = m_HP;
    pSave_current->max_HP = m_max_HP;
}

void Player::movement() {
    if (Input::is_key_down(VK_SPACE) && m_is_on_ground) {
        m_velocity_y = JUMP_STRENGTH;
        m_is_on_ground = false;
    }

    m_move_cool_down_timer += DeltaTime::delta_time();

    const bool is_a_down = Input::is_key_down('A');
    const bool is_d_down = Input::is_key_down('D');

    m_is_moving = is_a_down || is_d_down;

    if (m_move_cool_down_timer >= HORIZONTAL_MOVE_COOLDOWN) {
        bool moved_horizontally = false;

        if (is_a_down && !is_d_down) {
            int new_x = m_x - 1;
            if (is_walkable(new_x, m_y)) {
                sound_play_sound_effect(PLAYER_SOUND_FOOTSTEP);
                m_x = new_x;
                moved_horizontally = true;
                m_facing_direction = -1;
            }
        }
        else if (is_d_down && !is_a_down) {
            int new_x = m_x + 1;
            if (is_walkable(new_x, m_y)) {
                sound_play_sound_effect(PLAYER_SOUND_FOOTSTEP);
                m_x = new_x;
                moved_horizontally = true;
                m_facing_direction = 1;
            }
        }

        if (moved_horizontally)
            m_move_cool_down_timer = 0.0f;
    }
}

void Player::update_damage_texts() {
    const float delta = DeltaTime::delta_time();
    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i)
        if (m_damage_texts[i].active) {
            m_damage_texts[i].precise_y -= delta * 5.0f;
            m_damage_texts[i].timer -= delta;
            if (m_damage_texts[i].timer <= 0.0f)
                m_damage_texts[i].active = false;
        }
}

void Player::render_damage_texts() {
    const COORD center_position{
        .X = static_cast<SHORT>(Console::size().X / 2),
        .Y = static_cast<SHORT>(Console::size().Y / 2)
    };

    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i)
        if (m_damage_texts[i].active) {
            COORD draw_position{
                .X = center_position.X,
                .Y = static_cast<SHORT>(center_position.Y - (PLAYER_SPRITE_HEIGHT / 2) - 1 - (m_precise_y - m_damage_texts[i].precise_y))
            };

            const std::string text = std::format(" Hit - {} ", m_damage_texts[i].damage_value);
            draw_position.X -= static_cast<SHORT>(text.length() / 2);
            Console::print(text, draw_position, BG::black, FG::red);
        }
}

void Player::swing_tool() {
    if (m_is_swinging)
        return;

    const player_item_t* pEquipped_item = inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item;
    if (pEquipped_item && pEquipped_item->quantity > 0) {
        const item_information_t* pToolInfo = database_find_item_by_index(pEquipped_item->item_DB_index);
        if (pToolInfo && pToolInfo->type == ITEM_TYPE_TOOL) {
            m_is_swinging = true;
            m_swing_timer = 0.2f;
        }
    }
}

void Player::on_mouse_click(bool is_left_click) {
    if (is_left_click)
        swing_tool();
    sound_play_sound_effect(PLAYER_SOUND_SWING);
}

void Player::on_map_offset_change() {
    m_x += map.offset_x;
}