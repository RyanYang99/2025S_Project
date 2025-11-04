#pragma once

#include <string>

#include <Windows.h>

#define X_color_t(X) \
enum class X##_color_t { \
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
};

X_color_t(BACKGROUND);
X_color_t(FOREGROUND);

struct color_character_t {
    wchar_t character{};
    BACKGROUND_color_t background{};
    FOREGROUND_color_t foreground{};

    color_character_t(void) = default;

    color_character_t(char character_) noexcept : character(character_) {}

    color_character_t(wchar_t character_) noexcept : character(character_) {}

    color_character_t(char character_, BACKGROUND_color_t background_) noexcept  : character(character_), background(background_) {}

    color_character_t(wchar_t character_, BACKGROUND_color_t background_) noexcept : character(character_), background(background_) {}

    color_character_t(char character_, int background_) noexcept : character(character_), background(static_cast<BACKGROUND_color_t>(background_)) {}

    color_character_t(wchar_t character_, int background_) noexcept : character(character_), background(static_cast<BACKGROUND_color_t>(background_)) {}

    color_character_t(char character_,
                      BACKGROUND_color_t background_,
                      FOREGROUND_color_t foreground_) noexcept : character(character_),
                                                                 background(background_),
                                                                 foreground(foreground_) {}

    color_character_t(wchar_t character_,
                      BACKGROUND_color_t background_,
                      FOREGROUND_color_t foreground_) noexcept : character(character_),
                                                                 background(background_),
                                                                 foreground(foreground_) {}

    color_character_t(char character_,
                      BACKGROUND_color_t background_,
                      int foreground_) noexcept : character(character_),
                                                  background(background_),
                                                  foreground(static_cast<FOREGROUND_color_t>(foreground_)) {}

    color_character_t(wchar_t character_,
                      BACKGROUND_color_t background_,
                      int foreground_) noexcept : character(character_),
                                                  background(background_),
                                                  foreground(static_cast<FOREGROUND_color_t>(foreground_)) {}

    color_character_t(char character_,
                      int background_,
                      int foreground_) noexcept : character(character_),
                                                  background(static_cast<BACKGROUND_color_t>(background_)),
                                                  foreground(static_cast<FOREGROUND_color_t>(foreground_)) {}

    color_character_t(wchar_t character_,
                      int background_,
                      int foreground_) noexcept : character(character_),
                                                  background(static_cast<BACKGROUND_color_t>(background_)),
                                                  foreground(static_cast<FOREGROUND_color_t>(foreground_)) {}
};

class console {
private:
    static COORD size_;

    static int current_buffer;
    static HANDLE buffer[2];

    static int buffer_count;
    static PCHAR_INFO character_buffer;
    static SMALL_RECT written;

    static HANDLE handle;

    static HWND window;
    static float dpi_scale;

    static const COORD calculate_size(const HANDLE size_handle);
    static void initialize_double_buffering(void);
    static void hide_cursor(const HANDLE cursor_handle);
    static bool update_size(void);
    static void resize(const HANDLE size_handle);
    static void flip_double_buffer(void);
    static int index(const int x, const int y);
    static void write(const COORD &position, const wchar_t character, const WORD attribute);

public:
    static void initialize(void);
    static void update(void);

    static const COORD &size(void) noexcept;

    static bool is_new_windows_terminal(void);
    static const COORD convert_from_monitor(const POINT &point);
    static bool is_cursor_inside(const POINT &point);

    static void clear(void);

    static void fill(const color_character_t &character);
    static void print(const color_character_t &character, const COORD &position);
    static size_t print(const std::string &string,
                        COORD position,
                        const BACKGROUND_color_t background,
                        const FOREGROUND_color_t foreground);
    static void print_center(const std::string &string,
                             const int y,
                             const BACKGROUND_color_t background,
                             const FOREGROUND_color_t foreground);

    static void destroy(void);
};