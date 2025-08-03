#pragma once

#include <stdbool.h>
#include <Windows.h>

//https://stackoverflow.com/a/17125539
#define X_color_t(X) \
typedef enum \
{ \
    X##_T_BLACK = 0, \
    X##_T_DARKBLUE = X##_BLUE, \
    X##_T_DARKGREEN = X##_GREEN, \
    X##_T_DARKCYAN = X##_GREEN | X##_BLUE, \
    X##_T_DARKRED = X##_RED, \
    X##_T_DARKMAGENTA = X##_RED | X##_BLUE, \
    X##_T_DARKYELLOW = X##_RED | X##_GREEN, \
    X##_T_DARKGRAY = X##_RED | X##_GREEN | X##_BLUE, \
    X##_T_GRAY = X##_INTENSITY, \
    X##_T_BLUE = X##_INTENSITY | X##_BLUE, \
    X##_T_GREEN = X##_INTENSITY | X##_GREEN, \
    X##_T_CYAN = X##_INTENSITY | X##_GREEN | X##_BLUE, \
    X##_T_RED = X##_INTENSITY | X##_RED, \
    X##_T_MAGENTA = X##_INTENSITY | X##_RED | X##_BLUE, \
    X##_T_YELLOW = X##_INTENSITY | X##_RED | X##_GREEN, \
    X##_T_WHITE = X##_INTENSITY | X##_RED | X##_GREEN | X##_BLUE, \
    X##_T_TRANSPARENT = -1 \
} \
X##_color_t

X_color_t(BACKGROUND);
X_color_t(FOREGROUND);

typedef struct
{
    TCHAR character;
    BACKGROUND_color_t background;
    FOREGROUND_color_t foreground;
}
color_tchar_t;

typedef struct
{
    COORD size;
}
console_t;

extern console_t console;

bool is_new_console(void);
void initialize_console(const bool use_double_buffering, const bool should_switch_font);
void update_console(void);
void write(const COORD position, const TCHAR character, const WORD attribute);
void clear(void);
void print_color_tchar(const color_tchar_t character, const COORD position);
const COORD convert_monitor_to_console(const POINT point);
int fprint_string(const char * const pFormat, const COORD position, const BACKGROUND_color_t background, const FOREGROUND_color_t foreground, ...);
void destroy_console(void);