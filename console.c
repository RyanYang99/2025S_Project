#include <stdio.h>
#include "console.h"

console_t console = { 0 };

void set_console_information(void)
{
    console.handle = GetStdHandle(STD_OUTPUT_HANDLE);
    console.size = get_console_size();
}

const COORD get_console_size(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
    GetConsoleScreenBufferInfo(console.handle, &csbi);

    COORD console_size = { 0 };
    console_size.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    console_size.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return console_size;
}

void clear(void)
{
    const DWORD size = console.size.X * console.size.Y;
    const COORD coordinates = { 0 };
    DWORD written = 0;

    FillConsoleOutputCharacter(console.handle, TEXT(' '), size, coordinates, &written);
    FillConsoleOutputAttribute(console.handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, size, coordinates, &written);
    SetConsoleCursorPosition(console.handle, coordinates);
}