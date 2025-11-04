#pragma once

#include <string>
#include <format>

namespace formatter {
    template<typename... Args>
    std::string vformat(const char *message, Args &&... args) {
        return std::vformat(message, std::make_format_args(args...));
    }
}