#include "leak.hpp"
#include "input.hpp"

#include <conio.h>
#include <Windows.h>
#include <functional>

#include "console.hpp"

HANDLE Input::input_handle{};
DWORD Input::original_mode{};
HHOOK Input::hook{};

Callback<input_mouse_click_t, bool> Input::mouse_click_callback{};
Callback<input_mouse_position_t, COORD> Input::mouse_position_callback{};
Callback<input_mouse_in_console_t, bool> Input::mouse_in_console_callback{};

void Input::initialize(void) {
    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(input_handle, &original_mode);
    SetConsoleMode(input_handle, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);

    hook = SetWindowsHookEx(WH_MOUSE_LL, windows_callback, NULL, 0);
}

LRESULT CALLBACK Input::windows_callback(const int nCode, const WPARAM wParam, const LPARAM lParam) {
    if (nCode == HC_ACTION) {
        const POINT point = ((MSLLHOOKSTRUCT *)lParam)->pt;
        mouse_position_callback.call(console_convert_from_monitor(point));

        if (wParam == WM_LBUTTONUP)
            mouse_click_callback.call(true);
        else if (wParam == WM_RBUTTONUP)
            mouse_click_callback.call(false);

        mouse_in_console_callback.call(console_is_cursor_inside(point));
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Input::update(void) {
    MSG msg{};
    while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    keyboard_pressed = _kbhit();
    if (keyboard_pressed) {
        const int key = _getch();

        input_character = (char)key;
        if (!key || key == 0xE0)
            input_special_character = (char)_getch();
        else
            input_special_character = 0;
    }
}

bool Input::is_key_down(const int virtual_key_code) {
    return GetAsyncKeyState(virtual_key_code) & 0x8000;
}

void Input::destroy(void) {
    SetConsoleMode(input_handle, original_mode);

    mouse_click_callback.clear();
    mouse_position_callback.clear();
    mouse_in_console_callback.clear();

    UnhookWindowsHookEx(hook);
}

#if _DEBUG
void Input::pause_hook(void) {
    UnhookWindowsHookEx(hook);
}

void Input::resume_hook(void) {
    hook = SetWindowsHookEx(WH_MOUSE_LL, windows_callback, NULL, 0);
}
#endif