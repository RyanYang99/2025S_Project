#include "leak.hpp"
#include "console.hpp"

#include <string>
#include <vector>
#include <algorithm>

COORD console::size_{};

int console::current_buffer{};
HANDLE console::buffer[2]{};

//int console::buffer_count{};
std::vector<CHAR_INFO> console::character_buffer{};
SMALL_RECT console::written{};

HANDLE console::handle{};

HWND console::window{};
float console::dpi_scale{};

const COORD console::calculate_size(const HANDLE size_handle) {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    GetConsoleScreenBufferInfo(size_handle, &csbi);

    return {
        static_cast<SHORT>(csbi.srWindow.Right - csbi.srWindow.Left + 1),
        static_cast<SHORT>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
    };
}

void console::hide_cursor(const HANDLE cursor_handle) noexcept {
    CONSOLE_CURSOR_INFO cci{};
    GetConsoleCursorInfo(cursor_handle, &cci);

    cci.bVisible = false;
    SetConsoleCursorInfo(cursor_handle, &cci);
}

void console::resize_buffer(void) {
    character_buffer.resize(size_.X * size_.Y);
}

void console::initialize_double_buffering(void) {
    for (int i{}; i < 2; ++i) {
        const HANDLE new_handle{ CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr) };
        SetConsoleScreenBufferSize(new_handle, size_);

        written.Right = size_.X - 1;
        written.Bottom = size_.Y - 1;
        SetConsoleWindowInfo(new_handle, true, &written);

        hide_cursor(new_handle);

        buffer[i] = new_handle;
    }
}

void console::initialize(void) {
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    size_ = calculate_size(handle);
    window = GetConsoleWindow();

    SetProcessDPIAware();
    dpi_scale = static_cast<float>(GetDpiForWindow(window)) / 96.0f;

    initialize_double_buffering();
    resize_buffer();
}

void console::resize(const HANDLE size_handle) {
    SMALL_RECT rect{};
    rect.Right = 1;
    rect.Bottom = 1;

    const bool maximized{ static_cast<bool>(IsZoomed(window)) };
    if (maximized)
        ShowWindow(window, SW_NORMAL);

    SetConsoleWindowInfo(size_handle, true, &rect);
    SetConsoleScreenBufferSize(size_handle, size_);

    rect.Right = size_.X - 1;
    rect.Bottom = size_.Y - 1;
    SetConsoleWindowInfo(size_handle, true, &rect);

    if (maximized)
        ShowWindow(window, SW_MAXIMIZE);
}

bool console::update_size(void) {
    const COORD new_size{ calculate_size(buffer[current_buffer]) };

    if (size_.X == new_size.X && size_.Y == new_size.Y)
        return false;

    size_ = new_size;
    resize_buffer();

    written.Right = size_.X - 1;
    written.Bottom = size_.Y - 1;

    for (int i{}; i < 2; ++i)
        resize(buffer[i]);
    return true;
}

void console::flip_double_buffer(void) {
    if (character_buffer.empty())
        return;

    WriteConsoleOutput(buffer[current_buffer], &character_buffer[0], size_, {}, &written);
    SetConsoleActiveScreenBuffer(buffer[current_buffer]);

    if (!current_buffer)
        current_buffer = 1;
    else
        current_buffer = 0;
}

int console::index(const int x, const int y) {
    return x + y * size_.X;
}

void console::write(const COORD &position, const wchar_t character, const WORD attribute) {
    WORD new_attribute{ attribute };
    if (attribute == static_cast<WORD>(-1)) {
        DWORD read{};
        ReadConsoleOutputAttribute(handle, &new_attribute, 1, position, &read);
    }

    if (character_buffer.empty())
        return;

    const int i{ index(position.X, position.Y) };
    if (i < 0 || i >= character_buffer.size())
        return;

    character_buffer[i].Char.UnicodeChar = character;
    character_buffer[i].Attributes = new_attribute;
}

void console::update(void) {
    if (update_size())
        clear();

    flip_double_buffer();
}

const COORD &console::size(void) noexcept {
    return size_;
}

bool console::is_new_windows_terminal(void) {
    CONSOLE_FONT_INFO font{};
    GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);

    return font.dwFontSize.X <= 0;
}

const COORD console::convert_from_monitor(const POINT &point) {
    POINT client_point{ point.x, point.y };
    ScreenToClient(window, &client_point);

    CONSOLE_FONT_INFO font{};
    GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);

    return { static_cast<SHORT>(client_point.x / (font.dwFontSize.X * dpi_scale)),
             static_cast<SHORT>(client_point.y / (font.dwFontSize.Y * dpi_scale)) };
}

bool console::is_cursor_inside(const POINT &point) {
    RECT window_rect{};
    GetWindowRect(window, &window_rect);

    return point.x >= window_rect.left &&
           point.x <= window_rect.right &&
           point.y >= window_rect.top &&
           point.y <= window_rect.bottom &&
           GetForegroundWindow() == window;
}

void console::clear(void) {
    if (character_buffer.empty())
        return;

    std::fill(character_buffer.begin(), character_buffer.end(), CHAR_INFO{});
}

void console::fill(const color_character_t &character) {
    const WORD attribute = static_cast<WORD>(character.background) | static_cast<WORD>(character.foreground);
    if (character_buffer.empty())
        return;

    for (int i{}; i < character_buffer.size(); ++i) {
        character_buffer[i].Char.UnicodeChar = character.character;
        character_buffer[i].Attributes = attribute;
    }
}

void console::print(const color_character_t &character, const COORD &position) {
    WORD attribute{ static_cast<WORD>(-1) };
    if (character.background != BG::transparent)
        attribute = static_cast<WORD>(character.background) | static_cast<WORD>(character.foreground);

    write(position, character.character, attribute);
}

size_t console::print(const std::string &string,
                      COORD position,
                      const BG background,
                      const FG foreground) {
    for (const char character : string) {
        print({ character, background, foreground }, position);
        ++position.X;
    }

    return string.size();
}

void console::print_center(const std::string &string,
                           const int y,
                           const BG background,
                           const FG foreground) {
    COORD position{ static_cast<SHORT>(size_.X / 2), static_cast<SHORT>(y) };
    if (position.X < 0)
        return;

    position.X -= static_cast<SHORT>(string.size() / 2);
    print(string, position, background, foreground);
}

void console::destroy(void) {
    for (int i{}; i < 2; ++i)
        if (buffer[i]) {
            CloseHandle(buffer[i]);
            buffer[i] = nullptr;
        }
}