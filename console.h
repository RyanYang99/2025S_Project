#pragma once

#include <stdbool.h>

#include <Windows.h>

#define X_color_t(X) \
typedef enum { \
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
} X##_color_t

X_color_t(BACKGROUND);
X_color_t(FOREGROUND);

typedef struct {
    wchar_t character;
    BACKGROUND_color_t background;
    FOREGROUND_color_t foreground;
} color_character_t;

extern COORD console_size;

void console_initialize(const bool use_double_buffering, const bool should_switch_font);
void console_update(void);
void console_destroy(void);

const bool console_is_new_windows_terminal(void);
const COORD console_convert_from_monitor(const POINT point);
const bool console_is_cursor_inside(const POINT point);

void console_clear(void);
void console_fill(const color_character_t character);
void console_print_color_character(const color_character_t character, const COORD position);
int console_fprint_string(const char * const pFormat, const COORD position, const BACKGROUND_color_t background, const FOREGROUND_color_t foreground, ...);
void console_print_center(const char * const string, const int y, const BACKGROUND_color_t background, const FOREGROUND_color_t foreground, ...);