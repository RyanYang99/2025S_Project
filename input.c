#include "leak.h"
#include "input.h"

#include <Windows.h>

HANDLE input_handle = NULL;

keyhit_t *pKeyhit_callbacks = NULL;
int keyhit_callback_count = 0;

void initialize_input_handler(void)
{
    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(input_handle, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
}

static void keyhit_callback(const char character)
{
    for (int i = 0; i < keyhit_callback_count; ++i)
        if (pKeyhit_callbacks[i])
            pKeyhit_callbacks[i](character);
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
        if (pInput_records[i].EventType == KEY_EVENT)
            keyhit_callback(pInput_records[i].Event.KeyEvent.uChar.AsciiChar);

    free(pInput_records);
}

void destroy_input_handler(void)
{
    free(pKeyhit_callbacks);
    pKeyhit_callbacks = NULL;
    keyhit_callback_count = 0;
}

void subscribe_keyhit(const keyhit_t callback)
{
    if (!pKeyhit_callbacks)
        pKeyhit_callbacks = malloc(sizeof(keyhit_t));
    else
        pKeyhit_callbacks = realloc(pKeyhit_callbacks, sizeof(keyhit_t) * (keyhit_callback_count + 1));

    pKeyhit_callbacks[keyhit_callback_count++] = callback;
}

void unsubscribe_keyhit(const keyhit_t callback)
{
    for (int i = 0; i < keyhit_callback_count; ++i)
        if (pKeyhit_callbacks[i] == callback)
            pKeyhit_callbacks[i] = NULL;
}