//https://github.com/Kevger/DoubleBufferedWindowsConsole/blob/master/source/DoubleBufferedConsole.h

#include "leak.h"
#include "console.h"

#include <stdio.h>
#include "input.h"

bool use_double_buffer = false;

int current_buffer = 0;
HANDLE buffer[2] = { 0 };

int character_buffer_count = 0;
PCHAR_INFO character_buffer = NULL;
SMALL_RECT written = { 0 };

HANDLE handle = NULL;

console_t console = { 0 };

bool is_new_console(void)
{
    CONSOLE_FONT_INFO font = { 0 };
    GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);

    return font.dwFontSize.X <= 0;
}

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

    character_buffer_count = console.size.X * console.size.Y;
    character_buffer = malloc(sizeof(CHAR_INFO) * character_buffer_count);
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

static void switch_font(void)
{
    CONSOLE_FONT_INFOEX cfi =
    {
        .cbSize = sizeof(cfi),
        .nFont = 0,
        .dwFontSize = { 8, 8 },
        .FontFamily = FF_DONTCARE,
        .FontWeight = FW_NORMAL,
        .FaceName = TEXT("Terminal")
    };
    SetCurrentConsoleFontEx(handle, FALSE, &cfi);
}

void initialize_console(const bool use_double_buffering)
{
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    console.size = get_console_size(handle);

    switch_font();

    use_double_buffer = use_double_buffering;
    if (use_double_buffer)
        initialize_double_buffering();
    else
        hide_console_cursor(handle);
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
    HANDLE current_handle = handle;
    if (use_double_buffer)
        current_handle = buffer[current_buffer];
    const COORD new_size = get_console_size(current_handle);

    if (console.size.X == new_size.X && console.size.Y == new_size.Y)
        return false;

    console.size = new_size;

    if (use_double_buffer)
    {
        character_buffer_count = console.size.X * console.size.Y;
        character_buffer = realloc(character_buffer, sizeof(CHAR_INFO) * character_buffer_count);

        written.Right = console.size.X - 1;
        written.Bottom = console.size.Y - 1;
            
        for (int i = 0; i < 2; ++i)
            resize(buffer[i]);
    }
    else
        resize(handle);

    return true;
}

void update_console(void)
{
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
        if (i > character_buffer_count || i < 0)
            return;

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

void destroy_console(void)
{
    if (use_double_buffer)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (buffer[i])
            {
                CloseHandle(buffer[i]);
                buffer[i] = NULL;
            }
        }

        if (character_buffer)
        {
            free(character_buffer);
            character_buffer = NULL;
        }
    }
}