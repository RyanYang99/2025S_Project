#include "leak.hpp"
#include "input.hpp"

#include <conio.h>
#include <Windows.h>

#include "console.hpp"
#include "callback.hpp"

#define CALLBACK_MEMBER_INITIALIZE(name, return_type) Callback<name##_t, return_type> Input:: name{}

#define CALLBACK_METHODS_DEFINE(name) \
void Input::subscribe_##name(const name##_t callback) { \
    name.subscribe(callback); \
} \
\
void Input::unsubscribe_##name(const name##_t callback) noexcept { \
    name.unsubscribe(callback); \
}

bool Input::keyboard_pressed_{};
char Input::input_character_{}, Input::input_special_character_{};

HANDLE Input::input_handle{};
DWORD Input::original_mode{};
HHOOK Input::hook{};

CALLBACK_MEMBER_INITIALIZE(input_mouse_click, bool);
CALLBACK_MEMBER_INITIALIZE(input_mouse_position, COORD);
CALLBACK_MEMBER_INITIALIZE(input_mouse_in_console, bool);

LRESULT CALLBACK Input::windows_callback(const int nCode, const WPARAM wParam, const LPARAM lParam) {
    if (nCode == HC_ACTION) {
        const POINT point{ reinterpret_cast<MSLLHOOKSTRUCT *>(lParam)->pt };
        input_mouse_position.call(Console::convert_from_monitor(point));

        if (wParam == WM_LBUTTONUP)
            input_mouse_click.call(true);
        else if (wParam == WM_RBUTTONUP)
            input_mouse_click.call(false);

        input_mouse_in_console.call(Console::is_cursor_inside(point));
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Input::initialize(void) noexcept {
    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(input_handle, &original_mode);
    SetConsoleMode(input_handle, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);

    hook = SetWindowsHookEx(WH_MOUSE_LL, windows_callback, NULL, 0);
}

bool Input::keyboard_pressed(void) noexcept {
    return keyboard_pressed_;
}

char Input::input_character(void) noexcept {
    return input_character_;
}

char Input::input_special_character(void) noexcept {
    return input_special_character_;
}

bool Input::is_key_down(const int virtual_key_code) noexcept {
    return GetAsyncKeyState(virtual_key_code) & 0x8000;
}

void Input::update(void) noexcept {
    MSG msg{};
    while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    keyboard_pressed_ = _kbhit();
    if (keyboard_pressed_) {
        const int key = _getch();

        input_character_ = static_cast<char>(key);
        if (!key || key == 0xE0)
            input_special_character_ = static_cast<char>(_getch());
        else
            input_special_character_ = 0;
    }
}

CALLBACK_METHODS_DEFINE(input_mouse_click)
CALLBACK_METHODS_DEFINE(input_mouse_position)
CALLBACK_METHODS_DEFINE(input_mouse_in_console)

void Input::destroy(void) noexcept {
    SetConsoleMode(input_handle, original_mode);

    input_mouse_click.clear();
    input_mouse_position.clear();
    input_mouse_in_console.clear();

    UnhookWindowsHookEx(hook);
}

#if _DEBUG
void Input::pause_hook(void) noexcept {
    UnhookWindowsHookEx(hook);
}

void Input::resume_hook(void) noexcept {
    hook = SetWindowsHookEx(WH_MOUSE_LL, windows_callback, NULL, 0);
}
#endif