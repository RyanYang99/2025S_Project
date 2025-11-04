#pragma once

#include <cstdarg>

#include <string>
#include <format>

char *format_string(const char * const pFormat, ...);
char *format_string_v(const char * const pFormat, const va_list args) noexcept;

namespace formatter {
    template<typename... Args>
    std::string vformat(const char *message, Args &&... args) {
        return std::vformat(message, std::make_format_args(args...));
    }
}