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

#include <functional>

#include <Windows.h>

//left: true 일때 왼쪽 마우스 버튼, false 일때 오른쪽 마우스 버튼
typedef std::function<void(const bool left)> input_mouse_click_t;
typedef std::function<void(const COORD position)> input_mouse_position_t;
typedef std::function<void(const bool in_console)> input_mouse_in_console_t;

enum InputDirection {
    up = 72,
    left = 75,
    right = 77,
    down = 80
};

template <typename T, typename U>
class Callback {
private:
    std::vector<T> callbacks{};

public:
    void subscribe(const T &callback) {
        callbacks.push_back(callback);
    }

    void unsubscribe(const T &callback) {
        size_t i{};
        bool found{};

        for (; i < callbacks.size(); ++i)
            if (callback_ == callback) {
                found = true;
                break;
            }

        if (!found)
            return;

        callbacks.erase(callbacks.begin() + i);
    }

    void call(const U parameter) const {
        for (const T &callback : callbacks)
            callback(parameter);
    }

    void clear(void) {
        callbacks.clear();
    }
};

//윈도우 콜백을 사용해야 하기 때문에 static 클래스 사용
class Input {
private:
    static HANDLE input_handle;
    static DWORD original_mode;
    static HHOOK hook;

    static Callback<input_mouse_click_t, bool> mouse_click_callback;
    static Callback<input_mouse_position_t, COORD> mouse_position_callback;
    static Callback<input_mouse_in_console_t, bool> mouse_in_console_callback;

public:
    static bool keyboard_pressed;
    static char input_character, input_special_character;

    static void initialize(void);

    static LRESULT CALLBACK windows_callback(const int nCode, const WPARAM wParam, const LPARAM lParam);
    static void update(void);

    static bool is_key_down(const int virtual_key_code);

    static void destroy(void);

#if _DEBUG
    static void pause_hook(void);
    static void resume_hook(void);
#endif
};