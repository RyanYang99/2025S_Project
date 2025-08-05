#pragma once


/*
    input.h 사용법:
        마우스 클릭:
            subscribe_mouse_click 함수는 (bool)을 인자로 받는 함수 (mouse_click_t) 포인터를 인자로 받는다.
            필요 없을 때는 unsubscribe_mouse_click 함수를 호출한다.
            handle_input_event -> 마우스 클릭 감지 -> subscribe_mouse_click을 통해 구독된 모든 함수 포인터 호출 -> 마우스 클릭 사용

        마우스 위치:
            내용 위와 동일 (COORD, mouse_position_t)
            handle_input_event -> 마우스 위치 감지 -> subscribe_mouse_position을 통해 구독된 모든 함수 포인터 호출 -> 마우스 위치 사용

        마우스 창 안쪽:
            내용 위와 동일 (bool)

*/

#include <stdbool.h>
#include <Windows.h>

//left: true 일때 왼쪽 마우스 버튼, false 일때 오른쪽 마우스 버튼
typedef void (* mouse_click_t)(const bool left);
typedef void (* mouse_position_t)(const COORD position);
typedef void (* mouse_in_console_t)(const bool in_console);

extern bool keyboard_pressed;
extern char input_character;

// 키보드 상태를 직접 확인하는 함수
bool is_key_down(int virtual_key_code);

void initialize_input_handler(void);

void update_input(void);

void destroy_input_handler(void);

void subscribe_mouse_click(const mouse_click_t callback);
void unsubscribe_mouse_click(const mouse_click_t callback);

void subscribe_mouse_position(const mouse_position_t callback);
void unsubscribe_mouse_position(const mouse_position_t callback);

void subscribe_mouse_in_console(const mouse_in_console_t callback);
void unsubscribe_mouse_in_console(const mouse_in_console_t callback);

//디버깅 할때 사용
#if _DEBUG
void pause_hook(void);
void resume_hook(void);
#endif



//전투 시스템을 위한 마우스 콜백  -->mob.c 적용 위함 
typedef void (*mouse_click_with_pos_t)(const bool left, const COORD position);

static void mouse_click_with_pos_callback(const bool left, const COORD position);
void subscribe_mouse_click_with_pos(const mouse_click_with_pos_t callback);
void unsubscribe_mouse_click_with_pos(const mouse_click_with_pos_t callback);

//=========================================================승준 추가 
