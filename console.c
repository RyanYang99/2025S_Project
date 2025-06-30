//https://github.com/Kevger/DoubleBufferedWindowsConsole/blob/master/source/DoubleBufferedConsole.h

#include "leak.h"

#include <stdio.h>
#include "console.h"

bool console_size_changed = false, use_double_buffer = false, maximized = false;

int current_buffer = 0;
HANDLE buffer[2] = { 0 };
PCHAR_INFO character_buffer = NULL;
SMALL_RECT written = { 0 };

HANDLE handle = NULL, input_handle = NULL;
console_key_hit_callback_t key_callback = NULL;

console_t console = { 0 };

static void hide_console_cursor(const HANDLE cursor_handle)
{
    CONSOLE_CURSOR_INFO cci = { 0 };
    GetConsoleCursorInfo(cursor_handle, &cci);

    cci.bVisible = FALSE;
    SetConsoleCursorInfo(cursor_handle, &cci);
}

static void initialize_double_buffering(void)
{
    for (int i = 0; i < 2; ++i)
    {
        const HANDLE new_handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
        SetConsoleScreenBufferSize(new_handle, console.size);

        written.Right = console.size.X - 1;
        written.Bottom = console.size.Y - 1;
        SetConsoleWindowInfo(new_handle, TRUE, &written);

        hide_console_cursor(new_handle);

        buffer[i] = new_handle;
    }

    character_buffer = malloc(sizeof(CHAR_INFO) * console.size.X * console.size.Y);
}

//https://stackoverflow.com/a/12642749
const static COORD get_console_size(const HANDLE size_handle)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
    GetConsoleScreenBufferInfo(size_handle, &csbi);

    COORD console_size = { 0 };
    console_size.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    console_size.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return console_size;
}

void initialize_console(bool use_double_buffering, console_key_hit_callback_t key_hit_callback)
{
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    console.size = get_console_size(handle);

    use_double_buffer = use_double_buffering;
    if (use_double_buffer)
        initialize_double_buffering();
    else
        hide_console_cursor(handle);

    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(input_handle, ENABLE_WINDOW_INPUT | ENABLE_PROCESSED_INPUT);

    key_callback = key_hit_callback;
}

static void flip_double_buffer(void)
{
    if (!use_double_buffer)
        return;

    WriteConsoleOutput(buffer[current_buffer], character_buffer, console.size, (COORD){ 0, 0 }, &written);
    SetConsoleActiveScreenBuffer(buffer[current_buffer]);

    if (!current_buffer)
        current_buffer = 1;
    else
        current_buffer = 0;
}

static void console_event(void)
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
        WORD event = pInput_records[i].EventType;
        if (event == WINDOW_BUFFER_SIZE_EVENT)
            console_size_changed = true;
        else if (event == KEY_EVENT)
            key_callback(pInput_records[i].Event.KeyEvent.uChar.UnicodeChar);
    }

    free(pInput_records);
}

static void resize(const HANDLE size_handle)
{
    SMALL_RECT rect = { 0, 0, 1, 1 };
    SetConsoleWindowInfo(size_handle, TRUE, &rect);
    SetConsoleScreenBufferSize(size_handle, console.size);

    rect.Right = console.size.X - 1;
    rect.Bottom = console.size.Y - 1;
    SetConsoleWindowInfo(size_handle, TRUE, &rect);
}

static bool update_console_size(void)
{
    if (use_double_buffer)
    {
        WINDOWPLACEMENT window_placement =
		{
            .length = sizeof(window_placement)
        };
        GetWindowPlacement(GetConsoleWindow(), &window_placement);

        if (window_placement.showCmd == SW_MAXIMIZE)
            console_size_changed = true;

        if (console_size_changed)
        {
            console_size_changed = false;

            const COORD new_size = get_console_size(buffer[current_buffer]);
            if (console.size.X == new_size.X && console.size.Y == new_size.Y)
                return false;

            console.size = new_size;
            character_buffer = realloc(character_buffer, sizeof(CHAR_INFO) * console.size.X * console.size.Y);

            written.Right = console.size.X - 1;
            written.Bottom = console.size.Y - 1;
            
            for (int i = 0; i < 2; ++i)
                resize(buffer[i]);
            return true;
        }
    }
    else
    {
        const COORD new_size = get_console_size(handle);
        if (new_size.X == console.size.X && new_size.Y == console.size.Y)
            return false;

        console.size = new_size;
        resize(handle);
        return true;
    }

    return false;
}

void update_console(void)
{
    console_event();

    if (update_console_size())
        clear();

    if (use_double_buffer)
        flip_double_buffer();
}

static int index(const int x, const int y)
{
    return x + y * console.size.X;
}

void write(const COORD position, const TCHAR character, const WORD attribute)
{
    if (use_double_buffer)
    {
        const int i = index(position.X, position.Y);
        character_buffer[i].Char.UnicodeChar = character;
        character_buffer[i].Attributes = attribute;
    }
    else
    {
        SetConsoleCursorPosition(handle, position);
        SetConsoleTextAttribute(handle, attribute);
        WriteConsole(handle, &character, 1, NULL, NULL);
    }
}

void clear(void)
{
    const DWORD total = console.size.X * console.size.Y;
    if (use_double_buffer)
        memset(character_buffer, 0, sizeof(CHAR_INFO) * total);
    else
    {
        const COORD coordinates = { 0 };
        DWORD written_chars = 0;

        FillConsoleOutputCharacter(handle, TEXT(' '), total, coordinates, &written_chars);
        FillConsoleOutputAttribute(handle, (WORD)BACKGROUND_T_BLACK | FOREGROUND_T_WHITE, total, coordinates, &written_chars);
        SetConsoleCursorPosition(handle, coordinates);
    }
}

void print_color_tchar(const color_tchar_t character, const COORD position)
{
    write(position, character.character, (WORD)character.background | (WORD)character.foreground);
}