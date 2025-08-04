#include "leak.h"
#include "input.h"

#include <conio.h>
#include <Windows.h>
#include "console.h"

#define CALLBACK_CALLBACKS_NAME(type) p##type##_callbacks
#define CALLBACK_COUNT_NAME(type) type##_callback_count

#define CALLBACK_VARIABLES(type) \
static type *CALLBACK_CALLBACKS_NAME(type) = NULL; \
static int CALLBACK_COUNT_NAME(type)

#define CALLBACK_SUBSCRIBE_IMPLEMENTATION(type) \
    if (!CALLBACK_CALLBACKS_NAME(type)) \
        CALLBACK_CALLBACKS_NAME(type) = malloc(sizeof(type)); \
    else \
        CALLBACK_CALLBACKS_NAME(type) = realloc(CALLBACK_CALLBACKS_NAME(type), sizeof(type) * (CALLBACK_COUNT_NAME(type) + 1)); \
    CALLBACK_CALLBACKS_NAME(type)[CALLBACK_COUNT_NAME(type)++] = callback

#define CALLBACK_UNSUBSCRIBE_IMPLEMENTATION(type) \
    for (int i = 0; i < CALLBACK_COUNT_NAME(type); ++i) \
        if (CALLBACK_CALLBACKS_NAME(type)[i] == callback) \
            CALLBACK_CALLBACKS_NAME(type)[i] = NULL

#define CALLBACK_DESTROY(type) \
    free(CALLBACK_CALLBACKS_NAME(type)); \
    CALLBACK_CALLBACKS_NAME(type) = NULL; \
    CALLBACK_COUNT_NAME(type) = 0

bool keyboard_pressed = false;
char input_character = 0;

static HANDLE input_handle = NULL;
static DWORD original_mode = 0;
static HHOOK hook = NULL;

CALLBACK_VARIABLES(mouse_click_t);
CALLBACK_VARIABLES(mouse_position_t);
CALLBACK_VARIABLES(mouse_in_console_t);

static void mouse_click_callback(const bool left) {
    for (int i = 0; i < CALLBACK_COUNT_NAME(mouse_click_t); ++i)
        if (CALLBACK_CALLBACKS_NAME(mouse_click_t)[i])
            CALLBACK_CALLBACKS_NAME(mouse_click_t)[i](left);
}

static void mouse_position_callback(const COORD position) {
    for (int i = 0; i < CALLBACK_COUNT_NAME(mouse_position_t); ++i)
        if (CALLBACK_CALLBACKS_NAME(mouse_position_t)[i])
            CALLBACK_CALLBACKS_NAME(mouse_position_t)[i](position);
}

static void mouse_in_console_callback(const bool in_console) {
    for (int i = 0; i < CALLBACK_COUNT_NAME(mouse_in_console_t); ++i)
        if (CALLBACK_CALLBACKS_NAME(mouse_in_console_t)[i])
            CALLBACK_CALLBACKS_NAME(mouse_in_console_t)[i](in_console);
}

static LRESULT CALLBACK LowLevelMouseProc(const int nCode, const WPARAM wParam, const LPARAM lParam) {
    if (nCode == HC_ACTION) {
        const POINT point = ((MSLLHOOKSTRUCT *)lParam)->pt;
        mouse_position_callback(convert_monitor_to_console(point));

        if (wParam == WM_LBUTTONUP)
            mouse_click_callback(true);
        else if (wParam == WM_RBUTTONUP)
            mouse_click_callback(false);

        mouse_in_console_callback(is_cursor_inside_console(point));
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void initialize_input_handler(void) {
    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(input_handle, &original_mode);
    SetConsoleMode(input_handle, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);

    hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
}

void update_input(void) {
    keyboard_pressed = _kbhit();
    if (keyboard_pressed)
        input_character = (char)_getch();
}

void destroy_input_handler(void) {
    SetConsoleMode(input_handle, original_mode);

    CALLBACK_DESTROY(mouse_click_t);
    CALLBACK_DESTROY(mouse_position_t);
    CALLBACK_DESTROY(mouse_in_console_t);

    UnhookWindowsHookEx(hook);
}

void subscribe_mouse_click(const mouse_click_t callback) {
    CALLBACK_SUBSCRIBE_IMPLEMENTATION(mouse_click_t);
}

void unsubscribe_mouse_click(const mouse_click_t callback) {
    CALLBACK_UNSUBSCRIBE_IMPLEMENTATION(mouse_click_t);
}

void subscribe_mouse_position(const mouse_position_t callback) {
    CALLBACK_SUBSCRIBE_IMPLEMENTATION(mouse_position_t);
}

void unsubscribe_mouse_position(const mouse_position_t callback) {
    CALLBACK_UNSUBSCRIBE_IMPLEMENTATION(mouse_position_t);
}

void subscribe_mouse_in_console(const mouse_in_console_t callback) {
    CALLBACK_SUBSCRIBE_IMPLEMENTATION(mouse_in_console_t);
}

void unsubscribe_mouse_in_console(const mouse_in_console_t callback) {
    CALLBACK_UNSUBSCRIBE_IMPLEMENTATION(mouse_in_console_t);
}

#if _DEBUG
void pause_hook(void) {
    UnhookWindowsHookEx(hook);
}

void resume_hook(void) {
    hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
}
#endif