#include "leak.hpp"
#include "formatter.hpp"

#include <cstdio>
#include <cstdarg>

char *format_string(const char * const pFormat, ...) {
    va_list args{};
    va_start(args, pFormat);
    char *pResult = format_string_v(pFormat, args);
    va_end(args);

    return pResult;
}

char *format_string_v(const char * const pFormat, const va_list args) noexcept {
    const int length{ vsnprintf(NULL, 0, pFormat, args) };
    char *pBuffer{ static_cast<char *>(malloc(sizeof(char) * (length + 1))) };

    vsnprintf(pBuffer, length + 1, pFormat, args);
    return pBuffer;
}