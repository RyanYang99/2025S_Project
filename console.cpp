#include "leak.hpp"
#include "console.hpp"

#include <string>
#include <vector>
#include <algorithm>

COORD Console::size_{};

int Console::current_buffer{};
HANDLE Console::buffer[2]{};

std::vector<CHAR_INFO> Console::character_buffer{};
SMALL_RECT Console::written{};

HANDLE Console::handle{};

HWND Console::window{};
float Console::dpi_scale{};

const COORD Console::calculate_size(const HANDLE size_handle) noexcept {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    GetConsoleScreenBufferInfo(size_handle, &csbi);

    return {
        static_cast<SHORT>(csbi.srWindow.Right - csbi.srWindow.Left + 1),
        static_cast<SHORT>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
    };
}

void Console::hide_cursor(const HANDLE cursor_handle) noexcept {
    CONSOLE_CURSOR_INFO cci{};
    GetConsoleCursorInfo(cursor_handle, &cci);

    cci.bVisible = false;
    SetConsoleCursorInfo(cursor_handle, &cci);
}

void Console::resize_buffer(void) {
    character_buffer.resize(static_cast<size_t>(size_.X) * static_cast<size_t>(size_.Y));
}

void Console::initialize_double_buffering(void) noexcept {
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

void Console::initialize(void) {
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    size_ = calculate_size(handle);
    window = GetConsoleWindow();

    SetProcessDPIAware();
    dpi_scale = static_cast<float>(GetDpiForWindow(window)) / 96.0f;

    initialize_double_buffering();
    resize_buffer();
}

void Console::resize(const HANDLE size_handle) noexcept {
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

bool Console::update_size(void) {
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

void Console::flip_double_buffer(void) noexcept {
    if (character_buffer.empty())
        return;

    WriteConsoleOutput(buffer[current_buffer], &character_buffer[0], size_, {}, &written);
    SetConsoleActiveScreenBuffer(buffer[current_buffer]);

    if (!current_buffer)
        current_buffer = 1;
    else
        current_buffer = 0;
}

int Console::index(const int x, const int y) noexcept {
    return x + y * size_.X;
}

void Console::write(const COORD &position, const wchar_t character, const WORD attribute) {
    WORD new_attribute{ attribute };
    if (attribute == static_cast<WORD>(-1)) {
        DWORD read{};
        ReadConsoleOutputAttribute(handle, &new_attribute, 1, position, &read);
    }

    if (character_buffer.empty())
        return;

    const int i{ index(position.X, position.Y) };
    if (i < 0 || i >= static_cast<int>(character_buffer.size()))
        return;

    character_buffer[i].Char.UnicodeChar = character;
    character_buffer[i].Attributes = new_attribute;
}

void Console::update(void) {
    if (update_size())
        clear();

    flip_double_buffer();
}

const COORD &Console::size(void) noexcept {
    return size_;
}

bool Console::is_new_windows_terminal(void) noexcept {
    CONSOLE_FONT_INFO font{};
    GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);

    return font.dwFontSize.X <= 0;
}

const COORD Console::convert_from_monitor(const POINT &point) noexcept {
    POINT client_point{ point.x, point.y };
    ScreenToClient(window, &client_point);

    CONSOLE_FONT_INFO font{};
    GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);

    return { static_cast<SHORT>(client_point.x / (font.dwFontSize.X * dpi_scale)),
             static_cast<SHORT>(client_point.y / (font.dwFontSize.Y * dpi_scale)) };
}

bool Console::is_cursor_inside(const POINT &point) noexcept {
    RECT window_rect{};
    GetWindowRect(window, &window_rect);

    return point.x >= window_rect.left &&
           point.x <= window_rect.right &&
           point.y >= window_rect.top &&
           point.y <= window_rect.bottom &&
           GetForegroundWindow() == window;
}

void Console::clear(void) {
    if (character_buffer.empty())
        return;

    std::fill(character_buffer.begin(), character_buffer.end(), CHAR_INFO{});
}

void Console::fill(const cchar &character) noexcept {
    const WORD attribute = static_cast<WORD>(character.background) | static_cast<WORD>(character.foreground);
    if (character_buffer.empty())
        return;

    for (size_t i{}; i < character_buffer.size(); ++i) {
        character_buffer[i].Char.UnicodeChar = character.character;
        character_buffer[i].Attributes = attribute;
    }
}

void Console::print(const cchar &character, const COORD &position) {
    WORD attribute{ static_cast<WORD>(-1) };
    if (character.background != BG::transparent)
        attribute = static_cast<WORD>(character.background) | static_cast<WORD>(character.foreground);

    write(position, character.character, attribute);
}

size_t Console::print(const std::string &string,
                      COORD position,
                      const BG background,
                      const FG foreground) {
    for (const char character : string) {
        print({ character, background, foreground }, position);
        ++position.X;
    }

    return string.size();
}

void Console::print_center(const std::string &string,
                           const int y,
                           const BG background,
                           const FG foreground) {
    COORD position{ static_cast<SHORT>(size_.X / 2), static_cast<SHORT>(y) };
    if (position.X < 0)
        return;

    position.X -= static_cast<SHORT>(string.size() / 2);
    print(string, position, background, foreground);
}

void Console::destroy(void) noexcept {
    for (int i{}; i < 2; ++i)
        if (buffer[i]) {
            CloseHandle(buffer[i]);
            buffer[i] = nullptr;
        }
}