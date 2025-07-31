#include "leak.h"
#include "formatter.h"

#include <stdio.h>
#include <stdarg.h>

char *format_string(const char * const pFormat, ...) {
    va_list args = { 0 };
    va_start(args, pFormat);
    char *pResult = format_string_v(pFormat, args);
    va_end(args);

    return pResult;
}

char *format_string_v(const char * const pFormat, const va_list args) {
    const int length = vsnprintf(NULL, 0, pFormat, args);
    char *pBuffer = malloc(sizeof(char) * (length + 1));

    vsnprintf(pBuffer, length + 1, pFormat, args);
    return pBuffer;
}