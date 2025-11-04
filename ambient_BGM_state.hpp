#pragma once

#include <cstdint>

enum class BGM : uint8_t {
    none,
    day, //낮 BGM 재생 중
    night, //밤 BGM 재생 중
    boss
};