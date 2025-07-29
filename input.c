#include "leak.h"
#include "input.h"

#include <Windows.h>
#include "console.h"

#define CALLBACK_SUBSCRIBE_IMPLEMENTATION(type, array, count) \
    if (!array) \
        array = malloc(sizeof(type)); \
    else \
        array = realloc(array, sizeof(type) * (count + 1)); \
    array[count++] = callback

#define CALLBACK_UNSUBSCRIBE_IMPLEMENTATION(array, count) \
    for (int i = 0; i < count; ++i) \
        if (array[i] == callback) \
            array[i] = NULL

HANDLE input_handle = NULL;
DWORD original_mode = 0;
HHOOK hook = NULL;

mouse_click_t* pMouseClick_callbacks = NULL;
int mouse_click_callback_count = 0;

mouse_position_t* pMousePosition_callbacks = NULL;
int mouse_position_callback_count = 0;

static void mouse_click_callback(const bool left)
{
    for (int i = 0; i < mouse_click_callback_count; ++i)
        if (pMouseClick_callbacks[i])
            pMouseClick_callbacks[i](left);
}

static void mouse_position_callback(const COORD position)
{
    for (int i = 0; i < mouse_position_callback_count; ++i)
        if (pMousePosition_callbacks[i])
            pMousePosition_callbacks[i](position);
}

static LRESULT CALLBACK LowLevelMouseProc(const int nCode, const WPARAM wParam, const LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        const MSLLHOOKSTRUCT *pMouse_struct = (MSLLHOOKSTRUCT *)lParam;
        const COORD position = convert_monitor_to_console(pMouse_struct->pt);

        mouse_position_callback(position);

        switch (wParam)
        {
            case WM_LBUTTONUP:
                mouse_click_callback(true);
                break;

            case WM_RBUTTONUP:
                mouse_click_callback(false);
                break;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void initialize_input_handler(void)
{
    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(input_handle, &original_mode);
    SetConsoleMode(input_handle, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);

    //디버깅 할때 주석 처리
    hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
}

void destroy_input_handler(void)
{
    SetConsoleMode(input_handle, original_mode);

    free(pMouseClick_callbacks);
    pMouseClick_callbacks = NULL;
    mouse_click_callback_count = 0;

    free(pMousePosition_callbacks);
    pMousePosition_callbacks = NULL;
    mouse_position_callback_count = 0;

    UnhookWindowsHookEx(hook);
}

void subscribe_mouse_click(const mouse_click_t callback)
{
    CALLBACK_SUBSCRIBE_IMPLEMENTATION(mouse_click_t, pMouseClick_callbacks, mouse_click_callback_count);
}

void unsubscribe_mouse_click(const mouse_click_t callback)
{
    CALLBACK_UNSUBSCRIBE_IMPLEMENTATION(pMouseClick_callbacks, mouse_click_callback_count);
}

void subscribe_mouse_position(const mouse_position_t callback)
{
    CALLBACK_SUBSCRIBE_IMPLEMENTATION(mouse_position_t, pMousePosition_callbacks, mouse_position_callback_count);
}

void unsubscribe_mouse_position(const mouse_position_t callback)
{
    CALLBACK_UNSUBSCRIBE_IMPLEMENTATION(pMousePosition_callbacks, mouse_position_callback_count);
}