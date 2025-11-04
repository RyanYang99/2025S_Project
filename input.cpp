#include "leak.hpp"
#include "input.hpp"

#include <conio.h>
#include <Windows.h>

#include "console.hpp"
#include "callback.hpp"

#define CALLBACK_MEMBER_INITIALIZE(name, return_type) Callback<name##_t, return_type> input:: name{}

#define CALLBACK_METHODS_DEFINE(name) \
void input::subscribe_##name(const name##_t callback) { \
    name.subscribe(callback); \
} \
\
void input::unsubscribe_##name(const name##_t callback) noexcept { \
    name.unsubscribe(callback); \
}

bool input::keyboard_pressed_{};
char input::input_character_{}, input::input_special_character_{};

HANDLE input::input_handle{};
DWORD input::original_mode{};
HHOOK input::hook{};

CALLBACK_MEMBER_INITIALIZE(input_mouse_click, bool);
CALLBACK_MEMBER_INITIALIZE(input_mouse_position, COORD);
CALLBACK_MEMBER_INITIALIZE(input_mouse_in_console, bool);

LRESULT CALLBACK input::windows_callback(const int nCode, const WPARAM wParam, const LPARAM lParam) {
    if (nCode == HC_ACTION) {
        const POINT point{ reinterpret_cast<MSLLHOOKSTRUCT *>(lParam)->pt };
        input_mouse_position.call(console::convert_from_monitor(point));

        if (wParam == WM_LBUTTONUP)
            input_mouse_click.call(true);
        else if (wParam == WM_RBUTTONUP)
            input_mouse_click.call(false);

        input_mouse_in_console.call(console::is_cursor_inside(point));
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void input::initialize(void) noexcept {
    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(input_handle, &original_mode);
    SetConsoleMode(input_handle, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);

    hook = SetWindowsHookEx(WH_MOUSE_LL, windows_callback, NULL, 0);
}

bool input::keyboard_pressed(void) noexcept {
    return keyboard_pressed_;
}

char input::input_character(void) noexcept {
    return input_character_;
}

char input::input_special_character(void) noexcept {
    return input_special_character_;
}

bool input::is_key_down(const int virtual_key_code) noexcept {
    return GetAsyncKeyState(virtual_key_code) & 0x8000;
}

void input::update(void) noexcept {
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

void input::destroy(void) noexcept {
    SetConsoleMode(input_handle, original_mode);

    input_mouse_click.clear();
    input_mouse_position.clear();
    input_mouse_in_console.clear();

    UnhookWindowsHookEx(hook);
}

#if _DEBUG
void input::pause_hook(void) noexcept {
    UnhookWindowsHookEx(hook);
}

void input::resume_hook(void) noexcept {
    hook = SetWindowsHookEx(WH_MOUSE_LL, windows_callback, NULL, 0);
}
#endif