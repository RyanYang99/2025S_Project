#pragma once

typedef struct {
    int day, hour, minute;
    float second;
} date_time_t;

extern date_time_t date_time_elapsed_since_start;

void initialize_date_time(void);

void update_date_time(void);
void render_time(void);

void save_date_time(void);