#pragma once

#include <Windows.h>

typedef struct {
	COORD size;
	HANDLE handle;
} console_t;

extern console_t console;

void set_console_information(void);
const COORD get_console_size(void);
void clear(void);