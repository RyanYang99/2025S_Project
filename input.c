#include "leak.h"
#include "input.h"

#include <Windows.h>

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

keyhit_t *pKeyhit_callbacks = NULL;
int keyhit_callback_count = 0;

mouse_click_t *pMouseClick_callbacks = NULL;
int mouse_click_callback_count = 0;

mouse_position_t *pMousePosition_callbacks = NULL;
int mouse_position_callback_count = 0;

void initialize_input_handler(void)
{
    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(input_handle, &original_mode);
    SetConsoleMode(input_handle, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);
}

static void keyhit_callback(const char character)
{
    for (int i = 0; i < keyhit_callback_count; ++i)
        if (pKeyhit_callbacks[i])
            pKeyhit_callbacks[i](character);
}

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

void handle_input_event(void)
{
    DWORD events = 0;
    GetNumberOfConsoleInputEvents(input_handle, &events);

    if (!events)
        return;

    INPUT_RECORD *pInput_records = malloc(sizeof(INPUT_RECORD) * events);
    DWORD events_read = 0;
    ReadConsoleInput(input_handle, pInput_records, events, &events_read);

    for (DWORD i = 0; i < events; ++i)
    {
        const DWORD type = pInput_records[i].EventType;

        if (type == KEY_EVENT)
            keyhit_callback(pInput_records[i].Event.KeyEvent.uChar.AsciiChar);
        else if (type == MOUSE_EVENT)
        {
            const MOUSE_EVENT_RECORD event = pInput_records[i].Event.MouseEvent;
            const DWORD state = event.dwButtonState;
            if (state == FROM_LEFT_1ST_BUTTON_PRESSED)
                mouse_click_callback(true);
            else if (state == RIGHTMOST_BUTTON_PRESSED)
                mouse_click_callback(false);
            
            mouse_position_callback(event.dwMousePosition);
        }
    }

    free(pInput_records);
}

void destroy_input_handler(void)
{
    SetConsoleMode(input_handle, original_mode);

    free(pKeyhit_callbacks);
    pKeyhit_callbacks = NULL;
    keyhit_callback_count = 0;
}

void subscribe_keyhit(const keyhit_t callback)
{
    CALLBACK_SUBSCRIBE_IMPLEMENTATION(keyhit_t, pKeyhit_callbacks, keyhit_callback_count);
}

void unsubscribe_keyhit(const keyhit_t callback)
{
    CALLBACK_UNSUBSCRIBE_IMPLEMENTATION(pKeyhit_callbacks, keyhit_callback_count);
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