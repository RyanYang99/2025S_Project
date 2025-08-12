#pragma once

#include <stdbool.h>

typedef struct {
    int day, hour, minute;
    float second;
} date_time_t;

extern date_time_t date_time_elapsed_since_start;

void date_time_initialize(void);
void date_time_update(void);
void date_time_render(void);

const bool date_time_is_night(void);
void date_time_save(void);