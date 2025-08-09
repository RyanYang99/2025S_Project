#include "leak.h"
#include "player.h"

#include "Tool.h"
#include "map.h"
#include "save.h"
#include "input.h"
#include "delta.h"
#include "console.h"
#include <stdio.h> //swprintf 사용하기위해
#include <wchar.h>
#include <time.h>
#include <conio.h>
#include <Windows.h> //VK_SPACE 사용을 위해 추가
#include "sound.h" //발소리 추가
#include "formatter.h"

#define PLAYER_SPRITE_WIDTH 5
#define PLAYER_SPRITE_HEIGHT 5
#define ANIMATION_SPEED 10.0f //1초에 4번 프레임 변경

#define GRAVITY 50.0f         //중력 가속도
#define JUMP_STRENGTH -12.0f  //점프 시 부여되는 초기 수직 속도

// 수평 이동 속도 조절 (값이 작을수록 빨라짐)
#define HORIZONTAL_MOVE_COOLDOWN 0.08f  //약 1초에 12.5칸 이동8

#define HP_BAR_WIDTH 20

//데미지 텍스트 효과 추가
typedef struct {
    int damage_value;
    float precise_y;
    float timer;
    bool active;
} damage_text_t;

player_t player = { 0 };

static damage_text_t pDamage_texts[MAX_DAMAGE_TEXTS] = { 0 };

/*
    2개의 애니메이션 프레임
    프레임 1: 기본 서 있는 자세
*/
static const color_character_t pPlayer_sprite_stand[PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    //머리 (위:머리카락, 아래:피부)
    { { ' ', 0, 0 }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { ' ', 0, 0 } },
    //몸통과 팔
    { { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'▀', BACKGROUND_T_WHITE, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW } },
    //허리, 바지
    { { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW } },
    //다리
    { { ' ', 0, 0 }, { L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY }, { ' ', 0, 0 }, { L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY }, { ' ', 0, 0 } },
    //신발
    { { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK }, { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK }, { ' ', 0, 0 } }
},
//걷기 애니메이션 (2 프레임)
pPlayer_sprite_walk[2][PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    //프레임 0
    {
        { { ' ', 0, 0 }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { ' ', 0, 0 } },
        { { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'▀', BACKGROUND_T_WHITE, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, { ' ' , 0, 0 } },
        { { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK }, { ' ', 0, 0 }, { ' ', 0, 0 } }
    },
    //프레임 1
    {
        { { ' ', 0, 0 }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { ' ', 0, 0 } },
        { {L' '}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'▀', BACKGROUND_T_WHITE, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW } },
        { { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY }, { ' ', 0, 0 } },
        { { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } }
    }
},
//무장하고 서 있는 자세
pPlayer_sprite_stand_armed[PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    //머리
    { { ' ', 0, 0 }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { ' ', 0, 0 } },
    //몸통과 옆으로 뻗은 오른팔
    { { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'▀', BACKGROUND_T_WHITE, FOREGROUND_T_DARKYELLOW}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW } },
    //허리, 바지
    { { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { ' ', 0, 0 } },
    //다리 
    { { ' ', 0, 0 }, { L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY }, { ' ', 0, 0 }, { L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY }, { ' ', 0, 0 } },
    //신발
    { { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK }, { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK }, { ' ', 0, 0 } }
},
//무장하고 걷는 애니메이션
pPlayer_sprite_walk_armed[2][PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    //프레임 1
    {
        { { ' ', 0, 0 }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { ' ', 0, 0 } },
        //왼팔 뒤로, 오른팔은 고정
        { { ' ', 0, 0 }, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'▀', BACKGROUND_T_WHITE, FOREGROUND_T_DARKYELLOW}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW } },
        { { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK }, { ' ', 0, 0 }, { ' ', 0, 0 } }
    },
    //프레임 2
    {
        { { ' ', 0, 0 }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, { ' ', 0, 0 } },
        //왼팔 앞으로, 오른팔은 고정
        { { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW }, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'▀', BACKGROUND_T_WHITE, FOREGROUND_T_DARKYELLOW}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, { L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW } },
        { { ' ', 0, 0 }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE }, { ' ', 0, 0 } },
        { { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY }, { ' ', 0, 0 } },
        { { L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 }, { ' ', 0, 0 } }
    }
};

//플레이어의 도구 스윙
static void player_swing_tool(void) {
    //이미 스윙 중이거나, 도구를 들고 있지 않으면 실행하지 않음
    if (player.is_swinging)
        return;

    const player_item_t * const pEquipped_item = inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item;
    if (pEquipped_item && pEquipped_item->quantity > 0) {
        const item_information_t * const pToolInfo = database_find_item_by_index(pEquipped_item->item_DB_index);
        if (pToolInfo && pToolInfo->type == ITEM_TYPE_TOOL) {
            player.is_swinging = true;
            player.swing_timer = 0.2f; //0.2초 동안 스윙 애니메이션 표시
        }
    }
}

//좌 클릭을 했는지
static void handle_player_actions(const bool is_left_click) {
    if (is_left_click)
        player_swing_tool();
}

static void update_player_offset(void) {
    player.x += map.offset_x;
}

void player_initialize(void) {
    if (pSave_current) {
        player.x = pSave_current->x;
        player.y = pSave_current->y;
        player.HP = pSave_current->HP;
    } else {
        player.x = map.size.x / 2;
        player.y = player_find_ground_position(player.x);
        if (player.y - 1 >= 0)
            --player.y; //가능할 시 찾은 블록 위로 설정

        player.max_HP = 1000;
        player.HP = 1000; //초기 체력
        player.attack_power = 10;

        // 물리 변수 초기화
        player.precise_y = (float)player.y;
        player.velocity_y = 0.0f;
        player.is_on_ground = false; //시작 시 공중에서 떨어지도록

        // 애니메이션 변수 초기화
        player.is_moving = 0;
        player.current_frame = 0;
        player.animation_timer = 0.0f;
        player.is_swinging = false; //스윙 상태 초기화
        player.swing_timer = 0.0f;  //스윙 타이머 초기화

        //이동 쿨다운 타이머 초기화
        player.move_cool_down_timer = 0.0f;

        //초기 방향: 오른쪽
        player.facing_direction = 1;

        //마우스 클릭
        input_subscribe_mouse_click(handle_player_actions);
        map_subscribe_offset_change(update_player_offset);
    }
}

//충돌 감지 함수 구현
static const bool is_walkable(const int x, const int y) {
    //맵 경계 체크
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y)
        return false;

    //블록 타입에 따른 이동 가능 여부 판단
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

static void movement(void) {
    //점프 키 확인
    if (is_key_down(VK_SPACE) && player.is_on_ground) {
        player.velocity_y = JUMP_STRENGTH;
        player.is_on_ground = false;
    }

    //쿨다운 타이머 업데이트
    player.move_cool_down_timer += delta_time;

    const bool is_a_down = is_key_down('A'), is_d_down = is_key_down('D');

    //키가 눌렸는지 여부에 따라 애니메이션 상태 설정
    player.is_moving = is_a_down || is_d_down;

    //쿨타임이 다 되었는지 확인
    if (player.move_cool_down_timer >= HORIZONTAL_MOVE_COOLDOWN) {
        bool moved_horizontally = false;

        //왼쪽 이동 (D키와 동시에 눌리면 무시)
        if (is_a_down && !is_d_down) {
            int new_x = player.x - 1;
            if (is_walkable(new_x, player.y)) {
                sound_play_footstep();
                player.x = new_x;
                moved_horizontally = true;
                player.facing_direction = -1; //왼쪽 보기
            }
        } else if (is_d_down && !is_a_down) { //오른쪽 이동 (A키와 동시에 눌리면 무시)
            int new_x = player.x + 1;
            if (is_walkable(new_x, player.y)) {
                sound_play_footstep();
                player.x = new_x;
                moved_horizontally = true;
                player.facing_direction = 1; //오른쪽 보기
            }
        }

        //실제로 이동이 일어났다면 타이머를 리셋
        if (moved_horizontally)
            player.move_cool_down_timer = 0.0f;
    }
}

static void update_damage_texts(void) {
    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i) {
        if (pDamage_texts[i].active) {
            //위로 움직이는 효과
            pDamage_texts[i].precise_y -= delta_time * 5.0f; //움직이는 속도

            //타이머 감소
            pDamage_texts[i].timer -= delta_time;
            if (pDamage_texts[i].timer <= 0.0f) 
                pDamage_texts[i].active = false; //시간이 지나면 비활성화
        }
    }
}

void player_update(void) {
    //입력 처리
    movement();

    //데미지 출력 업데이트
    update_damage_texts();

    //스윙 타이머 업데이트
    if (player.is_swinging) {
        player.swing_timer -= delta_time;
        if (player.swing_timer <= 0.0f)
            player.is_swinging = false;
    }

    /*
        물리 업데이트
        땅에 있는지 확인
        플레이어 바로 아래 블록이 걸을 수 없는 블록인지 확인
    */
    if (!is_walkable(player.x, player.y + 1)) {
        player.is_on_ground = true;
        //땅에 있다면 수직 속도 초기화
        if (player.velocity_y > 0)
            player.velocity_y = 0;
    } else
        player.is_on_ground = false;

    //중력 적용 (공중에 있을 때만)
    if (!player.is_on_ground)
        player.velocity_y += GRAVITY * delta_time;

    //속도에 따라 정밀 y좌표 업데이트
    player.precise_y += player.velocity_y * delta_time;

    //정밀 y좌표를 정수 y좌표로 변환하여 충돌 처리
    int new_y = (int)player.precise_y;

    //수직 이동에 대한 충돌 처리
    if (new_y > player.y) { //아래로 이동 시
        while (!is_walkable(player.x, new_y)) {
            --new_y;
            player.velocity_y = 0;
            player.precise_y = (float)new_y;
            player.is_on_ground = true;
        }
    } else if (new_y < player.y) { //위로 이동 시
        while (!is_walkable(player.x, new_y)) {
            ++new_y;
            player.velocity_y = 0;
            player.precise_y = (float)new_y;
        }
    }
    player.y = new_y;

    //3. 애니메이션 업데이트
    if (player.is_moving) {
        player.animation_timer += delta_time;
        if (player.animation_timer >= 1.0f / ANIMATION_SPEED) {
            player.animation_timer = 0.0f;
            player.current_frame = (player.current_frame + 1) % 2;
        }
    } else {
        player.current_frame = 0;
        player.animation_timer = 0.0f;
    }
}

static void render_damage_texts(void) {
    const COORD center_position = {
        .X = console_size.X / 2,
        .Y = console_size.Y / 2
    };

    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i) {
        if (pDamage_texts[i].active) {
            COORD draw_position = {
                .X = center_position.X,
                .Y = (SHORT)(center_position.Y - (PLAYER_SPRITE_HEIGHT / 2) - 1 - (player.precise_y - pDamage_texts[i].precise_y))
            };

            char * const pText = format_string(" Hit - %d! ", pDamage_texts[i].damage_value);
            draw_position.X -= (SHORT)(strlen(pText) / 2);
            console_fprint_string(pText, draw_position, BACKGROUND_T_BLACK, FOREGROUND_T_RED);
            free(pText);
        }
    }
}

void player_render(void) {
    // 플레이어의 중심이 될 콘솔 위치 (화면 중앙)
    const COORD center_position = {
        .X = console_size.X / 2,
        .Y = console_size.Y / 2
    };

    //1. 장착한 아이템 확인
    const player_item_t * const pEquipped_item = inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item;
    const item_information_t *pToolInfo = NULL;
    bool is_tool_equipped = false;

    if (pEquipped_item && pEquipped_item->quantity > 0) {
        pToolInfo = database_find_item_by_index(pEquipped_item->item_DB_index);
        if (pToolInfo && pToolInfo->type == ITEM_TYPE_TOOL)
            is_tool_equipped = true;
    }

    //2. 상태에 맞는 스프라이트 선택
    const color_character_t(* pCurrent_sprite)[PLAYER_SPRITE_WIDTH];
    if (is_tool_equipped)
        pCurrent_sprite = player.is_moving ? pPlayer_sprite_walk_armed[player.current_frame] : pPlayer_sprite_stand_armed;
    else
        pCurrent_sprite = player.is_moving ? pPlayer_sprite_walk[player.current_frame] : pPlayer_sprite_stand;

    //3. 플레이어 스프라이트 렌더링 (좌우 반전 적용)
    for (int y = 0; y < PLAYER_SPRITE_HEIGHT; ++y) {
        for (int x = 0; x < PLAYER_SPRITE_WIDTH; ++x) {
            color_character_t pixel = pCurrent_sprite[y][(player.facing_direction == 1) ? x : (PLAYER_SPRITE_WIDTH - 1 - x)];

            if (pixel.character == ' ' && pixel.background == 0)
                continue;

            const COORD draw_position = {
                .X = (SHORT)(center_position.X + x - (PLAYER_SPRITE_WIDTH / 2)),
                .Y = (SHORT)(center_position.Y + y - (PLAYER_SPRITE_HEIGHT / 2))
            };

            if (draw_position.X >= 0 && draw_position.X < console_size.X &&
                draw_position.Y >= 0 && draw_position.Y < console_size.Y) {
                console_print_color_character(pixel, draw_position);
            }
        }
    }

    //4. 장착한 도구 렌더링 (좌우 반전 적용)
    if (is_tool_equipped) {
        // 도구를 들 손의 위치 계산
        int tool_hand_offset_x = 0, tool_hand_offset_y = 0;

        //스윙 상태에 따라 도구의 렌더링 위치를 조정
        if (player.is_swinging) {
            //스윙 애니메이션일 때의 위치
            tool_hand_offset_x = (player.facing_direction == 1) ? 6 : -TEXTURE_SIZE - 1;
            tool_hand_offset_y = 0;
        } else {
            //평상시 도구를 들고 있을 때의 위치
            tool_hand_offset_x = (player.facing_direction == 1) ? 4 : -TEXTURE_SIZE + 1;
            tool_hand_offset_y = -1;
        }

        for (int y = 0; y < TEXTURE_SIZE; ++y)
            for (int x = 0; x < TEXTURE_SIZE; ++x) {
                int source_x = (player.facing_direction == 1) ? x : (TEXTURE_SIZE - 1 - x);
                color_character_t tool_pixel = { 0 };
                if (player.is_swinging) {
                    //스윙 중일 때는 스윙 텍스처를 가져옴
                    tool_pixel = tool_get_swing_texture((tool_t)pToolInfo->index, source_x, y);
                } else {
                    //평상시에는 기본 도구 텍스처를 가져옴
                    tool_pixel = tool_get_texture((tool_t)pToolInfo->index, source_x, y);
                }

                if (tool_pixel.foreground == FOREGROUND_T_TRANSPARENT) continue;

                COORD draw_pos = {
                    (SHORT)(center_position.X + (tool_hand_offset_x - PLAYER_SPRITE_WIDTH / 2) + x),
                    (SHORT)(center_position.Y + (tool_hand_offset_y - PLAYER_SPRITE_HEIGHT / 2) + y)
                };

                if (draw_pos.X >= 0 && draw_pos.X < console_size.X &&
                    draw_pos.Y >= 0 && draw_pos.Y < console_size.Y) {
                    console_print_color_character(tool_pixel, draw_pos);
                }
            }
    }

    //데미지 텍스트 렌더링 함수 호출
    render_damage_texts();

    const int bar_width = HP_BAR_WIDTH,
              current_HP = player.HP < 0 ? 0 : player.HP,
              max_HP = player.max_HP > 0 ? player.max_HP : 1,
              filled = (current_HP * bar_width) / max_HP,
              empty = bar_width - filled;

    COORD position = {
        .X = (SHORT)(console_size.X - (bar_width + 22)), //오른쪽 끝에서 약간 여유
        .Y = 2
    };

    console_fprint_string("[", position, FOREGROUND_T_WHITE, BACKGROUND_T_TRANSPARENT);
    position.X += 1;

    for (int i = 0; i < filled; ++i) {
        console_fprint_string(" ", position, BACKGROUND_T_RED, 0); //빨간 체력 바
        position.X += 1;
    }

    for (int i = 0; i < empty; ++i) {
        console_fprint_string(" ", position, BACKGROUND_T_DARKGRAY, 0); //회색 빈 바
        position.X += 1;
    }

    console_fprint_string("]", position, FOREGROUND_T_WHITE, BACKGROUND_T_TRANSPARENT);
    position.X += 2;

    console_fprint_string("HP: %d / %d", position, BACKGROUND_T_BLACK, FOREGROUND_T_YELLOW, current_HP, max_HP);
}

//특정 x좌표에서 가장 높은 지상 의 Y좌표를 찾아 반환
const int player_find_ground_position(const int x) {
    for (int y = 0; y < map.size.y; ++y)
        if (!is_walkable(x, y))
            return y; //걷지 못하는 블록을 찾으면 y좌표 반환

    return map.size.y / 2; //블록을 찾지 못하면 맵 높이의 절반 반환;
}

//데미지 텍스트 생성 함수
static void create_damage_text(const int damage_value) {
    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i)
        if (!pDamage_texts[i].active) {
            pDamage_texts[i].active = true;
            pDamage_texts[i].damage_value = damage_value;
            pDamage_texts[i].precise_y = (float)player.y;
            pDamage_texts[i].timer = 1.5f; //1.5초 동안 화면에 표시
            break;
        }
}

void player_take_damage(const int damage) {
    player.HP -= damage;
    if (player.HP < 0) {
        player.HP = 0;
    }

    create_damage_text(damage);
}

void player_add_health(const int additional_health) {
    player.HP += additional_health;

    if (player.HP > player.max_HP)
        player.HP = player.max_HP;
}

void player_save(void) {
    if (!pSave_current)
        save_instantiate();

    pSave_current->x = player.x;
    pSave_current->y = player.y;
    pSave_current->HP = player.HP;
}