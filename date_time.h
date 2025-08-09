#pragma once
#include <stdbool.h>

typedef struct {
    int day, hour, minute;
    float second;
} date_time_t;

extern date_time_t date_time_elapsed_since_start;

void initialize_date_time(void);

void update_date_time(void);
void render_time(void);

void save_date_time(void);

int get_current_hour();
bool is_night_time();