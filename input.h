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
*/

#include <stdbool.h>
#include <Windows.h>

/*
    left: true 일때 왼쪽 마우스 버튼, false 일때 오른쪽 마우스 버튼
*/
typedef void (* mouse_click_t)(const bool left);
typedef void (* mouse_position_t)(const COORD position);

void initialize_input_handler(void);
void destroy_input_handler(void);
void subscribe_mouse_click(const mouse_click_t callback);
void unsubscribe_mouse_click(const mouse_click_t callback);
void subscribe_mouse_position(const mouse_position_t callback);
void unsubscribe_mouse_position(const mouse_position_t callback);