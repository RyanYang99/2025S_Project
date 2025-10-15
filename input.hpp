#pragma once

/*
    input.hpp 사용법:
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

#include <Windows.h>

#include "callback.hpp"

#define CALLBACK_MEMBER(name, return_type) Callback<name##_t, return_type> name

#define CALLBACK_METHODS_DECLARE(name) \
static void subscribe_##name(const name##_t callback); \
\
static void unsubscribe_##name(const name##_t callback) noexcept \

//left: true 일때 왼쪽 마우스 버튼, false 일때 오른쪽 마우스 버튼
typedef void (*input_mouse_click_t)(const bool left);
typedef void (*input_mouse_position_t)(const COORD position);
typedef void (*input_mouse_in_console_t)(const bool in_console);

//윈도우 콜백을 사용해야 하기 때문에 static 클래스 사용
class input {
private:
    static bool keyboard_pressed_;
    static char input_character_, input_special_character_;

    static HANDLE input_handle;
    static DWORD original_mode;
    static HHOOK hook;

    static CALLBACK_MEMBER(input_mouse_click, bool);
    static CALLBACK_MEMBER(input_mouse_position, COORD);
    static CALLBACK_MEMBER(input_mouse_in_console, bool);

    static LRESULT CALLBACK windows_callback(const int nCode, const WPARAM wParam, const LPARAM lParam);

public:
    static void initialize(void) noexcept;

    static bool keyboard_pressed(void) noexcept;
    static char input_character(void) noexcept;
    static char input_special_character(void) noexcept;
    static bool is_key_down(const int virtual_key_code) noexcept;

    static void update(void) noexcept;

    CALLBACK_METHODS_DECLARE(input_mouse_click);
    CALLBACK_METHODS_DECLARE(input_mouse_position);
    CALLBACK_METHODS_DECLARE(input_mouse_in_console);

    static void destroy(void) noexcept;

    //디버깅시 마우스 훅 제거
#if _DEBUG
    static void pause_hook(void) noexcept;
    static void resume_hook(void) noexcept;
#endif
};