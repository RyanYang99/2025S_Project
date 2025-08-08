#include "leak.h"
#include "date_time.h"

#include <math.h>
#include "save.h"
#include "delta.h"
#include "console.h"
#include "formatter.h"

date_time_t date_time_elapsed_since_start = { 0 };

void initialize_date_time(void) {
    if (pCurrent_save)
        date_time_elapsed_since_start = pCurrent_save->game_time;
    else {
        date_time_elapsed_since_start.second = 0.0f;
        date_time_elapsed_since_start.minute = date_time_elapsed_since_start.day = 0;
        date_time_elapsed_since_start.hour = 12;
    }
}

void update_date_time(void) {
    date_time_elapsed_since_start.second += delta_time * (86400.0f / 1200.0f); //1일당 게임 초 / 1일당 실제 초

    if (date_time_elapsed_since_start.second >= 60.0f) {
        const int minutes = (int)(date_time_elapsed_since_start.second / 60.0f);

        date_time_elapsed_since_start.minute += minutes;
        date_time_elapsed_since_start.second -= minutes * 60.0f;
    }

    if (date_time_elapsed_since_start.minute >= 60) {
        date_time_elapsed_since_start.hour += date_time_elapsed_since_start.minute / 60;
        date_time_elapsed_since_start.minute %= 60;
    }

    if (date_time_elapsed_since_start.hour >= 24) {
        date_time_elapsed_since_start.day += date_time_elapsed_since_start.hour / 24;
        date_time_elapsed_since_start.hour %= 24;
    }
}

void render_time(void) {
    static float blink = 0.0f;
    static char *pBlink = " ";
    blink += delta_time;
    if (blink >= 2.0f) {
        blink = 0.0f;
        pBlink = " ";
    } else if (blink >= 1.0f)
        pBlink = ":";
    
    char * const pDay = format_string("Day %d", date_time_elapsed_since_start.day),
         * const pTime = format_string("%02d%s%02d", date_time_elapsed_since_start.hour, pBlink, date_time_elapsed_since_start.minute);

    COORD position = {
        .X = (SHORT)(console.size.X - strlen(pDay))
    };
    fprint_string(pDay, position, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);

    position.X = (SHORT)(console.size.X - strlen(pTime));
    ++position.Y;
    fprint_string(pTime, position, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);

    free(pDay);
    free(pTime);
}

void save_date_time(void) {
    if (!pCurrent_save)
        instantiate_save();

    pCurrent_save->game_time = date_time_elapsed_since_start;
}