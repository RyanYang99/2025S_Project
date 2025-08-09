#pragma once

/*
    input.h 사용법:
        마우스 클릭:
            input_subscribe_mouse_click 함수는 (bool)을 인자로 받는 함수 (mouse_click_t) 포인터를 인자로 받는다.
            필요 없을 때는 input_unsubscribe_mouse_click 함수를 호출한다.
            input_handle_input_event -> 마우스 클릭 감지 -> input_subscribe_mouse_click을 통해 구독된 모든 함수 포인터 호출 -> 마우스 클릭 사용

        마우스 위치:
            내용 위와 동일 (COORD, mouse_position_t)
            input_handle_input_event -> 마우스 위치 감지 -> input_subscribe_mouse_position을 통해 구독된 모든 함수 포인터 호출 -> 마우스 위치 사용

        마우스 창 안쪽:
            내용 위와 동일 (bool)
*/

#include <stdbool.h>
#include <Windows.h>

//left: true 일때 왼쪽 마우스 버튼, false 일때 오른쪽 마우스 버튼
typedef void (* input_mouse_click_t)(const bool left);
typedef void (* input_mouse_position_t)(const COORD position);
typedef void (* input_mouse_in_console_t)(const bool in_console);

extern bool input_keyboard_pressed;
extern char input_character;

void input_initialize(void);
void input_update(void);
void input_destroy(void);

// 키보드 상태를 직접 확인하는 함수
const bool is_key_down(const int virtual_key_code);

void input_subscribe_mouse_click(const input_mouse_click_t callback);
void input_unsubscribe_mouse_click(const input_mouse_click_t callback);

void input_subscribe_mouse_position(const input_mouse_position_t callback);
void input_unsubscribe_mouse_position(const input_mouse_position_t callback);

void input_subscribe_mouse_in_console(const input_mouse_in_console_t callback);
void input_unsubscribe_mouse_in_console(const input_mouse_in_console_t callback);

#if _DEBUG
void input_pause_hook(void);
void input_resume_hook(void);
#endif