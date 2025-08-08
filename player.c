#include "leak.h"
#include "player.h"

#include "Tool.h"
#include "map.h"
#include "save.h"
#include "input.h"
#include "delta.h"
#include "console.h"
#include <stdio.h> // swprintf 사용하기위해
#include <wchar.h>
#include <time.h>
#include <conio.h>
#include <Windows.h> // VK_SPACE 사용을 위해 추가

// 전역 인벤토리 객체 extern 선언
extern inventory_t inventory;

// ItemDB에서 아이템 정보를 가져오는 함수 extern 선언 (가정)
// item_db_index를 받아 해당 아이템의 정보를 담은 구조체 포인터를 반환합니다.
extern item_information_t* find_item_by_index(const int index);

player_t player = { 0 };

#define PLAYER_SPRITE_WIDTH 5
#define PLAYER_SPRITE_HEIGHT 5
#define ANIMATION_SPEED 10.0f // 1초에 4번 프레임 변경

#define GRAVITY 50.0f         // 중력 가속도
#define JUMP_STRENGTH -12.0f  // 점프 시 부여되는 초기 수직 속도

// 수평 이동 속도 조절 (값이 작을수록 빨라짐)
#define HORIZONTAL_MOVE_COOLDOWN 0.08f  // 약 1초에 12.5칸 이동

DamageText damage_texts[MAX_DAMAGE_TEXTS];


#define HP_BAR_WIDTH 20

// 각 픽셀을 표현할 구조체
typedef struct {
    wchar_t character; // 표시할 유니코드 문자
    int background_color; // 배경색
    int foreground_color; // 전경색
} PlayerSpritePixel;

// 2개의 애니메이션 프레임

// 프레임 1: 기본 서 있는 자세
static const PlayerSpritePixel player_sprite_stand[PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    // {문자, 배경색, 전경색}
    { {L' ',0,0}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} }, // 머리 (위:머리카락, 아래:피부)
    { {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW} }, // 몸통과 팔
    { {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW} }, // 허리, 바지
    { {L' ',0,0}, {L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0}, {L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0} }, // 다리
    { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK}, {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK}, {L' ',0,0} }  // 신발
};

// 걷기 애니메이션 (2 프레임)
static const PlayerSpritePixel player_sprite_walk[2][PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    // 프레임 0
    {
        { {L' ',0,0}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} },
        { {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L' '}},
        { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L' ',0,0} },
        { {L' ',0,0}, {L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0} },
        { {L' ',0,0}, {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK}, {L' ',0,0}, {L' ',0,0} }
    },
    // 프레임 1
    {
        { {L' ',0,0}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} },
        { {L' '}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW} },
        { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L' ',0,0} },
        { {L' ',0,0}, {L' ',0,0}, {L' ',0,0}, {L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0} },
        { {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0} }
    }
};

// 무장하고 서 있는 자세
static const PlayerSpritePixel player_sprite_stand_armed[PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    // 머리
    { {L' ',0,0}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} },
    // 몸통과 옆으로 뻗은 오른팔
    { {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW} },
    // 허리, 바지
    { {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L' ',0,0} },
    // 다리 
    { {L' ',0,0}, {L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0}, {L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0} },
    // 신발
    { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK}, {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK}, {L' ',0,0} }
};

// 무장하고 걷는 애니메이션
static const PlayerSpritePixel player_sprite_walk_armed[2][PLAYER_SPRITE_HEIGHT][PLAYER_SPRITE_WIDTH] = {
    { // 프레임 1
        { {L' ',0,0}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} },
        { {L' ',0,0}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}}, // 왼팔 뒤로, 오른팔은 고정
        { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L' ',0,0} },
        { {L' ',0,0}, {L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0} },
        { {L' ',0,0}, {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK}, {L' ',0,0}, {L' ',0,0} }
    },
    { // 프레임 2
        { {L' ',0,0}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'▄', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L' ',0,0} },
        { {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_WHITE}, {L'█', FOREGROUND_T_DARKYELLOW, FOREGROUND_T_DARKYELLOW}}, // 왼팔 앞으로, 오른팔은 고정
        { {L' ',0,0}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_WHITE}, {L' ',0,0} },
        { {L' ',0,0}, {L' ',0,0}, {L' ',0,0}, {L'▓', BACKGROUND_T_BLACK, FOREGROUND_T_GRAY}, {L' ',0,0} },
        { {L'█', BACKGROUND_T_BLACK, FOREGROUND_T_BLACK}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0}, {L' ',0,0} }
    }
};

// 대미지 텍스트 생성 함수
void create_damage_text(int damage_value) {
    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i) {
        if (!damage_texts[i].active) {
            damage_texts[i].active = true;
            damage_texts[i].damage_value = damage_value;
            damage_texts[i].precise_y = (float)player.y;
            damage_texts[i].timer = 1.5f; // 1.5초 동안 화면에 표시
            break;
        }
    }
}

static void update_damage_texts() 
{
    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i) {
        if (damage_texts[i].active) {
            // 위로 움직이는 효과
            damage_texts[i].precise_y -= delta_time * 5.0f; //움직이는 속도

            // 타이머 감소
            damage_texts[i].timer -= delta_time;
            if (damage_texts[i].timer <= 0.0f) {
                damage_texts[i].active = false; // 시간이 지나면 비활성화
            }
        }
    }
}

static void render_damage_texts() 
{
    COORD center_pos = { console.size.X / 2, console.size.Y / 2 };

    for (int i = 0; i < MAX_DAMAGE_TEXTS; ++i) {
        if (damage_texts[i].active) {
            COORD draw_pos;

            draw_pos.X = center_pos.X;
            draw_pos.Y = (SHORT)(center_pos.Y - (PLAYER_SPRITE_HEIGHT / 2) - 1 - (player.precise_y - damage_texts[i].precise_y));

            wchar_t damage_str[20];
            swprintf(damage_str, 20, L" Hit -%d! ", damage_texts[i].damage_value);

            for (int j = 0; damage_str[j] != L'\0'; ++j) {
                COORD char_pos = draw_pos;
                char_pos.X = draw_pos.X + (SHORT)j - (SHORT)(wcslen(damage_str) / 2);

                if (char_pos.X >= 0 && char_pos.X < console.size.X &&
                    char_pos.Y >= 0 && char_pos.Y < console.size.Y) {
                    print_color_tchar((color_tchar_t) { damage_str[j], BACKGROUND_T_BLACK, FOREGROUND_T_RED }, char_pos);
                }
            }
        }
    }
}



static void movement(void) {
    // 점프 키 확인
    if (is_key_down(VK_SPACE) && player.is_on_ground) {
        player.velocity_y = JUMP_STRENGTH;
        player.is_on_ground = false;
    }

    // 쿨다운 타이머 업데이트
    player.move_cooldown_timer += delta_time;

    bool is_a_down = is_key_down('A');
    bool is_d_down = is_key_down('D');

    // 키가 눌렸는지 여부에 따라 애니메이션 상태 설정
    if (is_a_down || is_d_down) {
        player.is_moving = 1;
    }
    else {
        player.is_moving = 0;
    }

    // 쿨타임이 다 되었는지 확인
    if (player.move_cooldown_timer >= HORIZONTAL_MOVE_COOLDOWN) {
        bool moved_horizontally = false;

        // 왼쪽 이동 (D키와 동시에 눌리면 무시)
        if (is_a_down && !is_d_down) {
            int new_x = player.x - 1;
            if (is_walkable(new_x, player.y)) {
                player.x = new_x;
                moved_horizontally = true;
                player.facing_direction = -1; // 왼쪽 보기
            }
        }
        // 오른쪽 이동 (A키와 동시에 눌리면 무시)
        else if (is_d_down && !is_a_down) {
            int new_x = player.x + 1;
            if (is_walkable(new_x, player.y)) {
                player.x = new_x;
                moved_horizontally = true;
                player.facing_direction = 1; // 오른쪽 보기
            }
        }

        // 실제로 이동이 일어났다면 타이머를 리셋
        if (moved_horizontally) {
            player.move_cooldown_timer = 0.0f;
        }
    }
}




        static void update_player_offset(void) 
        {
            player.x += map.offset_x;
        }


        
        void player_update(void)
        {

            // 입력 처리
            movement();

            //데미지 출력 업데이트
            update_damage_texts();

            // 물리 업데이트
            // 땅에 있는지 확인
            // 플레이어 바로 아래 블록이 걸을 수 없는 블록인지 확인
            if (!is_walkable(player.x, player.y + 1)) {
                player.is_on_ground = true;
                // 땅에 있다면 수직 속도 초기화
                if (player.velocity_y > 0) {
                    player.velocity_y = 0;
                }
            }
            else {
                player.is_on_ground = false;
            }

            // 중력 적용 (공중에 있을 때만)
            if (!player.is_on_ground) {
                player.velocity_y += GRAVITY * delta_time;
            }

            // 속도에 따라 정밀 y좌표 업데이트
            player.precise_y += player.velocity_y * delta_time;

            // 정밀 y좌표를 정수 y좌표로 변환하여 충돌 처리
            int new_y = (int)player.precise_y;

            // 수직 이동에 대한 충돌 처리
            if (new_y > player.y) { // 아래로 이동 시
                while (!is_walkable(player.x, new_y)) {
                    new_y--;
                    player.velocity_y = 0;
                    player.precise_y = (float)new_y;
                    player.is_on_ground = true;
                }
            }
            else if (new_y < player.y) { // 위로 이동 시
                while (!is_walkable(player.x, new_y)) {
                    new_y++;
                    player.velocity_y = 0;
                    player.precise_y = (float)new_y;
                }
            }
            player.y = new_y;


            // 3. 애니메이션 업데이트
            if (player.is_moving) {
                player.animation_timer += delta_time;
                if (player.animation_timer >= 1.0f / ANIMATION_SPEED) {
                    player.animation_timer = 0.0f;
                    player.current_frame = (player.current_frame + 1) % 2;
                }
            }
            else {
                player.current_frame = 0;
                player.animation_timer = 0.0f;
            }
        }

    


        void player_init(void) 
        {
            if (pCurrent_save) {
                player.x = pCurrent_save->x;
                player.y = pCurrent_save->y;
                player.hp = pCurrent_save->hp;
            }
            else {
                player.x = map.size.x / 2;
                player.y = find_ground_pos(player.x);
                if (player.y - 1 >= 0)
                    --player.y; // 가능할 시 찾은 블록 위로 설정


                player.max_hp = 1000;
                player.hp = 1000; // 초기 체력
                player.atk_power = 10;

                // 물리 변수 초기화
                player.precise_y = (float)player.y;
                player.velocity_y = 0.0f;
                player.is_on_ground = false; // 시작 시 공중에서 떨어지도록

                // 애니메이션 변수 초기화
                player.is_moving = 0;
                player.current_frame = 0;
                player.animation_timer = 0.0f;

                // 이동 쿨다운 타이머 초기화
                player.move_cooldown_timer = 0.0f;

                // 초기 방향: 오른쪽
                player.facing_direction = 1;

                //subscribe_keyhit(movement);
                subscribe_offset_change(update_player_offset);
            }
        }
    


// 충돌 감지 함수 구현
bool is_walkable(int x, int y) {
    // 맵 경계 체크
    if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y) {
        return false;
    }

    // 블록 정보 가져오기 
    block_info_t block = get_block_info_at(x, y);

    // 블록 타입에 따른 이동 가능 여부 판단
    switch (block.type) {

        case BLOCK_AIR:
        case BLOCK_LOG:
        case BLOCK_LEAF:
        case BLOCK_WATER:
        case BLOCK_STAR:
            return true;
    }

    return false;
}

void render_player(void) {
    // 플레이어의 중심이 될 콘솔 위치 (화면 중앙)
    COORD center_pos = { console.size.X / 2, console.size.Y / 2 };

    // 1. 장착한 아이템 확인
    player_item_t* equipped_item = inventory.pHotbar[inventory.selected_hotbar_index].pPlayer_Item;
    item_information_t* pToolInfo = NULL;
    bool is_tool_equipped = false;

    if (equipped_item && equipped_item->quantity > 0) {
        pToolInfo = find_item_by_index(equipped_item->item_db_index);
        if (pToolInfo && pToolInfo->type == ITEM_TYPE_TOOL) {
            is_tool_equipped = true;
        }
    }

    // 2. 상태에 맞는 스프라이트 선택
    const PlayerSpritePixel(*current_sprite_ptr)[PLAYER_SPRITE_WIDTH];
    if (is_tool_equipped) {
        current_sprite_ptr = player.is_moving ? player_sprite_walk_armed[player.current_frame] : player_sprite_stand_armed;
    }
    else {
        current_sprite_ptr = player.is_moving ? player_sprite_walk[player.current_frame] : player_sprite_stand;
    }

    // 3. 플레이어 스프라이트 렌더링 (좌우 반전 적용)
    for (int y = 0; y < PLAYER_SPRITE_HEIGHT; ++y) {
        for (int x = 0; x < PLAYER_SPRITE_WIDTH; ++x) {
            int source_x = (player.facing_direction == 1) ? x : (PLAYER_SPRITE_WIDTH - 1 - x);
            PlayerSpritePixel pixel = current_sprite_ptr[y][source_x];

            if (pixel.character == L' ' && pixel.background_color == 0) continue;

            COORD draw_pos = {
                (SHORT)(center_pos.X + x - (PLAYER_SPRITE_WIDTH / 2)),
                (SHORT)(center_pos.Y + y - (PLAYER_SPRITE_HEIGHT / 2))
            };

            if (draw_pos.X >= 0 && draw_pos.X < console.size.X &&
                draw_pos.Y >= 0 && draw_pos.Y < console.size.Y) {
                print_color_tchar((color_tchar_t) { pixel.character, pixel.background_color, pixel.foreground_color }, draw_pos);
            }
        }
    }

    // 4. 장착한 도구 렌더링 (좌우 반전 적용)
    if (is_tool_equipped) {
        // 도구를 들 손의 위치 계산
        int tool_hand_offset_x = (player.facing_direction == 1) ? 5 : -TEXTURE_SIZE;
        int tool_hand_offset_y = -1;

        for (int y = 0; y < TEXTURE_SIZE; ++y) {
            for (int x = 0; x < TEXTURE_SIZE; ++x) {
                int source_x = (player.facing_direction == 1) ? x : (TEXTURE_SIZE - 1 - x);
                color_tchar_t tool_pixel = get_tool_texture((tool_t)pToolInfo->index, source_x, y);

                if (tool_pixel.foreground == FOREGROUND_T_TRANSPARENT) continue;

                COORD draw_pos = {
                    (SHORT)(center_pos.X + (tool_hand_offset_x - PLAYER_SPRITE_WIDTH / 2) + x),
                    (SHORT)(center_pos.Y + (tool_hand_offset_y - PLAYER_SPRITE_HEIGHT / 2) + y)
                };

                if (draw_pos.X >= 0 && draw_pos.X < console.size.X &&
                    draw_pos.Y >= 0 && draw_pos.Y < console.size.Y) {
                    print_color_tchar(tool_pixel, draw_pos);
                }
            }
        }
    }

    //데미지 텍스트 렌더링 함수 호출
    render_damage_texts();

   

    const int bar_width = HP_BAR_WIDTH;

    int current_hp = player.hp < 0 ? 0 : player.hp;
    int max_hp = player.max_hp > 0 ? player.max_hp : 1;

    int filled = (current_hp * bar_width) / max_hp;
    int empty = bar_width - filled;

    COORD pos = {
        .X = (SHORT)(console.size.X - (bar_width + 22)),  // 오른쪽 끝에서 약간 여유
        .Y = 2
    };

    fprint_string("[", pos, FOREGROUND_T_WHITE, BACKGROUND_T_TRANSPARENT);
    pos.X += 1;

    for (int i = 0; i < filled; ++i) {
        fprint_string(" ", pos, BACKGROUND_T_RED, 0);  // 빨간 체력 바
        pos.X += 1;
    }
    for (int i = 0; i < empty; ++i) {
        fprint_string(" ", pos, BACKGROUND_T_DARKGRAY, 0);  // 회색 빈 바
        pos.X += 1;
    }

    fprint_string("]", pos, FOREGROUND_T_WHITE, BACKGROUND_T_TRANSPARENT);
    pos.X += 2;

    fprint_string("HP: %d / %d", pos, BACKGROUND_T_BLACK, FOREGROUND_T_YELLOW, current_hp, max_hp);
}



//특정 x좌표에서 가장 높은 지상 의 Y좌표를 찾아 반환
int find_ground_pos(int x)
{
    for (int y = 0; y < map.size.y; ++y)
        if (!is_walkable(x, y))
            return y; //걷지 못하는 블록을 찾으면 y좌표 반환

    return map.size.y / 2; //블록을 찾지 못하면 맵 높이의 절반 반환;
}


void save_player(void) {
    if (!pCurrent_save)
        instantiate_save();

    pCurrent_save->x = player.x;
    pCurrent_save->y = player.y;
    pCurrent_save->hp = player.hp;
}

void player_take_damage(int damage)
{
    player.hp -= damage;
    if (player.hp < 0) {
        player.hp = 0;
    }
    create_damage_text(damage);

}

void add_health_to_player(const int additional_health) {

    player.hp += additional_health;

    if (player.hp > player.max_hp)
        player.hp = player.max_hp;
}