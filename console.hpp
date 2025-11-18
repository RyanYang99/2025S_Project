#pragma once

#include <string>
#include <vector>

#include <Windows.h>

#define DEFINE_COLORS(name, ground) \
enum class name { \
    black = 0, \
    dark_blue = ground##_BLUE, \
    dark_green = ground##_GREEN, \
    dark_cyan = ground##_GREEN | ground##_BLUE, \
    dark_red = ground##_RED, \
    dark_magenta = ground##_RED | ground##_BLUE, \
    dark_yellow = ground##_RED | ground##_GREEN, \
    dark_gray = ground##_RED | ground##_GREEN | ground##_BLUE, \
    gray = ground##_INTENSITY, \
    blue = ground##_INTENSITY | ground##_BLUE, \
    green = ground##_INTENSITY | ground##_GREEN, \
    cyan = ground##_INTENSITY | ground##_GREEN | ground##_BLUE, \
    red = ground##_INTENSITY | ground##_RED, \
    magenta = ground##_INTENSITY | ground##_RED | ground##_BLUE, \
    yellow = ground##_INTENSITY | ground##_RED | ground##_GREEN, \
    white = ground##_INTENSITY | ground##_RED | ground##_GREEN | ground##_BLUE, \
    transparent = -1 \
}

DEFINE_COLORS(BG, BACKGROUND);
DEFINE_COLORS(FG, FOREGROUND);

struct cchar {
    wchar_t character{};
    BG background{};
    FG foreground{};

    cchar(void) = default;

    cchar(char character_) noexcept : character(character_) {}

    cchar(wchar_t character_) noexcept : character(character_) {}

    cchar(char character_, BG background_) noexcept  : character(character_), background(background_) {}

    cchar(wchar_t character_, BG background_) noexcept : character(character_), background(background_) {}

    cchar(char character_, int background_) noexcept : character(character_), background(static_cast<BG>(background_)) {}

    cchar(wchar_t character_, int background_) noexcept : character(character_), background(static_cast<BG>(background_)) {}

    cchar(char character_,
          BG background_,
          FG foreground_) noexcept : character(character_),
                                     background(background_),
                                     foreground(foreground_) {}

    cchar(wchar_t character_,
          BG background_,
          FG foreground_) noexcept : character(character_),
                                                     background(background_),
                                                     foreground(foreground_) {}

    cchar(char character_,
          BG background_,
          int foreground_) noexcept : character(character_),
                                      background(background_),
                                      foreground(static_cast<FG>(foreground_)) {}

    cchar(wchar_t character_,
          BG background_,
          int foreground_) noexcept : character(character_),
                                      background(background_),
                                      foreground(static_cast<FG>(foreground_)) {}

    cchar(char character_,
          int background_,
          int foreground_) noexcept : character(character_),
                                      background(static_cast<BG>(background_)),
                                      foreground(static_cast<FG>(foreground_)) {}

    cchar(wchar_t character_,
          int background_,
          int foreground_) noexcept : character(character_),
                                      background(static_cast<BG>(background_)),
                                      foreground(static_cast<FG>(foreground_)) {}
};

class Console {
private:
    static COORD size_;

    //더블 버퍼링
    static int current_buffer;
    static HANDLE buffer[2];

    //문자 버퍼
    static std::vector<CHAR_INFO> character_buffer;
    static SMALL_RECT written;

    static HANDLE handle;

    static HWND window;
    static float dpi_scale;

    static const COORD calculate_size(const HANDLE size_handle) noexcept;
    static void resize_buffer(void);
    static void initialize_double_buffering(void) noexcept;
    static void hide_cursor(const HANDLE cursor_handle) noexcept;
    static bool update_size(void);
    static void resize(const HANDLE size_handle) noexcept;
    static void flip_double_buffer(void) noexcept;
    static int index(const int x, const int y) noexcept;
    static void write(const COORD &position, const wchar_t character, const WORD attribute);

public:
    static void initialize(void);
    static void update(void);

    static const COORD &size(void) noexcept;

    static bool is_new_windows_terminal(void) noexcept;
    static const COORD convert_from_monitor(const POINT &point) noexcept;
    static bool is_cursor_inside(const POINT &point) noexcept;

    static void clear(void);

    static void fill(const cchar &character) noexcept;
    static void print(const cchar &character, const COORD &position);
    static size_t print(const std::string &string,
                        COORD position,
                        const BG background,
                        const FG foreground);
    static void print_center(const std::string &string,
                             const int y,
                             const BG background,
                             const FG foreground);

    static void destroy(void) noexcept;
};