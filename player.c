#include "leak.h"
#include "player.h"

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

player_t player = { 0 };

#define PLAYER_SPRITE_WIDTH 5
#define PLAYER_SPRITE_HEIGHT 5
#define ANIMATION_SPEED 10.0f // 1초에 4번 프레임 변경

#define GRAVITY 50.0f         // 중력 가속도
#define JUMP_STRENGTH -12.0f  // 점프 시 부여되는 초기 수직 속도

// 수평 이동 속도 조절 (값이 작을수록 빨라짐)
#define HORIZONTAL_MOVE_COOLDOWN 0.08f  // 약 1초에 12.5칸 이동

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
            }
        }
        // 오른쪽 이동 (A키와 동시에 눌리면 무시)
        else if (is_d_down && !is_a_down) {
            int new_x = player.x + 1;
            if (is_walkable(new_x, player.y)) {
                player.x = new_x;
                moved_horizontally = true;
            }
        }

        // 실제로 이동이 일어났다면 타이머를 리셋
        if (moved_horizontally) {
            player.move_cooldown_timer = 0.0f;
        }
    }
}

static void update_player_offset(void) {
    player.x += map.offset_x;
}


void player_update(void) {

    // 입력 처리
    movement();

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


void player_init(void) {
    if (pCurrent_save) {
        player.x = pCurrent_save->x;
        player.y = pCurrent_save->y;
        player.hp = pCurrent_save->hp;
    } else {
        player.x = map.size.x / 2;
        player.y = find_ground_pos(player.x);
        if (player.y - 1 >= 0)
            --player.y; // 가능할 시 찾은 블록 위로 설정

        player.hp = 1000; // 초기 체력
    }

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

    //subscribe_keyhit(movement);
    subscribe_offset_change(update_player_offset);
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

    // 현재 상태에 맞는 스프라이트 포인터 선택
    const PlayerSpritePixel(*current_sprite_ptr)[PLAYER_SPRITE_WIDTH];
    if (player.is_moving) {
        current_sprite_ptr = player_sprite_walk[player.current_frame];
    }
    else {
        current_sprite_ptr = player_sprite_stand;
    }

    for (int y = 0; y < PLAYER_SPRITE_HEIGHT; ++y) {
        for (int x = 0; x < PLAYER_SPRITE_WIDTH; ++x) {
            PlayerSpritePixel pixel = current_sprite_ptr[y][x];

            if (pixel.character == L' ') continue;

            COORD draw_pos;
            draw_pos.X = (SHORT)(center_pos.X + x - (PLAYER_SPRITE_WIDTH / 2));
            draw_pos.Y = (SHORT)(center_pos.Y + y - (PLAYER_SPRITE_HEIGHT / 2));

            if (draw_pos.X >= 0 && draw_pos.X < console.size.X &&
                draw_pos.Y >= 0 && draw_pos.Y < console.size.Y) {

                // 캐릭터 '픽셀'의 전경색과 배경색을 동시에 사용
                color_tchar_t tchar;
                tchar.character = pixel.character;

                tchar.background = (pixel.character == L'▀') ? pixel.foreground_color : BACKGROUND_T_BLACK;
                tchar.foreground = (pixel.character == L'▀') ? pixel.background_color : pixel.foreground_color;

                print_color_tchar(tchar, draw_pos);
            }
        }
    }

    // 체력(HP) 바 렌더링
    wchar_t hp_str[16];
    swprintf(hp_str, 16, L"HP: %d", player.hp);

    // 체력 문자열을 플레이어 스프라이트 머리 위에 출력
    COORD hp_pos;
    // 새로운 스프라이트의 최상단 좌표를 기준으로 Y 위치 조정
    hp_pos.Y = center_pos.Y - (PLAYER_SPRITE_HEIGHT / 2) - 1;
    if (hp_pos.Y < 0) hp_pos.Y = 0; // 화면 위로 벗어나지 않도록 처리

    // HP 문자열을 중앙에 정렬하여 출력
    for (int i = 0; hp_str[i] != L'\0'; ++i) {
        COORD char_pos = hp_pos;
        char_pos.X = center_pos.X + (SHORT)i - (SHORT)(wcslen(hp_str) / 2);
        if (char_pos.X >= 0 && char_pos.X < console.size.X) {
            print_color_tchar((color_tchar_t) { hp_str[i], BACKGROUND_T_BLACK, FOREGROUND_T_RED }, char_pos);
        }
    }
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
}