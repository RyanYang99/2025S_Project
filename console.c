#include "leak.h"
#include "console.h"

#include <stdio.h>
#include "formatter.h"

COORD console_size = { 0 };

static bool use_double_buffer = false;
static int current_buffer = 0;
static HANDLE buffer[2] = { 0 };

static int buffer_count = 0;
static PCHAR_INFO character_buffer = NULL;
static SMALL_RECT written = { 0 };

static HANDLE handle = NULL;

static HWND window = NULL;
static float dpi_scale = 0.0f;

static void switch_font(void) {
    CONSOLE_FONT_INFOEX cfi = {
        .cbSize = sizeof(cfi),
        .dwFontSize = { 8, 8 },
        .FontFamily = FF_DONTCARE,
        .FontWeight = FW_NORMAL,
        .FaceName = TEXT("Terminal")
    };
    SetCurrentConsoleFontEx(handle, FALSE, &cfi);
}

static void hide_cursor(const HANDLE cursor_handle) {
    CONSOLE_CURSOR_INFO cci = { 0 };
    GetConsoleCursorInfo(cursor_handle, &cci);

    cci.bVisible = FALSE;
    SetConsoleCursorInfo(cursor_handle, &cci);
}

static void initialize_double_buffering(void) {
    for (int i = 0; i < 2; ++i) {
        const HANDLE new_handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
        SetConsoleScreenBufferSize(new_handle, console_size);

        written.Right = console_size.X - 1;
        written.Bottom = console_size.Y - 1;
        SetConsoleWindowInfo(new_handle, TRUE, &written);

        hide_cursor(new_handle);

        buffer[i] = new_handle;
    }

    buffer_count = console_size.X * console_size.Y;
    character_buffer = malloc(sizeof(CHAR_INFO) * buffer_count);
}

static const COORD console_get_size(const HANDLE size_handle) {
    CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
    GetConsoleScreenBufferInfo(size_handle, &csbi);

    COORD console_size_new = { 0 };
	console_size_new.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	console_size_new.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return console_size_new;
}

void console_initialize(const bool use_double_buffering, const bool should_switch_font) {
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    console_size = console_get_size(handle);
    window = GetConsoleWindow();

    SetProcessDPIAware();
    dpi_scale = (float)GetDpiForWindow(window) / 96.0f;

    if (should_switch_font)
        switch_font();

    use_double_buffer = use_double_buffering;
    if (use_double_buffer)
        initialize_double_buffering();
    else
        hide_cursor(handle);
}

static void resize(const HANDLE size_handle) {
    SMALL_RECT rect = {
        .Left =  0,
        .Top = 0,
        .Right = 1,
        .Bottom = 1
    };

    const bool maximized = IsZoomed(window);
    if (maximized)
        ShowWindow(window, SW_NORMAL);

    SetConsoleWindowInfo(size_handle, TRUE, &rect);
    SetConsoleScreenBufferSize(size_handle, console_size);

    rect.Right = console_size.X - 1;
    rect.Bottom = console_size.Y - 1;
    SetConsoleWindowInfo(size_handle, TRUE, &rect);

    if (maximized)
        ShowWindow(window, SW_MAXIMIZE);
}

static const bool update_size(void) {
    HANDLE current_handle = handle;
    if (use_double_buffer)
        current_handle = buffer[current_buffer];
    const COORD new_size = console_get_size(current_handle);

    if (console_size.X == new_size.X && console_size.Y == new_size.Y)
        return false;

    console_size = new_size;

    if (use_double_buffer) {
        buffer_count = console_size.X * console_size.Y;
        if (!buffer_count) {
            free(character_buffer);
            character_buffer = NULL;
        } else {
            const size_t size = sizeof(CHAR_INFO) * buffer_count;
            if (!character_buffer)
                character_buffer = malloc(size);
            else
                character_buffer = realloc(character_buffer, size);
        }

        written.Right = console_size.X - 1;
        written.Bottom = console_size.Y - 1;

        for (int i = 0; i < 2; ++i)
            resize(buffer[i]);
    } else
        resize(handle);

    return true;
}

static void flip_double_buffer(void) {
    if (!use_double_buffer || !character_buffer)
        return;

    WriteConsoleOutput(buffer[current_buffer], character_buffer, console_size, (COORD){ 0 }, &written);
    SetConsoleActiveScreenBuffer(buffer[current_buffer]);

    if (!current_buffer)
        current_buffer = 1;
    else
        current_buffer = 0;
}

void console_update(void) {
    if (update_size())
        console_clear();

    if (use_double_buffer)
        flip_double_buffer();
}

void console_destroy(void) {
    if (use_double_buffer) {
        for (int i = 0; i < 2; ++i) {
            if (buffer[i]) {
                CloseHandle(buffer[i]);
                buffer[i] = NULL;
            }
        }

        if (character_buffer) {
            free(character_buffer);
            character_buffer = NULL;
        }
    }
}

const bool console_is_new_windows_terminal(void) {
    CONSOLE_FONT_INFO font = { 0 };
    GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);

    return font.dwFontSize.X <= 0;
}

const COORD console_convert_from_monitor(const POINT point) {
    POINT client_point = {
        .x = point.x,
        .y = point.y
    };
    ScreenToClient(window, &client_point);

    CONSOLE_FONT_INFO font = { 0 };
    GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);

    const COORD consoleCoord = {
        .X = (SHORT)(client_point.x / (font.dwFontSize.X * dpi_scale)),
        .Y = (SHORT)(client_point.y / (font.dwFontSize.Y * dpi_scale))
    };
    return consoleCoord;
}

const bool console_is_cursor_inside(const POINT point) {
    RECT window_rect = { 0 };
    GetWindowRect(window, &window_rect);

    return point.x >= window_rect.left &&
           point.x <= window_rect.right &&
           point.y >= window_rect.top &&
           point.y <= window_rect.bottom &&
           GetForegroundWindow() == window;
}

void console_clear(void) {
    const DWORD total = console_size.X * console_size.Y;
    if (use_double_buffer) {
        if (!character_buffer)
            return;

        memset(character_buffer, 0, sizeof(CHAR_INFO) * total);
    } else {
        const COORD coordinates = { 0 };
        DWORD written_chars = 0;

        FillConsoleOutputCharacter(handle, TEXT(' '), total, coordinates, &written_chars);
        FillConsoleOutputAttribute(handle, (WORD)BACKGROUND_T_BLACK | FOREGROUND_T_WHITE, total, coordinates, &written_chars);
        SetConsoleCursorPosition(handle, coordinates);
    }
}

void console_fill(const color_character_t character) {
    const WORD attribute = (WORD)character.background | (WORD)character.foreground;
    if (use_double_buffer) {
        if (!character_buffer)
            return;

        for (int i = 0; i < buffer_count; ++i) {
            character_buffer[i].Char.UnicodeChar = character.character;
            character_buffer[i].Attributes = attribute;
        }
    } else {
        const DWORD total = console_size.X * console_size.Y;
        const COORD coordinates = { 0 };
        DWORD written_chars = 0;

        FillConsoleOutputCharacter(handle, character.character, total, coordinates, &written_chars);
        FillConsoleOutputAttribute(handle, attribute, total, coordinates, &written_chars);
        SetConsoleCursorPosition(handle, coordinates);
    }
}

static int index(const int x, const int y) {
    return x + y * console_size.X;
}

static void write(const COORD position, const wchar_t character, const WORD attribute) {
    WORD new_attribute = attribute;
    if (attribute == (WORD)-1) {
        DWORD read = 0;
        ReadConsoleOutputAttribute(handle, &new_attribute, 1, position, &read);
    }

    if (use_double_buffer) {
        if (!character_buffer)
            return;

        const int i = index(position.X, position.Y);
        if (i >= buffer_count || i < 0)
            return;

        character_buffer[i].Char.UnicodeChar = character;
        character_buffer[i].Attributes = new_attribute;
    } else {
        SetConsoleCursorPosition(handle, position);

        if (attribute != -1)
            SetConsoleTextAttribute(handle, attribute);

        putchar(character);
    }
}

void console_print_color_character(const color_character_t character, const COORD position) {
    WORD attribute = (WORD)-1;
    if (character.background != BACKGROUND_T_TRANSPARENT)
        attribute = (WORD)character.background | (WORD)character.foreground;

    write(position, character.character, attribute);
}

static int fprint_string_v(const char * const pFormat, const COORD position, const BACKGROUND_color_t background, const FOREGROUND_color_t foreground, const va_list args) {
    char *pBuffer = format_string_v(pFormat, args);

    const int wide_length = MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, NULL, 0);
    LPWSTR pWBuffer = malloc(sizeof(WCHAR) * wide_length);
    MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, pWBuffer, wide_length);

    const WORD attribute = (WORD)background | (WORD)foreground;

    COORD print_position = position;
    for (size_t i = 0; i < wcslen(pWBuffer); ++i) {
        write(print_position, pWBuffer[i], attribute);
        ++print_position.X;
    }

    free(pBuffer);
    free(pWBuffer);

    return wide_length - 1;
}

int console_fprint_string(const char * const pFormat, const COORD position, const BACKGROUND_color_t background, const FOREGROUND_color_t foreground, ...) {
    va_list args = { 0 };
    va_start(args, foreground);
    int result = fprint_string_v(pFormat, position, background, foreground, args);
    va_end(args);

    return result;
}

void console_print_center(const char * const string,
                          const int y,
                          const BACKGROUND_color_t background,
                          const FOREGROUND_color_t foreground,
                          ...) {
    COORD position = {
        .X = console_size.X / 2,
        .Y = (SHORT)y
    };
    if (position.X < 0)
        return;

    va_list args = { 0 };
    va_start(args, foreground);

    char * const pFormatted = format_string_v(string, args);
    va_end(args);

    position.X -= (SHORT)strlen(pFormatted) / 2;
    console_fprint_string(pFormatted, position, background, foreground);
    free(pFormatted);
}